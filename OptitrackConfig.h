#pragma once
#ifndef OPTITRACKCONFIG_H
#define OPTITRACKCONFIG_H

#include <QString>

/**
 * @brief Configuration spécifique à OptiTrack
 */
struct OptiTrackConfig {
    // Connexion
    QString localAddress = "";              // Adresse locale (auto si vide)
    QString serverAddress = "";             // Adresse serveur (auto-détection si vide)
    QString multicastAddress = "127.0.0.1"; // Adresse multicast par défaut
    int commandPort = 1510;                 // Port de commande
    int dataPort = 1511;                    // Port de données

    // Type de connexion
    enum ConnectionType {
        Multicast = 0,
        Unicast = 1
    } connectionType = Multicast;

    // Acquisition
    int rigidBodyId = -1;                   // ID du rigid body (-1 = premier trouvé)
    QString rigidBodyName = "";             // Nom du rigid body (prioritaire sur ID)

    // Options
    bool enableLogging = false;             // Activer les logs SDK
    QString logFilePath = "";               // Chemin du fichier de log

    OptiTrackConfig() = default;
};

#endif // OPTITRACKCONFIG_H
