#ifndef VIDEOWIDGETPRIVATE_H
#define VIDEOWIDGETPRIVATE_H

#include "videowidget.h"

#include <QString>
#include <QPixmap>

#include "yuvframe.h"

#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
#include "opengl/openglrenderthread.h"
#include "opengl/openglwindow.h"
#elif INCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT
#include "opengl2/yuvvideowindow.h"
#endif

class VideoFrame_CpuImpl;

/*!
 */
class VideoWidgetPrivate
{
public:
  VideoWidgetPrivate(ClientVideoWidget *o) : owner(o) {}

public:
  ClientVideoWidget *owner;
  ClientVideoWidget::Type type;
  QWidget *frameWidget;

  // Implementations for the "this->frameWidget".
  VideoFrame_CpuImpl *cpuImageImpl;
#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
  OpenGLWindow *oglWindow;
#elif INCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT
  YuvVideoWindowSub *yuvWindow;
#endif
};

/*!
  Video frame rendering implementation based on CPU with conversion from YUV to RGB-QImage.
 */
class VideoFrame_CpuImpl : public QWidget
{
  Q_OBJECT

public:
  VideoFrame_CpuImpl(QWidget *parent);
  void setFrame(const QImage &image);
  void setAvatar(const QPixmap &avatar);
  void setText(const QString &text);

protected:
  virtual void paintEvent(QPaintEvent *ev);

private:
  QImage _frameImage; ///< Holds the real QImage, in case we need it later.
  QPixmap _avatar;
  QString _text;
};

#endif