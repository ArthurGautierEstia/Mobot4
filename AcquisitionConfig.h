#pragma once
#ifndef ACQUISITIONCONFIG_H
#define ACQUISITIONCONFIG_H

#include <QString>
#include <QVector>

/**
 * @brief Convention d'angles pour l'affichage et l'export
 */
struct AngleConvention {
    QString name;          // Nom de la convention ("KUKA", "ABB", etc.)
    QString labelA;        // Label du premier angle
    QString labelB;        // Label du deuxième angle
    QString labelC;        // Label du troisième angle
    QString description;   // Description détaillée

    // Ordre des axes pour mapping
    enum AxisOrder {
        RxRyRz,  // A=Rx, B=Ry, C=Rz
        RzRyRx,  // A=Rz, B=Ry, C=Rx (KUKA)
        RzRxRy,  // A=Rz, B=Rx, C=Ry
        RyRxRz,  // A=Ry, B=Rx, C=Rz
        RyRzRx,  // A=Ry, B=Rz, C=Rx
        RxRzRy   // A=Rx, B=Rz, C=Ry
    } order;
};

// Conventions prédéfinies
namespace AngleConventions {
    static const AngleConvention KUKA = {
        "KUKA",
        "A (Rz)", "B (Ry)", "C (Rx)",
        "KUKA ABC: A=Rotation Z, B=Rotation Y, C=Rotation X",
        AngleConvention::RzRyRx
    };

    static const AngleConvention ABB_EULER = {
        "ABB Euler ZYX",
        "A (Rz)", "B (Ry)", "C (Rx)",
        "ABB Euler ZYX: Rotation Z puis Y puis X",
        AngleConvention::RzRyRx
    };

    static const AngleConvention FANUC_WPR = {
        "Fanuc WPR",
        "W (Rz)", "P (Ry)", "R (Rx)",
        "Fanuc WPR: W=Yaw, P=Pitch, R=Roll",
        AngleConvention::RzRyRx
    };

    static const AngleConvention UNIVERSAL_RPY = {
        "Universal Robots RPY",
        "R (Rx)", "P (Ry)", "Y (Rz)",
        "Universal Robots Roll-Pitch-Yaw",
        AngleConvention::RxRyRz
    };

    static const AngleConvention STAUBLI_RXRYRZ = {
        "Staubli RxRyRz",
        "Rx", "Ry", "Rz",
        "Staubli: Rotations directes Rx, Ry, Rz",
        AngleConvention::RxRyRz
    };

    // Liste de toutes les conventions
    static const QVector<AngleConvention> ALL = {
        KUKA, ABB_EULER, FANUC_WPR, UNIVERSAL_RPY, STAUBLI_RXRYRZ
    };
}

/**
 * @brief Configuration d'une acquisition multi-systèmes
 */
struct AcquisitionConfig {
    // Fréquence et mode
    double targetFrequency = 1000.0;  // Hz pour la synchronisation

    enum class Mode {
        Individual,      // Fichiers CSV séparés par système
        Synchronized,    // Un fichier CSV synchronisé
        Both            // Les deux modes
    } mode = Mode::Synchronized;

    // Composantes actives
    bool enableX = true;
    bool enableY = true;
    bool enableZ = true;
    bool enableRx = true;
    bool enableRy = true;
    bool enableRz = true;

    // Convention d'angles
    AngleConvention angleConvention = AngleConventions::KUKA;

    // Interpolation pour mode synchronisé
    enum class InterpolationMode {
        Repeat,         // Répéter la dernière frame valide
        Linear          // Interpolation linéaire
    } interpolation = InterpolationMode::Repeat;

    // Export
    QString outputDirectory = "./acquisitions";
    bool timestampInFilename = true;

    // Constructeur par défaut
    AcquisitionConfig() = default;
};

#endif // ACQUISITIONCONFIG_H
