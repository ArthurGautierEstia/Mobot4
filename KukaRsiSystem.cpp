#include "KukaRsiSystem.h"

#include "MeasurementFrame.h"
#include "ConnectionConfig.h"
#include "SystemCapabilities.h"
#include "RsiTrame.h"
#include "TrameHelper.h"
#include "RsiTag.h"

#include <QThread>
#include <QDateTime>
#include <QMutexLocker>

// ============================================================================
// Constructeur / Destructeur
// ============================================================================

KukaRsiSystem::KukaRsiSystem(const KukaRsiConfig& config, QObject* parent)
    : IMeasurementSystem(parent)
    , m_config(config)
{
    m_capabilities.supportsLatency = true;
    m_capabilities.supportsFrequency = true;
    m_capabilities.nativeFrequencyHz = 250.0;
}

KukaRsiSystem::~KukaRsiSystem()
{
    if (m_isAcquiring)
        stopAcquisition();
    if (m_isConnected)
        disconnect();
}

// ============================================================================
// Cycle de vie
// ============================================================================

bool KukaRsiSystem::initialize()
{
    return true;
}

bool KukaRsiSystem::connect(const ConnectionConfig& /*config*/)
{
    if (m_isConnected)
        return true;

    m_socket = std::make_unique<NativeUdpSocket>(
        m_config.hostAddress.toStdString(),
        static_cast<u_short>(m_config.hostPort),
        static_cast<int>(k_bufSize * 2)
    );

    if (!m_socket->open()) {
        emit errorOccurred(QStringLiteral("KukaRsi: échec ouverture socket (%1:%2)")
            .arg(m_config.hostAddress).arg(m_config.hostPort));
        m_socket.reset();
        return false;
    }

    if (!m_socket->bind()) {
        emit errorOccurred(QStringLiteral("KukaRsi: échec bind (%1:%2)")
            .arg(m_config.hostAddress).arg(m_config.hostPort));
        m_socket.reset();
        return false;
    }

    m_robotAddrKnown = false;
    m_isConnected = true;
    emit connected();
    emit logMessage(QStringLiteral("KukaRsi: socket ouvert sur %1:%2")
        .arg(m_config.hostAddress).arg(m_config.hostPort));
    return true;
}

bool KukaRsiSystem::disconnect()
{
    if (!m_isConnected)
        return true;

    if (m_socket)
        m_socket->close();
    m_socket.reset();

    m_isConnected = false;
    m_robotAddrKnown = false;
    emit disconnected();
    return true;
}

bool KukaRsiSystem::isConnected() const
{
    return m_isConnected;
}

// ============================================================================
// Acquisition
// ============================================================================

bool KukaRsiSystem::startAcquisition()
{
    if (!m_isConnected) {
        emit errorOccurred(QStringLiteral("KukaRsi: startAcquisition sans connexion active"));
        return false;
    }
    if (m_isAcquiring)
        return true;

    resetSessionStats();
    m_isAcquiring = true;

    m_acquisitionThread = QThread::create([this]() { acquisitionLoop(); });
    m_acquisitionThread->setObjectName(QStringLiteral("KukaRsiAcqThread"));
    m_acquisitionThread->start(QThread::TimeCriticalPriority);

    emit logMessage(QStringLiteral("KukaRsi: acquisition démarrée"));
    return true;
}

bool KukaRsiSystem::stopAcquisition()
{
    if (!m_isAcquiring)
        return true;

    m_isAcquiring = false;

    if (m_acquisitionThread) {
        m_acquisitionThread->wait(3000);
        delete m_acquisitionThread;
        m_acquisitionThread = nullptr;
    }

    const AcquisitionSummary summary = buildSummary(m_config.robotName);
    emit acquisitionCompleted(summary);
    emit logMessage(QStringLiteral("KukaRsi: acquisition arrêtée — %1 frames")
        .arg(summary.totalFrames));
    return true;
}

bool KukaRsiSystem::isAcquiring() const
{
    return m_isAcquiring;
}

// ============================================================================
// Données / Capacités
// ============================================================================

MeasurementFrame KukaRsiSystem::getLatestFrame() const
{
    QMutexLocker lk(&m_frameMutex);
    return m_latestFrame;
}

