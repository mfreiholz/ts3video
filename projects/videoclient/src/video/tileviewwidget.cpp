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

#include "videolib/elws.h"
#include "videolib/cliententity.h"
#include "videolib/channelentity.h"
#include "videolib/networkusageentity.h"

#include "conferencevideowindow.h"
#include "clientcameravideowidget.h"
#include "flowlayout.h"
#include "remoteclientvideowidget.h"
#include "aboutwidget.h"
#include "movablewidgetcontainer.h"
#include "adminauthwidget.h"
#include "networkclient/clientlistmodel.h"

HUMBLE_LOGGER(HL, "gui.tileview");

///////////////////////////////////////////////////////////////////////

TileViewWidget::TileViewWidget(ConferenceVideoWindow* window,
							   Qt::WindowFlags f) :
	QWidget(window),
	d(new TileViewWidgetPrivate(this))
{
	d->window = window;
	d->tilesCurrentSize.scale(640, 360, Qt::KeepAspectRatio);
	const auto iconSize = fontMetrics().height() * 2;

	auto mainLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	// Central scroll area
	auto scrollArea = new QScrollArea(this);
	scrollArea->setFrameStyle(QFrame::NoFrame);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	mainLayout->addWidget(scrollArea, 1);

	auto scrollAreaContent = new QWidget(this);
	auto scrollAreaContentLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	scrollAreaContent->setLayout(scrollAreaContentLayout);
	scrollArea->setWidget(scrollAreaContent);

	// Options on top (zoom, ...)
	if (true)
	{
		auto topButtonsWidget = new QWidget();
		auto topButtonsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
		topButtonsLayout->setContentsMargins(0, 0, 0, 0);
		topButtonsLayout->setSpacing(0);
		topButtonsWidget->setLayout(topButtonsLayout);
		scrollAreaContentLayout->addWidget(topButtonsWidget);

		d->zoomInButton = new QPushButton();
		d->zoomInButton->setIcon(QIcon(":/ic_add_circle_outline_grey600_48dp.png"));
		d->zoomInButton->setIconSize(QSize(iconSize, iconSize));
		d->zoomInButton->setFlat(true);
		d->zoomInButton->setToolTip(tr("Increase video size (CTRL+Mousewheel-Up)"));
		topButtonsLayout->addWidget(d->zoomInButton);

		d->zoomOutButton = new QPushButton();
		d->zoomOutButton->setIcon(QIcon(":/ic_remove_circle_outline_grey600_48dp.png"));
		d->zoomOutButton->setIconSize(QSize(iconSize, iconSize));
		d->zoomOutButton->setFlat(true);
		d->zoomOutButton->setToolTip(tr("Decrease video size (CTRL+Mousewheel-Down)"));
		topButtonsLayout->addWidget(d->zoomOutButton);

		topButtonsLayout->addStretch(1);

		QObject::connect(d->zoomInButton, &QPushButton::clicked, this,
						 &TileViewWidget::increaseTileSize);
		QObject::connect(d->zoomOutButton, &QPushButton::clicked, this,
						 &TileViewWidget::decreaseTileSize);
	}

	// Video tiles container.
	auto tilesContainer = new MovableWidgetContainer(this, 0);
	auto tilesContainerLayout = static_cast<FlowLayout*>(tilesContainer->layout());
	tilesContainerLayout->setContentsMargins(0, 0, 0, 0);
	tilesContainer->setLayout(tilesContainerLayout);
	scrollAreaContentLayout->addWidget(tilesContainer, 1);
	d->tilesLayout = tilesContainerLayout;

	// Camera
	d->cameraWidget = new TileViewCameraWidget(this, this);
	d->cameraWidget->setFixedSize(d->tilesCurrentSize);
	d->cameraWidget->setVisible(false);
	d->tilesLayout->addWidget(d->cameraWidget);

	// Window events
	QObject::connect(d->window, &ConferenceVideoWindow::cameraChanged, this,
					 &TileViewWidget::onCameraChanged);

	// Network events
	auto nc = d->window->networkClient();
	QObject::connect(nc.data(), &NetworkClient::clientEnabledVideo, this,
					 &TileViewWidget::onClientEnabledVideo);
	QObject::connect(nc.data(), &NetworkClient::clientDisabledVideo, this,
					 &TileViewWidget::onClientDisabledVideo);
}

