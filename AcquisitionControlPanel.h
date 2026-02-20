#pragma once
#ifndef ACQUISITIONCONTROLPANEL_H
#define ACQUISITIONCONTROLPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QVector>

#include "IMeasurementSystem.h"

class AcquisitionControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit AcquisitionControlPanel(QWidget* parent = nullptr);

    void setStartEnabled(bool enabled);
    void setAcquiring(bool acquiring);
    void updateStats(const QVector<IMeasurementSystem*>& systems);

signals:
    void startRequested();
    void stopRequested();

private:
    void buildUi();
    void onElapsedTick();

    QPushButton* m_mainBtn = nullptr;
    QLabel* m_stateLabel = nullptr;
    QLabel* m_elapsedLabel = nullptr;
    QLabel* m_statsLabel = nullptr;

    QTimer* m_elapsedTimer = nullptr;
    qint64       m_startMs = 0;
    bool         m_acquiring = false;
};

#endif // ACQUISITIONCONTROLPANEL_H
