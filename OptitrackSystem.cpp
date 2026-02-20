#include "OptiTrackSystem.h"
#include <QDebug>
#include <cmath>

OptiTrackSystem::OptiTrackSystem(QObject* parent)
    : IMeasurementSystem(parent)
    , m_pClient(nullptr)
    , m_isConnected(false)
    , m_latency(0.0)
    , m_frequency(0.0)
    , m_lastFrameTime(0)
    , m_frameCount(0)
    , m_logFile(nullptr)
    , m_currentRigidBodyId(-1)
{
    initializeCapabilities();
    m_timer.start();
}

OptiTrackSystem::~OptiTrackSystem()
{
    cleanup();
}

void OptiTrackSystem::initializeCapabilities()
{
    m_capabilities.systemName = "OptiTrack";
    m_capabilities.version = "NatNet 4.x"; // Sera mis à jour à la connexion
    m_capabilities.maxNativeFrequency = 240.0;  // OptiTrack max 240 Hz
    m_capabilities.minNativeFrequency = 30.0;
    m_capabilities.supportsVariableRate = false; // Fréquence fixée par Motive
    m_capabilities.supportsQuaternions = true;   // OptiTrack fournit des quaternions
    m_capabilities.supportsMultipleObjects = true;
    m_capabilities.typicalLatency = 4.0;  // ~4ms typique
}

bool OptiTrackSystem::initialize()
{
    emit logMessage("Initializing OptiTrack system...");

    // Créer le client NatNet
    m_pClient = new NatNetClient();
    if (!m_pClient) {
        emit errorOccurred("Failed to create NatNet client");
        return false;
    }

    // Configurer les callbacks
    m_pClient->SetFrameReceivedCallback(dataHandler, this);

    emit logMessage("OptiTrack system initialized successfully");
    return true;
}

bool OptiTrackSystem::connect(const ConnectionConfig& config)
{
    if (m_isConnected) {
        emit logMessage("Already connected, disconnecting first...");
        disconnect();
    }

    if (!m_pClient) {
        if (!initialize()) {
            return false;
        }
    }

    emit logMessage(QString("Connecting to OptiTrack server: %1:%2")
        .arg(config.serverAddress)
        .arg(config.port));

    // Préparer les paramètres de connexion
    m_connectParams.connectionType = kDefaultConnectionType;
    m_connectParams.serverAddress = config.serverAddress.isEmpty()
        ? nullptr
        : config.serverAddress.toStdString().c_str();
    m_connectParams.localAddress = nullptr; // Auto-détection
    m_connectParams.multicastAddress = m_optitrackConfig.multicastAddress.toStdString().c_str();

    // Connecter
    ErrorCode ret = m_pClient->Connect(m_connectParams);
    if (ret != ErrorCode_OK) {
        QString error = QString("Failed to connect to OptiTrack server (error code: %1)")
            .arg(static_cast<int>(ret));
        emit errorOccurred(error);
        return false;
    }

    // Récupérer la description du serveur
    memset(&m_serverDescription, 0, sizeof(m_serverDescription));
    ret = m_pClient->GetServerDescription(&m_serverDescription);
    if (ret != ErrorCode_OK) {
        emit logMessage("Warning: Could not get server description");
    }
    else {
        m_capabilities.version = QString("NatNet %1.%2.%3.%4")
            .arg(m_serverDescription.NatNetVersion[0])
            .arg(m_serverDescription.NatNetVersion[1])
            .arg(m_serverDescription.NatNetVersion[2])
            .arg(m_serverDescription.NatNetVersion[3]);

        emit logMessage(QString("Connected to: %1 (NatNet %2.%3)")
            .arg(m_serverDescription.szHostComputerName)
            .arg(m_serverDescription.NatNetVersion[0])
            .arg(m_serverDescription.NatNetVersion[1]));
    }

    // Stocker la config
    m_currentRigidBodyName = config.objectName;

    m_isConnected = true;
    emit connected();
    emit logMessage("OptiTrack connected successfully");

    return true;
}