TileViewWidget::~TileViewWidget()
{
	if (d->camera)
	{
		d->camera->disconnect(this);
		d->camera.clear();
	}
}

ConferenceVideoWindow* TileViewWidget::window() const
{
	return d->window;
}

void TileViewWidget::addClient(const ClientEntity& client,
							   const ChannelEntity& channel)
{
	if (client.videoEnabled && !d->tilesMap.contains(client.id))
	{
		auto tileWidget = new TileViewTileWidget(this, client, this);
		tileWidget->setFixedSize(d->tilesCurrentSize);
		d->tilesLayout->addWidget(tileWidget);
		d->tilesMap.insert(client.id, tileWidget);
	}
}

void TileViewWidget::removeClient(const ClientEntity& client,
								  const ChannelEntity& channel)
{
	auto tileWidget = d->tilesMap.value(client.id);
	if (!tileWidget)
		return;

	// Mappings
	d->tilesMap.remove(client.id);

	// Remove from layout.
	tileWidget->setVisible(false);
	d->tilesLayout->removeWidget(tileWidget);
	delete tileWidget;
}

void TileViewWidget::updateClientVideo(YuvFrameRefPtr frame,
									   ocs::clientid_t senderId)
{
	auto tileWidget = d->tilesMap.value(senderId);
	if (!tileWidget)
	{
		//ClientEntity ce;
		//ce.id = senderId;
		//ce.videoEnabled = true;
		//addClient(ce, ChannelEntity());
		//tileWidget = d->tilesMap.value(senderId);
		//if (!tileWidget)
		return;
	}
	tileWidget->_videoWidget->videoWidget()->setFrame(frame);
}

void TileViewWidget::increaseTileSize()
{
	QSettings settings;
	auto maxSize = this->rect().size() - QSize(30, 30);

	auto newSize = d->tilesCurrentSize + QSize(25, 25);
	if (newSize.width() > maxSize.width() || newSize.height() > maxSize.height())
	{
		newSize = maxSize;
	}
	setTileSize(newSize);
}

void TileViewWidget::decreaseTileSize()
{
	QSettings settings;
	auto minSize = d->tilesAspectRatio.scaled(QSize(300, 300), Qt::KeepAspectRatio);

	auto newSize = d->tilesCurrentSize - QSize(25, 25);
	if (newSize.width() < minSize.width() || newSize.height() < minSize.height())
	{
		newSize = minSize;
	}
	setTileSize(newSize);
}

void TileViewWidget::setTileSize(const QSize& size)
{
	auto newSize = d->tilesAspectRatio.scaled(size, Qt::KeepAspectRatio);
	d->tilesCurrentSize = newSize;

	QList<QWidget*> widgets;
	widgets.append(d->cameraWidget);
	foreach (auto w, d->tilesMap.values())
	{
		widgets.append(w);
	}
	foreach (auto w, widgets)
	{
		w->setFixedSize(newSize);
	}
	d->tilesLayout->update();
}

