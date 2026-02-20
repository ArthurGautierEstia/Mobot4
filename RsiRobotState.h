#pragma once
#ifndef RSIROBOTSTATE_H
#define RSIROBOTSTATE_H

#include <cstdint>

/**
 * @brief État complet retourné par le robot à chaque cycle RSI.
 *
 * Émis via KukaRsiSystem::robotStateUpdated(RsiRobotState).
 * Fusion du bloc <Rob> (données process) et du bloc <Log> (métriques RT).
 */
struct RsiRobotState {
    // ── Bloc <Rob> ────────────────────────────────────────────────────────────
    uint64_t ipoc = 0;
    int      krl = 0;
    int      mode = 0;
    int      blocSteps = 0;
    int      blocStart = 0;
    int      blocWaiting = 0;
    int      blocEnd = 0;
    int      blocContinue = 0;
    int      blocCancel = 0;
    int      blocId = 0;
    uint32_t digin = 0;
    uint32_t digout = 0;

    // ── Bloc <Log> ────────────────────────────────────────────────────────────
    double   dtSendMs = 0.0; // Période réelle entre deux trames (ms) → fréquence
    double   durationJobMs = 0.0; // Durée traitement robot (ms) → latence
    double   timeToWaitUs = 0.0; // Temps avant timeout RSI (µs) → santé RT
    int      connectionStatus = 0;   // 4 = connexion saine
    int      status = 0;
    int      reqStatus = 0;
};

#endif // RSIROBOTSTATE_H
#pragma once
