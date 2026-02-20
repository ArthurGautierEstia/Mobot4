#pragma once
#ifndef PERFORMANCEMETRICS_H
#define PERFORMANCEMETRICS_H

#include <QtGlobal>

/**
 * @brief Métriques de performance d'un système de mesure
 */
struct PerformanceMetrics {
    double averageLatency = 0.0;      // Latence moyenne en ms
    double maxLatency = 0.0;          // Latence max en ms
    double minLatency = 0.0;          // Latence min en ms
    double jitter = 0.0;              // Jitter (écart-type) en ms
    double actualFrequency = 0.0;     // Fréquence réelle mesurée en Hz
    quint64 totalFrames = 0;          // Nombre total de frames reçues
    quint64 droppedFrames = 0;        // Frames perdues
    quint64 bufferOverruns = 0;       // Dépassements de buffer

    // Constructeur par défaut
    PerformanceMetrics() = default;
};

#endif // PERFORMANCEMETRICS_H
