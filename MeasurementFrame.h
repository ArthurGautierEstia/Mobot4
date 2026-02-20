#pragma once
#ifndef MEASUREMENTFRAME_H
#define MEASUREMENTFRAME_H

#include <QString>
#include <QtGlobal>
#include <QMap>
#include <QVector>
/**
 * @brief Structure représentant une frame de mesure d'un système
 *
 * Contient la position (X,Y,Z) et l'orientation (Rx,Ry,Rz) à un instant donné.
 * Les angles sont stockés en rx,ry,rz (explicite) et seront mappés selon
 * la convention robot choisie lors de l'export.
 */
struct MeasurementFrame {
    qint64 timestamp;           // Timestamp en microsecondes depuis epoch
    QString systemName;         // Nom du système ("OptiTrack", "Vicon", etc.)
    QString objectName;         // Nom de l'objet/rigid body tracké

    // Position (mm)
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    // Orientation (degrés) - NOMS EXPLICITES
    double rx = 0.0;  // Rotation autour de l'axe X
    double ry = 0.0;  // Rotation autour de l'axe Y
    double rz = 0.0;  // Rotation autour de l'axe Z

    // Quaternion (optionnel, pour interpolation avancée)
    struct Quaternion {
        double w = 1.0;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        bool valid = false;
    } quaternion;

    // Métadonnées
    bool isValid = true;        // Frame valide ou perdue/corrompue
    double quality = 1.0;       // Qualité du tracking (0.0 à 1.0)
    int frameNumber = 0;        // Numéro de frame du système source

    // Tags optionnels spécifiques au système source
    // Clé   : nom du tag (ex: "AIPos", "Log_DtSend", "Krl")
    // Valeur : liste de doubles (1 valeur pour scalaire, 6 pour vecteur articulaire)
    // Vide par défaut → zéro overhead pour OptiTrack/Vicon/Qualisys
    QMap<QString, QVector<double>> extras;

    // Constructeur par défaut
    MeasurementFrame() = default;

    // Constructeur avec valeurs de base
    MeasurementFrame(qint64 ts, const QString& sysName, const QString& objName)
        : timestamp(ts), systemName(sysName), objectName(objName) {
    }
};

#endif // MEASUREMENTFRAME_H
