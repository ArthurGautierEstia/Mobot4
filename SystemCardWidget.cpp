#include "SystemCardWidget.h"
#include "ConnectionConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGridLayout>

static const char* k_axisNames[] = { "X", "Y", "Z", "Rx", "Ry", "Rz" };

SystemCardWidget::SystemCardWidget(IMeasurementSystem* system,
    const QString& displayName,
    QWidget* parent)
    : QWidget(parent)
    , m_system(system)
    , m_displayName(displayName)
{
    setObjectName(QStringLiteral("SystemCard"));
    buildCard();

    connect(m_system, &IMeasurementSystem::connected,
        this, [this]() {
            m_connected = true;
            updateConnectionUI(true);
            emit connected();
        });
    connect(m_system, &IMeasurementSystem::disconnected,
        this, [this]() {
            m_connected = false;
            updateConnectionUI(false);
            emit disconnected();
        });
}

// =============================================================================
// Construction
// =============================================================================

void SystemCardWidget::buildCard()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QFrame* card = new QFrame();
    card->setObjectName(QStringLiteral("CardFrame"));

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 12, 16, 12);
    cardLayout->setSpacing(8);

    // ── Header ───────────────────────────────────────────────────────────────
    QHBoxLayout* header = new QHBoxLayout();

    m_statusDot = new QLabel(QStringLiteral("○"));
    m_statusDot->setObjectName(QStringLiteral("StatusDotDisconnected"));
    m_statusDot->setFixedWidth(16);

    QLabel* nameLabel = new QLabel(m_displayName);
    nameLabel->setObjectName(QStringLiteral("CardTitle"));

    m_statusLabel = new QLabel(QStringLiteral("Déconnecté"));
    m_statusLabel->setObjectName(QStringLiteral("StatusLabelDisconnected"));

    m_freqLabel = new QLabel(QStringLiteral("—"));
    m_freqLabel->setObjectName(QStringLiteral("MetricLabel"));

    m_latLabel = new QLabel(QStringLiteral("—"));
    m_latLabel->setObjectName(QStringLiteral("MetricLabel"));

    m_connectBtn = new QPushButton(QStringLiteral("🔌 Connecter"));
    m_connectBtn->setObjectName(QStringLiteral("ConnectButton"));
    m_connectBtn->setFixedHeight(30);

    m_removeBtn = new QPushButton(QStringLiteral("✕"));
    m_removeBtn->setObjectName(QStringLiteral("RemoveButton"));
    m_removeBtn->setFixedSize(28, 28);
    m_removeBtn->setToolTip(QStringLiteral("Supprimer ce système"));

    header->addWidget(m_statusDot);
    header->addWidget(nameLabel);
    header->addStretch();
    header->addWidget(m_statusLabel);
    header->addSpacing(12);
    header->addWidget(m_freqLabel);
    header->addSpacing(6);
    header->addWidget(m_latLabel);
    header->addSpacing(12);
    header->addWidget(m_connectBtn);
    header->addSpacing(4);
    header->addWidget(m_removeBtn);
    cardLayout->addLayout(header);

    // ── Séparateur ───────────────────────────────────────────────────────────
    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName(QStringLiteral("CardSeparator"));
    cardLayout->addWidget(sep);

    // ── Données live ─────────────────────────────────────────────────────────
    buildDataSection(cardLayout);

    root->addWidget(card);

    connect(m_connectBtn, &QPushButton::clicked, this, &SystemCardWidget::onConnectClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &SystemCardWidget::removeRequested);
}

void SystemCardWidget::buildDataSection(QVBoxLayout* layout)
{
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(8);

    for (int i = 0; i < 6; ++i) {
        QLabel* axisLbl = new QLabel(QString::fromUtf8(k_axisNames[i]));
        axisLbl->setObjectName(QStringLiteral("AxisLabel"));
        axisLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        axisLbl->setFixedWidth(24);

        m_vals[i] = new QLabel(QStringLiteral("—"));
        m_vals[i]->setObjectName(QStringLiteral("ValueLabel"));
        m_vals[i]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_vals[i]->setFixedWidth(80);

        grid->addWidget(axisLbl, 0, i * 2);
        grid->addWidget(m_vals[i], 0, i * 2 + 1);
    }

    layout->addLayout(grid);
}

// =============================================================================
// Refresh (appelé à 20 Hz)
// =============================================================================

void SystemCardWidget::refreshDisplay()
{
    if (!m_system)
        return;

    m_lastFrame = m_system->getLatestFrame();

    // Position
    if (m_lastFrame.isValid) {
        m_vals[0]->setText(QString::number(m_lastFrame.x, 'f', 2));
        m_vals[1]->setText(QString::number(m_lastFrame.y, 'f', 2));
        m_vals[2]->setText(QString::number(m_lastFrame.z, 'f', 2));
        m_vals[3]->setText(QString::number(m_lastFrame.rx, 'f', 2));
        m_vals[4]->setText(QString::number(m_lastFrame.ry, 'f', 2));
        m_vals[5]->setText(QString::number(m_lastFrame.rz, 'f', 2));
    }

    // Métriques header
    if (m_connected) {
        m_freqLabel->setText(
            QString::number(m_system->getNativeFrequency(), 'f', 1)
            + QStringLiteral(" Hz"));
        const double lat = m_system->getLatency();
        m_latLabel->setText(lat > 0.0
            ? QString::number(lat, 'f', 1) + QStringLiteral(" ms")
            : QStringLiteral("—"));
    }
}

void SystemCardWidget::setAcquiring(bool acquiring)
{
    m_acquiring = acquiring;
    m_connectBtn->setEnabled(!acquiring);
    m_removeBtn->setEnabled(!acquiring);
}

// =============================================================================
// Connexion
// =============================================================================

void SystemCardWidget::onConnectClicked()
{
    if (m_connected) {
        m_system->disconnect();
    }
    else {
        m_system->initialize();
        ConnectionConfig cfg;
        m_system->connect(cfg);
    }
}

void SystemCardWidget::updateConnectionUI(bool connected)
{
    if (connected) {
        m_statusDot->setText(QStringLiteral("●"));
        m_statusDot->setObjectName(QStringLiteral("StatusDotConnected"));
        m_statusLabel->setText(QStringLiteral("Connecté"));
        m_statusLabel->setObjectName(QStringLiteral("StatusLabelConnected"));
        m_connectBtn->setText(QStringLiteral("⏏ Déconnecter"));
    }
    else {
        m_statusDot->setText(QStringLiteral("○"));
        m_statusDot->setObjectName(QStringLiteral("StatusDotDisconnected"));
        m_statusLabel->setText(QStringLiteral("Déconnecté"));
        m_statusLabel->setObjectName(QStringLiteral("StatusLabelDisconnected"));
        m_connectBtn->setText(QStringLiteral("🔌 Connecter"));
        for (int i = 0; i < 6; ++i)
            m_vals[i]->setText(QStringLiteral("—"));
        m_freqLabel->setText(QStringLiteral("—"));
        m_latLabel->setText(QStringLiteral("—"));
    }
    // ✅ Forcer le rechargement du style via setProperty
    m_statusDot->setProperty("connected", connected);
    m_statusDot->update();
    m_statusLabel->setProperty("connected", connected);
    m_statusLabel->update();
}
