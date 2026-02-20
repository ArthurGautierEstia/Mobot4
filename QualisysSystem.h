#pragma once
#ifndef QUALIYSSYSTEM_H
#define QUALIYSSYSTEM_H

#include "IMeasurementSystem.h"
#include "QualisysConfig.h"
#include <QThread>
#include <QElapsedTimer>

// SDK Qualisys
#include <RTProtocol.h>
#include <RTPacket.h>

class QualisysSystem : public IMeasurementSystem {
    Q_OBJECT

public:
    explicit QualisysSystem(QObject* parent = nullptr);
    ~QualisysSystem() override;

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
    QStringList getAvailableObjects() const override;
    double getLatency() const override;

    SystemCapabilities getCapabilities() const override;
    QString getSystemName() const override;
    QString getSystemVersion() const override;
    ThreadingModel getThreadingModel() const override;

private:
    // ========== Thread d'acquisition ==========

    /**
     * @brief Boucle d'acquisition - tourne dans m_acquisitionThread
     *
     * Utilise Receive() bloquant avec timeout : PAS de sleep() ici.
     * Le timeout IS l'attente réseau.
     */
    void acquisitionLoop();

    // ========== Méthodes internes ==========

    /**
     * @brief Parse une frame 6DEuler depuis un CRTPacket
     */
    MeasurementFrame parseFrame(CRTPacket* packet);

    /**
     * @brief Cherche l'index d'un corps par nom
     */
    int findBodyIndex(const QString& name) const;

    /**
     * @brief Calcule le timeout réseau optimal pour Receive()
     * = 2 périodes de frame, borné entre 100ms et 1000ms
     * NE PAS confondre avec un sleep : c'est un timeout réseau
     */
    int computeReceiveTimeoutMs() const;

    void initializeCapabilities();
    void cleanup();

    // ========== SDK ==========
    CRTProtocol* m_rtProtocol;

    // ========== Config ==========
    QualisysConfig m_qualisysConfig;

    // ========== État ==========
    bool     m_isConnected;
    QThread* m_acquisitionThread;

    // ========== Corps 6DOF ==========
    QStringList m_bodyNames;       // Noms récupérés via GetParameters 6D
    int         m_targetBodyIndex; // Index dans le flux de données
    QString     m_targetBodyName;

    // ========== Performance ==========
    QElapsedTimer m_timer;
    qint64        m_lastFrameTime;   // µs (horloge locale)
    double        m_latency;         // ms (toujours 0 : protocole ne le fournit pas)
    double        m_frequency;       // Hz mesuré

    // Détection frames perdues (via Marker Frame Number)
    quint32 m_lastFrameNumber;

    static constexpr int kDefaultReceiveTimeoutMs = 1000;
};

#endif // QUALIYSSYSTEM_H