bool OptiTrackSystem::disconnect()
{
    if (!m_isConnected) {
        return true;
    }

    emit logMessage("Disconnecting from OptiTrack...");

    // Arrêter l'acquisition si active
    if (m_isAcquiring) {
        stopAcquisition();
    }

    // Déconnecter le client
    if (m_pClient) {
        m_pClient->Disconnect();
    }

    m_isConnected = false;
    emit disconnected();
    emit logMessage("OptiTrack disconnected");

    return true;
}

bool OptiTrackSystem::isConnected() const
{
    return m_isConnected;
}

bool OptiTrackSystem::startAcquisition()
{
    if (!m_isConnected) {
        emit errorOccurred("Cannot start acquisition: not connected");
        return false;
    }

    if (m_isAcquiring) {
        emit logMessage("Acquisition already running");
        return true;
    }

    emit logMessage("Starting OptiTrack acquisition...");

    // Pas de thread à créer, le SDK gère via callback
    m_isAcquiring = true;

    emit logMessage("OptiTrack acquisition started");
    return true;
}

bool OptiTrackSystem::stopAcquisition()
{
    if (!m_isAcquiring) {
        return true;
    }

    emit logMessage("Stopping OptiTrack acquisition...");

    m_isAcquiring = false;

    emit logMessage("OptiTrack acquisition stopped");
    return true;
}

bool OptiTrackSystem::isAcquiring() const
{
    return m_isAcquiring.load();
}

MeasurementFrame OptiTrackSystem::getLatestFrame() const
{
    QMutexLocker locker(&m_frameMutex);
    return m_latestFrame;
}

double OptiTrackSystem::getNativeFrequency() const
{
    return m_frequency > 0 ? m_frequency : m_capabilities.maxNativeFrequency;
}

QStringList OptiTrackSystem::getAvailableObjects() const
{
    QStringList objects;

    if (!m_isConnected || !m_pClient) {
        return objects;
    }

    // Récupérer la liste réelle depuis le serveur
    sDataDescriptions* pDataDefs = nullptr;
    ErrorCode ret = m_pClient->GetDataDescriptionList(&pDataDefs);

    if (ret == ErrorCode_OK && pDataDefs) {
        for (int i = 0; i < pDataDefs->nDataDescriptions; i++) {
            if (pDataDefs->arrDataDescriptions[i].type == Descriptor_RigidBody) {
                sRigidBodyDescription* rb = pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription;
                objects << QString::fromUtf8(rb->szName);
            }
        }
        NatNet_FreeDescriptions(pDataDefs);
    }

    return objects;
}

SystemCapabilities OptiTrackSystem::getCapabilities() const
{
    return m_capabilities;
}

QString OptiTrackSystem::getSystemName() const
{
    return "OptiTrack";
}

QString OptiTrackSystem::getSystemVersion() const
{
    return m_capabilities.version;
}

IMeasurementSystem::ThreadingModel OptiTrackSystem::getThreadingModel() const
{
    return ThreadingModel::SdkManaged; // SDK gère le thread via callback
}

int OptiTrackSystem::discoverServers()
{
    emit logMessage("Discovering OptiTrack servers on network...");

    m_discoveredServers.clear();

    NatNetDiscoveryHandle discovery;
    NatNet_CreateAsyncServerDiscovery(&discovery, serverDiscoveredCallback, this);

    // On attend ici dans le thread UI → OK, mais attention au freeze
    // Préférer un QTimer + signal si tu veux non-bloquant
    QThread::msleep(2000);

    NatNet_FreeAsyncServerDiscovery(discovery);

    emit logMessage(QString("Found %1 OptiTrack server(s)")
        .arg(m_discoveredServers.size()));

    return static_cast<int>(m_discoveredServers.size());
}

