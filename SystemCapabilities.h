#pragma once
#ifndef SYSTEMCAPABILITIES_H
#define SYSTEMCAPABILITIES_H

#include <QString>
#include <QStringList>

/**
 * @brief Décrit les capacités d'un système de mesure
 */
struct SystemCapabilities {
    QString systemName;                    // Nom du système ("OptiTrack", "Vicon", etc.)
    QString version;                       // Version du SDK

    // Fréquences supportées
    double maxNativeFrequency = 1000.0;    // Fréquence max en Hz (ex: 240 pour OptiTrack)
    double minNativeFrequency = 30.0;      // Fréquence min en Hz
    bool supportsVariableRate = false;     // Peut changer la fréquence en live

    // Capacités de mesure
    QStringList supportedComponents;       // Liste: "X", "Y", "Z", "Rx", "Ry", "Rz"
    bool supportsQuaternions = false;      // Fournit des quaternions natifs
    bool supportsMultipleObjects = true;   // Peut tracker plusieurs objets simultanément

    // Performance
    double typicalLatency = 5.0;           // Latence typique en ms

    // Constructeur par défaut
    SystemCapabilities() {
        // Par défaut, tous les composants sont supportés
        supportedComponents << "X" << "Y" << "Z" << "Rx" << "Ry" << "Rz";
    }
};

#endif // SYSTEMCAPABILITIES_H
