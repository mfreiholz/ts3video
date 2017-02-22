#ifndef VIDEOWIDGETPRIVATE_H
#define VIDEOWIDGETPRIVATE_H

#include "videowidget.h"

#include <QString>
#include <QPixmap>

#include "videolib/yuvframe.h"

#include "opengl/openglrenderthread.h"
#include "opengl/openglwindow.h"
#include "opengl2/yuvvideowindow.h"

class VideoFrame_CpuImpl;
class VideoFrame_OpenGL;

/*!
*/
class VideoWidgetPrivate
{
public:
	VideoWidgetPrivate(VideoWidget* o) :
		owner(o),
		type(VideoWidget::CPU),
		frameWidget(nullptr),
		frameWidgetImpl(nullptr),
		cpuImageImpl(nullptr),
		oglWindow(nullptr),
		yuvWindow(nullptr),
		glImageImpl(nullptr)
	{}

public:
	VideoWidget* owner;
	VideoWidget::Type type;
	QWidget* frameWidget;
	VideoWidgetI* frameWidgetImpl;

	// CPU
	VideoFrame_CpuImpl* cpuImageImpl;

	// OpenGL_ImageWidget
	VideoFrame_OpenGL* glImageImpl;

	// OpenGL_RenderThread
	OpenGLWindow* oglWindow;

	// OpenGL_WindowSurface
	YuvVideoWindowSub* yuvWindow;
};

/*!
    Video frame rendering implementation based on CPU with conversion from YUV to RGB-QImage.
*/
class VideoFrame_CpuImpl : public QWidget
{
	Q_OBJECT

public:
	VideoFrame_CpuImpl(QWidget* parent);
	void setFrame(const QImage& image);
	void setAvatar(const QPixmap& avatar);
	void setText(const QString& text);

protected:
	virtual void paintEvent(QPaintEvent* ev);

private:
	QImage _frameImage; ///< Holds the real QImage, in case we need it later.
	QPixmap _avatar;
	QString _text;
};

/*!
    Video frame rendering based on OpenGL
*/
#include <QGLWidget>
class VideoFrame_OpenGL : public QGLWidget
{
	Q_OBJECT

public:
	VideoFrame_OpenGL(QWidget* parent = 0, const QGLWidget* shareWidget = 0,
					  Qt::WindowFlags f = 0);
	void setFrame(const QImage& image);

protected:
	virtual void paintEvent(QPaintEvent* ev);

private:
	QImage _image;
};

#endif