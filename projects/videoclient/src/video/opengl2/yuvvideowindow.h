#if defined(OCS_INCLUDE_OPENGL)
#ifndef YUVVIDEOWINDOW_H
#define YUVVIDEOWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions>

#include "videolib/yuvframe.h"

class QOpenGLContext;
class QOpenGLPaintDevice;
class QOpenGLShaderProgram;

class YuvVideoWindow : public QWindow, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  YuvVideoWindow(QWindow *parent);
  ~YuvVideoWindow();

protected:
  virtual void initialize();
  virtual void render();
  virtual void render(QPainter *painter);

protected slots:
  void renderLater();
  void renderNow();

protected:
  bool event(QEvent *ev);
  void exposeEvent(QExposeEvent *ev);

private:
  bool _updatePending;
  QOpenGLContext *_context;
  QOpenGLPaintDevice *_device;
};

/*!
 */
class YuvVideoWindowSub : public YuvVideoWindow
{
public:
  YuvVideoWindowSub(QWindow *parent);
  ~YuvVideoWindowSub();
  void setFrame(YuvFrameRefPtr frame);

protected:
  virtual void initialize() override;
  virtual void render() override;
  virtual void resizeEvent(QResizeEvent *ev) override;

private:
  QOpenGLShaderProgram *_program;
  unsigned int _textureIds[3];
  YuvFrameRefPtr _frame;
  bool _resize;
};

#endif
#endif