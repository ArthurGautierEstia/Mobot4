#include "AddSystemDialog.h"

#include "SystemCardWidget.h"
#include "KukaRsiSystem.h"
#include "KukaRsiConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>

AddSystemDialog::AddSystemDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Ajouter un système de mesure"));
    setMinimumWidth(400);
    setModal(true);
    buildUi();
}

void AddSystemDialog::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(12);

    // Type de système
    QHBoxLayout* typeRow = new QHBoxLayout();
    typeRow->addWidget(new QLabel(QStringLiteral("Système :")));
    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({
        QStringLiteral("KUKA RSI"),
        QStringLiteral("OptiTrack"),
        QStringLiteral("Vicon"),
        QStringLiteral("Qualisys")
        });
    typeRow->addWidget(m_typeCombo, 1);
    root->addLayout(typeRow);

    // Pages de config par système
    m_pages = new QStackedWidget();
    buildKukaPage();
    buildOptitrackPage();
    buildGenericPage(QStringLiteral("Vicon"));
    buildGenericPage(QStringLiteral("Qualisys"));
    root->addWidget(m_pages);

    // Boutons OK / Annuler
    QDialogButtonBox* btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btns->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Ajouter"));
    btns->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Annuler"));
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(btns);

    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &AddSystemDialog::onSystemTypeChanged);
}

void AddSystemDialog::buildKukaPage()
{
    QWidget* page = new QWidget();
    QFormLayout* form = new QFormLayout(page);
    form->setSpacing(8);

    m_kukaName = new QLineEdit(QStringLiteral("KUKA RSI"));
    m_kukaIp = new QLineEdit(QStringLiteral("172.31.2.100"));
    m_kukaPort = new QSpinBox();
    m_kukaPort->setRange(1024, 65535);
    m_kukaPort->setValue(49152);

    form->addRow(QStringLiteral("Nom :"), m_kukaName);
    form->addRow(QStringLiteral("IP hôte :"), m_kukaIp);
    form->addRow(QStringLiteral("Port hôte :"), m_kukaPort);

    m_pages->addWidget(page);
}

void AddSystemDialog::buildOptitrackPage()
{
    QWidget* page = new QWidget();
    QFormLayout* form = new QFormLayout(page);
    form->setSpacing(8);

    m_optiIp = new QLineEdit(QStringLiteral("127.0.0.1"));
    m_optiBody = new QLineEdit(QStringLiteral("RigidBody1"));

    form->addRow(QStringLiteral("Serveur IP :"), m_optiIp);
    form->addRow(QStringLiteral("Rigid Body :"), m_optiBody);

    m_pages->addWidget(page);
}

void AddSystemDialog::buildGenericPage(const QString& label)
{
    QWidget* page = new QWidget();
    QVBoxLayout* l = new QVBoxLayout(page);
    l->addWidget(new QLabel(
        QStringLiteral("Configuration %1\n(à compléter)").arg(label)));
    m_pages->addWidget(page);
}

void AddSystemDialog::onSystemTypeChanged(int index)
{
    m_pages->setCurrentIndex(index);
}

SystemCardWidget* AddSystemDialog::createCard(QWidget* cardParent)
{
    const int idx = m_typeCombo->currentIndex();

    switch (idx) {
    case 0: { // KUKA RSI
        KukaRsiConfig cfg;
        cfg.hostAddress = m_kukaIp->text().trimmed();
        cfg.hostPort = m_kukaPort->value();
        cfg.robotName = m_kukaName->text().trimmed();

        auto* sys = new KukaRsiSystem(cfg, cardParent);
        auto* card = new SystemCardWidget(sys, cfg.robotName, cardParent);
        return card;
    }
    default:
        return nullptr; // OptiTrack/Vicon/Qualisys à brancher quand leurs systèmes seront prêts
    }
}
