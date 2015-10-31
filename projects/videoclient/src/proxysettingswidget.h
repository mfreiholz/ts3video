#ifndef PROXYSETTINGSWIDGET_H
#define PROXYSETTINGSWIDGET_H

#include <QWidget>
#include "ui_proxysettingswidget.h"

class ProxySettingsWidget : public QWidget
{
	Q_OBJECT

public:
	ProxySettingsWidget(QWidget* parent);
	virtual ~ProxySettingsWidget();

private:
	Ui::ProxySettingsWidgetForm _ui;
};

#endif