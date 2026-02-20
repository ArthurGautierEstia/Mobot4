#pragma once
#ifndef ADDSYSTEMDIALOG_H
#define ADDSYSTEMDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>

class SystemCardWidget;

class AddSystemDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddSystemDialog(QWidget* parent = nullptr);

    // Crée la carte + le système sous-jacent selon la sélection
    SystemCardWidget* createCard(QWidget* cardParent);

private slots:
    void onSystemTypeChanged(int index);

private:
    void buildUi();
    void buildKukaPage();
    void buildOptitrackPage();
    void buildGenericPage(const QString& label);

    QComboBox*     m_typeCombo   = nullptr;
    QStackedWidget* m_pages      = nullptr;

    // ── KUKA RSI ─────────────────────────────────────────────────────────────
    QLineEdit* m_kukaIp          = nullptr;
    QSpinBox*  m_kukaPort        = nullptr;
    QLineEdit* m_kukaName        = nullptr;

    // ── OptiTrack ─────────────────────────────────────────────────────────────
    QLineEdit* m_optiIp          = nullptr;
    QLineEdit* m_optiBody        = nullptr;
};

#endif // ADDSYSTEMDIALOG_H
