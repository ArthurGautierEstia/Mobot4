#pragma once
#ifndef ACQUISITIONSUMMARY_H
#define ACQUISITIONSUMMARY_H

#include <QString>
#include <QtGlobal>

/**
 * @brief Résumé statistique d'une session d'acquisition
 *
 * Émis une seule fois par stopAcquisition() via le signal acquisitionCompleted().
 * Destiné à :
 *   - l'affichage récapitulatif en fin de session dans la UI
 *   - l'en-tête du fichier de mesures exporté
 *
 * Les statistiques (min/max/mean) sont calculées en continu par la classe de
 * base IMeasurementSystem via updateRunningStats(), et assemblées ici par
 * buildSummary() à l'arrêt.
 */
struct AcquisitionSummary {

    QString systemName = "";     // Nom du système ("OptiTrack", "Vicon", ...)
    QString objectName = "";     // Nom de l'objet/rigid body tracké

    // -------------------------------------------------------------------------
    // Timing de session
    // -------------------------------------------------------------------------
    qint64 startTimestamp  = 0;      // Horodatage de début (µs epoch)
    qint64 endTimestamp    = 0;      // Horodatage de fin   (µs epoch)
    double durationSeconds = 0.0;    // Durée totale de la session (s)

    // -------------------------------------------------------------------------
    // Frames
    // -------------------------------------------------------------------------
    quint64 totalFrames     = 0;     // Frames reçues et traitées
    quint64 droppedFrames   = 0;     // Frames manquantes détectées
    double  dropRatePercent = 0.0;   // droppedFrames / (total + dropped) × 100

    // -------------------------------------------------------------------------
    // Fréquence (Hz) — toujours disponible
    // -------------------------------------------------------------------------
    double freqMeanHz = 0.0;
    double freqMinHz  = 0.0;
    double freqMaxHz  = 0.0;

    // -------------------------------------------------------------------------
    // Latence (ms) — conditionnelle selon le système
    //   latencyAvailable = false : le système ne peut pas mesurer la latence
    //                              (ex: Qualisys RT protocol) → afficher "N/A"
    // -------------------------------------------------------------------------
    bool   latencyAvailable = false;
    double latencyMeanMs    = 0.0;
    double latencyMinMs     = 0.0;
    double latencyMaxMs     = 0.0;

    AcquisitionSummary() = default;
};

#endif // ACQUISITIONSUMMARY_H
