#pragma once
#ifndef IMEASUREMENTSYSTEM_H
#define IMEASUREMENTSYSTEM_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <atomic>
#include <limits>
#include "MeasurementFrame.h"
#include "SystemCapabilities.h"
#include "ConnectionConfig.h"
#include "PerformanceMetrics.h"
#include "AcquisitionSummary.h"
#include "CircularBuffer.h"

/**
 * @brief Interface abstraite pour tous les systèmes de mesure
 *
 * Chaque système (OptiTrack, Vicon, Qualisys) doit hériter de cette classe
 * et implémenter toutes les méthodes virtuelles pures.
 *
 * Gestion des statistiques de session :
 *   - Appeler resetSessionStats() dans startAcquisition()
 *   - Appeler updateRunningStats() à chaque frame reçue
 *   - Appeler buildSummary() dans stopAcquisition() puis emit acquisitionCompleted()
 */
class IMeasurementSystem : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Modèle de threading du système
     */
    enum class ThreadingModel {
        SdkManaged,           // Le SDK gère le thread (ex: OptiTrack callback)
        ApplicationManaged    // L'application crée un thread (ex: Vicon, Qualisys)
    };

    virtual ~IMeasurementSystem();

    // ========== Cycle de vie ==========

    virtual bool initialize() = 0;
    virtual bool connect(const ConnectionConfig& config) = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() const = 0;

    // ========== Acquisition ==========

    virtual bool startAcquisition() = 0;
    virtual bool stopAcquisition() = 0;
    virtual bool isAcquiring() const = 0;

    // ========== Données ==========

    virtual MeasurementFrame getLatestFrame() const = 0;
    virtual double getNativeFrequency() const = 0;

    /**
     * @brief Retourne la meilleure estimation de latence disponible (ms)
     * Chaque système implémente la méthode la plus précise possible.
     * Retourne 0.0 si le protocole ne permet pas de la mesurer.
     */
    virtual double getLatency() const = 0;

    virtual QStringList getAvailableObjects() const = 0;

    // ========== Capacités ==========

    virtual SystemCapabilities getCapabilities() const = 0;
    virtual QString getSystemName() const = 0;
    virtual QString getSystemVersion() const = 0;
    virtual ThreadingModel getThreadingModel() const = 0;

signals:
    void connected();
    void disconnected();
    void newFrameAvailable(const MeasurementFrame& frame);
    void errorOccurred(const QString& error);
    void logMessage(const QString& message);

    /** @brief Émis à chaque frame avec les métriques instantanées */
    void performanceUpdate(const PerformanceMetrics& metrics);

    /** @brief Émis une seule fois à l'arrêt de l'acquisition */
    void acquisitionCompleted(const AcquisitionSummary& summary);

protected:
    explicit IMeasurementSystem(QObject* parent = nullptr);

    // =========================================================================
    // Gestion des statistiques de session
    // =========================================================================

    /**
     * @brief Réinitialise tous les accumulateurs de session.
     *        À appeler au début de startAcquisition().
     */
    void resetSessionStats();

    /**
     * @brief Met à jour m_metrics (valeurs live) et accumule pour le résumé final.
     *        À appeler à chaque frame dans la boucle d'acquisition.
     *
     * @param latencyMs    Latence instantanée (ms). Ignorée si latencyKnown = false.
     * @param freqHz       Fréquence instantanée mesurée (Hz).
     * @param latencyKnown true si le système peut mesurer la latence (false = Qualisys).
     */
    void updateRunningStats(double latencyMs, double freqHz, bool latencyKnown = true);

    /**
     * @brief Construit l'AcquisitionSummary à partir des accumulateurs.
     *        À appeler dans stopAcquisition() avant emit acquisitionCompleted().
     *
     * @param objectName Nom de l'objet tracké (pour renseigner summary.objectName).
     */
    AcquisitionSummary buildSummary(const QString& objectName = QString()) const;

    // =========================================================================
    // Membres protégés
    // =========================================================================

    SystemCapabilities m_capabilities;
    std::atomic<bool>  m_isAcquiring;
    QThread*           m_acquisitionThread;

    mutable QMutex   m_frameMutex;
    MeasurementFrame m_latestFrame;

    CircularBuffer<MeasurementFrame, 100> m_frameBuffer;

    PerformanceMetrics m_metrics;           // Métriques live (frame courante)
    qint64             m_lastFrameTimestamp;

private:
    // =========================================================================
    // Accumulateurs de session (privés, manipulés via les méthodes protégées)
    // =========================================================================

    qint64  m_sessionStartTimestamp = 0;

    // Fréquence
    double  m_freqSum   = 0.0;
    double  m_freqMin   = std::numeric_limits<double>::max();
    double  m_freqMax   = 0.0;
    quint64 m_freqCount = 0;

    // Latence
    bool    m_latencyKnown = false;
    double  m_latencySum   = 0.0;
    double  m_latencyMin   = std::numeric_limits<double>::max();
    double  m_latencyMax   = 0.0;
    quint64 m_latencyCount = 0;
};

#endif // IMEASUREMENTSYSTEM_H
