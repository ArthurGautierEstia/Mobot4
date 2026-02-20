#pragma once
#ifndef KUKARSICONFIG_H
#define KUKARSICONFIG_H

#include <QString>
#include <QList>
#include "RsiTag.h"

/**
 * @brief Configuration du système KukaRsi.
 *
 * hostAddress / hostPort : IP et port sur lesquels CE PC écoute.
 *   → Configurés côté robot dans le fichier .rsi (balise <IP> / <PORT>).
 *   → Valeurs par défaut issues de l'ancienne RobotConfig.
 *
 * L'adresse et le port du robot sont appris dynamiquement
 * au premier paquet UDP reçu — pas besoin de les configurer ici.
 *
 * selectedTags : tags optionnels à parser et exposer dans MeasurementFrame::extras.
 *   RIst est toujours parsé — ne pas l'inclure ici.
 */
class KukaRsiConfig {
public:
    // ── Réseau ────────────────────────────────────────────────────────────────
    QString hostAddress = QStringLiteral("172.31.2.100");
    int     hostPort = 49152;

    // ── Identité ──────────────────────────────────────────────────────────────
    QString robotName = QStringLiteral("KUKA RSI");

    // ── Tags optionnels à extraire ────────────────────────────────────────────
    // Sélection par défaut : angles articulaires + métriques RT
    QList<RsiTag> selectedTags = {
        RsiTag::AIPos,
        RsiTag::LogDtSend,
        RsiTag::LogDurationJob,
        RsiTag::LogConnectionStatus
    };
};

#endif // KUKARSICONFIG_H
#pragma once