#if defined(OCS_INCLUDE_AUDIO)
void TileViewWidget::setAudioInputEnabled(bool b)
{
	auto nc = ConferenceVideoWindow::instance()->networkClient();
	auto dev = ConferenceVideoWindow::instance()->audioInput();
	if (dev)
	{
		if (b)
		{
			if (dev->state() != QAudio::ActiveState)
			{
				//dev->start();
			}
			if (nc)
			{
				auto reply = nc->enableAudioInputStream();
				QCORREPLY_AUTODELETE(reply);
			}
		}
		else
		{
			if (dev->state() == QAudio::ActiveState)
			{
				//dev->stop();
			}
			if (nc)
			{
				auto reply = nc->disableAudioInputStream();
				QCORREPLY_AUTODELETE(reply);
			}
		}
	}
	d->enableAudioInputToggleButton->setChecked(b);
}
#endif

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
		this->increaseTileSize();
	}
	else if (delta.y() < 0)
	{
		this->decreaseTileSize();
	}
}

void TileViewWidget::showEvent(QShowEvent* e)
{
	QSettings settings;
	setTileSize(settings.value("UI/TileViewWidget-TileSize",
							   d->tilesCurrentSize).toSize());
}

void TileViewWidget::hideEvent(QHideEvent* e)
{
	QSettings settings;
	settings.setValue("UI/TileViewWidget-TileSize", d->tilesCurrentSize);
}

void TileViewWidget::onClientEnabledVideo(const ClientEntity& c)
{
	addClient(c, ChannelEntity());
}

void TileViewWidget::onClientDisabledVideo(const ClientEntity& c)
{
	//auto w = d->tilesMap.value(c.id);
	//if (w)
	//{
	//	auto yuv = YuvFrameRefPtr(YuvFrame::createBlackImage(10, 10));
	//	w->_videoWidget->videoWidget()->setFrame(yuv->toQImage());
	//}
	removeClient(c, ChannelEntity());
}

void TileViewWidget::onCameraChanged()
{
	// Clear old camera
	if (d->camera)
	{
		d->camera->disconnect(this);
	}

	// Setup new camera
	d->camera = d->window->camera();
	if (d->camera)
	{
		QObject::connect(d->camera.data(), &QCamera::statusChanged, this,
						 &TileViewWidget::onCameraStatusChanged);
	}

	// Update UI
	//d->cameraWidget->setVisible(!d->camera.isNull());
}

void TileViewWidget::onCameraStatusChanged(QCamera::Status s)
{
	switch (s)
	{
		case QCamera::ActiveStatus:
			d->cameraWidget->setVisible(true);
			break;
		default:
			d->cameraWidget->setVisible(false);
			d->cameraWidget->_cameraWidget->setFrame(QImage());
			break;
	}
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewCameraWidget::TileViewCameraWidget(TileViewWidget* tileView,
		QWidget* parent) :
	QFrame(parent),
	_tileView(tileView),
	_mainLayout(nullptr),
	_cameraWidget(nullptr)
{
	ConferenceVideoWindow::addDropShadowEffect(this);

	_mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	_mainLayout->setContentsMargins(0, 0, 0, 0);
	_mainLayout->setSpacing(0);
	setLayout(_mainLayout);

	// Events from ConferenceVideoWindow
	auto win = _tileView->window();
	QObject::connect(win, &ConferenceVideoWindow::cameraChanged, this,
					 &TileViewCameraWidget::onCameraChanged);
}

void TileViewCameraWidget::onCameraChanged()
{
	if (_cameraWidget)
	{
		delete _cameraWidget;
		_cameraWidget = nullptr;
	}

	auto win = _tileView->window();
	auto cam = win->camera();
	if (!cam.isNull())
	{
		_cameraWidget = new ClientCameraVideoWidget(win, this);
		_mainLayout->addWidget(_cameraWidget, 1);
	}
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewTileWidget::TileViewTileWidget(TileViewWidget* tileView,
									   const ClientEntity& client, QWidget* parent) :
	QFrame(parent),
	_tileView(tileView)
{
	ConferenceVideoWindow::addDropShadowEffect(this);

	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	_videoWidget = ConferenceVideoWindow::createRemoteVideoWidget(
					   _tileView->window()->options(), client, this);
	_videoWidget->setToolTip(client.name);
	mainLayout->addWidget(_videoWidget);
}