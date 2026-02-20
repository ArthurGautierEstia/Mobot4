#include "AcquisitionControlPanel.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDateTime>

AcquisitionControlPanel::AcquisitionControlPanel(QWidget* parent)
    : QWidget(parent)
{
    buildUi();

    m_elapsedTimer = new QTimer(this);
    m_elapsedTimer->setInterval(200);
    connect(m_elapsedTimer, &QTimer::timeout,
        this, &AcquisitionControlPanel::onElapsedTick);
}

void AcquisitionControlPanel::buildUi()
{
    QGroupBox* box = new QGroupBox(QStringLiteral("Contrôles d'acquisition"));
    QVBoxLayout* boxLay = new QVBoxLayout(box);
    boxLay->setSpacing(8);

    m_mainBtn = new QPushButton(QStringLiteral("▶  START ACQUISITION"));
    m_mainBtn->setObjectName(QStringLiteral("StartButton"));
    m_mainBtn->setFixedHeight(44);
    m_mainBtn->setEnabled(false);

    m_stateLabel = new QLabel(QStringLiteral("○  En attente de connexion"));
    m_stateLabel->setObjectName(QStringLiteral("StateLabel"));
    m_stateLabel->setAlignment(Qt::AlignCenter);

    m_elapsedLabel = new QLabel();
    m_elapsedLabel->setAlignment(Qt::AlignCenter);
    m_elapsedLabel->hide();

    m_statsLabel = new QLabel();
    m_statsLabel->setObjectName(QStringLiteral("StatsLabel"));
    m_statsLabel->setWordWrap(true);
    m_statsLabel->hide();

    boxLay->addWidget(m_mainBtn);
    boxLay->addWidget(m_stateLabel);
    boxLay->addWidget(m_elapsedLabel);
    boxLay->addWidget(m_statsLabel);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(box);

    connect(m_mainBtn, &QPushButton::clicked, this, [this]() {
        if (m_acquiring)
            emit stopRequested();
        else
            emit startRequested();
        });
}

void AcquisitionControlPanel::setStartEnabled(bool enabled)
{
    m_mainBtn->setEnabled(enabled || m_acquiring);
    if (!m_acquiring) {
        m_stateLabel->setText(enabled
            ? QStringLiteral("○  Prêt")
            : QStringLiteral("○  En attente de connexion"));
    }
}

void AcquisitionControlPanel::setAcquiring(bool acquiring)
{
    m_acquiring = acquiring;

    if (acquiring) {
        m_mainBtn->setText(QStringLiteral("⏹  STOP & SAVE"));
        m_mainBtn->setObjectName(QStringLiteral("StopButton"));
        m_stateLabel->setText(QStringLiteral("●  Acquisition en cours..."));
        m_elapsedLabel->show();
        m_statsLabel->show();
        m_startMs = QDateTime::currentMSecsSinceEpoch();
        m_elapsedTimer->start();
    }
    else {
        m_mainBtn->setText(QStringLiteral("▶  START ACQUISITION"));
        m_mainBtn->setObjectName(QStringLiteral("StartButton"));
        m_stateLabel->setText(QStringLiteral("○  Prêt"));
        m_elapsedLabel->hide();
        m_statsLabel->hide();
        m_elapsedTimer->stop();
    }
}

void AcquisitionControlPanel::onElapsedTick()
{
    const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_startMs;
    const int h = elapsed / 3600000;
    const int m = (elapsed % 3600000) / 60000;
    const int s = (elapsed % 60000) / 1000;
    const int ms = elapsed % 1000;
    m_elapsedLabel->setText(
        QString::asprintf("Durée : %02d:%02d:%02d.%03d", h, m, s, ms));
}

void AcquisitionControlPanel::updateStats(
    const QVector<IMeasurementSystem*>& systems)
{
    if (!m_acquiring || systems.isEmpty())
        return;

    QStringList lines;
    for (auto* sys : systems) {
        const auto summary = sys->getLatestFrame();
        lines << QString(QStringLiteral("  %1: %2 Hz"))
            .arg(sys->getSystemName())
            .arg(sys->getNativeFrequency(), 0, 'f', 1);
    }
    m_statsLabel->setText(lines.join(QStringLiteral("\n")));
}
