#include "QualisysSystem.h"
#include <QDebug>
#include <cmath>

QualisysSystem::QualisysSystem(QObject* parent)
    : IMeasurementSystem(parent)
    , m_rtProtocol(nullptr)
    , m_isConnected(false)
    , m_acquisitionThread(nullptr)
    , m_targetBodyIndex(-1)
    , m_lastFrameTime(0)
    , m_latency(0.0)
    , m_frequency(0.0)
    , m_lastFrameNumber(0)
{
    initializeCapabilities();
    m_timer.start();
}

QualisysSystem::~QualisysSystem()
{
    cleanup();
}

void QualisysSystem::initializeCapabilities()
{
    m_capabilities.systemName           = "Qualisys";
    m_capabilities.version              = "QTM RT 1.19+";
    m_capabilities.maxNativeFrequency   = 2000.0; // Qualisys jusqu'à 2000 Hz
    m_capabilities.minNativeFrequency   = 1.0;
    m_capabilities.supportsVariableRate    = true;  // Fréquence fixée dans QTM
    m_capabilities.supportsQuaternions     = false; // 6DEuler → Euler uniquement
    m_capabilities.supportsMultipleObjects = true;
    m_capabilities.typicalLatency          = 5.0;   // ~5ms typique
}

// ========== Cycle de vie ==========

bool QualisysSystem::initialize()
{
    emit logMessage("Initializing Qualisys system...");

    m_rtProtocol = new CRTProtocol();
    if (!m_rtProtocol) {
        emit errorOccurred("Failed to create CRTProtocol instance");
        return false;
    }

    emit logMessage("Qualisys system initialized");
    return true;
}

bool QualisysSystem::connect(const ConnectionConfig& config)
{
    if (m_isConnected) {
        emit logMessage("Already connected, disconnecting first...");
        disconnect();
    }

    if (!m_rtProtocol) {
        if (!initialize()) return false;
    }

    const unsigned short port = config.port > 0
        ? static_cast<unsigned short>(config.port)
        : m_qualisysConfig.basePort;

    emit logMessage(QString("Connecting to QTM: %1:%2")
        .arg(config.serverAddress).arg(port));

    unsigned short udpServerPort = 0;
    if (!m_rtProtocol->Connect(
        config.serverAddress.toStdString().c_str(),
        port,
        &udpServerPort,
        m_qualisysConfig.majorVersion,
        m_qualisysConfig.minorVersion,
        m_qualisysConfig.bigEndian))
    {
        emit errorOccurred(QString("Failed to connect to QTM: %1")
            .arg(m_rtProtocol->GetErrorString()));
        return false;
    }

    // ✅ Récupérer la liste des corps 6DOF
    bool dataAvailable = false;
    if (!m_rtProtocol->Read6DOFSettings(dataAvailable)) {
        emit logMessage("Warning: Could not read 6DOF settings");
    }
    else {
        m_bodyNames.clear();
        const unsigned int nBodies = m_rtProtocol->Get6DOFBodyCount();
        for (unsigned int i = 0; i < nBodies; ++i) {
            m_bodyNames << QString::fromUtf8(m_rtProtocol->Get6DOFBodyName(i));
        }
        emit logMessage(QString("Found %1 6DOF body/bodies: %2")
            .arg(nBodies).arg(m_bodyNames.join(", ")));
    }

    m_targetBodyName  = config.objectName;
    m_targetBodyIndex = findBodyIndex(config.objectName);

    if (m_targetBodyIndex < 0 && !m_bodyNames.isEmpty()) {
        m_targetBodyIndex = 0;
        m_targetBodyName  = m_bodyNames.first();
        emit logMessage(QString("Body '%1' not found, using first: '%2'")
            .arg(config.objectName).arg(m_targetBodyName));
    }

    m_isConnected = true;
    emit connected();
    emit logMessage(QString("Qualisys connected. Tracking: '%1'").arg(m_targetBodyName));
    return true;
}

