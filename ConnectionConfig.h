#pragma once
#ifndef CONNECTIONCONFIG_H
#define CONNECTIONCONFIG_H

#include <QString>
#include <QVariantMap>

/**
 * @brief Configuration de connexion pour un système de mesure
 */
struct ConnectionConfig {
    QString systemType;        // Type de système ("OptiTrack", "Vicon", "Qualisys", "RSI")

    // Réseau
    QString serverAddress = "127.0.0.1";
    int port = 0;
    QString multicastAddress = "";

    // Acquisition
    double requestedFrequency = 240.0;  // Fréquence souhaitée en Hz
    QString objectName = "";            // Nom du rigid body à tracker
    int objectId = -1;                  // ID de l'objet (si applicable)

    // Paramètres optionnels spécifiques au système
    QVariantMap customParams;

    // Constructeur par défaut
    ConnectionConfig() = default;

    // Constructeur avec valeurs de base
    ConnectionConfig(const QString& sysType, const QString& addr = "127.0.0.1", int p = 0)
        : systemType(sysType), serverAddress(addr), port(p) {
    }
};

#endif // CONNECTIONCONFIG_H
