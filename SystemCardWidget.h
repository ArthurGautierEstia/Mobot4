#pragma once
#ifndef SYSTEMCARDWIDGET_H
#define SYSTEMCARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

#include "IMeasurementSystem.h"
#include "MeasurementFrame.h"

class SystemCardWidget : public QWidget {
    Q_OBJECT

public:
    explicit SystemCardWidget(IMeasurementSystem* system,
        const QString& displayName,
        QWidget* parent = nullptr);
    ~SystemCardWidget() override = default;

    IMeasurementSystem* system()     const { return m_system; }
    QString             systemName() const { return m_displayName; }
    bool                isConnected()const { return m_connected; }

    // Appelé par MainWindow::onUiRefreshTick() à 20 Hz
    virtual void refreshDisplay();
    void         setAcquiring(bool acquiring);

signals:
    void connected();
    void disconnected();
    void removeRequested();

protected:
    virtual void buildDataSection(QVBoxLayout* layout);
    void         updateConnectionUI(bool connected);

    IMeasurementSystem* m_system = nullptr;
    QString             m_displayName;
    bool                m_connected = false;
    bool                m_acquiring = false;

    // Données live (mis à jour à 20 Hz depuis m_system->getLatestFrame())
    MeasurementFrame    m_lastFrame;

private:
    void buildCard();
    void onConnectClicked();

    // ── Header ───────────────────────────────────────────────────────────────
    QLabel* m_statusDot = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_freqLabel = nullptr;
    QLabel* m_latLabel = nullptr;
    QPushButton* m_connectBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;

    // ── Data grid X/Y/Z/Rx/Ry/Rz ─────────────────────────────────────────────
    QLabel* m_vals[6] = {};  // X Y Z Rx Ry Rz
};

#endif // SYSTEMCARDWIDGET_H
