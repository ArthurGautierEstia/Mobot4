#include "LogPositionDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QProgressBar>

LogPositionDialog::LogPositionDialog(const QVector<IMeasurementSystem*>& systems,
    QWidget* parent)
    : QDialog(parent)
    , m_systems(systems)
{
    setWindowTitle(QStringLiteral("Log position actuelle"));
    setMinimumWidth(640);
    setModal(true);
    buildUi();
    startSampling();
}

// =============================================================================
// UI
// =============================================================================

void LogPositionDialog::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(10);

    // Statut
    m_statusLabel = new QLabel(
        QStringLiteral("⏳ Échantillonnage en cours (1 seconde)..."));
    m_statusLabel->setObjectName(QStringLiteral("StatusLabel"));
    m_statusLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_statusLabel);

    // Tableau résultats
    // Colonnes : Système | X | Y | Z | Rx | Ry | Rz | σX | σY | σZ
    static const char* headers[] = {
        "Système", "X (mm)", "Y (mm)", "Z (mm)",
        "Rx (°)", "Ry (°)", "Rz (°)",
        "σX", "σY", "σZ"
    };
    constexpr int cols = 10;

    m_table = new QTableWidget(0, cols);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int i = 1; i < cols; ++i)
        m_table->horizontalHeader()->setSectionResizeMode(
            i, QHeaderView::ResizeToContents);

    QStringList hLabels;
    for (int i = 0; i < cols; ++i)
        hLabels << QString::fromUtf8(headers[i]);
    m_table->setHorizontalHeaderLabels(hLabels);

    root->addWidget(m_table);

    // Note d'explication
    QLabel* note = new QLabel(
        QStringLiteral("Moyennes calculées sur 1 seconde. "
            "σ = écart-type (indicateur de stabilité)."));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setAlignment(Qt::AlignCenter);
    root->addWidget(note);

    // Boutons
    QHBoxLayout* btns = new QHBoxLayout();
    m_copyBtn = new QPushButton(QStringLiteral("📋 Copier"));
    m_closeBtn = new QPushButton(QStringLiteral("Fermer"));
    m_copyBtn->setEnabled(false);

    btns->addStretch();
    btns->addWidget(m_copyBtn);
    btns->addWidget(m_closeBtn);
    root->addLayout(btns);

    connect(m_copyBtn, &QPushButton::clicked,
        this, &LogPositionDialog::onCopyToClipboard);
    connect(m_closeBtn, &QPushButton::clicked,
        this, &QDialog::accept);
}

// =============================================================================
// Échantillonnage
// =============================================================================

void LogPositionDialog::startSampling()
{
    // Initialiser les buffers
    m_samples.clear();
    for (auto* sys : m_systems) {
        SystemSamples s;
        s.name = sys->getSystemName();
        m_samples[s.name] = s;
    }

    m_samplesLeft = k_totalSamples;

    m_sampleTimer = new QTimer(this);
    m_sampleTimer->setInterval(k_sampleIntervalMs);
    connect(m_sampleTimer, &QTimer::timeout,
        this, &LogPositionDialog::onSampleTick);
    m_sampleTimer->start();
}

void LogPositionDialog::onSampleTick()
{
    // Collecter une frame de chaque système
    for (auto* sys : m_systems) {
        const MeasurementFrame f = sys->getLatestFrame();
        if (!f.isValid)
            continue;

        auto& s = m_samples[sys->getSystemName()];
        s.x.append(f.x);   s.y.append(f.y);   s.z.append(f.z);
        s.rx.append(f.rx);  s.ry.append(f.ry);  s.rz.append(f.rz);
    }

    --m_samplesLeft;

    // Mise à jour statut
    const int progress = 100 - (m_samplesLeft * 100 / k_totalSamples);
    m_statusLabel->setText(
        QStringLiteral("⏳ Échantillonnage... %1%").arg(progress));

    if (m_samplesLeft <= 0) {
        m_sampleTimer->stop();
        computeAndDisplay();
    }
}

// =============================================================================
// Calcul + affichage
// =============================================================================

static double mean(const QVector<double>& v) {
    if (v.isEmpty()) return 0.0;
    double s = 0.0;
    for (double x : v) s += x;
    return s / v.size();
}

static double stddev(const QVector<double>& v) {
    if (v.size() < 2) return 0.0;
    const double m = mean(v);
    double s = 0.0;
    for (double x : v) s += (x - m) * (x - m);
    return std::sqrt(s / (v.size() - 1));
}

void LogPositionDialog::computeAndDisplay()
{
    m_table->setRowCount(0);

    int row = 0;
    for (auto it = m_samples.begin(); it != m_samples.end(); ++it, ++row) {
        const auto& s = it.value();
        if (s.x.isEmpty()) continue;

        m_table->insertRow(row);

        const double means[6] = {
            mean(s.x), mean(s.y), mean(s.z),
            mean(s.rx), mean(s.ry), mean(s.rz)
        };
        const double sigmas[3] = {
            stddev(s.x), stddev(s.y), stddev(s.z)
        };

        // Nom système
        auto* nameItem = new QTableWidgetItem(s.name);
        nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setItem(row, 0, nameItem);

        // Moyennes X Y Z Rx Ry Rz
        for (int i = 0; i < 6; ++i) {
            auto* item = new QTableWidgetItem(
                QString::number(means[i], 'f', 3));
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, i + 1, item);
        }

        // Écarts-types σX σY σZ
        for (int i = 0; i < 3; ++i) {
            auto* item = new QTableWidgetItem(
                QString::number(sigmas[i], 'f', 3));
            item->setTextAlignment(Qt::AlignCenter);
            // Colorier en orange si σ > 0.1 mm (instabilité)
            if (sigmas[i] > 0.1)
                item->setForeground(QColor(QStringLiteral("#fab387")));
            m_table->setItem(row, i + 7, item);
        }
    }

    m_statusLabel->setText(
        QStringLiteral("✅ Position loguée — %1 échantillons par système")
        .arg(k_totalSamples));
    m_copyBtn->setEnabled(true);
}

// =============================================================================
// Copier dans le presse-papier
// =============================================================================

void LogPositionDialog::onCopyToClipboard()
{
    QString text;
    // En-tête
    text += QStringLiteral("Système\tX\tY\tZ\tRx\tRy\tRz\tsX\tsY\tsZ\n");

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QStringList cols;
        for (int col = 0; col < m_table->columnCount(); ++col)
            cols << m_table->item(row, col)->text();
        text += cols.join(QStringLiteral("\t")) + QStringLiteral("\n");
    }

    QApplication::clipboard()->setText(text);
    m_copyBtn->setText(QStringLiteral("✅ Copié !"));
}