bool QualisysSystem::disconnect()
{
    if (!m_isConnected) return true;

    emit logMessage("Disconnecting from Qualisys...");
    if (m_isAcquiring)  stopAcquisition();
    if (m_rtProtocol)   m_rtProtocol->Disconnect();

    m_isConnected = false;
    emit disconnected();
    emit logMessage("Qualisys disconnected");
    return true;
}

bool QualisysSystem::isConnected() const
{
    return m_isConnected;
}

// ========== Acquisition ==========

bool QualisysSystem::startAcquisition()
{
    if (!m_isConnected) {
        emit errorOccurred("Cannot start acquisition: not connected");
        return false;
    }
    if (m_isAcquiring) {
        emit logMessage("Acquisition already running");
        return true;
    }

    // ✅ Démarrer le streaming 6DEuler depuis QTM
    const unsigned short udpPort = m_qualisysConfig.useUDP
        ? m_qualisysConfig.udpPort : 0;

    if (!m_rtProtocol->StreamFrames(
        CRTProtocol::EStreamRate::RateAllFrames,
        0,
        udpPort,
        nullptr,
        CRTProtocol::cComponent6dEuler))
    {
        emit errorOccurred(QString("Failed to start streaming: %1")
            .arg(m_rtProtocol->GetErrorString()));
        return false;
    }

    // ✅ Réinitialiser les statistiques de session
    resetSessionStats();

    m_isAcquiring     = true;
    m_lastFrameTime   = 0;
    m_lastFrameNumber = 0;

    m_acquisitionThread = QThread::create([this]() { acquisitionLoop(); });
    m_acquisitionThread->start();

    emit logMessage("Qualisys acquisition started (StreamFrames AllFrames 6DEuler)");
    return true;
}

bool QualisysSystem::stopAcquisition()
{
    if (!m_isAcquiring) return true;

    emit logMessage("Stopping Qualisys acquisition...");
    m_isAcquiring = false; // Signal au thread de s'arrêter

    if (m_rtProtocol) m_rtProtocol->StreamFramesStop();

    // Attendre la fin propre du thread
    if (m_acquisitionThread) {
        m_acquisitionThread->wait(3000);
        delete m_acquisitionThread;
        m_acquisitionThread = nullptr;
    }

    // ✅ Émettre le résumé de session (après la fin du thread → compteurs finaux)
    AcquisitionSummary summary = buildSummary(m_targetBodyName);
    emit acquisitionCompleted(summary);

    emit logMessage("Qualisys acquisition stopped");
    return true;
}

bool QualisysSystem::isAcquiring() const
{
    return m_isAcquiring.load();
}

// ========== Boucle d'acquisition ==========

void QualisysSystem::acquisitionLoop()
{
    emit logMessage("Qualisys acquisition loop started");

    while (m_isAcquiring.load()) {
        CRTPacket::EPacketType packetType;
        const int timeoutMs = computeReceiveTimeoutMs();

        auto response = m_rtProtocol->Receive(packetType, false, timeoutMs);

        if (!m_isAcquiring.load()) break;

        if (response == CNetwork::ResponseType::timeout) continue;

        if (response != CNetwork::ResponseType::success) {
            emit logMessage("Qualisys: receive error, retrying...");
            continue;
        }

        if (packetType == CRTPacket::PacketNoMoreData) {
            emit logMessage("QTM: No more data");
            m_isAcquiring = false;
            break;
        }

        if (packetType != CRTPacket::PacketData) continue;

        CRTPacket* pPacket = m_rtProtocol->GetRTPacket();
        if (!pPacket) continue;

        const unsigned int bodyCount = pPacket->Get6DOFEulerBodyCount();
        if (bodyCount == 0) continue;

        // Détecter les frames perdues
        const quint32 frameNumber = pPacket->GetFrameNumber();
        if (m_lastFrameNumber > 0 && frameNumber > m_lastFrameNumber + 1) {
            m_metrics.droppedFrames += frameNumber - m_lastFrameNumber - 1;
        }
        m_lastFrameNumber = frameNumber;

        MeasurementFrame frame = parseFrame(pPacket);
        if (!frame.isValid) continue;

        {
            QMutexLocker locker(&m_frameMutex);
            m_latestFrame = frame;
        }
        m_frameBuffer.push(frame);

        // Calcul fréquence instantanée
        const qint64 nowUs = m_timer.nsecsElapsed() / 1000;
        if (m_lastFrameTime > 0) {
            const qint64 delta = nowUs - m_lastFrameTime;
            if (delta > 0)
                m_frequency = 1000000.0 / static_cast<double>(delta);
        }
        m_lastFrameTime = nowUs;

        // ✅ Qualisys : latence non mesurable via RT protocol → latencyKnown = false
        updateRunningStats(0.0, m_frequency, false);

        emit newFrameAvailable(frame);
        emit performanceUpdate(m_metrics);
    }

    emit logMessage("Qualisys acquisition loop ended");
}

