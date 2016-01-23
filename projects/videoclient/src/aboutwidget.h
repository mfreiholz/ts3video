#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QDialog>

#include "ui_aboutwidget.h"

class AboutWidget : public QDialog
{
	Q_OBJECT

public:
	AboutWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0);

private:
	Ui::AboutWidget ui;
};

#endif