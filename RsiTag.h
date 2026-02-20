#pragma once
#ifndef RSITAG_H
#define RSITAG_H

#include <QString>
#include <QList>

/**
 * @brief Tags RSI disponibles en lecture dans la trame reçue du robot KUKA.
 *
 * RIst est toujours parsé (position primaire du MeasurementFrame).
 * Les autres sont optionnels, configurables via KukaRsiConfig::selectedTags.
 * Leurs valeurs atterrissent dans MeasurementFrame::extras.
 */
enum class RsiTag {
    RIst,            // Toujours parsé — ne pas mettre dans selectedTags
    RSol,            // Position cartésienne consigne  (X,Y,Z,A,B,C)
    AIPos,           // Angles articulaires mesurés    (A1–A6)
    ASPos,           // Angles articulaires consigne   (A1–A6)
    MACur,           // Couples moteur                 (A1–A6)
    Delay,           // Délai                          (D)
    Digin,           // Entrées digitales              (scalaire)
    Digout,          // Sorties digitales              (scalaire)
    Krl,             // Registre KRL custom            (scalaire)
    Mode,            // Mode custom                    (scalaire)
    BlocSteps,
    BlocStart,
    BlocWaiting,
    BlocEnd,
    BlocContinue,
    BlocCancel,
    BlocId,
    LogDtSend,           // Période trame (ms)
    LogDurationJob,      // Latence robot (ms)
    LogTimeToWait,       // Temps avant timeout RSI (µs)
    LogConnectionStatus  // État connexion (4 = OK)
};

namespace RsiTagMeta {

    /// Clé utilisée dans MeasurementFrame::extras et pour l'affichage UI
    inline QString key(RsiTag tag)
    {
        switch (tag) {
        case RsiTag::RIst:               return QStringLiteral("RIst");
        case RsiTag::RSol:               return QStringLiteral("RSol");
        case RsiTag::AIPos:              return QStringLiteral("AIPos");
        case RsiTag::ASPos:              return QStringLiteral("ASPos");
        case RsiTag::MACur:              return QStringLiteral("MACur");
        case RsiTag::Delay:              return QStringLiteral("Delay");
        case RsiTag::Digin:              return QStringLiteral("Digin");
        case RsiTag::Digout:             return QStringLiteral("Digout");
        case RsiTag::Krl:                return QStringLiteral("Krl");
        case RsiTag::Mode:               return QStringLiteral("Mode");
        case RsiTag::BlocSteps:          return QStringLiteral("Bloc_Steps");
        case RsiTag::BlocStart:          return QStringLiteral("Bloc_Start");
        case RsiTag::BlocWaiting:        return QStringLiteral("Bloc_Waiting");
        case RsiTag::BlocEnd:            return QStringLiteral("Bloc_End");
        case RsiTag::BlocContinue:       return QStringLiteral("Bloc_Continue");
        case RsiTag::BlocCancel:         return QStringLiteral("Bloc_Cancel");
        case RsiTag::BlocId:             return QStringLiteral("Bloc_ID");
        case RsiTag::LogDtSend:          return QStringLiteral("Log_DtSend");
        case RsiTag::LogDurationJob:     return QStringLiteral("Log_DurationJob");
        case RsiTag::LogTimeToWait:      return QStringLiteral("Log_TimeToWait");
        case RsiTag::LogConnectionStatus:return QStringLiteral("Log_ConnectionStatus");
        default:                         return QStringLiteral("Unknown");
        }
    }

    /// Nombre de valeurs dans la liste extras (6 pour les vecteurs, 1 pour les scalaires)
    inline int arity(RsiTag tag)
    {
        switch (tag) {
        case RsiTag::RIst:
        case RsiTag::RSol:
        case RsiTag::AIPos:
        case RsiTag::ASPos:
        case RsiTag::MACur: return 6;
        default:            return 1;
        }
    }

    /// Liste de tous les tags optionnels (pour peupler une UI de sélection)
    inline QList<RsiTag> allOptional()
    {
        return {
            RsiTag::RSol,
            RsiTag::AIPos,  RsiTag::ASPos,  RsiTag::MACur,
            RsiTag::Delay,
            RsiTag::Digin,  RsiTag::Digout,
            RsiTag::Krl,    RsiTag::Mode,
            RsiTag::BlocSteps,   RsiTag::BlocStart,  RsiTag::BlocWaiting,
            RsiTag::BlocEnd,     RsiTag::BlocContinue, RsiTag::BlocCancel,
            RsiTag::BlocId,
            RsiTag::LogDtSend,   RsiTag::LogDurationJob,
            RsiTag::LogTimeToWait, RsiTag::LogConnectionStatus
        };
    }

} // namespace RsiTagMeta

#endif // RSITAG_H
#pragma once
