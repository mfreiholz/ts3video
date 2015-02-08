#ifndef CLIENTVIDEOWIDGET_H
#define CLIENTVIDEOWIDGET_H

#include <QScopedPointer>

#include <QWidget>

#include "yuvframe.h"

class VideoWidgetPrivate;
class ClientVideoWidget : public QWidget
{
  Q_OBJECT
  QScopedPointer<VideoWidgetPrivate> d;

public:
  enum Type { OpenGL, CPU };

  explicit ClientVideoWidget(Type type = CPU, QWidget *parent = 0);
  ~ClientVideoWidget();

public slots:
  void setFrame(YuvFrameRefPtr frame); ///< Optimized for OpenGL type.
  void setFrame(const QImage &frame); ///< Optimized for CPU type.

  void setAvatar(const QPixmap &pm);
  void setText(const QString &text);
};

#endif // CLIENTVIDEOWIDGET_H
