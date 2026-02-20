#pragma once
#ifndef ACQUISITIONCONFIGPANEL_H
#define ACQUISITIONCONFIGPANEL_H

#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

/**
 * @brief Panneau de configuration de l'acquisition (panneau gauche).
 *
 * Expose les paramètres via des getters — l'AcquisitionManager
 * viendra les lire au moment du Start.
 */
class AcquisitionConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit AcquisitionConfigPanel(QWidget* parent = nullptr);

    // ── Getters ───────────────────────────────────────────────────────────────
    int     targetFrequency()   const;
    bool    modeIndividual()    const;
    bool    modeSynchronized()  const;
    bool    modeBoth()          const;
    bool    interpolationRepeat() const;   // false = linéaire
    bool    componentEnabled(int idx) const; // 0=X 1=Y 2=Z 3=Rx 4=Ry 5=Rz
    QString outputFolder()      const;
    QString angleConvention()   const;

    // ── Notification ─────────────────────────────────────────────────────────
    void setWarning(const QString& msg); // appelé par MainWindow si freq incompatible

signals:
    void configChanged();

private:
    void buildUi();

    QSpinBox* m_freqSpin = nullptr;
    QLabel* m_warningLabel = nullptr;

    QRadioButton* m_modeIndiv = nullptr;
    QRadioButton* m_modeSync = nullptr;
    QRadioButton* m_modeBoth = nullptr;

    QRadioButton* m_interpRepeat = nullptr;
    QRadioButton* m_interpLinear = nullptr;

    QCheckBox* m_components[6] = {};

    QComboBox* m_angleConvention = nullptr;
    QLineEdit* m_outputFolder = nullptr;
    QPushButton* m_browseBtn = nullptr;
};

#endif // ACQUISITIONCONFIGPANEL_H
