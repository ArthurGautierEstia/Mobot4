#pragma once
#ifndef COORDINATECONVERTER_H
#define COORDINATECONVERTER_H

#include "MeasurementFrame.h"
#include "AcquisitionConfig.h"
#include <QString>
#include <QStringList>

/**
 * @brief Utilitaire pour convertir les frames selon les conventions d'angles
 */
class CoordinateConverter {
public:
    /**
     * @brief Applique une convention d'angles à une frame
     * Réorganise rx, ry, rz selon l'ordre de la convention
     */
    static MeasurementFrame applyConvention(
        const MeasurementFrame& frame,
        const AngleConvention& convention);

    /**
     * @brief Génère l'en-tête CSV pour un système avec convention
     * @param systemName Nom du système (ex: "OptiTrack")
     * @param convention Convention d'angles à utiliser
     * @param config Configuration d'acquisition (composantes actives)
     * @return En-tête CSV (ex: "OptiTrack_X_mm,OptiTrack_Y_mm,...")
     */
    static QString getCSVHeader(
        const QString& systemName,
        const AngleConvention& convention,
        const AcquisitionConfig& config);

    /**
     * @brief Formate une frame en ligne CSV selon la convention
     */
    static QString frameToCSVLine(
        const MeasurementFrame& frame,
        const AngleConvention& convention,
        const AcquisitionConfig& config);

    /**
     * @brief Récupère les labels dynamiques selon la convention
     * @return QStringList contenant [labelA, labelB, labelC]
     */
    static QStringList getAngleLabels(const AngleConvention& convention);

private:
    CoordinateConverter() = default; // Classe utilitaire, pas d'instanciation
};

#endif // COORDINATECONVERTER_H
