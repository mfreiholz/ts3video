#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include <QWidget>

#include "ui_adminwidget.h"

class AdminWidget : public QWidget
{
	Q_OBJECT

public:
	AdminWidget(QWidget* parent);
	virtual ~AdminWidget();

private:
	Ui::AdminWidgetForm _ui;
};

#endif