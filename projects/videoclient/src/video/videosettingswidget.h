#ifndef VIDEOSETTINGSWIDGET_H
#define VIDEOSETTINGSWIDGET_H

#include <QtWidgets/QDialog>

#include "ui_videosettingswidget.h"

class VideoSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	VideoSettingsDialog(QWidget* parent);

private:
	Ui::VideoSettingsWidgetForm _ui;
};

#endif