#ifndef VIDEOSETTINGSWIDGET_H
#define VIDEOSETTINGSWIDGET_H

#include <QtWidgets/QDialog>

#include "video/conferencevideowindow.h"

#include "ui_videosettingswidget.h"

class VideoSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	VideoSettingsDialog(QWidget* parent);
	void preselect(const ConferenceVideoWindow::Options& opts);
	const ConferenceVideoWindow::Options& values();

private:
	Ui::VideoSettingsWidgetForm _ui;
	ConferenceVideoWindow::Options _opts;
};

#endif