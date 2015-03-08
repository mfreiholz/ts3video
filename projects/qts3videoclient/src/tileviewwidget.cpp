#include "tileviewwidget_p.h"

#include <QSettings>
#include <QBoxLayout>
#include <QWheelEvent>
#include <QPushButton>
#include <QScrollArea>

#include "cliententity.h"
#include "channelentity.h"

#include "flowlayout.h"
#include "remoteclientvideowidget.h"
#include "aboutwidget.h"

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
  auto scrollAreaContent = new QWidget();
  pal = scrollAreaContent->palette();
  pal.setColor(QPalette::Background, __darkBackgroundColor);
  scrollAreaContent->setPalette(pal);

  auto scrollArea = new QScrollArea();
  scrollArea->setFrameStyle(QFrame::NoFrame);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scrollArea->setWidget(scrollAreaContent);
  scrollArea->setStyleSheet(
      "QScrollBar { background: rgb(30, 30, 30); }"
      "QScrollBar:vertical { width: 10px; }"
      "QScrollBar::add-line, QScrollBar::sub-line { background: none; }"
      "QScrollBar::add-page, QScrollBar::sub-page { background: none; }"
      "QScrollBar::handle { background: rgb(63, 63, 70); border: 1px solid rgb(63, 63, 70); }"
    );

  d->tilesLayout = new FlowLayout(nullptr, 6, 6, 6);
  d->tilesLayout->setContentsMargins(6, 6, 6, 6);
  d->tilesLayout->setSpacing(6);
  scrollArea->widget()->setLayout(d->tilesLayout);

  // Camera.
  d->cameraWidget = new TileViewCameraWidget();
  d->cameraWidget->setFixedSize(d->tilesCurrentSize);
  d->tilesLayout->addWidget(d->cameraWidget);

  // Buttons.
  d->zoomInButton = new QPushButton();
  d->zoomInButton->setIcon(QIcon(":/ic_add_circle_outline_grey600_48dp.png"));
  d->zoomInButton->setIconSize(QSize(24, 24));
  d->zoomInButton->setFlat(true);
  d->zoomInButton->setToolTip(tr("Zoom-in video"));

  d->zoomOutButton = new QPushButton();
  d->zoomOutButton->setIcon(QIcon(":/ic_remove_circle_outline_grey600_48dp.png"));
  d->zoomOutButton->setIconSize(QSize(24, 24));
  d->zoomOutButton->setFlat(true);
  d->zoomOutButton->setToolTip(tr("Zoom-out video"));

  auto aboutButton = new QPushButton();
  aboutButton->setIcon(QIcon(":/ic_info_outline_grey600_48dp.png"));
  aboutButton->setIconSize(QSize(24, 24));
  aboutButton->setFlat(true);

  // Layout
  auto buttonLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  buttonLayout->addWidget(d->zoomInButton);
  buttonLayout->addWidget(d->zoomOutButton);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(aboutButton);

  auto mainLayout = new QBoxLayout(QBoxLayout::LeftToRight);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addLayout(buttonLayout, 0);
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