#pragma once

#include <QOpenGLWidget>

#include "videowidget.h"

class YuvVideoOpenGLWidget :
	public QOpenGLWidget,
	public VideoWidgetI
{
	Q_OBJECT
	class Private;
	QSharedPointer<Private> d;

public:
	YuvVideoOpenGLWidget(QWidget* parent = nullptr,
						 Qt::WindowFlags f = Qt::WindowFlags());
	virtual ~YuvVideoOpenGLWidget();
	virtual void setFrame(YuvFrameRefPtr frame);
	virtual void setFrame(const QImage& frame);

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();
};