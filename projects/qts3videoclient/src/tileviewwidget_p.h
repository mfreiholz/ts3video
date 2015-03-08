#ifndef TILEVIEWWIDGETPRIVATE
#define TILEVIEWWIDGETPRIVATE

#include "tileviewwidget.h"

#include <QSize>
#include <QHash>
#include <QObject>
#include <QFrame>

class FlowLayout;
class RemoteClientVideoWidget;
class TileViewTileWidget;
class TileViewCameraWidget;

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
    cameraWidget(nullptr)
  {}

public:
  TileViewWidget *owner;

  FlowLayout *tilesLayout;
  QSize tilesAspectRatio;
  QSize tilesCurrentSize;
  TileViewCameraWidget *cameraWidget;

  QHash<int, TileViewTileWidget*> tilesMap; ///< Maps client's ID to it's widget.
};

///////////////////////////////////////////////////////////////////////

class TileViewCameraWidget : public QFrame
{
  Q_OBJECT
  friend class TileViewWidget;

public:
  TileViewCameraWidget(QWidget *parent);
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

#endif