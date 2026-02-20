#include "RealTimeTableWidget.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>

static const char* k_headers[] = { "Système", "X", "Y", "Z", "Rx", "Ry", "Rz" };
static const int   k_cols = 7;

RealTimeTableWidget::RealTimeTableWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
}

void RealTimeTableWidget::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(4);

    m_table = new QTableWidget(0, k_cols);
    m_table->setObjectName(QStringLiteral("RealTimeTable"));
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int i = 1; i < k_cols; ++i)
        m_table->horizontalHeader()->setSectionResizeMode(
            i, QHeaderView::ResizeToContents);

    QStringList headers;
    for (int i = 0; i < k_cols; ++i)
        headers << QString::fromUtf8(k_headers[i]);
    m_table->setHorizontalHeaderLabels(headers);

    m_table->setMaximumHeight(160);

    root->addWidget(m_table);
}

void RealTimeTableWidget::addSystem(const QString& name, IMeasurementSystem* system)
{
    if (m_systems.contains(name))
        return;

    m_systems[name] = system;
    rebuildRows();
}

void RealTimeTableWidget::removeSystem(const QString& name)
{
    m_systems.remove(name);
    rebuildRows();
}

void RealTimeTableWidget::rebuildRows()
{
    m_table->setRowCount(0);
    m_rowIndex.clear();

    int row = 0;
    for (auto it = m_systems.begin(); it != m_systems.end(); ++it, ++row) {
        m_table->insertRow(row);
        m_rowIndex[it.key()] = row;

        auto* nameItem = new QTableWidgetItem(it.key());
        nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setItem(row, 0, nameItem);

        for (int c = 1; c < k_cols; ++c) {
            auto* item = new QTableWidgetItem(QStringLiteral("—"));
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, c, item);
        }
    }
}

void RealTimeTableWidget::refresh()
{
    for (auto it = m_systems.begin(); it != m_systems.end(); ++it) {
        const int row = m_rowIndex.value(it.key(), -1);
        if (row < 0)
            continue;

        const MeasurementFrame f = it.value()->getLatestFrame();
        if (!f.isValid)
            continue;

        const double vals[6] = { f.x, f.y, f.z, f.rx, f.ry, f.rz };
        for (int c = 0; c < 6; ++c) {
            m_table->item(row, c + 1)
                ->setText(QString::number(vals[c], 'f', 2));
        }
    }
}
