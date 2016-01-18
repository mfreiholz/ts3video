#ifndef VIDEOSETTINGSWIDGET_H
#define VIDEOSETTINGSWIDGET_H

#include <QtWidgets/QDialog>

#include "video/conferencevideowindow.h"

#include "ui_videosettingswidget.h"

class VideoSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	VideoSettingsDialog(ConferenceVideoWindow *window, QWidget* parent);
	void preselect(const ConferenceVideoWindow::Options& opts);
	const ConferenceVideoWindow::Options& values();

private slots:
	void onCurrentDeviceIndexChanged(int index);

private:
	Ui::VideoSettingsWidgetForm _ui;
	ConferenceVideoWindow* _window;
	ConferenceVideoWindow::Options _opts;
};

#endif