#include "MainWindow.h"

#include "AcquisitionConfigPanel.h"
#include "AcquisitionControlPanel.h"
#include "RealTimeTableWidget.h"
#include "AddSystemDialog.h"
#include "SystemCardWidget.h"
#include "LogPositionDialog.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScreen>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("MoBot 2.0 — Multi-System Acquisition"));
    setMinimumSize(1280, 800);

    // Plein écran au démarrage
    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    setGeometry(screen);

    buildLayout();

    m_uiTimer = new QTimer(this);
    m_uiTimer->setInterval(50); // 20 Hz
    connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::onUiRefreshTick);
    m_uiTimer->start();
}

// =============================================================================
// Layout
// =============================================================================

void MainWindow::buildLayout()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    buildLeftPanel();
    buildCentralArea();

    root->addWidget(m_leftPanel);

    // Séparateur vertical
    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::VLine);
    sep->setObjectName(QStringLiteral("PanelSeparator"));
    root->addWidget(sep);

    root->addWidget(m_cardsScrollArea, 1);
}

void MainWindow::buildLeftPanel()
{
    m_leftPanel = new QWidget();
    m_leftPanel->setObjectName(QStringLiteral("LeftPanel"));
    m_leftPanel->setFixedWidth(320);

    QVBoxLayout* layout = new QVBoxLayout(m_leftPanel);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    // Titre
    QLabel* title = new QLabel(QStringLiteral("MoBot 2.0"));
    title->setObjectName(QStringLiteral("AppTitle"));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QFrame* titleSep = new QFrame();
    titleSep->setFrameShape(QFrame::HLine);
    titleSep->setObjectName(QStringLiteral("TitleSeparator"));
    layout->addWidget(titleSep);

    // Panneau config acquisition
    m_configPanel = new AcquisitionConfigPanel();
    layout->addWidget(m_configPanel);

    // Panneau contrôle (START/STOP)
    m_controlPanel = new AcquisitionControlPanel();
    layout->addWidget(m_controlPanel);

    connect(m_controlPanel, &AcquisitionControlPanel::startRequested,
        this, &MainWindow::onStartAcquisition);
    connect(m_controlPanel, &AcquisitionControlPanel::stopRequested,
        this, &MainWindow::onStopAcquisition);

    m_logPosBtn = new QPushButton(QStringLiteral("📍 Log position actuelle"));
    m_logPosBtn->setFixedHeight(36);
    m_logPosBtn->setEnabled(false); // activé seulement en acquisition
    connect(m_logPosBtn, &QPushButton::clicked,
        this, &MainWindow::onLogPosition);
    layout->addWidget(m_logPosBtn);

    // Tableau temps réel
    QLabel* rtLabel = new QLabel(QStringLiteral("Affichage Temps Réel"));
    rtLabel->setObjectName(QStringLiteral("SectionLabel"));
    layout->addWidget(rtLabel);

    m_realTimeTable = new RealTimeTableWidget();
    layout->addWidget(m_realTimeTable);

    layout->addStretch();
}

void MainWindow::buildCentralArea()
{
    m_cardsScrollArea = new QScrollArea();
    m_cardsScrollArea->setWidgetResizable(true);
    m_cardsScrollArea->setObjectName(QStringLiteral("CardsScrollArea"));
    m_cardsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_cardsContainer = new QWidget();
    m_cardsContainer->setObjectName(QStringLiteral("CardsContainer"));

    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(16, 16, 16, 16);
    m_cardsLayout->setSpacing(12);
    m_cardsLayout->setAlignment(Qt::AlignTop);

    // Bouton Ajouter système
    QPushButton* addBtn = new QPushButton(QStringLiteral("＋  Ajouter un système"));
    addBtn->setObjectName(QStringLiteral("AddSystemButton"));
    addBtn->setFixedHeight(48);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddSystemClicked);
    m_cardsLayout->addWidget(addBtn);

    m_cardsScrollArea->setWidget(m_cardsContainer);
}

// =============================================================================
// Slots
// =============================================================================

void MainWindow::onAddSystemClicked()
{
    AddSystemDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    SystemCardWidget* card = dlg.createCard(this);
    if (!card)
        return;

    addSystemCard(card);
}

void MainWindow::addSystemCard(SystemCardWidget* card)
{
    m_cards.append(card);
    m_systems.append(card->system());

    // Insérer avant le bouton "+" (dernier item)
    m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, card);

    connect(card, &SystemCardWidget::connected,
        this, [this, card]() { onSystemConnected(card); });
    connect(card, &SystemCardWidget::disconnected,
        this, [this, card]() { onSystemDisconnected(card); });
    connect(card, &SystemCardWidget::removeRequested,
        this, [this, card]() {
            m_cards.removeOne(card);
            m_systems.removeOne(card->system());
            m_cardsLayout->removeWidget(card);
            card->deleteLater();
            updateStartButtonState();
            m_realTimeTable->removeSystem(card->systemName());
        });

    m_realTimeTable->addSystem(card->systemName(), card->system());
    updateStartButtonState();
}

void MainWindow::onSystemConnected(SystemCardWidget* /*card*/)
{
    updateStartButtonState();
}

void MainWindow::onSystemDisconnected(SystemCardWidget* /*card*/)
{
    updateStartButtonState();
}

void MainWindow::onStartAcquisition()
{
    for (auto* sys : m_systems)
        sys->startAcquisition();

    m_isAcquiring = true;
    m_controlPanel->setAcquiring(true);
    m_logPosBtn->setEnabled(true);

    for (auto* card : m_cards)
        card->setAcquiring(true);
}

void MainWindow::onStopAcquisition()
{
    for (auto* sys : m_systems)
        sys->stopAcquisition();

    m_isAcquiring = false;
    m_controlPanel->setAcquiring(false);
    m_logPosBtn->setEnabled(false);

    for (auto* card : m_cards)
        card->setAcquiring(false);
}

void MainWindow::onUiRefreshTick()
{
    for (auto* card : m_cards)
        card->refreshDisplay();

    if (m_isAcquiring) {
        m_realTimeTable->refresh();
        m_controlPanel->updateStats(m_systems);
    }
}

void MainWindow::updateStartButtonState()
{
    const bool allConnected = !m_systems.isEmpty() &&
        std::all_of(m_systems.begin(), m_systems.end(),
            [](IMeasurementSystem* s) { return s->isConnected(); });
    m_controlPanel->setStartEnabled(allConnected);
}

void MainWindow::onLogPosition()
{
    LogPositionDialog dlg(m_systems, this);
    dlg.exec();
}

