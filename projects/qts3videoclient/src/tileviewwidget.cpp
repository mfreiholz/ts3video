#include "tileviewwidget_p.h"

#include <QBoxLayout>

#include "cliententity.h"
#include "channelentity.h"

#include "flowlayout.h"
#include "remoteclientvideowidget.h"

///////////////////////////////////////////////////////////////////////

static QColor __lightBackgroundColor(45, 45, 48);
static QColor __darkBackgroundColor(30, 30, 30);
static QColor __frameColor(63, 63, 70);
static QColor __frameColorActive(0, 122, 204);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewWidget::TileViewWidget(QWidget *parent, Qt::WindowFlags f) :
  QWidget(parent),
  d(new TileViewWidgetPrivate(this))
{
  auto pal = palette();
  pal.setColor(QPalette::Background, __darkBackgroundColor);
  setPalette(pal);

  d->tilesCurrentSize.scale(200, 200, Qt::KeepAspectRatio);

  // Tiles.
  d->tilesLayout = new FlowLayout(this, 6, 6, 6);
  d->tilesLayout->setContentsMargins(6, 6, 6, 6);
  d->tilesLayout->setSpacing(6);

  // Camera.
  d->cameraWidget = new TileViewCameraWidget(this);
  d->cameraWidget->setFixedSize(d->tilesCurrentSize);
  d->tilesLayout->addWidget(d->cameraWidget);

  // Layout
  setLayout(d->tilesLayout);
}

TileViewWidget::~TileViewWidget()
{
}

void TileViewWidget::setCameraWidget(QWidget *w)
{
  d->cameraWidget->setWidget(w);
}

void TileViewWidget::addClient(const ClientEntity &client, const ChannelEntity &channel)
{
  auto tileWidget = new TileViewTileWidget(client, this);
  tileWidget->setFixedSize(d->tilesCurrentSize);

  // Add to layout.
  d->tilesLayout->addWidget(tileWidget);

  // Mappings.
  d->tilesMap.insert(client.id, tileWidget);
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

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

TileViewCameraWidget::TileViewCameraWidget(QWidget *parent) :
  QFrame(parent),
  _widget(nullptr)
{
  auto pal = palette();
  pal.setColor(QPalette::Foreground, __frameColor);
  setPalette(pal);
  setAutoFillBackground(true);
  setFrameShape(QFrame::Box);
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
  auto pal = palette();
  pal.setColor(QPalette::Foreground, __frameColor);
  setPalette(pal);
  setAutoFillBackground(true);
  setFrameShape(QFrame::Box);
  ViewBase::addDropShadowEffect(this);

  _videoWidget = ViewBase::createRemoteVideoWidget(client, this);

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(_videoWidget);
  setLayout(mainLayout);
}