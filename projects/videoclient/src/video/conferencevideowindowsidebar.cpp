#include "conferencevideowindowsidebar.h"

#include <QtCore/QSize>

#include <QtGui/QIcon>

#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

#include "hintoverlaywidget.h"
#include "libclient/networkclient/clientlistmodel.h"
#include "libclient/networkclient/networkclient.h"
#include "video/conferencevideowindow.h"
#include "video/userlistwidget.h"

#include "util/qwidgetutil.h"

///////////////////////////////////////////////////////////////////////

ConferenceVideoWindowSidebar::ConferenceVideoWindowSidebar(ConferenceVideoWindow* parent)
	: QFrame(parent)
	, _window(parent)
	, _camera(_window->camera())
	, _panelVisible(true)
{
	auto nc = _window->networkClient();
	const auto iconSize = fontMetrics().height() * 4;

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
		_enableVideoToggleButton->setIconSize(QSize(iconSize, iconSize));
		_enableVideoToggleButton->setToolTip(tr("Start/stop video."));
		_enableVideoToggleButton->setFlat(true);
		_enableVideoToggleButton->setCheckable(true);
		mainLayout->addWidget(_enableVideoToggleButton);

		QObject::connect(_enableVideoToggleButton, &QPushButton::clicked, this, &ConferenceVideoWindowSidebar::setVideoEnabled);
		QObject::connect(_window, &ConferenceVideoWindow::cameraChanged, this, &ConferenceVideoWindowSidebar::onCameraChanged);

		_enableVideoToggleButton->setVisible(_camera.isNull());
	}

#if defined(OCS_INCLUDE_AUDIO)
	// Audio control
	if (true)
	{
		_enableAudioInputToggleButton = new QPushButton();
		_enableAudioInputToggleButton->setIcon(QIcon(":/ic_mic_grey600_48dp.png"));
		_enableAudioInputToggleButton->setIconSize(QSize(iconSize, iconSize));
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
		_userListButton->setIconSize(QSize(iconSize, iconSize));
		_userListButton->setFlat(true);
		_userListButton->setToolTip(tr("Participants"));
		mainLayout->addWidget(_userListButton);

		// User count label over user list toggle button.
		auto userCountLabel = new QLabel(_userListButton);
		userCountLabel->setObjectName("userCount");
		userCountLabel->setText(QString::number(nc->clientModel()->rowCount()));
		_userListButton->stackUnder(userCountLabel);

		QObject::connect(nc->clientModel(), &ClientListModel::rowsInserted, [this, userCountLabel](const QModelIndex& parent, int first, int last) {
			userCountLabel->setText(QString::number(_window->networkClient()->clientModel()->rowCount()));
		});

		QObject::connect(nc->clientModel(), &ClientListModel::rowsRemoved, [this, userCountLabel](const QModelIndex& parent, int first, int last) {
			userCountLabel->setText(QString::number(_window->networkClient()->clientModel()->rowCount()));
		});

		QObject::connect(_userListButton, &QPushButton::clicked, [this]() {
			auto w = new UserListWidget(_window, nullptr);
			auto hint = HintOverlayWidget::showHint(w, _userListButton);
			hint->resize(this->geometry().width() * 4, this->geometry().height() / 2);
		});
	}

	mainLayout->addStretch(1);
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
				const auto& opts = _window->options();
				auto reply = nc->enableVideoStream(opts.cameraResolution.width(), opts.cameraResolution.height(), opts.cameraBitrate);
				QObject::connect(reply, &QCorReply::finished, _window, &ConferenceVideoWindow::onReplyFinsihedHandleError);
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
	if (_enableVideoToggleButton)
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
	if (_enableVideoToggleButton)
	{
		_enableVideoToggleButton->setEnabled(!_camera.isNull());
		_enableVideoToggleButton->setVisible(!_camera.isNull());
	}
}

void ConferenceVideoWindowSidebar::onCameraStatusChanged(QCamera::Status s)
{
	if (_enableVideoToggleButton)
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
}
