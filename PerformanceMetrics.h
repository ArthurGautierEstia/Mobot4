#pragma once
#ifndef PERFORMANCEMETRICS_H
#define PERFORMANCEMETRICS_H

#include <QString>
#include <QtGlobal>

/**
 * @brief Métriques de performance instantanées d'un système de mesure
 *
 * Émis à chaque frame via le signal performanceUpdate().
 * Contient uniquement des valeurs instantanées — pas de statistiques.
 * Les statistiques de session complètes sont dans AcquisitionSummary,
 * émis une seule fois en fin d'acquisition.
 */
struct PerformanceMetrics {
    QString systemName    = "";    // Nom du système source (routing UI multi-systèmes)
    double  frequencyHz   = 0.0;   // Fréquence instantanée mesurée (Hz)
    double  latencyMs     = 0.0;   // Latence instantanée (ms) — 0.0 si indisponible
    quint64 frameCount    = 0;     // Frames reçues depuis startAcquisition()
    quint64 droppedFrames = 0;     // Frames manquantes détectées

    PerformanceMetrics() = default;
};

#endif // PERFORMANCEMETRICS_H
