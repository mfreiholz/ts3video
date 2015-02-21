#ifndef HANGOUTVIEWWIDGETPRIVATE_H
#define HANGOUTVIEWWIDGETPRIVATE_H

#include "hangoutviewwidget.h"

#include <QList>
#include <QSize>
#include <QFrame>

class QScrollArea;
class QBoxLayout;
class QSizeGrip;
class RemoteClientVideoWidget;
class HangoutViewFullViewWidget;
class HangoutViewCameraWidget;
class HangoutViewThumbnailWidget;

///////////////////////////////////////////////////////////////////////

class HangoutViewWidgetPrivate : public QObject
{
  Q_OBJECT

public:
  HangoutViewWidgetPrivate(HangoutViewWidget *o) :
    owner(o),
    fullViewContainer(nullptr),
    fullViewWidget(nullptr),
    fullViewClientId(-1),
    thumbnailWidgetSize(4, 3),
    thumbnailScrollArea(nullptr),
    thumbnailContainer(nullptr),
    thumbnailContainerLayout(nullptr),
    cameraWidgetSize(160, 113),
    cameraWidget(nullptr)
  {}

  void doFullViewVideoLayout();
  void doCameraLayout();
  
public:
  HangoutViewWidget *owner;

  QWidget *fullViewContainer;
  HangoutViewFullViewWidget *fullViewWidget;
  int fullViewClientId;

  QSize thumbnailWidgetSize;
  QScrollArea *thumbnailScrollArea;
  QWidget *thumbnailContainer;
  QBoxLayout *thumbnailContainerLayout;

  QSize cameraWidgetSize;
  HangoutViewCameraWidget *cameraWidget;

  QHash<int, HangoutViewThumbnailWidget*> thumbnailWidgets; ///< Maps client-id to it's thumbnail-widget.
  QHash<int, RemoteClientVideoWidget*> videoWidgets; ///< Maps client-id to it's video-widget.
};

///////////////////////////////////////////////////////////////////////

class HangoutViewFullViewWidget : public QFrame
{
  Q_OBJECT

public:
  HangoutViewFullViewWidget(QWidget *parent);
  RemoteClientVideoWidget* remoteVideoWidget() const { return _videoWidget; }

private slots:
  void createRemoteVideoWidget();

private:
  QSize _aspectRatio;
  RemoteClientVideoWidget *_videoWidget;
};

///////////////////////////////////////////////////////////////////////

class HangoutViewCameraWidget : public QFrame
{
  Q_OBJECT

public:
  HangoutViewCameraWidget(QWidget *parent);
  virtual ~HangoutViewCameraWidget();
  void setWidget(QWidget *w);

private:
  QSize _minSize;
  QSizeGrip *_sizeGrip;
  QWidget *_widget;
};

///////////////////////////////////////////////////////////////////////

class HangoutViewThumbnailWidget : public QFrame
{
  Q_OBJECT

public:
  HangoutViewThumbnailWidget(const ClientEntity &client, QWidget *parent);
  RemoteClientVideoWidget* remoteVideoWidget() const { return _videoWidget; }

signals:
  void clicked();

protected:
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);

private:
  bool _mouseDown;
  RemoteClientVideoWidget *_videoWidget;
};

///////////////////////////////////////////////////////////////////////

#endif