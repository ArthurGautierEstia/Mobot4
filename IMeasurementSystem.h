#pragma once
#ifndef IMEASUREMENTSYSTEM_H
#define IMEASUREMENTSYSTEM_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <atomic>
#include "MeasurementFrame.h"
#include "SystemCapabilities.h"
#include "ConnectionConfig.h"
#include "PerformanceMetrics.h"
#include "CircularBuffer.h"

/**
 * @brief Interface abstraite pour tous les systèmes de mesure
 *
 * Chaque système (OptiTrack, Vicon, Qualisys, RSI) doit hériter de cette classe
 * et implémenter toutes les méthodes virtuelles pures.
 */
class IMeasurementSystem : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Modèle de threading du système
     */
    enum class ThreadingModel {
        SdkManaged,           // Le SDK gère le thread (ex: OptiTrack callback)
        ApplicationManaged    // L'application crée un thread (ex: Vicon, Qualisys, RSI)
    };

    virtual ~IMeasurementSystem();

    // ========== Cycle de vie ==========

    /**
     * @brief Initialise le système (charge SDK, etc.)
     * @return true si succès
     */
    virtual bool initialize() = 0;

    /**
     * @brief Connecte au système avec la configuration donnée
     * @param config Configuration de connexion
     * @return true si succès
     */
    virtual bool connect(const ConnectionConfig& config) = 0;

    /**
     * @brief Déconnecte du système
     * @return true si succès
     */
    virtual bool disconnect() = 0;

    /**
     * @brief Vérifie si le système est connecté
     */
    virtual bool isConnected() const = 0;

    // ========== Acquisition ==========

    /**
     * @brief Démarre l'acquisition de données
     * @return true si succès
     */
    virtual bool startAcquisition() = 0;

    /**
     * @brief Arrête l'acquisition de données
     * @return true si succès
     */
    virtual bool stopAcquisition() = 0;

    /**
     * @brief Vérifie si l'acquisition est en cours
     */
    virtual bool isAcquiring() const = 0;

    // ========== Données ==========

    /**
     * @brief Récupère la dernière frame mesurée (thread-safe)
     */
    virtual MeasurementFrame getLatestFrame() const = 0;

    /**
     * @brief Fréquence native actuelle du système en Hz
     */
    virtual double getNativeFrequency() const = 0;

    /**
     * @brief Retourne la meilleure estimation de latence disponible (ms)
     * Chaque système implémente la méthode la plus précise possible.
     */
    virtual double getLatency() const = 0;


    /**
     * @brief Liste des objets/rigid bodies disponibles
     */
    virtual QStringList getAvailableObjects() const = 0;

    // ========== Capacités ==========

    /**
     * @brief Récupère les capacités du système
     */
    virtual SystemCapabilities getCapabilities() const = 0;

    /**
     * @brief Nom du système (ex: "OptiTrack", "Vicon")
     */
    virtual QString getSystemName() const = 0;

    /**
     * @brief Version du SDK
     */
    virtual QString getSystemVersion() const = 0;

    /**
     * @brief Modèle de threading utilisé par ce système
     */
    virtual ThreadingModel getThreadingModel() const = 0;

signals:
    /**
     * @brief Émis lors de la connexion réussie
     */
    void connected();

    /**
     * @brief Émis lors de la déconnexion
     */
    void disconnected();

    /**
     * @brief Émis à chaque nouvelle frame disponible
     */
    void newFrameAvailable(const MeasurementFrame& frame);

    /**
     * @brief Émis en cas d'erreur
     */
    void errorOccurred(const QString& error);

    /**
     * @brief Émis pour les messages de log
     */
    void logMessage(const QString& message);

    /**
     * @brief Émis périodiquement avec les métriques de performance
     */
    void performanceUpdate(const PerformanceMetrics& metrics);

protected:
    // Constructeur protégé (classe abstraite)
    IMeasurementSystem(QObject* parent = nullptr);

    

    // ========== Membres protégés ==========

    SystemCapabilities m_capabilities;      // Capacités du système
    std::atomic<bool> m_isAcquiring;       // Flag d'acquisition
    QThread* m_acquisitionThread;          // Thread d'acquisition (si ApplicationManaged)

    mutable QMutex m_frameMutex;           // Mutex pour accès thread-safe à m_latestFrame
    MeasurementFrame m_latestFrame;        // Dernière frame reçue

    CircularBuffer<MeasurementFrame, 100> m_frameBuffer;  // Buffer circulaire

    PerformanceMetrics m_metrics;          // Métriques de performance
    qint64 m_lastFrameTimestamp;          // Timestamp de la dernière frame (pour calculs)
};

#endif // IMEASUREMENTSYSTEM_H

