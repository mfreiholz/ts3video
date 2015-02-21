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

class HangoutViewWidgetPrivate : public QObject
{
  Q_OBJECT

public:
  HangoutViewWidgetPrivate(HangoutViewWidget *o) :
    owner(o),
    cameraWidgetSize(160, 113),
    thumbnailWidgetSize(160, 113),
    cameraWidget(nullptr)
  {}

  void doFullViewVideoLayout();
  void doCameraLayout();
  
public:
  HangoutViewWidget *owner;

  QWidget *fullViewContainer;
  HangoutViewFullViewWidget *fullViewWidget;

  QSize thumbnailWidgetSize;
  QScrollArea *thumbnailScrollArea;
  QWidget *thumbnailContainer;
  QBoxLayout *thumbnailContainerLayout;

  QSize cameraWidgetSize;
  HangoutViewCameraWidget *cameraWidget;

  QHash<int, HangoutViewThumbnailWidget*> thumbnailWidgets; ///< Maps client-id to it's thumbnail-widget.
  QHash<int, RemoteClientVideoWidget*> videoWidgets; ///< Maps client-id to it's video-widget.
};


class HangoutViewFullViewWidget : public QFrame
{
  Q_OBJECT

public:
  HangoutViewFullViewWidget(QWidget *parent);
  virtual ~HangoutViewFullViewWidget();

private:
  QSize _aspectRatio;
};


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


class HangoutViewThumbnailWidget : public QFrame
{
  Q_OBJECT

public:
  HangoutViewThumbnailWidget(QWidget *parent);
  virtual ~HangoutViewThumbnailWidget();
  void setWidget(QWidget *w);

signals:
  void clicked();

protected:
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);

private:
  bool _mouseDown;
  QWidget *_videoWidget;
};

#endif