double KukaRsiSystem::getNativeFrequency() const { return 250.0; }
double KukaRsiSystem::getLatency()         const { return m_latencyMs.load(); }

QStringList KukaRsiSystem::getAvailableObjects() const
{
    return { m_config.robotName };
}

SystemCapabilities KukaRsiSystem::getCapabilities() const { return m_capabilities; }
QString KukaRsiSystem::getSystemName()    const { return QStringLiteral("KUKA RSI"); }
QString KukaRsiSystem::getSystemVersion() const { return QStringLiteral("3.1"); }

IMeasurementSystem::ThreadingModel KukaRsiSystem::getThreadingModel() const
{
    return ThreadingModel::ApplicationManaged;
}

// ============================================================================
// Boucle RT
// ============================================================================

void KukaRsiSystem::acquisitionLoop()
{
    while (m_isAcquiring) {

        // 1. Attente données — 100 ms timeout (tolère les délais d'établissement)
        if (!m_socket->waitForData(100))
            continue;

        // 2. Réception
        std::string senderIp;
        u_short     senderPort = 0;
        const int len = m_socket->recvFrom(
            m_recvBuf, static_cast<int>(k_bufSize), senderIp, senderPort);
        if (len <= 0)
            continue;

        // 3. Mémorisation adresse robot au 1er paquet
        if (!m_robotAddrKnown) {
            m_robotIp = senderIp;
            m_robotPort = senderPort;
            m_robotAddrKnown = true;
            emit logMessage(
                QStringLiteral("KukaRsi: robot détecté → %1:%2")
                .arg(QString::fromStdString(senderIp)).arg(senderPort));
        }

        const std::size_t dataLen = static_cast<std::size_t>(len);

        // 4. Parsing
        MeasurementFrame frame;
        RsiRobotState    state;

        if (!parseRobBlock(m_recvBuf, dataLen, frame, state))
            continue; // IPOC absent ou RIst absent → trame invalide, pas d'ACK

        parseLogBlock(m_recvBuf, dataLen, state);
        parseOptionalTags(m_recvBuf, dataLen, frame, state);

        // 5. ACK immédiat — corrections nulles, IPOC en écho
        sendAck(std::to_string(state.ipoc), m_robotIp, m_robotPort);

        // 6. Timestamp wall-clock + complétion frame
        frame.timestamp = QDateTime::currentMSecsSinceEpoch() * 1000LL;
        frame.systemName = QStringLiteral("KUKA RSI");
        frame.objectName = m_config.robotName;

        // 7. Métriques — fréquence depuis DtSend, latence depuis DurationJob
        const double freqHz = (state.dtSendMs > 0.0)
            ? 1000.0 / state.dtSendMs
            : 250.0;
        const bool latencyKnown = (state.durationJobMs > 0.0);
        m_latencyMs.store(state.durationJobMs);
        updateRunningStats(state.durationJobMs, freqHz, latencyKnown);

        // 8. Mise à jour frame partagée + émissions
        {
            QMutexLocker lk(&m_frameMutex);
            m_latestFrame = frame;
        }

        emit newFrameAvailable(frame);
        emit robotStateUpdated(state);
        emit performanceUpdate(m_metrics);
    }
}

// ============================================================================
// Parsing — bloc <Rob>
// ============================================================================

