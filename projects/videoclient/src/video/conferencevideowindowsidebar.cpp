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
#include "video/userlistwidget.h"

#include "adminauthwidget.h"
#include "aboutwidget.h"
#include "hintoverlaywidget.h"

#include "networkclient/networkclient.h"
#include "networkclient/clientlistmodel.h"

// Static Helpers /////////////////////////////////////////////////////

static QSize __sideBarIconSize(52, 52);

///////////////////////////////////////////////////////////////////////

ConferenceVideoWindowSidebar::ConferenceVideoWindowSidebar(ConferenceVideoWindow* parent) :
	QFrame(parent),
	_window(parent),
	_camera(_window->camera()),
	_panelVisible(true)
{
	auto nc = _window->networkClient();

	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	// Video control
	if (true)
	{
		QIcon ico(":/ic_videocam_grey600_48dp.png");
		ico.addPixmap(QPixmap(":/ic_videocam_red600_48dp"), QIcon::Normal, QIcon::On);

		_enableVideoToggleButton = new QPushButton();
		_enableVideoToggleButton->setIcon(ico);
		_enableVideoToggleButton->setIconSize(__sideBarIconSize);
		_enableVideoToggleButton->setToolTip(tr("Start/stop video."));
		_enableVideoToggleButton->setFlat(true);
		_enableVideoToggleButton->setCheckable(true);
		_enableVideoToggleButton->setVisible(_camera.isNull());
		mainLayout->addWidget(_enableVideoToggleButton);

		QObject::connect(_enableVideoToggleButton, &QPushButton::toggled, this, &ConferenceVideoWindowSidebar::setVideoEnabled);
		QObject::connect(_window, &ConferenceVideoWindow::cameraChanged, this, &ConferenceVideoWindowSidebar::onCameraChanged);
	}

#if defined(OCS_INCLUDE_AUDIO)
	// Audio control
	if (true)
	{
		_enableAudioInputToggleButton = new QPushButton();
		_enableAudioInputToggleButton->setIcon(QIcon(":/ic_mic_grey600_48dp.png"));
		_enableAudioInputToggleButton->setIconSize(__sideBarIconSize);
		_enableAudioInputToggleButton->setToolTip(tr("Start/stop microphone."));
		_enableAudioInputToggleButton->setFlat(true);
		_enableAudioInputToggleButton->setCheckable(true);
		//QObject::connect(_enableAudioInputToggleButton, &QPushButton::toggled, this, &TileViewWidget::setAudioInputEnabled);
		mainLayout->addWidget(_enableAudioInputToggleButton);
	}
#endif

	// User list control
	if (true)
	{
		_userListButton = new QPushButton();
		_userListButton->setIcon(QIcon(":/ic_supervisor_account_grey600_48dp.png"));
		_userListButton->setIconSize(__sideBarIconSize);
		_userListButton->setFlat(true);
		_userListButton->setToolTip(tr("Participants"));
		mainLayout->addWidget(_userListButton);

		// User count label over user list toggle button.
		auto userCountLabel = new QLabel(_userListButton);
		userCountLabel->setObjectName("userCount");
		userCountLabel->setText(QString::number(nc->clientModel()->rowCount()));
		_userListButton->stackUnder(userCountLabel);

		QObject::connect(nc->clientModel(), &ClientListModel::rowsInserted, [this, userCountLabel](const QModelIndex & parent, int first, int last)
		{
			userCountLabel->setText(QString::number(_window->networkClient()->clientModel()->rowCount()));
		});

		QObject::connect(nc->clientModel(), &ClientListModel::rowsRemoved, [this, userCountLabel](const QModelIndex & parent, int first, int last)
		{
			userCountLabel->setText(QString::number(_window->networkClient()->clientModel()->rowCount()));
		});

		QObject::connect(_userListButton, &QPushButton::clicked, [this]()
		{
			auto w = new UserListWidget(_window, nullptr);
			auto hint = HintOverlayWidget::showHint(w, _userListButton);
			hint->resize(200, 350);
		});
	}

	mainLayout->addStretch(1);

	// Show/hide sidebar control
	if (true)
	{
		_hideLeftPanelButton = new QPushButton();
		_hideLeftPanelButton->setIcon(QIcon(":/ic_chevron_left_grey600_48dp.png"));
		_hideLeftPanelButton->setIconSize(__sideBarIconSize / 2);
		_hideLeftPanelButton->setToolTip(tr("Hide action bar"));
		_hideLeftPanelButton->setFlat(true);
		_hideLeftPanelButton->setVisible(true);
		mainLayout->addWidget(_hideLeftPanelButton);

		_showLeftPanelButton = new QPushButton(parentWidget());
		_showLeftPanelButton->setObjectName("showLeftPanelButton");
		_showLeftPanelButton->setIcon(QIcon(":/ic_chevron_right_grey600_48dp.png"));
		_showLeftPanelButton->setIconSize(__sideBarIconSize / 2);
		_showLeftPanelButton->setToolTip(tr("Show action bar"));
		_showLeftPanelButton->setFlat(true);
		_showLeftPanelButton->setVisible(!_panelVisible);
		_showLeftPanelButton->resize(_showLeftPanelButton->iconSize());
		_showLeftPanelButton->move(QPoint(0, 0));

		QObject::connect(_hideLeftPanelButton, &QPushButton::clicked, [this]()
		{
			this->setVisible(false);
			_showLeftPanelButton->setVisible(true);
			//d->tilesLayout->setContentsMargins(d->showLeftPanelButton->width() - 8, 0, 0, 0);
			_panelVisible = false;
		});
		QObject::connect(_showLeftPanelButton, &QPushButton::clicked, [this]()
		{
			this->setVisible(true);
			_showLeftPanelButton->setVisible(false);
			//d->tilesLayout->setContentsMargins(0, 0, 0, 0);
			_panelVisible = true;
		});
	}

	// Admin login control
	if (true)
	{
		_adminAuthButton = new QPushButton();
		_adminAuthButton->setIcon(QIcon(":/ic_lock_grey600_48dp.png"));
		_adminAuthButton->setIconSize(__sideBarIconSize / 2);
		_adminAuthButton->setToolTip(tr("Login as admin"));
		_adminAuthButton->setFlat(true);
		mainLayout->addWidget(_adminAuthButton);

		QObject::connect(_adminAuthButton, &QPushButton::clicked, [this, nc]()
		{
			auto w = new AdminAuthWidget(nc, this);
			w->setModal(true);
			w->exec();
			if (nc->isAdmin())
				_adminAuthButton->setVisible(false);
		});
	}

	// Network usage statistics
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

void ConferenceVideoWindowSidebar::setVideoEnabled(bool b)
{
	auto nc = _window->networkClient();
	auto cam = _camera;
	if (cam)
	{
		if (b)
		{
			if (cam->state() != QCamera::ActiveState)
			{
				cam->start();
			}
			if (nc)
			{
				auto reply = nc->enableVideoStream(_window->options().cameraResolution.width(), _window->options().cameraResolution.height());
				QCORREPLY_AUTODELETE(reply);
			}
		}
		else
		{
			if (cam->state() == QCamera::ActiveState)
			{
				cam->stop();
			}
			if (nc)
			{
				auto reply = nc->disableVideoStream();
				QCORREPLY_AUTODELETE(reply);
			}
		}
	}
	_enableVideoToggleButton->setChecked(b);
}

void ConferenceVideoWindowSidebar::onCameraChanged()
{
	// Reset old camera stuff
	if (_camera)
	{
		_camera->disconnect(this);
		_camera.clear();
	}

	// Setup new camera
	_camera = _window->camera();
	if (_camera)
	{
		QObject::connect(_camera.data(), &QCamera::statusChanged, this, &ConferenceVideoWindowSidebar::onCameraStatusChanged);
	}

	// Update UI
	_enableVideoToggleButton->setEnabled(!_camera.isNull());
	_enableVideoToggleButton->setVisible(!_camera.isNull());
}

void ConferenceVideoWindowSidebar::onCameraStatusChanged(QCamera::Status s)
{
	switch (s)
	{
	case QCamera::ActiveStatus:
		_enableVideoToggleButton->setEnabled(true);
		break;
	case QCamera::StartingStatus:
		_enableVideoToggleButton->setEnabled(false);
		break;
	case QCamera::StoppingStatus:
		_enableVideoToggleButton->setEnabled(false);
		break;
	case QCamera::StandbyStatus:
		_enableVideoToggleButton->setEnabled(true);
		break;
	case QCamera::LoadedStatus:
		_enableVideoToggleButton->setEnabled(true);
		break;
	case QCamera::LoadingStatus:
		_enableVideoToggleButton->setEnabled(false);
		break;
	case QCamera::UnloadingStatus:
		_enableVideoToggleButton->setEnabled(false);
		break;
	case QCamera::UnloadedStatus:
		_enableVideoToggleButton->setEnabled(true);
		_enableVideoToggleButton->setChecked(false);
		break;
	case QCamera::UnavailableStatus:
		_enableVideoToggleButton->setEnabled(false);
		_enableVideoToggleButton->setChecked(false);
		break;
	}
}