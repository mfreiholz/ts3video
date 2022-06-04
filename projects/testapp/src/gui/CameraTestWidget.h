#pragma once
#include <QWidget>
#include "ui_CameraTestWidget.h"
#include <QCameraInfo>
#include <QAbstractVideoSurface>
#include <QImage>

class CameraTestWidget :public QWidget
{
	Q_OBJECT
public:
	CameraTestWidget(QWidget* parent, Qt::WindowFlags f = 0);
	~CameraTestWidget() override;

private:
	QCameraInfo currentCameraInfo() const;
	QSize currentResolution() const;

	Q_SLOT void onCamInfoChanged(int index);
	Q_SLOT void onApplyClicked();

private:
	Ui_CameraTestWidgetForm _ui;
};

class MyVideoWidget : public QWidget
{
	Q_OBJECT
public:
	MyVideoWidget(QWidget* parent);
	~MyVideoWidget() override;

	Q_SLOT void setImage(const QImage& image);

protected:
	void paintEvent(QPaintEvent* e) override;

private:
	QImage _currentImage;
};

class MyVideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT
public:
	MyVideoSurface(const QCameraInfo& camInfo, QObject* parent);
	~MyVideoSurface() override;

	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;
	bool start(const QVideoSurfaceFormat& format) override;
	bool present(const QVideoFrame& frame) override;

	Q_SIGNAL void newImage(const QImage&);

private:
	QCameraInfo _camInfo;
	bool _mirrored = false;
};