QStringList OptiTrackSystem::getDiscoveredServers() const
{
    QStringList servers;

    for (const auto& server : m_discoveredServers) {
        QString info = QString("%1 (%2)")
            .arg(server.serverDescription.szHostComputerName)
            .arg(server.serverAddress);
        servers << info;
    }

    return servers;
}

bool OptiTrackSystem::connectToServer(int serverIndex)
{
    if (serverIndex < 0 || serverIndex >= static_cast<int>(m_discoveredServers.size())) {
        emit errorOccurred("Invalid server index");
        return false;
    }

    const auto& server = m_discoveredServers[serverIndex];

    ConnectionConfig config;
    config.systemType = "OptiTrack";
    config.serverAddress = QString(server.serverAddress);
    config.port = m_optitrackConfig.commandPort;

    return connect(config);
}

// ========== Callbacks SDK ==========

void NATNET_CALLCONV OptiTrackSystem::serverDiscoveredCallback(
    const sNatNetDiscoveredServer* pDiscoveredServer,
    void* pUserContext)
{
    OptiTrackSystem* system = static_cast<OptiTrackSystem*>(pUserContext);

    if (!system || !pDiscoveredServer) return;

    system->m_discoveredServers.push_back(*pDiscoveredServer);

    // Log via qDebug (thread-safe), pas emit
    qDebug() << "[OptiTrack] Discovered server:"
        << pDiscoveredServer->serverDescription.szHostComputerName
        << "at" << pDiscoveredServer->serverAddress;
}

void NATNET_CALLCONV OptiTrackSystem::dataHandler(
    sFrameOfMocapData* data,
    void* pUserData)
{
    OptiTrackSystem* system = static_cast<OptiTrackSystem*>(pUserData);

    if (!system || !data || !system->m_isAcquiring) {
        return;
    }

    // Convertir la frame
    MeasurementFrame frame = system->convertNatNetFrame(data);

    if (!frame.isValid) {
        return;
    }

    // Mettre à jour la dernière frame (thread-safe)
    {
        QMutexLocker locker(&system->m_frameMutex);
        system->m_latestFrame = frame;
    }

    // Ajouter au buffer circulaire
    system->m_frameBuffer.push(frame);

    // Calcul de fréquence 
    qint64 now = system->m_timer.nsecsElapsed() / 1000; // µs
    if (system->m_lastFrameTime > 0) {
        qint64 delta = now - system->m_lastFrameTime;
        if (delta > 0) {
            system->m_frequency = 1000000.0 / delta; // Hz
        }
    }
    system->m_lastFrameTime = now;

   
	// On calcule la latence entre la capture de la frame et sa réception dans le callback
    const bool bClientLatencyAvailable =
        (data->CameraMidExposureTimestamp != 0);

    if (bClientLatencyAvailable) {
        // Latence totale précise (A→E) : caméras Ethernet
        system->m_latency =
            system->m_pClient->SecondsSinceHostTimestamp(
                data->CameraMidExposureTimestamp) * 1000.0;
    }
    else {
        // Fallback : Transmission Latency (D→E) : toujours disponible
        // Moins précis car ne compte pas le traitement Motive
        system->m_latency =
            system->m_pClient->SecondsSinceHostTimestamp(
                data->TransmitTimestamp) * 1000.0;
    }

    // Émettre via QueuedConnection (thread-safe vers le thread Qt)
    // Qt gère automatiquement le cross-thread avec QueuedConnection
    emit system->newFrameAvailable(frame);
}

void NATNET_CALLCONV OptiTrackSystem::messageHandler(
    Verbosity msgType,
    const char* msg)
{
    // ✅ Pas d'emit ici, juste un qDebug (thread-safe)
    if (msgType == Verbosity_Error) {
        qCritical() << "[OptiTrack SDK Error]" << msg;
    }
    else if (msgType == Verbosity_Warning) {
        qWarning() << "[OptiTrack SDK Warning]" << msg;
    }
    else {
        qDebug() << "[OptiTrack SDK]" << msg;
    }
}

