#pragma once

#include <QScopedPointer>
#include <QWidget>
class QImage;

#include "libapp/yuvframe.h"


class VideoWidgetI
{
public:
	virtual void setFrame(YuvFrameRefPtr frame) = 0;
	virtual void setFrame(const QImage& frame) = 0;
};


class VideoWidget
	: public QWidget
{
	Q_OBJECT
	class Private;
	QScopedPointer<Private> d;

public:
	enum Type { CPU, OpenGL };

	explicit VideoWidget(Type type = CPU, QWidget* parent = 0);
	virtual ~VideoWidget();

public slots:
	void setFrame(YuvFrameRefPtr frame);
	void setFrame(const QImage& frame);

	void setAvatar(const QPixmap& pm);
	void setText(const QString& text);
};