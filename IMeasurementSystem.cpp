#include "IMeasurementSystem.h"

IMeasurementSystem::IMeasurementSystem(QObject* parent)
    : QObject(parent)
    , m_isAcquiring(false)
    , m_acquisitionThread(nullptr)
    , m_lastFrameTimestamp(0)
{
}

IMeasurementSystem::~IMeasurementSystem()
{
    
    // Nettoyer le thread si existant
    if (m_acquisitionThread) {
        m_acquisitionThread->quit();
        m_acquisitionThread->wait();
        delete m_acquisitionThread;
        m_acquisitionThread = nullptr;
    }
}