// ========== Méthodes de conversion ==========

MeasurementFrame OptiTrackSystem::convertNatNetFrame(const sFrameOfMocapData* data)
{
    MeasurementFrame frame;
    frame.systemName = "OptiTrack";
    frame.timestamp = static_cast<qint64>(data->fTimestamp * 1000000.0); // Convertir en µs
    frame.frameNumber = data->iFrame;
    frame.isValid = false;

    // Chercher le rigid body
    const sRigidBodyData* rigidBody = findRigidBody(data);

    if (!rigidBody) {
        return frame; // Rigid body non trouvé
    }

    // Vérifier que le tracking est valide
    bool bTrackingValid = rigidBody->params & 0x01;
    if (!bTrackingValid) {
        return frame;
    }

    frame.isValid = true;
    frame.objectName = m_currentRigidBodyName;

    // Position (convertir mètres → mm)
    frame.x = rigidBody->x * 1000.0;
    frame.y = rigidBody->y * 1000.0;
    frame.z = rigidBody->z * 1000.0;

    // Stocker le quaternion
    frame.quaternion.x = rigidBody->qx;
    frame.quaternion.y = rigidBody->qy;
    frame.quaternion.z = rigidBody->qz;
    frame.quaternion.w = rigidBody->qw;
    frame.quaternion.valid = true;

    // Convertir quaternion en angles d'Euler (rx, ry, rz en degrés)
    quaternionToEuler(rigidBody->qx, rigidBody->qy, rigidBody->qz, rigidBody->qw,
        frame.rx, frame.ry, frame.rz);

    // Qualité (basée sur l'erreur moyenne des marqueurs)
    frame.quality = rigidBody->MeanError < 1.0 ? (1.0 - rigidBody->MeanError) : 0.0;

    return frame;
}

const sRigidBodyData* OptiTrackSystem::findRigidBody(const sFrameOfMocapData* data)
{
    if (!data || data->nRigidBodies == 0) {
        return nullptr;
    }

    // Si un ID spécifique est demandé
    if (m_currentRigidBodyId >= 0) {
        for (int i = 0; i < data->nRigidBodies; i++) {
            if (data->RigidBodies[i].ID == m_currentRigidBodyId) {
                return &data->RigidBodies[i];
            }
        }
    }

    // Sinon, prendre le premier rigid body valide
    return &data->RigidBodies[0];
}

void OptiTrackSystem::quaternionToEuler(float qx, float qy, float qz, float qw,
    double& rx, double& ry, double& rz)
{
    // Conversion quaternion → Euler (ZYX convention)
    // Formules standard

    double sqw = qw * qw;
    double sqx = qx * qx;
    double sqy = qy * qy;
    double sqz = qz * qz;

    // Rx (rotation autour X)
    rx = std::atan2(2.0 * (qy * qz + qw * qx), sqw - sqx - sqy + sqz);

    // Ry (rotation autour Y)
    double sinRy = -2.0 * (qx * qz - qw * qy);
    if (std::abs(sinRy) >= 1.0) {
        ry = std::copysign(M_PI / 2.0, sinRy); // Gimbal lock
    }
    else {
        ry = std::asin(sinRy);
    }

    // Rz (rotation autour Z)
    rz = std::atan2(2.0 * (qx * qy + qw * qz), sqw + sqx - sqy - sqz);

    // Convertir radians → degrés
    rx = rx * 180.0 / M_PI;
    ry = ry * 180.0 / M_PI;
    rz = rz * 180.0 / M_PI;
}

void OptiTrackSystem::cleanup()
{
    disconnect();

    if (m_pClient) {
        delete m_pClient;
        m_pClient = nullptr;
    }

    if (m_logFile) {
        fclose(m_logFile);
        m_logFile = nullptr;
    }
}
