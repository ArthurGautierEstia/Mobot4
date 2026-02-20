#include "IMeasurementSystem.h"
#include <QDateTime>
#include <limits>

IMeasurementSystem::IMeasurementSystem(QObject* parent)
    : QObject(parent)
    , m_isAcquiring(false)
    , m_acquisitionThread(nullptr)
    , m_lastFrameTimestamp(0)
{
}

IMeasurementSystem::~IMeasurementSystem()
{
}

// =============================================================================
// Gestion des statistiques de session
// =============================================================================

void IMeasurementSystem::resetSessionStats()
{
    // Horodatage de début de session en µs
    m_sessionStartTimestamp = QDateTime::currentMSecsSinceEpoch() * 1000LL;

    // Réinitialisation des accumulateurs fréquence
    m_freqSum   = 0.0;
    m_freqMin   = std::numeric_limits<double>::max();
    m_freqMax   = 0.0;
    m_freqCount = 0;

    // Réinitialisation des accumulateurs latence
    m_latencyKnown = false;
    m_latencySum   = 0.0;
    m_latencyMin   = std::numeric_limits<double>::max();
    m_latencyMax   = 0.0;
    m_latencyCount = 0;

    // Réinitialisation des métriques live
    m_metrics             = PerformanceMetrics();
    m_metrics.systemName  = getSystemName();
}

void IMeasurementSystem::updateRunningStats(double latencyMs, double freqHz, bool latencyKnown)
{
    // --- Métriques live (signal performanceUpdate) ---
    m_metrics.frequencyHz = freqHz;
    m_metrics.latencyMs   = latencyKnown ? latencyMs : 0.0;
    m_metrics.frameCount++;
    // m_metrics.droppedFrames est mis à jour directement par chaque système
    // (détection via delta frame number, spécifique à chaque protocole)

    // --- Accumulation fréquence ---
    if (freqHz > 0.0) {
        m_freqSum += freqHz;
        if (freqHz < m_freqMin) m_freqMin = freqHz;
        if (freqHz > m_freqMax) m_freqMax = freqHz;
        ++m_freqCount;
    }

    // --- Accumulation latence ---
    if (latencyKnown && latencyMs > 0.0) {
        m_latencyKnown  = true;
        m_latencySum   += latencyMs;
        if (latencyMs < m_latencyMin) m_latencyMin = latencyMs;
        if (latencyMs > m_latencyMax) m_latencyMax = latencyMs;
        ++m_latencyCount;
    }
}

AcquisitionSummary IMeasurementSystem::buildSummary(const QString& objectName) const
{
    AcquisitionSummary summary;

    summary.systemName = getSystemName();
    summary.objectName = objectName;

    // Timing
    summary.startTimestamp  = m_sessionStartTimestamp;
    summary.endTimestamp    = QDateTime::currentMSecsSinceEpoch() * 1000LL;
    summary.durationSeconds = static_cast<double>(summary.endTimestamp - summary.startTimestamp)
                              / 1'000'000.0;

    // Frames
    summary.totalFrames   = m_metrics.frameCount;
    summary.droppedFrames = m_metrics.droppedFrames;
    const quint64 totalPossible = summary.totalFrames + summary.droppedFrames;
    summary.dropRatePercent = totalPossible > 0
        ? (static_cast<double>(summary.droppedFrames) / static_cast<double>(totalPossible)) * 100.0
        : 0.0;

    // Fréquence
    summary.freqMeanHz = m_freqCount > 0
        ? m_freqSum / static_cast<double>(m_freqCount)
        : 0.0;
    summary.freqMinHz  = m_freqCount > 0 ? m_freqMin : 0.0;
    summary.freqMaxHz  = m_freqMax;

    // Latence
    summary.latencyAvailable = m_latencyKnown;
    if (m_latencyKnown && m_latencyCount > 0) {
        summary.latencyMeanMs = m_latencySum / static_cast<double>(m_latencyCount);
        summary.latencyMinMs  = m_latencyMin;
        summary.latencyMaxMs  = m_latencyMax;
    }

    return summary;
}
