#pragma once
#ifndef KUKARSISYSTEM_H
#define KUKARSISYSTEM_H

#include "IMeasurementSystem.h"
#include "KukaRsiConfig.h"
#include "NativeUdpSocket.h"
#include "RsiRobotState.h"

#include <memory>
#include <atomic>

class KukaRsiSystem : public IMeasurementSystem {
    Q_OBJECT

public:
    explicit KukaRsiSystem(const KukaRsiConfig& config, QObject* parent = nullptr);
    ~KukaRsiSystem() override;

    // ── IMeasurementSystem ───────────────────────────────────────────────────
    bool initialize()                            override;
    bool connect(const ConnectionConfig& config) override;
    bool disconnect()                            override;
    bool isConnected()                     const override;

    bool startAcquisition()                      override;
    bool stopAcquisition()                       override;
    bool isAcquiring()                     const override;

    MeasurementFrame   getLatestFrame()    const override;
    double             getNativeFrequency()const override;
    double             getLatency()        const override;
    QStringList        getAvailableObjects()const override;
    SystemCapabilities getCapabilities()   const override;
    QString            getSystemName()     const override;
    QString            getSystemVersion()  const override;
    ThreadingModel     getThreadingModel() const override;

signals:
    /**
     * @brief Émis à chaque cycle avec l'état complet du robot.
     * Utile pour monitoring UI (Bloc_*, Krl, Log RT).
     * Distinct de newFrameAvailable — ne pas confondre.
     */
    void robotStateUpdated(const RsiRobotState& state);

private:
    void acquisitionLoop();

    bool parseRobBlock(const char* data, std::size_t len,
        MeasurementFrame& frame, RsiRobotState& state);
    void parseLogBlock(const char* data, std::size_t len,
        RsiRobotState& state);
    void parseOptionalTags(const char* data, std::size_t len,
        MeasurementFrame& frame,
        const RsiRobotState& state);
    bool sendAck(const std::string& ipoc,
        const std::string& destIp, u_short destPort);

private:
    KukaRsiConfig                    m_config;
    std::unique_ptr<NativeUdpSocket> m_socket;
    std::atomic<bool>                m_isConnected{ false };

    // Adresse robot apprise dynamiquement au 1er paquet reçu
    std::string                      m_robotIp;
    u_short                          m_robotPort{ 0 };
    bool                             m_robotAddrKnown{ false };

    // Latence issue de Log.DurationJob
    std::atomic<double>              m_latencyMs{ 0.0 };

    static constexpr std::size_t     k_bufSize = 4096;
    char                             m_recvBuf[k_bufSize];
    char                             m_ackBuf[1024];
};

#endif // KUKARSISYSTEM_H
#pragma once
