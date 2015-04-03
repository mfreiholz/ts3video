#include "tileviewwidget_p.h"

#include <QSettings>
#include <QBoxLayout>
#include <QWheelEvent>
#include <QPushButton>
#include <QScrollArea>

#include "elws.h"
#include "cliententity.h"
#include "channelentity.h"
#include "networkusageentity.h"

#include "flowlayout.h"
#include "remoteclientvideowidget.h"
#include "aboutwidget.h"
#include "movablewidgetcontainer.h"

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

TileViewWidget::TileViewWidget(QWidget *parent, Qt::WindowFlags f) :
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
  d->tilesLayout->addWidget(d->cameraWidget);
  d->cameraWidget->setVisible(false);

  // Buttons.
  d->zoomInButton = new QPushButton();
  d->zoomInButton->setIcon(QIcon(":/ic_add_circle_outline_grey600_48dp.png"));
  d->zoomInButton->setIconSize(__sideBarIconSize);
  d->zoomInButton->setFlat(true);
  d->zoomInButton->setToolTip(tr("Zoom-in video"));

  d->zoomOutButton = new QPushButton();
  d->zoomOutButton->setIcon(QIcon(":/ic_remove_circle_outline_grey600_48dp.png"));
  d->zoomOutButton->setIconSize(__sideBarIconSize);
  d->zoomOutButton->setFlat(true);
  d->zoomOutButton->setToolTip(tr("Zoom-out video"));

  d->userListButton = new QPushButton();
  d->userListButton->setIcon(QIcon(":/ic_supervisor_account_grey600_48dp.png"));
  d->userListButton->setIconSize(__sideBarIconSize);
  d->userListButton->setFlat(true);
  d->userListButton->setToolTip(tr("User list"));

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

  // Layout
  auto leftPanel = new QWidget();
  leftPanel->setObjectName("leftPanelContainer");
  auto leftPanelLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  leftPanelLayout->setContentsMargins(0, 0, 0, 0);
  leftPanelLayout->setSpacing(0);
  leftPanelLayout->addWidget(d->zoomInButton);
  leftPanelLayout->addWidget(d->zoomOutButton);
  leftPanelLayout->addWidget(d->userListButton);
  leftPanelLayout->addStretch(1);
  leftPanelLayout->addWidget(bandwidthContainer);
  leftPanelLayout->addWidget(aboutButton);
  leftPanel->setLayout(leftPanelLayout);

  auto mainLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(leftPanel, 0);
  mainLayout->addWidget(scrollArea, 1);
  setLayout(mainLayout);

  // Connections.
  QObject::connect(d->zoomInButton, &QPushButton::clicked, [this]() {
    auto newSize = d->tilesCurrentSize;
    newSize += QSize(25, 25);
    newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
    setTileSize(newSize);
    d->tilesLayout->update();
  });
  QObject::connect(d->zoomOutButton, &QPushButton::clicked, [this]() {
    auto newSize = d->tilesCurrentSize;
    newSize -= QSize(25, 25);
    newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
    setTileSize(newSize);
  });
  QObject::connect(aboutButton, &QPushButton::clicked, [this]() {
    auto about = new AboutWidget(this);
    about->setWindowFlags(Qt::Dialog);
    about->show();
  });

  resize(800, 600);
  setVisible(true);
}

TileViewWidget::~TileViewWidget()
{
}

void TileViewWidget::setCameraWidget(QWidget *w)
{
  d->cameraWidget->setWidget(w);
  d->cameraWidget->setVisible(true);
}

void TileViewWidget::addClient(const ClientEntity &client, const ChannelEntity &channel)
{
  if (client.videoEnabled) {
    auto tileWidget = new TileViewTileWidget(client, this);
    tileWidget->setFixedSize(d->tilesCurrentSize);
    d->tilesLayout->addWidget(tileWidget);
    d->tilesMap.insert(client.id, tileWidget);
  }
}

void TileViewWidget::removeClient(const ClientEntity &client, const ChannelEntity &channel)
{
  auto tileWidget = d->tilesMap.value(client.id);
  if (!tileWidget) {
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
  if (!tileWidget) {
    return;
  }
  tileWidget->_videoWidget->videoWidget()->setFrame(frame);
}

void TileViewWidget::updateNetworkUsage(const NetworkUsageEntity &networkUsage)
{
  d->bandwidthRead->setText(QString("D: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthRead)));
  d->bandwidthWrite->setText(QString("U: %1").arg(ELWS::humanReadableBandwidth(networkUsage.bandwidthWrite)));
}

void TileViewWidget::setTileSize(const QSize &size)
{
  auto newSize = d->tilesAspectRatio.scaled(size, Qt::KeepAspectRatio);
  d->tilesCurrentSize = newSize;

  QList<QWidget*> widgets;
  widgets.append(d->cameraWidget);
  Q_FOREACH(auto w, d->tilesMap.values()) {
    widgets.append(w);
  }
  Q_FOREACH(auto w, widgets) {
    w->setFixedSize(newSize);
  }
}

void TileViewWidget::wheelEvent(QWheelEvent *e)
{
  if (e->modifiers() != Qt::ControlModifier) {
    e->ignore();
    return;
  }
  auto delta = e->angleDelta();
  if (delta.y() > 0) {
    auto newSize = d->tilesCurrentSize;
    newSize += QSize(25, 25);
    newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
    setTileSize(newSize);
    d->tilesLayout->update();
  }
  else if (delta.y() < 0) {
    auto newSize = d->tilesCurrentSize;
    newSize -= QSize(25, 25);
    newSize = d->tilesAspectRatio.scaled(newSize, Qt::KeepAspectRatio);
    setTileSize(newSize);
  }
}

void TileViewWidget::showEvent(QShowEvent *e)
{
  QSettings settings;
  restoreGeometry(settings.value("UI/TileViewWidget-Geometry").toByteArray());
  setTileSize(settings.value("UI/TileViewWidget-TileSize", d->tilesCurrentSize).toSize());
}

void TileViewWidget::closeEvent(QCloseEvent *e)
{
  QSettings settings;
  settings.setValue("UI/TileViewWidget-Geometry", saveGeometry());
  settings.setValue("UI/TileViewWidget-TileSize", d->tilesCurrentSize);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewCameraWidget::TileViewCameraWidget(QWidget *parent) :
  QFrame(parent),
  _widget(nullptr)
{
  ViewBase::addDropShadowEffect(this);
}

TileViewCameraWidget::~TileViewCameraWidget()
{
  if (_widget) {
    _widget->setVisible(false);
    _widget->setParent(nullptr);
  }
}

void TileViewCameraWidget::setWidget(QWidget *w)
{
  if (_widget) {
    _widget->setParent(nullptr);
    _widget->setVisible(false);
  }
  _widget = w;
  _widget->setParent(this);
  _widget->setVisible(true);

  auto l = new QBoxLayout(QBoxLayout::TopToBottom);
  l->setContentsMargins(0, 0, 0, 0);
  l->setSpacing(0);
  l->addWidget(_widget);
  setLayout(l);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewTileWidget::TileViewTileWidget(const ClientEntity &client, QWidget *parent) :
  QFrame(parent)
{
  ViewBase::addDropShadowEffect(this);

  _videoWidget = ViewBase::createRemoteVideoWidget(client, this);

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(_videoWidget);
  setLayout(mainLayout);
}
