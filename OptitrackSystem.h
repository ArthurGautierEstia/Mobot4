#pragma once
#ifndef OPTITRACKSYSTEM_H
#define OPTITRACKSYSTEM_H

#include "IMeasurementSystem.h"
#include "OptiTrackConfig.h"
#include <QElapsedTimer>
#include <vector>

#include <NatNetTypes.h>
#include <NatNetCAPI.h>
#include <NatNetClient.h>

class OptiTrackSystem : public IMeasurementSystem {
    Q_OBJECT

public:
    explicit OptiTrackSystem(QObject* parent = nullptr);
    ~OptiTrackSystem() override;

    // ========== Interface IMeasurementSystem ==========
    bool initialize() override;
    bool connect(const ConnectionConfig& config) override;
    bool disconnect() override;
    bool isConnected() const override;

    bool startAcquisition() override;
    bool stopAcquisition() override;
    bool isAcquiring() const override;

    MeasurementFrame getLatestFrame() const override;
    double getNativeFrequency() const override;
    double getLatency() const override;              // ✅ manquait → classe abstraite
    QStringList getAvailableObjects() const override;

    SystemCapabilities getCapabilities() const override;
    QString getSystemName() const override;
    QString getSystemVersion() const override;
    ThreadingModel getThreadingModel() const override;

    // ========== Méthodes spécifiques OptiTrack ==========
    int discoverServers();
    QStringList getDiscoveredServers() const;
    bool connectToServer(int serverIndex);

private:
    // ========== Callbacks SDK (threads SDK) ==========
    static void NATNET_CALLCONV serverDiscoveredCallback(
        const sNatNetDiscoveredServer* pDiscoveredServer,
        void* pUserContext);

    static void NATNET_CALLCONV dataHandler(
        sFrameOfMocapData* data,
        void* pUserData);

    static void NATNET_CALLCONV messageHandler(
        Verbosity msgType,
        const char* msg);

    // ========== Méthodes internes ==========
    MeasurementFrame convertNatNetFrame(const sFrameOfMocapData* data);
    const sRigidBodyData* findRigidBody(const sFrameOfMocapData* data);
    void quaternionToEuler(float qx, float qy, float qz, float qw,
        double& rx, double& ry, double& rz);
    void initializeCapabilities();
    void cleanup();

    // ========== Membres privés ==========
    NatNetClient* m_pClient;
    std::vector<sNatNetDiscoveredServer> m_discoveredServers;
    sNatNetClientConnectParams m_connectParams;
    sServerDescription m_serverDescription;

    OptiTrackConfig m_optitrackConfig;

    bool m_isConnected;

    // ✅ Timestamps séparés pour calcul latence correct
    QElapsedTimer m_timer;
    double m_latency;
    double m_frequency;
    qint64 m_lastFrameTime;    // Timestamp Qt de la dernière frame (µs)
    int    m_frameCount;       // Conservé pour compatibilité (non utilisé dans updateRunningStats)

    FILE* m_logFile;

    QString m_currentRigidBodyName;
    int     m_currentRigidBodyId;

    static const ConnectionType kDefaultConnectionType = ConnectionType_Multicast;
};

#endif // OPTITRACKSYSTEM_H
