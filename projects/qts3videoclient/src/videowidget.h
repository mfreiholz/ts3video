#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QScopedPointer>
#include <QWidget>
#include "yuvframe.h"

class VideoWidgetPrivate;
class VideoWidget : public QWidget
{
  Q_OBJECT
  QScopedPointer<VideoWidgetPrivate> d;

public:
  enum Type {
    CPU,                      ///< Paints everything on via CPU (Converts YUV->RGB on CPU)
    OpenGL_ImageWidget,       ///< Paints everything via OpenGL (Converts YUV->RGB on CPU)
    OpenGL_RenderThread,      ///< Paints everything via OpenGL in a separate thread (Converts YUV->RGB with shader on GPU) (by mstein)
    OpenGL_WindowSurface,     ///< Paints everything via OpenGL based on QOpenGLSurface (Converts YUV->RGB with shader on GPU)
    OpenGL_YuvWidget          ///< Paints everything via OpenGL based on QGLWidget (Converts YUV->RGB with shader GPU)
  };

  explicit VideoWidget(Type type = CPU, QWidget *parent = 0);
  ~VideoWidget();

public slots:
  void setFrame(YuvFrameRefPtr frame);
  void setFrame(const QImage &frame);

  void setAvatar(const QPixmap &pm);
  void setText(const QString &text);
};

#endif