#include "AcquisitionConfigPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>

static const char* k_components[] = { "X", "Y", "Z", "Rx", "Ry", "Rz" };

AcquisitionConfigPanel::AcquisitionConfigPanel(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
}

void AcquisitionConfigPanel::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    // ── Fréquence cible ───────────────────────────────────────────────────────
    QGroupBox* freqBox = new QGroupBox(QStringLiteral("Fréquence cible"));
    QVBoxLayout* freqLayout = new QVBoxLayout(freqBox);

    QHBoxLayout* freqRow = new QHBoxLayout();
    m_freqSpin = new QSpinBox();
    m_freqSpin->setRange(1, 10000);
    m_freqSpin->setValue(250);
    m_freqSpin->setSuffix(QStringLiteral(" Hz"));
    m_freqSpin->setFixedWidth(100);
    freqRow->addWidget(m_freqSpin);
    freqRow->addStretch();
    freqLayout->addLayout(freqRow);

    m_warningLabel = new QLabel();
    m_warningLabel->setObjectName(QStringLiteral("WarningLabel"));
    m_warningLabel->setWordWrap(true);
    m_warningLabel->hide();
    freqLayout->addWidget(m_warningLabel);

    root->addWidget(freqBox);

    // ── Mode d'enregistrement ─────────────────────────────────────────────────
    QGroupBox* modeBox = new QGroupBox(QStringLiteral("Mode d'enregistrement"));
    QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);

    m_modeIndiv = new QRadioButton(QStringLiteral("Individuel (CSV par système)"));
    m_modeSync = new QRadioButton(QStringLiteral("Synchronisé (CSV multi-systèmes)"));
    m_modeBoth = new QRadioButton(QStringLiteral("Les deux"));
    m_modeSync->setChecked(true);

    modeLayout->addWidget(m_modeIndiv);
    modeLayout->addWidget(m_modeSync);
    modeLayout->addWidget(m_modeBoth);

    root->addWidget(modeBox);

    // ── Interpolation ─────────────────────────────────────────────────────────
    QGroupBox* interpBox = new QGroupBox(QStringLiteral("Interpolation (mode synchronisé)"));
    QVBoxLayout* interpLayout = new QVBoxLayout(interpBox);

    m_interpRepeat = new QRadioButton(QStringLiteral("Répétition dernière frame"));
    m_interpLinear = new QRadioButton(QStringLiteral("Interpolation linéaire"));
    m_interpRepeat->setChecked(true);

    interpLayout->addWidget(m_interpRepeat);
    interpLayout->addWidget(m_interpLinear);

    root->addWidget(interpBox);

    // ── Composantes ───────────────────────────────────────────────────────────
    QGroupBox* compBox = new QGroupBox(QStringLiteral("Composantes"));
    QHBoxLayout* compLayout = new QHBoxLayout(compBox);
    compLayout->setSpacing(6);

    for (int i = 0; i < 6; ++i) {
        m_components[i] = new QCheckBox(QString::fromUtf8(k_components[i]));
        m_components[i]->setChecked(true);
        compLayout->addWidget(m_components[i]);
    }
    compLayout->addStretch();

    root->addWidget(compBox);

    // ── Convention angles ─────────────────────────────────────────────────────
    QGroupBox* convBox = new QGroupBox(QStringLiteral("Convention angles"));
    QVBoxLayout* convLayout = new QVBoxLayout(convBox);

    m_angleConvention = new QComboBox();
    m_angleConvention->addItems({
        QStringLiteral("KUKA (A=Rz, B=Ry, C=Rx)"),
        QStringLiteral("XYZ"),
        QStringLiteral("ZYX")
        });
    convLayout->addWidget(m_angleConvention);

    root->addWidget(convBox);

    // ── Dossier de sortie ─────────────────────────────────────────────────────
    QGroupBox* outBox = new QGroupBox(QStringLiteral("Dossier de sortie"));
    QHBoxLayout* outLayout = new QHBoxLayout(outBox);

    m_outputFolder = new QLineEdit(QStringLiteral("./acquisitions"));
    m_browseBtn = new QPushButton(QStringLiteral("📁"));
    m_browseBtn->setFixedWidth(32);

    outLayout->addWidget(m_outputFolder, 1);
    outLayout->addWidget(m_browseBtn);

    connect(m_browseBtn, &QPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(
            this, QStringLiteral("Choisir le dossier de sortie"),
            m_outputFolder->text());
        if (!dir.isEmpty())
            m_outputFolder->setText(dir);
        });

    root->addWidget(outBox);

    // Signaux → configChanged
    connect(m_freqSpin, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &AcquisitionConfigPanel::configChanged);
    connect(m_modeIndiv, &QRadioButton::toggled,
        this, &AcquisitionConfigPanel::configChanged);
    connect(m_modeSync, &QRadioButton::toggled,
        this, &AcquisitionConfigPanel::configChanged);
}

// ── Getters ───────────────────────────────────────────────────────────────────

int     AcquisitionConfigPanel::targetFrequency()    const { return m_freqSpin->value(); }
bool    AcquisitionConfigPanel::modeIndividual()     const { return m_modeIndiv->isChecked(); }
bool    AcquisitionConfigPanel::modeSynchronized()   const { return m_modeSync->isChecked(); }
bool    AcquisitionConfigPanel::modeBoth()           const { return m_modeBoth->isChecked(); }
bool    AcquisitionConfigPanel::interpolationRepeat()const { return m_interpRepeat->isChecked(); }
bool    AcquisitionConfigPanel::componentEnabled(int i) const {
    return (i >= 0 && i < 6) ? m_components[i]->isChecked() : false;
}
QString AcquisitionConfigPanel::outputFolder()       const { return m_outputFolder->text(); }
QString AcquisitionConfigPanel::angleConvention()    const { return m_angleConvention->currentText(); }

void AcquisitionConfigPanel::setWarning(const QString& msg)
{
    if (msg.isEmpty()) {
        m_warningLabel->hide();
    }
    else {
        m_warningLabel->setText(QStringLiteral("⚠ ") + msg);
        m_warningLabel->show();
    }
}