bool KukaRsiSystem::parseRobBlock(const char* data, std::size_t len,
    MeasurementFrame& frame, RsiRobotState& state)
{
    // ── IPOC (obligatoire — valide la trame) ─────────────────────────────────
    const char* vb = nullptr;
    std::size_t vl = 0;
    if (!TrameHelper::findLastTag(data, len, "IPOC>", "</IPOC>", vb, vl))
        return false;

    state.ipoc = 0;
    for (std::size_t i = 0; i < vl; ++i) {
        if (vb[i] < '0' || vb[i] > '9') break;
        state.ipoc = state.ipoc * 10 + static_cast<uint64_t>(vb[i] - '0');
    }

    // ── RIst — position cartésienne mesurée (toujours parsé) ─────────────────
    const char* wb = nullptr, * we = nullptr;
    if (!TrameHelper::findOpenTagWindow(data, len, "RIst ", wb, we))
        return false;

    static const char* kXYZABC[] = { "X", "Y", "Z", "A", "B", "C" };
    double rist[6] = {};
    TrameHelper::extractAttrDoubles(wb, we, kXYZABC, rist, 6);

    frame.x = rist[0];
    frame.y = rist[1];
    frame.z = rist[2];
    frame.a = rist[3];
    frame.b = rist[4];
    frame.c = rist[5];

    // ── Scalaires état robot ──────────────────────────────────────────────────
    auto readInt = [&](const char* open, const char* close, int& out) {
        const char* pb = nullptr; std::size_t pl = 0;
        if (!TrameHelper::findLastTag(data, len, open, close, pb, pl)) return;
        double d = 0.0;
        if (TrameHelper::parseDoubleFromSpan(pb, pb + pl, d))
            out = static_cast<int>(d);
        };
    auto readU32 = [&](const char* open, const char* close, uint32_t& out) {
        const char* pb = nullptr; std::size_t pl = 0;
        if (!TrameHelper::findLastTag(data, len, open, close, pb, pl)) return;
        double d = 0.0;
        if (TrameHelper::parseDoubleFromSpan(pb, pb + pl, d))
            out = static_cast<uint32_t>(d);
        };

    readInt("Krl>", "</Krl>", state.krl);
    readInt("Mode>", "</Mode>", state.mode);
    readInt("Bloc_Steps>", "</Bloc_Steps>", state.blocSteps);
    readInt("Bloc_Start>", "</Bloc_Start>", state.blocStart);
    readInt("Bloc_Waiting>", "</Bloc_Waiting>", state.blocWaiting);
    readInt("Bloc_End>", "</Bloc_End>", state.blocEnd);
    readInt("Bloc_Continue>", "</Bloc_Continue>", state.blocContinue);
    readInt("Bloc_Cancel>", "</Bloc_Cancel>", state.blocCancel);
    readInt("Bloc_ID>", "</Bloc_ID>", state.blocId);
    readU32("Digin>", "</Digin>", state.digin);
    readU32("Digout>", "</Digout>", state.digout);

    return true;
}

// ============================================================================
// Parsing — bloc <Log>
// ============================================================================

void KukaRsiSystem::parseLogBlock(const char* data, std::size_t len,
    RsiRobotState& state)
{
    auto readDouble = [&](const char* open, const char* close, double& out) {
        const char* pb = nullptr; std::size_t pl = 0;
        if (!TrameHelper::findLastTag(data, len, open, close, pb, pl)) return;
        TrameHelper::parseDoubleFromSpan(pb, pb + pl, out);
        };
    auto readInt = [&](const char* open, const char* close, int& out) {
        double d = 0.0;
        const char* pb = nullptr; std::size_t pl = 0;
        if (!TrameHelper::findLastTag(data, len, open, close, pb, pl)) return;
        if (TrameHelper::parseDoubleFromSpan(pb, pb + pl, d))
            out = static_cast<int>(d);
        };

    readDouble("DtSend>", "</DtSend>", state.dtSendMs);
    readDouble("DurationJob>", "</DurationJob>", state.durationJobMs);
    readDouble("TimeToWait>", "</TimeToWait>", state.timeToWaitUs);
    readInt("ConnectionStatus>", "</ConnectionStatus>", state.connectionStatus);
    readInt("Status>", "</Status>", state.status);
    readInt("ReqStatus>", "</ReqStatus>", state.reqStatus);
}

// ============================================================================
// Parsing — tags optionnels → frame.extras
// ============================================================================

