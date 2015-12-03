#include "conferencevideowindowsidebar.h"

#include <QtCore/QSize>

#include <QtGui/QIcon>

#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

#include "videolib/src/elws.h"
#include "videolib/src/networkusageentity.h"

#include "video/conferencevideowindow.h"

#include "aboutwidget.h"

#include "networkclient/networkclient.h"

// Static Helpers /////////////////////////////////////////////////////

static QSize  __sideBarIconSize(52, 52);

///////////////////////////////////////////////////////////////////////

ConferenceVideoWindowSidebar::ConferenceVideoWindowSidebar(ConferenceVideoWindow* parent) :
	QFrame(parent),
	_window(parent)
{
	auto nc = _window->networkClient();

	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	_enableVideoToggleButton = new QPushButton();
	_enableVideoToggleButton->setIcon(QIcon(":/ic_videocam_grey600_48dp.png"));
	_enableVideoToggleButton->setIconSize(__sideBarIconSize);
	_enableVideoToggleButton->setToolTip(tr("Start/stop video."));
	_enableVideoToggleButton->setFlat(true);
	_enableVideoToggleButton->setCheckable(true);
	_enableVideoToggleButton->setVisible(false);
	//QObject::connect(_enableVideoToggleButton, &QPushButton::toggled, this, &TileViewWidget::setVideoEnabled);
	mainLayout->addWidget(_enableVideoToggleButton);

#if defined(OCS_INCLUDE_AUDIO)
	_enableAudioInputToggleButton = new QPushButton();
	_enableAudioInputToggleButton->setIcon(QIcon(":/ic_mic_grey600_48dp.png"));
	_enableAudioInputToggleButton->setIconSize(__sideBarIconSize);
	_enableAudioInputToggleButton->setToolTip(tr("Start/stop microphone."));
	_enableAudioInputToggleButton->setFlat(true);
	_enableAudioInputToggleButton->setCheckable(true);
	//QObject::connect(_enableAudioInputToggleButton, &QPushButton::toggled, this, &TileViewWidget::setAudioInputEnabled);
	mainLayout->addWidget(_enableAudioInputToggleButton);
#endif

	_zoomInButton = new QPushButton();
	_zoomInButton->setIcon(QIcon(":/ic_add_circle_outline_grey600_48dp.png"));
	_zoomInButton->setIconSize(__sideBarIconSize);
	_zoomInButton->setFlat(true);
	_zoomInButton->setToolTip(tr("Zoom-in video (CTRL+Mousewheel-Up)"));
	mainLayout->addWidget(_zoomInButton);

	_zoomOutButton = new QPushButton();
	_zoomOutButton->setIcon(QIcon(":/ic_remove_circle_outline_grey600_48dp.png"));
	_zoomOutButton->setIconSize(__sideBarIconSize);
	_zoomOutButton->setFlat(true);
	_zoomOutButton->setToolTip(tr("Zoom-out video (CTRL+Mousewheel-Down)"));
	mainLayout->addWidget(_zoomOutButton);

	_userListButton = new QPushButton();
	_userListButton->setIcon(QIcon(":/ic_supervisor_account_grey600_48dp.png"));
	_userListButton->setIconSize(__sideBarIconSize);
	_userListButton->setFlat(true);
	_userListButton->setToolTip(tr("User list"));
	_userListButton->setCheckable(true);
	mainLayout->addWidget(_userListButton);

	mainLayout->addStretch(1);

	_hideLeftPanelButton = new QPushButton();
	_hideLeftPanelButton->setIcon(QIcon(":/ic_chevron_left_grey600_48dp.png"));
	_hideLeftPanelButton->setIconSize(__sideBarIconSize / 2);
	_hideLeftPanelButton->setToolTip(tr("Hide action bar"));
	_hideLeftPanelButton->setFlat(true);
	mainLayout->addWidget(_hideLeftPanelButton);

	_showLeftPanelButton = new QPushButton(parentWidget());
	_showLeftPanelButton->setObjectName("showLeftPanelButton");
	_showLeftPanelButton->setIcon(QIcon(":/ic_chevron_right_grey600_48dp.png"));
	_showLeftPanelButton->setIconSize(__sideBarIconSize / 2);
	_showLeftPanelButton->setToolTip(tr("Show action bar"));
	_showLeftPanelButton->setFlat(true);
	_showLeftPanelButton->setVisible(!isVisible());
	_showLeftPanelButton->resize(_showLeftPanelButton->iconSize());
	_showLeftPanelButton->move(QPoint(0, 0));

	_adminAuthButton = new QPushButton();
	_adminAuthButton->setIcon(QIcon(":/ic_lock_grey600_48dp.png"));
	_adminAuthButton->setIconSize(__sideBarIconSize / 2);
	_adminAuthButton->setToolTip(tr("Login as admin"));
	_adminAuthButton->setFlat(true);
	mainLayout->addWidget(_adminAuthButton);

	// Download and upload indicators.
	if (true)
	{
		auto bandwidthContainer = new QWidget();
		auto bandwidthContainerLayout = new QBoxLayout(QBoxLayout::TopToBottom);
		bandwidthContainerLayout->setContentsMargins(3, 3, 3, 3);
		bandwidthContainer->setLayout(bandwidthContainerLayout);

		_bandwidthRead = new QLabel("D: 0.0 KB/s");
		_bandwidthRead->setObjectName("bandwidthRead");
		bandwidthContainerLayout->addWidget(_bandwidthRead);

		_bandwidthWrite = new QLabel("U: 0.0 KB/s");
		_bandwidthWrite->setObjectName("bandwidthWrite");
		bandwidthContainerLayout->addWidget(_bandwidthWrite);

		mainLayout->addWidget(bandwidthContainer);

		QObject::connect(nc.data(), &NetworkClient::networkUsageUpdated, [this](const NetworkUsageEntity & networkUsage)
		{
			_bandwidthRead->setText(QString("D: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthRead)));
			_bandwidthWrite->setText(QString("U: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthWrite)));
			if (_bandwidthRead->parentWidget())
			{
				_bandwidthRead->parentWidget()->setToolTip(tr("Received: %1\nSent: %2")
						.arg(ELWS::humanReadableSize(networkUsage.bytesRead))
						.arg(ELWS::humanReadableSize(networkUsage.bytesWritten)));
			}
		});
	}

	_aboutButton = new QPushButton();
	_aboutButton->setIcon(QIcon(":/ic_info_outline_grey600_48dp.png"));
	_aboutButton->setIconSize(__sideBarIconSize);
	_aboutButton->setFlat(true);
	mainLayout->addWidget(_aboutButton);
	QObject::connect(_aboutButton, &QPushButton::clicked, [this]()
	{
		auto about = new AboutWidget(this);
		about->setWindowFlags(Qt::Dialog);
		about->show();
	});
}