#ifndef GLVIDEOWIDGET_H
#define GLVIDEOWIDGET_H

#include <QScopedPointer>
#include <QGLWidget>

#include "yuvframe.h"

class GLVideoWidgetPrivate;
class GLVideoWidget : public QGLWidget
{
  Q_OBJECT
  friend class GLVideoWidgetPrivate;
  QScopedPointer<GLVideoWidgetPrivate> d;

public:
  GLVideoWidget(QWidget *parent = 0, const QGLWidget *shareWidget = 0, Qt::WindowFlags = 0);
  virtual ~GLVideoWidget();

public slots:
  void setFrame(YuvFrameRefPtr frame);

protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
};

#endif