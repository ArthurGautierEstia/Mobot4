#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>
#include <QVector>

#include "IMeasurementSystem.h"
#include "SystemCardWidget.h"

class AcquisitionConfigPanel;
class AcquisitionControlPanel;
class RealTimeTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onAddSystemClicked();
    void onSystemConnected(SystemCardWidget* card);
    void onSystemDisconnected(SystemCardWidget* card);
    void onStartAcquisition();
    void onStopAcquisition();
    void onUiRefreshTick();
    void onLogPosition();

private:
    void buildLayout();
    void buildLeftPanel();
    void buildCentralArea();
    void updateStartButtonState();
    void addSystemCard(SystemCardWidget* card);
    

    // ── Layout ───────────────────────────────────────────────────────────────
    QWidget* m_leftPanel = nullptr;
    QScrollArea* m_cardsScrollArea = nullptr;
    QWidget* m_cardsContainer = nullptr;
    QVBoxLayout* m_cardsLayout = nullptr;

    AcquisitionConfigPanel* m_configPanel = nullptr;
    AcquisitionControlPanel* m_controlPanel = nullptr;
    RealTimeTableWidget* m_realTimeTable = nullptr;

    // ── Systèmes ─────────────────────────────────────────────────────────────
    QVector<SystemCardWidget*>          m_cards;
    QVector<IMeasurementSystem*>        m_systems;

    // ── Timer UI (20 Hz) ─────────────────────────────────────────────────────
    QTimer* m_uiTimer = nullptr;

    bool m_isAcquiring = false;

    QPushButton* m_logPosBtn = nullptr;
};

#endif // MAINWINDOW_H
