#pragma once
#ifndef REALTIMETABLEWIDGET_H
#define REALTIMETABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QMap>
#include <QLabel>

#include "IMeasurementSystem.h"

class RealTimeTableWidget : public QWidget {
    Q_OBJECT

public:
    explicit RealTimeTableWidget(QWidget* parent = nullptr);

    void addSystem(const QString& name, IMeasurementSystem* system);
    void removeSystem(const QString& name);
    void refresh();   // Appelé à 20 Hz par MainWindow

private:
    void buildUi();
    void rebuildRows();

    QTableWidget* m_table = nullptr;

    // Clé = nom du système, valeur = pointeur système
    QMap<QString, IMeasurementSystem*> m_systems;
    // Clé = nom du système, valeur = index de ligne dans la table
    QMap<QString, int> m_rowIndex;
};

#endif // REALTIMETABLEWIDGET_H
