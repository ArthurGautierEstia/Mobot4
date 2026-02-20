#include "CoordinateConverter.h"
#include <algorithm>

MeasurementFrame CoordinateConverter::applyConvention(
    const MeasurementFrame& frame,
    const AngleConvention& convention)
{
    MeasurementFrame converted = frame;

    // Réorganiser rx, ry, rz selon l'ordre de la convention
    double originalRx = frame.rx;
    double originalRy = frame.ry;
    double originalRz = frame.rz;

    switch (convention.order) {
    case AngleConvention::RxRyRz:
        // Ordre déjà correct (A=Rx, B=Ry, C=Rz)
        converted.rx = originalRx;
        converted.ry = originalRy;
        converted.rz = originalRz;
        break;

    case AngleConvention::RzRyRx:
        // A=Rz, B=Ry, C=Rx (KUKA, ABB, Fanuc)
        converted.rx = originalRz;
        converted.ry = originalRy;
        converted.rz = originalRx;
        break;

    case AngleConvention::RzRxRy:
        // A=Rz, B=Rx, C=Ry
        converted.rx = originalRz;
        converted.ry = originalRx;
        converted.rz = originalRy;
        break;

    case AngleConvention::RyRxRz:
        // A=Ry, B=Rx, C=Rz
        converted.rx = originalRy;
        converted.ry = originalRx;
        converted.rz = originalRz;
        break;

    case AngleConvention::RyRzRx:
        // A=Ry, B=Rz, C=Rx
        converted.rx = originalRy;
        converted.ry = originalRz;
        converted.rz = originalRx;
        break;

    case AngleConvention::RxRzRy:
        // A=Rx, B=Rz, C=Ry
        converted.rx = originalRx;
        converted.ry = originalRz;
        converted.rz = originalRy;
        break;
    }

    return converted;
}

QString CoordinateConverter::getCSVHeader(
    const QString& systemName,
    const AngleConvention& convention,
    const AcquisitionConfig& config)
{
    QStringList headers;

    // Position
    if (config.enableX) headers << QString("%1_X_mm").arg(systemName);
    if (config.enableY) headers << QString("%1_Y_mm").arg(systemName);
    if (config.enableZ) headers << QString("%1_Z_mm").arg(systemName);

    // Angles avec labels de la convention
    if (config.enableRx) {
        QString label = convention.labelA;
        label.replace(" ", "_"); // Remplacer espaces par underscore
        headers << QString("%1_%2_deg").arg(systemName, label);
    }
    if (config.enableRy) {
        QString label = convention.labelB;
        label.replace(" ", "_");
        headers << QString("%1_%2_deg").arg(systemName, label);
    }
    if (config.enableRz) {
        QString label = convention.labelC;
        label.replace(" ", "_");
        headers << QString("%1_%2_deg").arg(systemName, label);
    }

    return headers.join(",");
}

QString CoordinateConverter::frameToCSVLine(
    const MeasurementFrame& frame,
    const AngleConvention& convention,
    const AcquisitionConfig& config)
{
    QStringList values;

    // Appliquer la convention
    MeasurementFrame converted = applyConvention(frame, convention);

    // Position
    if (config.enableX) values << QString::number(converted.x, 'f', 3);
    if (config.enableY) values << QString::number(converted.y, 'f', 3);
    if (config.enableZ) values << QString::number(converted.z, 'f', 3);

    // Angles (déjà réorganisés par applyConvention)
    if (config.enableRx) values << QString::number(converted.rx, 'f', 3);
    if (config.enableRy) values << QString::number(converted.ry, 'f', 3);
    if (config.enableRz) values << QString::number(converted.rz, 'f', 3);

    return values.join(",");
}

QStringList CoordinateConverter::getAngleLabels(const AngleConvention& convention)
{
    return QStringList() << convention.labelA << convention.labelB << convention.labelC;
}
