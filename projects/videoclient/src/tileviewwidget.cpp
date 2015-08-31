#include "tileviewwidget_p.h"

#include <QSettings>
#include <QBoxLayout>
#include <QWheelEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QListView>
#include <QLineEdit>
#include <QMenu>

#include "humblelogging/api.h"

#include "elws.h"
#include "cliententity.h"
#include "channelentity.h"
#include "networkusageentity.h"

#include "clientapplogic.h"
#include "clientcameravideowidget.h"
#include "flowlayout.h"
#include "remoteclientvideowidget.h"
#include "aboutwidget.h"
#include "movablewidgetcontainer.h"
#include "adminauthwidget.h"
#include "networkclient/clientlistmodel.h"

HUMBLE_LOGGER(HL, "gui.tileview");

///////////////////////////////////////////////////////////////////////

//static QColor __lightBackgroundColor(45, 45, 48);
//static QColor __darkBackgroundColor(30, 30, 30);
//static QColor __lightForegroundColor(200, 200, 200);
//static QColor __frameColor(63, 63, 70);
//static QColor __frameColorActive(0, 122, 204);
static QSize  __sideBarIconSize(52, 52);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewWidget::TileViewWidget(QWidget* parent, Qt::WindowFlags f) :
	QWidget(parent),
	d(new TileViewWidgetPrivate(this))
{
	d->tilesCurrentSize.scale(200, 200, Qt::KeepAspectRatio);

	// Scroll area content widget.
	auto scrollAreaContent = new QWidget();
	auto scrollAreaContentLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	scrollAreaContent->setLayout(scrollAreaContentLayout);

	// Video tiles container.
	auto tilesContainer = new MovableWidgetContainer(nullptr, 0);
	auto tilesContainerLayout = static_cast<FlowLayout*>(tilesContainer->layout());
	tilesContainerLayout->setContentsMargins(0, 0, 0, 0);
	tilesContainer->setLayout(tilesContainerLayout);
	scrollAreaContentLayout->addWidget(tilesContainer, 1);
	d->tilesLayout = tilesContainerLayout;

	auto scrollArea = new QScrollArea();
	scrollArea->setFrameStyle(QFrame::NoFrame);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scrollArea->setWidget(scrollAreaContent);

	// Camera.
	d->cameraWidget = new TileViewCameraWidget();
	d->cameraWidget->setFixedSize(d->tilesCurrentSize);
	d->cameraWidget->setVisible(false);
	d->tilesLayout->addWidget(d->cameraWidget);

	// Buttons.
	d->enableVideoToggleButton = new QPushButton();
	d->enableVideoToggleButton->setIcon(QIcon(":/ic_videocam_grey600_48dp.png"));
	d->enableVideoToggleButton->setIconSize(__sideBarIconSize);
	d->enableVideoToggleButton->setToolTip(tr("Start/stop video."));
	d->enableVideoToggleButton->setFlat(true);
	d->enableVideoToggleButton->setCheckable(true);
	d->enableVideoToggleButton->setVisible(false);
	QObject::connect(d->enableVideoToggleButton, &QPushButton::toggled, this, &TileViewWidget::setVideoEnabled);

	d->zoomInButton = new QPushButton();
	d->zoomInButton->setIcon(QIcon(":/ic_add_circle_outline_grey600_48dp.png"));
	d->zoomInButton->setIconSize(__sideBarIconSize);
	d->zoomInButton->setFlat(true);
	d->zoomInButton->setToolTip(tr("Zoom-in video (CTRL+Mousewheel-Up)"));

	d->zoomOutButton = new QPushButton();
	d->zoomOutButton->setIcon(QIcon(":/ic_remove_circle_outline_grey600_48dp.png"));
	d->zoomOutButton->setIconSize(__sideBarIconSize);
	d->zoomOutButton->setFlat(true);
	d->zoomOutButton->setToolTip(tr("Zoom-out video (CTRL+Mousewheel-Down)"));

	d->userListButton = new QPushButton();
	d->userListButton->setIcon(QIcon(":/ic_supervisor_account_grey600_48dp.png"));
	d->userListButton->setIconSize(__sideBarIconSize);
	d->userListButton->setFlat(true);
	d->userListButton->setToolTip(tr("User list"));
	d->userListButton->setCheckable(true);

	auto hideLeftPanelButton = new QPushButton();
	hideLeftPanelButton->setIcon(QIcon(":/ic_chevron_left_grey600_48dp.png"));
	hideLeftPanelButton->setIconSize(__sideBarIconSize / 2);
	hideLeftPanelButton->setToolTip(tr("Hide action bar"));
	hideLeftPanelButton->setFlat(true);

	auto showLeftPanelButton = new QPushButton(scrollAreaContent);
	showLeftPanelButton->setObjectName("showLeftPanelButton");
	showLeftPanelButton->setIcon(QIcon(":/ic_chevron_right_grey600_48dp.png"));
	showLeftPanelButton->setIconSize(__sideBarIconSize / 2);
	showLeftPanelButton->setToolTip(tr("Show action bar"));
	showLeftPanelButton->setFlat(true);
	showLeftPanelButton->setVisible(!d->leftPanelVisible);
	showLeftPanelButton->resize(showLeftPanelButton->iconSize());
	showLeftPanelButton->move(QPoint(0, 0));

	auto adminAuthButton = new QPushButton(scrollAreaContent);
	adminAuthButton->setIcon(QIcon(":/ic_lock_grey600_48dp.png"));
	adminAuthButton->setIconSize(__sideBarIconSize / 2);
	adminAuthButton->setToolTip(tr("Login as admin"));
	adminAuthButton->setFlat(true);

	auto aboutButton = new QPushButton();
	aboutButton->setIcon(QIcon(":/ic_info_outline_grey600_48dp.png"));
	aboutButton->setIconSize(__sideBarIconSize);
	aboutButton->setFlat(true);

	// Download and upload indicators.
	auto bandwidthContainer = new QWidget();
	auto bandwidthContainerLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	bandwidthContainerLayout->setContentsMargins(3, 3, 3, 3);
	bandwidthContainer->setLayout(bandwidthContainerLayout);

	d->bandwidthRead = new QLabel("D: 0.0 KB/s");
	d->bandwidthRead->setObjectName("bandwidthRead");
	bandwidthContainerLayout->addWidget(d->bandwidthRead);

	d->bandwidthWrite = new QLabel("U: 0.0 KB/s");
	d->bandwidthWrite->setObjectName("bandwidthWrite");
	bandwidthContainerLayout->addWidget(d->bandwidthWrite);

	// User list.
	auto userListWidget = new TileViewUserListWidget();

	// User count label over user list toggle button.
	auto userCountLabel = new QLabel(d->userListButton);
	userCountLabel->setObjectName("userCount");
	userCountLabel->setText(QString::number(0));
	d->userListButton->stackUnder(userCountLabel);

	// Layout
	auto leftPanel = new QWidget();
	leftPanel->setObjectName("leftPanelContainer");
	auto leftPanelLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	leftPanelLayout->setContentsMargins(0, 0, 0, 0);
	leftPanelLayout->setSpacing(0);
	leftPanelLayout->addWidget(d->enableVideoToggleButton);
	leftPanelLayout->addWidget(d->zoomInButton);
	leftPanelLayout->addWidget(d->zoomOutButton);
	leftPanelLayout->addWidget(d->userListButton);
	leftPanelLayout->addStretch(1);
	leftPanelLayout->addWidget(hideLeftPanelButton);
	leftPanelLayout->addWidget(adminAuthButton);
	leftPanelLayout->addWidget(bandwidthContainer);
	leftPanelLayout->addWidget(aboutButton);
	leftPanel->setLayout(leftPanelLayout);

	auto rightPanel = new QWidget();
	rightPanel->setObjectName("rightPanelContainer");
	auto rightPanelLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	rightPanelLayout->setContentsMargins(0, 0, 0, 0);
	rightPanelLayout->setSpacing(0);
	rightPanelLayout->addWidget(userListWidget);
	rightPanel->setLayout(rightPanelLayout);
	rightPanel->setVisible(d->rightPanelVisible);

	auto mainLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(leftPanel, 0);
	mainLayout->addWidget(scrollArea, 1);
	mainLayout->addWidget(rightPanel, 0);
	setLayout(mainLayout);

	// Events
	auto nc = ClientAppLogic::instance()->networkClient();
	QObject::connect(nc.data(), &NetworkClient::clientEnabledVideo, this, &TileViewWidget::onClientEnabledVideo);
	QObject::connect(nc.data(), &NetworkClient::clientDisabledVideo, this, &TileViewWidget::onClientDisabledVideo);
	QObject::connect(ClientAppLogic::instance()->networkClient().data(), &NetworkClient::networkUsageUpdated, [this](const NetworkUsageEntity & networkUsage)
	{
		d->bandwidthRead->setText(QString("D: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthRead)));
		d->bandwidthWrite->setText(QString("U: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthWrite)));
	});
	QObject::connect(d->zoomInButton, &QPushButton::clicked, [this]()
	{
		auto newSize = d->tilesCurrentSize;
		newSize += QSize(25, 25);
		newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
		setTileSize(newSize);
		d->tilesLayout->update();
	});
	QObject::connect(d->zoomOutButton, &QPushButton::clicked, [this]()
	{
		auto newSize = d->tilesCurrentSize;
		newSize -= QSize(25, 25);
		newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
		setTileSize(newSize);
	});
	QObject::connect(d->userListButton, &QPushButton::toggled, [this, rightPanel](bool checked)
	{
		rightPanel->setVisible(checked);
		d->rightPanelVisible = checked;
	});
	QObject::connect(hideLeftPanelButton, &QPushButton::clicked, [this, leftPanel, showLeftPanelButton]()
	{
		leftPanel->setVisible(false);
		showLeftPanelButton->setVisible(true);
		d->tilesLayout->setContentsMargins(d->showLeftPanelButton->width() - 8, 0, 0, 0);
		d->leftPanelVisible = false;
	});
	QObject::connect(showLeftPanelButton, &QPushButton::clicked, [this, leftPanel, showLeftPanelButton]()
	{
		leftPanel->setVisible(true);
		showLeftPanelButton->setVisible(false);
		d->tilesLayout->setContentsMargins(0, 0, 0, 0);
		d->leftPanelVisible = true;
	});
	QObject::connect(adminAuthButton, &QPushButton::clicked, [this, adminAuthButton]()
	{
		auto w = new AdminAuthWidget(ClientAppLogic::instance()->networkClient(), this);
		w->setModal(true);
		w->exec();
		if (ClientAppLogic::instance()->networkClient()->isAdmin())
			adminAuthButton->setVisible(false);
	});
	QObject::connect(aboutButton, &QPushButton::clicked, [this]()
	{
		auto about = new AboutWidget(this);
		about->setWindowFlags(Qt::Dialog);
		about->show();
	});

	// Save some references.
	d->leftPanel = leftPanel;
	d->rightPanel = rightPanel;
	d->userCountLabel = userCountLabel;
	d->hideLeftPanelButton = hideLeftPanelButton;
	d->showLeftPanelButton = showLeftPanelButton;
	d->userListWidget = userListWidget;


	resize(800, 600);
	setVisible(true);
}

TileViewWidget::~TileViewWidget()
{
}

void TileViewWidget::setClientListModel(ClientListModel* model)
{
	// Use sorted model.
	auto proxyModel = new SortFilterClientListProxyModel(this);
	proxyModel->setSourceModel(model);
	proxyModel->sort(0, Qt::AscendingOrder);
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	// Set and handle model actions.
	QAbstractItemModel* m = proxyModel;
	d->userListWidget->_listView->setModel(m);
	QObject::connect(m, &ClientListModel::rowsInserted, [this, m](const QModelIndex&, int, int)
	{
		d->userCountLabel->setText(QString::number(m->rowCount()));
	});
	QObject::connect(m, &ClientListModel::rowsRemoved, [this, m](const QModelIndex&, int, int)
	{
		d->userCountLabel->setText(QString::number(m->rowCount()));
	});
	d->userCountLabel->setText(QString::number(m->rowCount()));
}

void TileViewWidget::setCamera(const QSharedPointer<QCamera>& c)
{
	d->cameraWidget->setCamera(c);
	d->cameraWidget->setVisible(true);
	d->enableVideoToggleButton->setVisible(true);
	QObject::connect(c.data(), &QCamera::statusChanged, [this](QCamera::Status s)
	{
		switch (s)
		{
		case QCamera::ActiveStatus:
			d->enableVideoToggleButton->setEnabled(true);
			break;
		case QCamera::StartingStatus:
			d->enableVideoToggleButton->setEnabled(false);
			break;
		case QCamera::StoppingStatus:
			d->enableVideoToggleButton->setEnabled(false);
			break;
		case QCamera::StandbyStatus:
			d->enableVideoToggleButton->setEnabled(true);
			break;
		case QCamera::LoadedStatus:
			d->enableVideoToggleButton->setEnabled(true);
			d->cameraWidget->_cameraWidget->setFrame(QImage());
			break;
		case QCamera::LoadingStatus:
			d->enableVideoToggleButton->setEnabled(false);
			break;
		case QCamera::UnloadingStatus:
			d->enableVideoToggleButton->setEnabled(false);
			break;
		case QCamera::UnloadedStatus:
			d->enableVideoToggleButton->setEnabled(true);
			break;
		case QCamera::UnavailableStatus:
			d->enableVideoToggleButton->setEnabled(false);
			break;
		}
	});
}

void TileViewWidget::addClient(const ClientEntity& client, const ChannelEntity& channel)
{
	if (client.videoEnabled && !d->tilesMap.contains(client.id))
	{
		auto tileWidget = new TileViewTileWidget(client, this);
		tileWidget->setFixedSize(d->tilesCurrentSize);
		d->tilesLayout->addWidget(tileWidget);
		d->tilesMap.insert(client.id, tileWidget);
	}
}

void TileViewWidget::removeClient(const ClientEntity& client, const ChannelEntity& channel)
{
	auto tileWidget = d->tilesMap.value(client.id);
	if (!tileWidget)
	{
		return;
	}

	// Mappings
	d->tilesMap.remove(client.id);

	// Remove from layout.
	tileWidget->setVisible(false);
	d->tilesLayout->removeWidget(tileWidget);
	delete tileWidget;
}

void TileViewWidget::updateClientVideo(YuvFrameRefPtr frame, int senderId)
{
	auto tileWidget = d->tilesMap.value(senderId);
	if (!tileWidget)
	{
		return;
	}
	tileWidget->_videoWidget->videoWidget()->setFrame(frame);
}

void TileViewWidget::setTileSize(const QSize& size)
{
	auto newSize = d->tilesAspectRatio.scaled(size, Qt::KeepAspectRatio);
	d->tilesCurrentSize = newSize;

	QList<QWidget*> widgets;
	widgets.append(d->cameraWidget);
	Q_FOREACH (auto w, d->tilesMap.values())
	{
		widgets.append(w);
	}
	Q_FOREACH (auto w, widgets)
	{
		w->setFixedSize(newSize);
	}
}

void TileViewWidget::setVideoEnabled(bool b)
{
	auto nc = ClientAppLogic::instance()->networkClient();
	auto cam = d->cameraWidget->_camera;
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
				auto reply = nc->enableVideoStream();
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
	d->enableVideoToggleButton->setChecked(b);
}

void TileViewWidget::wheelEvent(QWheelEvent* e)
{
	if (e->modifiers() != Qt::ControlModifier)
	{
		e->ignore();
		return;
	}
	auto delta = e->angleDelta();
	if (delta.y() > 0)
	{
		auto newSize = d->tilesCurrentSize;
		newSize += QSize(25, 25);
		newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
		setTileSize(newSize);
		d->tilesLayout->update();
	}
	else if (delta.y() < 0)
	{
		auto newSize = d->tilesCurrentSize;
		newSize -= QSize(25, 25);
		newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
		setTileSize(newSize);
	}
}

void TileViewWidget::showEvent(QShowEvent* e)
{
	QSettings settings;
	setTileSize(settings.value("UI/TileViewWidget-TileSize", d->tilesCurrentSize).toSize());
	if (!settings.value("UI/TileViewWidget-LeftPanelVisible", d->leftPanelVisible).toBool())
		d->hideLeftPanelButton->click();
	if (settings.value("UI/TileViewWidget-RightPanelVisible", d->rightPanelVisible).toBool())
		d->userListButton->click();
}

void TileViewWidget::hideEvent(QHideEvent* e)
{
	QSettings settings;
	settings.setValue("UI/TileViewWidget-TileSize", d->tilesCurrentSize);
	settings.setValue("UI/TileViewWidget-LeftPanelVisible", d->leftPanelVisible);
	settings.setValue("UI/TileViewWidget-RightPanelVisible", d->rightPanelVisible);
}

void TileViewWidget::onClientEnabledVideo(const ClientEntity& c)
{
	addClient(c, ChannelEntity());
}

void TileViewWidget::onClientDisabledVideo(const ClientEntity& c)
{
	auto w = d->tilesMap.value(c.id);
	if (w)
	{
		w->_videoWidget->videoWidget()->setFrame(QImage());
	}
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewCameraWidget::TileViewCameraWidget(QWidget* parent) :
	QFrame(parent),
	_mainLayout(nullptr),
	_cameraWidget(nullptr)
{
	ViewBase::addDropShadowEffect(this);

	auto l = new QBoxLayout(QBoxLayout::TopToBottom);
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);
	setLayout(l);

	_mainLayout = l;
}

TileViewCameraWidget::~TileViewCameraWidget()
{
}

void TileViewCameraWidget::setCamera(const QSharedPointer<QCamera>& c)
{
	_camera = c;
	_cameraWidget = new ClientCameraVideoWidget(ClientAppLogic::instance()->networkClient(), _camera, this);
	_mainLayout->addWidget(_cameraWidget, 1);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewTileWidget::TileViewTileWidget(const ClientEntity& client, QWidget* parent) :
	QFrame(parent)
{
	ViewBase::addDropShadowEffect(this);

	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	_videoWidget = ViewBase::createRemoteVideoWidget(client, this);
	_videoWidget->setToolTip(client.name);
	mainLayout->addWidget(_videoWidget);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewUserListWidget::TileViewUserListWidget(QWidget* parent) :
	QFrame(parent)
{
	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	auto filterEdit = new QLineEdit();
	filterEdit->setPlaceholderText(tr("Participants..."));
	filterEdit->setClearButtonEnabled(true);
	mainLayout->addWidget(filterEdit);

	auto listView = new QListView();
	listView->setContextMenuPolicy(Qt::CustomContextMenu);
	mainLayout->addWidget(listView);

	_listView = listView;

	QObject::connect(filterEdit, &QLineEdit::textChanged, [listView](const QString & text)
	{
		auto m = listView->model();
		if (!m)
			return;
		auto pm = qobject_cast<QSortFilterProxyModel*>(m);
		if (!pm)
			return;
		pm->setFilterWildcard(text);
	});

	QObject::connect(listView, &QWidget::customContextMenuRequested, [listView](const QPoint & point)
	{
		const auto mi = listView->indexAt(point);
		if (!mi.isValid())
		{
			return;
		}
		const auto ci = mi.data(ClientListModel::ClientEntityRole).value<ClientEntity>();

		// Create context menu.
		QMenu menu;

		// Admin actions.
		if (ClientAppLogic::instance()->networkClient()->isAdmin() && !ClientAppLogic::instance()->networkClient()->isSelf(ci))
		{
			// Kick client.
			auto kickAction = menu.addAction(QIcon(), tr("Kick client"));
			QObject::connect(kickAction, &QAction::triggered, [ci]()
			{
				const auto reply = ClientAppLogic::instance()->networkClient()->kickClient(ci.id, false);
				QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
			});
			// Ban client.
			auto banAction = menu.addAction(QIcon(), tr("Ban client"));
			QObject::connect(banAction, &QAction::triggered, [ci]()
			{
				const auto reply = ClientAppLogic::instance()->networkClient()->kickClient(ci.id, true);
				QObject::connect(reply, &QCorReply::finished, reply, &QCorReply::deleteLater);
			});
		}
		if (menu.actions().isEmpty())
		{
			auto a = menu.addAction(tr("No actions available."));
			a->setEnabled(false);
		}
		menu.exec(listView->mapToGlobal(point));
	});
}