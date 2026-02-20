#include <QCoreApplication>
#include <QDebug>
#include "SystemFactory.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========================================";
    qDebug() << "MoBot4 - SDK Detection Test";
    qDebug() << "========================================";
    qDebug() << "";

    // Test 1 : Vérifier les SDKs disponibles
    qDebug() << "Available SDKs:";

#ifdef OPTITRACK_AVAILABLE
    qDebug() << "  ✓ OptiTrack NatNet SDK";
#else
    qDebug() << "  ✗ OptiTrack NatNet SDK (not available)";
#endif

#ifdef VICON_AVAILABLE
    qDebug() << "  ✓ Vicon DataStream SDK";
#else
    qDebug() << "  ✗ Vicon DataStream SDK (not available)";
#endif

#ifdef QUALISYS_AVAILABLE
    qDebug() << "  ✓ Qualisys RTClient SDK";
#else
    qDebug() << "  ✗ Qualisys RTClient SDK (not available)";
#endif

    qDebug() << "";

    // Test 2 : Lister les systèmes disponibles via la Factory
    qDebug() << "Systems available through SystemFactory:";
    QStringList systems = SystemFactory::availableSystems();

    if (systems.isEmpty()) {
        qDebug() << "  (none - no SDKs found)";
    }
    else {
        for (const QString& system : systems) {
            qDebug() << "  -" << system;
        }
    }

    qDebug() << "";

    // Test 3 : Tester la création d'un système OptiTrack si disponible
#ifdef OPTITRACK_AVAILABLE
    qDebug() << "Testing OptiTrack system creation...";

    IMeasurementSystem* optitrack = SystemFactory::createSystem("OptiTrack");

    if (optitrack) {
        qDebug() << "  ✓ OptiTrack system created successfully";
        qDebug() << "    - System name:" << optitrack->getSystemName();
        qDebug() << "    - Version:" << optitrack->getSystemVersion();
        qDebug() << "    - Threading model:"
            << (optitrack->getThreadingModel() == IMeasurementSystem::ThreadingModel::SdkManaged
                ? "SDK Managed" : "Application Managed");

        SystemCapabilities caps = optitrack->getCapabilities();
        qDebug() << "    - Max frequency:" << caps.maxNativeFrequency << "Hz";
        qDebug() << "    - Supports quaternions:" << (caps.supportsQuaternions ? "Yes" : "No");

        // Test d'initialisation
        qDebug() << "";
        qDebug() << "  Testing initialization...";
        if (optitrack->initialize()) {
            qDebug() << "    ✓ Initialization successful";
        }
        else {
            qDebug() << "    ✗ Initialization failed";
        }

        delete optitrack;
    }
    else {
        qDebug() << "  ✗ Failed to create OptiTrack system";
    }
#endif

    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << "Test completed!";
    qDebug() << "========================================";

    return 0;
}
