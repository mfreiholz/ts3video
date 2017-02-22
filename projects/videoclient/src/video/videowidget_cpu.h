#pragma once

#include <QWidget>

#include "videowidget.h"

class CpuVideoWidget :
	public QWidget,
	public VideoWidgetI
{
	Q_OBJECT
	class Private;
	QSharedPointer<Private> d;

public:
	CpuVideoWidget(QWidget* parent);
	virtual void setFrame(YuvFrameRefPtr frame);
	virtual void setFrame(const QImage& frame);

protected:
	virtual void paintEvent(QPaintEvent* e);
};