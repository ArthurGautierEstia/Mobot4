#pragma once
#ifndef SYSTEMFACTORY_H
#define SYSTEMFACTORY_H

#include <QString>
#include <QMap>
#include <memory>
#include "IMeasurementSystem.h"

/**
 * @brief Factory pour créer des instances de systèmes de mesure
 *
 * Utilise le pattern Factory pour instancier les systèmes selon leur type.
 * Facilite l'ajout de nouveaux systèmes sans modifier le code client.
 */
class SystemFactory {
public:
    /**
     * @brief Types de systèmes supportés
     */
    enum class SystemType {
        OptiTrack,
        Vicon,
        Qualisys,
        RSI_KUKA,
        Unknown
    };

    /**
     * @brief Crée une instance d'un système de mesure
     * @param type Type de système à créer
     * @param parent Parent Qt optionnel
     * @return Pointeur vers le système créé, ou nullptr si échec
     */
    static IMeasurementSystem* createSystem(SystemType type, QObject* parent = nullptr);

    /**
     * @brief Crée une instance d'un système depuis son nom
     * @param systemName Nom du système ("OptiTrack", "Vicon", etc.)
     * @param parent Parent Qt optionnel
     * @return Pointeur vers le système créé, ou nullptr si échec
     */
    static IMeasurementSystem* createSystem(const QString& systemName, QObject* parent = nullptr);

    /**
     * @brief Convertit un nom de système en SystemType
     * @param systemName Nom du système
     * @return Type correspondant ou Unknown
     */
    static SystemType systemTypeFromString(const QString& systemName);

    /**
     * @brief Convertit un SystemType en nom de système
     * @param type Type de système
     * @return Nom du système
     */
    static QString systemTypeToString(SystemType type);

    /**
     * @brief Liste tous les types de systèmes disponibles
     * @return Liste des noms de systèmes
     */
    static QStringList availableSystems();

    /**
     * @brief Vérifie si un système est disponible (SDK présent)
     * @param type Type de système à vérifier
     * @return true si le système peut être créé
     */
    static bool isSystemAvailable(SystemType type);

    /**
     * @brief Vérifie si un système est disponible depuis son nom
     * @param systemName Nom du système
     * @return true si le système peut être créé
     */
    static bool isSystemAvailable(const QString& systemName);

private:
    SystemFactory() = default; // Classe utilitaire, pas d'instanciation

    // Map pour la correspondance nom <-> type
    static const QMap<QString, SystemType> s_systemTypeMap;
};

#endif // SYSTEMFACTORY_H
