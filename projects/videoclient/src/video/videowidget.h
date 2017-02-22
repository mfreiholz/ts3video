#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "videolib/yuvframe.h"

class VideoWidgetI
{
public:
	virtual void setFrame(YuvFrameRefPtr frame) = 0;
	virtual void setFrame(const QImage& frame) = 0;
};

class VideoWidgetPrivate;
class VideoWidget : public QWidget
{
	Q_OBJECT
	QScopedPointer<VideoWidgetPrivate> d;

public:
	enum Type { CPU, OpenGL };

	explicit VideoWidget(Type type = CPU, QWidget* parent = 0);
	~VideoWidget();

public slots:
	void setFrame(YuvFrameRefPtr frame);
	void setFrame(const QImage& frame);

	void setAvatar(const QPixmap& pm);
	void setText(const QString& text);
};

#endif