void KukaRsiSystem::parseOptionalTags(const char* data, std::size_t len,
    MeasurementFrame& frame,
    const RsiRobotState& state)
{
    static const char* kXYZABC[] = { "X", "Y", "Z", "A", "B", "C" };
    static const char* kA1A6[] = { "A1", "A2", "A3", "A4", "A5", "A6" };

    for (const RsiTag tag : m_config.selectedTags) {
        const QString key = RsiTagMeta::key(tag);
        switch (tag) {

            // ── Vecteurs 6D — attributs X/Y/Z/A/B/C ──
        case RsiTag::RSol: {
            const char* wb2 = nullptr, * we2 = nullptr;
            if (TrameHelper::findOpenTagWindow(data, len, "RSol ", wb2, we2)) {
                double v[6] = {};
                TrameHelper::extractAttrDoubles(wb2, we2, kXYZABC, v, 6);
                frame.extras[key] = { v[0], v[1], v[2], v[3], v[4], v[5] };
            }
            break;
        }

                         // ── Vecteurs 6D — attributs A1/A2/.../A6 ──
        case RsiTag::AIPos:
        case RsiTag::ASPos:
        case RsiTag::MACur: {
            const std::string xmlTag = key.toStdString() + " ";
            const char* wb2 = nullptr, * we2 = nullptr;
            if (TrameHelper::findOpenTagWindow(data, len, xmlTag.c_str(), wb2, we2)) {
                double v[6] = {};
                TrameHelper::extractAttrDoubles(wb2, we2, kA1A6, v, 6);
                frame.extras[key] = { v[0], v[1], v[2], v[3], v[4], v[5] };
            }
            break;
        }

                          // ── Delay — attribut D ──
        case RsiTag::Delay: {
            const char* wb2 = nullptr, * we2 = nullptr;
            if (TrameHelper::findOpenTagWindow(data, len, "Delay ", wb2, we2)) {
                static const char* kD[] = { "D" };
                double v[1] = {};
                TrameHelper::extractAttrDoubles(wb2, we2, kD, v, 1);
                frame.extras[key] = { v[0] };
            }
            break;
        }

                          // ── Scalaires déjà parsés dans state → réexposés dans extras ──
        case RsiTag::Digin:        frame.extras[key] = { static_cast<double>(state.digin) };         break;
        case RsiTag::Digout:       frame.extras[key] = { static_cast<double>(state.digout) };        break;
        case RsiTag::Krl:          frame.extras[key] = { static_cast<double>(state.krl) };           break;
        case RsiTag::Mode:         frame.extras[key] = { static_cast<double>(state.mode) };          break;
        case RsiTag::BlocSteps:    frame.extras[key] = { static_cast<double>(state.blocSteps) };     break;
        case RsiTag::BlocStart:    frame.extras[key] = { static_cast<double>(state.blocStart) };     break;
        case RsiTag::BlocWaiting:  frame.extras[key] = { static_cast<double>(state.blocWaiting) };   break;
        case RsiTag::BlocEnd:      frame.extras[key] = { static_cast<double>(state.blocEnd) };       break;
        case RsiTag::BlocContinue: frame.extras[key] = { static_cast<double>(state.blocContinue) };  break;
        case RsiTag::BlocCancel:   frame.extras[key] = { static_cast<double>(state.blocCancel) };    break;
        case RsiTag::BlocId:       frame.extras[key] = { static_cast<double>(state.blocId) };        break;

            // ── Log RT ──
        case RsiTag::LogDtSend:          frame.extras[key] = { state.dtSendMs };                              break;
        case RsiTag::LogDurationJob:     frame.extras[key] = { state.durationJobMs };                         break;
        case RsiTag::LogTimeToWait:      frame.extras[key] = { state.timeToWaitUs };                          break;
        case RsiTag::LogConnectionStatus:frame.extras[key] = { static_cast<double>(state.connectionStatus) }; break;

        case RsiTag::RIst: break; // toujours parsé séparément, pas dans extras
        default:           break;
        }
    }
}

// ============================================================================
// ACK — corrections nulles, IPOC en écho
// ============================================================================

bool KukaRsiSystem::sendAck(const std::string& ipoc,
    const std::string& destIp, u_short destPort)
{
    RsiTrame trame;
    trame.setIPOC(ipoc);

    float zeros[6] = {};
    trame.setPose(/*isCartesian=*/true, zeros, /*isInRobotBase=*/true);
    trame.setStopFlag(false);
    trame.setKrl(0);
    trame.setMode(0);
    trame.setBlocContinue(false);
    trame.setBlocCancel(false);

    std::size_t ackLen = 0;
    if (!trame.build(m_ackBuf, sizeof(m_ackBuf), ackLen))
        return false;

    m_socket->sendTo(m_ackBuf, static_cast<int>(ackLen), destIp, destPort);
    return true;
}
