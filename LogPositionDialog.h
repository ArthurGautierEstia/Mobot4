#pragma once
#ifndef LOGPOSITIONDIALOG_H
#define LOGPOSITIONDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QDateTime>

#include "IMeasurementSystem.h"
#include "MeasurementFrame.h"

class LogPositionDialog : public QDialog {
    Q_OBJECT

public:
    explicit LogPositionDialog(const QVector<IMeasurementSystem*>& systems,
        QWidget* parent = nullptr);

private slots:
    void onSampleTick();
    void onCopyToClipboard();

private:
    void buildUi();
    void startSampling();
    void computeAndDisplay();

    struct SystemSamples {
        QString              name;
        QVector<double>      x, y, z, rx, ry, rz;
    };

    QVector<IMeasurementSystem*> m_systems;
    QMap<QString, SystemSamples> m_samples;

    QTableWidget* m_table = nullptr;
    QLabel* m_statusLabel = nullptr;
    QPushButton* m_copyBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;

    QTimer* m_sampleTimer = nullptr;
    int           m_samplesLeft = 0;

    static constexpr int k_sampleDurationMs = 1000;
    static constexpr int k_sampleIntervalMs = 10;   // 100 Hz interne
    static constexpr int k_totalSamples =
        k_sampleDurationMs / k_sampleIntervalMs;     // = 100
};

#endif // LOGPOSITIONDIALOG_H
