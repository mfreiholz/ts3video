#ifndef VIDEOSETTINGSWIDGET_H
#define VIDEOSETTINGSWIDGET_H

#include <QtWidgets/QDialog>

#include "video/conferencevideowindow.h"
#include "networkclient/networkclient.h"

#include "ui_videosettingswidget.h"

class VideoSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	VideoSettingsDialog(const QSharedPointer<NetworkClient>& nc, QWidget* parent);
	void preselect(const ConferenceVideoWindow::Options& opts);
	const ConferenceVideoWindow::Options& values();

private slots:
	void onCurrentDeviceIndexChanged(int index);
	void onCurrentResolutionIndexChanged(int index);

private:
	Ui::VideoSettingsWidgetForm _ui;
	QSharedPointer<NetworkClient> _nc;
	ConferenceVideoWindow::Options _opts;
};

#endif