#ifndef CONFERENCEVIDEOWINDOWSIDBAR_H
#define CONFERENCEVIDEOWINDOWSIDBAR_H

#include <QtWidgets/QFrame>

class ConferenceVideoWindow;
class QPushButton;
class QLabel;

class ConferenceVideoWindowSidebar : public QFrame
{
	Q_OBJECT

public:
	ConferenceVideoWindowSidebar(ConferenceVideoWindow* parent);

private:
	ConferenceVideoWindow* _window;

	// Controlling buttons
	QPushButton* _enableVideoToggleButton;
#if defined(OCS_INCLUDE_AUDIO)
	QPushButton* _enableAudioInputToggleButton;
#endif
	QPushButton* _zoomInButton;
	QPushButton* _zoomOutButton;
	QPushButton* _userListButton;
	QPushButton* _hideLeftPanelButton;
	QPushButton* _showLeftPanelButton;
	QPushButton* _adminAuthButton;
	QPushButton* _aboutButton;

	// Bandwidth statistics
	QLabel* _bandwidthRead;
	QLabel* _bandwidthWrite;
};

#endif