// ========== Parsing ==========

MeasurementFrame QualisysSystem::parseFrame(CRTPacket* packet)
{
    MeasurementFrame frame;
    frame.systemName  = "Qualisys";
    frame.isValid     = false;
    frame.frameNumber = packet->GetFrameNumber();
    frame.timestamp   = static_cast<qint64>(packet->GetTimeStamp());

    if (m_targetBodyIndex < 0) return frame;

    float x, y, z, ang1, ang2, ang3;
    if (!packet->Get6DOFEulerBody(
        static_cast<unsigned int>(m_targetBodyIndex),
        x, y, z, ang1, ang2, ang3))
    {
        return frame;
    }

    if (std::isnan(x) || std::isnan(y) || std::isnan(z)) return frame;

    frame.isValid    = true;
    frame.objectName = m_targetBodyName;

    frame.x  = static_cast<double>(x);
    frame.y  = static_cast<double>(y);
    frame.z  = static_cast<double>(z);
    frame.rx = static_cast<double>(ang1);
    frame.ry = static_cast<double>(ang2);
    frame.rz = static_cast<double>(ang3);

    frame.quaternion.valid = false;
    frame.quality          = 1.0;

    return frame;
}

// ========== Helpers ==========

int QualisysSystem::findBodyIndex(const QString& name) const
{
    if (name.isEmpty()) return m_bodyNames.isEmpty() ? -1 : 0;
    for (int i = 0; i < m_bodyNames.size(); ++i) {
        if (m_bodyNames[i].compare(name, Qt::CaseInsensitive) == 0)
            return i;
    }
    return -1;
}

int QualisysSystem::computeReceiveTimeoutMs() const
{
    if (m_frequency > 1.0) {
        const int twoPeriods = static_cast<int>(2000.0 / m_frequency);
        return qBound(100, twoPeriods, 1000);
    }
    return kDefaultReceiveTimeoutMs;
}

// ========== Getters ==========

MeasurementFrame QualisysSystem::getLatestFrame() const
{
    QMutexLocker locker(&m_frameMutex);
    return m_latestFrame;
}

double QualisysSystem::getNativeFrequency() const
{
    return m_frequency > 0.0 ? m_frequency : m_capabilities.maxNativeFrequency;
}

double QualisysSystem::getLatency() const
{
    // Le protocole QTM RT ne fournit pas de mesure de latence.
    return 0.0;
}

QStringList QualisysSystem::getAvailableObjects() const { return m_bodyNames; }
SystemCapabilities QualisysSystem::getCapabilities() const { return m_capabilities; }
QString QualisysSystem::getSystemName()    const { return "Qualisys"; }
QString QualisysSystem::getSystemVersion() const { return m_capabilities.version; }

IMeasurementSystem::ThreadingModel QualisysSystem::getThreadingModel() const
{
    return ThreadingModel::ApplicationManaged;
}

void QualisysSystem::cleanup()
{
    disconnect();
    if (m_rtProtocol) { delete m_rtProtocol; m_rtProtocol = nullptr; }
}
