#ifndef CONFERENCEVIDEOWINDOWSIDBAR_H
#define CONFERENCEVIDEOWINDOWSIDBAR_H

#include <QtWidgets/QFrame>

#include <QtMultimedia/QCamera>

class ConferenceVideoWindow;
class QPushButton;
class QLabel;

class ConferenceVideoWindowSidebar : public QFrame
{
	Q_OBJECT

public:
	ConferenceVideoWindowSidebar(ConferenceVideoWindow* parent);

public slots:
	void setVideoEnabled(bool b);

protected:
	void setupCamera();

protected slots:
	void onCameraStatusChanged(QCamera::Status s);

private:
	ConferenceVideoWindow* _window;

	// Controlling buttons
	QPushButton* _enableVideoToggleButton;
#if defined(OCS_INCLUDE_AUDIO)
	QPushButton* _enableAudioInputToggleButton;
#endif
	QPushButton* _userListButton;
	
	// Visibility control.
	QPushButton* _hideLeftPanelButton;
	QPushButton* _showLeftPanelButton;
	bool _panelVisible;

	QPushButton* _adminAuthButton;
	QPushButton* _aboutButton;

	// Bandwidth statistics
	QLabel* _bandwidthRead;
	QLabel* _bandwidthWrite;
};

#endif