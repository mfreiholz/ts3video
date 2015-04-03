#ifndef TILEVIEWWIDGETPRIVATE
#define TILEVIEWWIDGETPRIVATE

#include "tileviewwidget.h"

#include <QSize>
#include <QHash>
#include <QObject>
#include <QFrame>

class QPushButton;
class QLabel;
class FlowLayout;
class RemoteClientVideoWidget;
class TileViewTileWidget;
class TileViewCameraWidget;
class TileViewUserListWidget;

///////////////////////////////////////////////////////////////////////

class TileViewWidgetPrivate : public QObject
{
  Q_OBJECT

public:
  TileViewWidgetPrivate(TileViewWidget *o) :
    owner(o),
    tilesLayout(nullptr),
    tilesAspectRatio(4, 3),
    tilesCurrentSize(tilesAspectRatio),
    cameraWidget(nullptr),
    zoomInButton(nullptr),
    zoomOutButton(nullptr)
  {}

public:
  TileViewWidget *owner;

  FlowLayout *tilesLayout;
  FlowLayout *noVideoTilesLayout;
  QSize tilesAspectRatio;
  QSize tilesCurrentSize;
  TileViewUserListWidget *userListWidget;
  TileViewCameraWidget *cameraWidget;
  QPushButton *zoomInButton;
  QPushButton *zoomOutButton;
  QPushButton *userListButton;
  QLabel *userCountLabel;
  QLabel *bandwidthRead;
  QLabel *bandwidthWrite;

  QHash<int, TileViewTileWidget*> tilesMap; ///< Maps client's ID to it's widget.
};

///////////////////////////////////////////////////////////////////////

class TileViewCameraWidget : public QFrame
{
  Q_OBJECT
  friend class TileViewWidget;

public:
  TileViewCameraWidget(QWidget *parent = nullptr);
  ~TileViewCameraWidget();
  void setWidget(QWidget *w);

private:
  QWidget *_widget;
};

///////////////////////////////////////////////////////////////////////

class TileViewTileWidget : public QFrame
{
  Q_OBJECT
  friend class TileViewWidget;

public:
  TileViewTileWidget(const ClientEntity &client, QWidget *parent = nullptr);

private:
  RemoteClientVideoWidget *_videoWidget;
};

///////////////////////////////////////////////////////////////////////

class TileViewUserListWidget : public QFrame
{
  Q_OBJECT
  friend class TileViewWidget;

public:
  TileViewUserListWidget(QWidget *parent = nullptr);

private:
  class QListView *_listView;
};

#endif