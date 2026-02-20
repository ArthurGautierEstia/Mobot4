#pragma once
#ifndef QUALISYSCONFIG_H
#define QUALISYSCONFIG_H

#include <QString>

struct QualisysConfig {
    // Port
    // Base port 22222 = legacy v1.0 uniquement, NE PAS UTILISER
    // Base port+1 = 22223 = little-endian v1.1+  ← celui-ci
    // Base port+2 = 22224 = big-endian v1.1+
    unsigned short basePort = 22223;

    // Version du protocole QTM RT
    int majorVersion = 1;
    int minorVersion = 19;   // 1.19 minimum (skeleton, timecode, etc.)
    bool bigEndian = false; // Little-endian par défaut

    // Streaming UDP (moins de latence, risque de perte de frames)
    bool           useUDP = false;
    unsigned short udpPort = 0;  // 0 = port auto-assigné

    QualisysConfig() = default;
};

#endif // QUALISYSCONFIG_H

