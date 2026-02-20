#include "SystemFactory.h"
#include <QDebug>

// Inclusions conditionnelles selon les systèmes implémentés
// Pour l'instant, on prépare la structure, les includes seront ajoutés au fur et à mesure
// #include "measurement_systems/optitrack/OptiTrackSystem.h"
// #include "measurement_systems/vicon/ViconSystem.h"
// #include "measurement_systems/qualisys/QualisysSystem.h"
// #include "measurement_systems/rsi_kuka/RSISystem.h"

// Map statique pour la correspondance nom <-> type
const QMap<QString, SystemFactory::SystemType> SystemFactory::s_systemTypeMap = {
    {"OptiTrack", SystemType::OptiTrack},
    {"Vicon", SystemType::Vicon},
    {"Qualisys", SystemType::Qualisys},
    {"RSI_KUKA", SystemType::RSI_KUKA},
    {"RSI", SystemType::RSI_KUKA}, // Alias
    {"KUKA", SystemType::RSI_KUKA}  // Alias
};

IMeasurementSystem* SystemFactory::createSystem(SystemType type, QObject* parent)
{
    switch (type) {
    case SystemType::OptiTrack:
        // return new OptiTrackSystem(parent);
        qWarning() << "OptiTrack system not yet implemented";
        return nullptr;

    case SystemType::Vicon:
        // return new ViconSystem(parent);
        qWarning() << "Vicon system not yet implemented";
        return nullptr;

    case SystemType::Qualisys:
        // return new QualisysSystem(parent);
        qWarning() << "Qualisys system not yet implemented";
        return nullptr;

    case SystemType::RSI_KUKA:
        // return new RSISystem(parent);
        qWarning() << "RSI KUKA system not yet implemented";
        return nullptr;

    case SystemType::Unknown:
    default:
        qWarning() << "Unknown system type";
        return nullptr;
    }
}

IMeasurementSystem* SystemFactory::createSystem(const QString& systemName, QObject* parent)
{
    SystemType type = systemTypeFromString(systemName);
    return createSystem(type, parent);
}

SystemFactory::SystemType SystemFactory::systemTypeFromString(const QString& systemName)
{
    return s_systemTypeMap.value(systemName, SystemType::Unknown);
}

QString SystemFactory::systemTypeToString(SystemType type)
{
    switch (type) {
    case SystemType::OptiTrack:
        return "OptiTrack";
    case SystemType::Vicon:
        return "Vicon";
    case SystemType::Qualisys:
        return "Qualisys";
    case SystemType::RSI_KUKA:
        return "RSI_KUKA";
    case SystemType::Unknown:
    default:
        return "Unknown";
    }
}

QStringList SystemFactory::availableSystems()
{
    QStringList systems;

    // Pour chaque système, vérifier s'il est disponible
    if (isSystemAvailable(SystemType::OptiTrack)) {
        systems << "OptiTrack";
    }
    if (isSystemAvailable(SystemType::Vicon)) {
        systems << "Vicon";
    }
    if (isSystemAvailable(SystemType::Qualisys)) {
        systems << "Qualisys";
    }
    if (isSystemAvailable(SystemType::RSI_KUKA)) {
        systems << "RSI_KUKA";
    }

    return systems;
}

bool SystemFactory::isSystemAvailable(SystemType type)
{
    switch (type) {
    case SystemType::OptiTrack:
        // Vérifier si le SDK OptiTrack est présent
#ifdef OPTITRACK_AVAILABLE
        return true;
#else
        return false;
#endif

    case SystemType::Vicon:
        // Vérifier si le SDK Vicon est présent
#ifdef VICON_AVAILABLE
        return true;
#else
        return false;
#endif

    case SystemType::Qualisys:
        // Vérifier si le SDK Qualisys est présent
#ifdef QUALISYS_AVAILABLE
        return true;
#else
        return false;
#endif

    case SystemType::RSI_KUKA:
        // RSI utilise juste UDP/XML, toujours disponible
        return true;

    case SystemType::Unknown:
    default:
        return false;
    }
}

bool SystemFactory::isSystemAvailable(const QString& systemName)
{
    SystemType type = systemTypeFromString(systemName);
    return isSystemAvailable(type);
}
