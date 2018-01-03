#if defined(OCS_INCLUDE_OPENGL)
#pragma once

#include <QScopedPointer>
#include <QGLWidget>

#include "videowidget.h"

class VideoWidgetQGLWidget :
	public QGLWidget,
	public VideoWidgetI
{
	Q_OBJECT
	class Private;
	QScopedPointer<Private> d;

public:
	VideoWidgetQGLWidget(QWidget* parent = 0, const QGLWidget* shareWidget = 0,
						 Qt::WindowFlags = 0);
	virtual ~VideoWidgetQGLWidget();
	virtual void setFrame(YuvFrameRefPtr frame);
	virtual void setFrame(const QImage& frame);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
};

#endif
