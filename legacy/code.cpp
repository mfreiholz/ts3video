//switch (d->type)
//{
//	case CPU:
//		d->cpuImageImpl = new VideoFrame_CpuImpl(this);
//		d->frameWidget = d->cpuImageImpl;
//		break;
//	case OpenGL_ImageWidget:
//		d->glImageImpl = new VideoFrame_OpenGL(this);
//		d->frameWidget = d->glImageImpl;
//		break;
//	case OpenGL_RenderThread:
//		d->oglWindow = new OpenGLWindow(nullptr);
//		d->oglWindow->setRenderMode(OpenGLWindow::RM_CROPPED);
//		d->oglWindow->setBackgroundColor(QColor(Qt::black));
//		d->frameWidget = d->oglWindow->widget();
//		break;
//	case OpenGL_WindowSurface:
//		d->yuvWindow = new YuvVideoWindowSub(nullptr);
//		d->frameWidget = QWidget::createWindowContainer(d->yuvWindow, this);
//		break;
//	case OpenGL_YuvWidget:
//		d->glVideoWidget = new GLVideoWidget(this);
//		d->frameWidget = d->glVideoWidget;
//		break;
//}

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

VideoFrame_CpuImpl::VideoFrame_CpuImpl(QWidget* parent) :
	QWidget(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
}

void VideoFrame_CpuImpl::setFrame(const QImage& image)
{
	_frameImage = image;
	update();
}

void VideoFrame_CpuImpl::setAvatar(const QPixmap& avatar)
{
	_avatar = avatar;
	update();
}

void VideoFrame_CpuImpl::setText(const QString& text)
{
	_text = text;
	update();
}

void VideoFrame_CpuImpl::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	// Paint background.
	if (_frameImage.isNull())
	{
		p.setPen(Qt::black);
		p.fillRect(rect(), Qt::SolidPattern);
	}

	// Paint frame.
	if (!_frameImage.isNull())
	{
		// Scale and center image.
		// TODO Optimize: Only do calculation once with every resize, instead of calculating it for every image.
		if (true)
		{
			auto imageRect = _frameImage.rect();
			auto offset = QPoint(0, 0);
			ELWS::calcScaledAndCenterizedImageRect(rect(), imageRect, offset);
			auto scaledImage = _frameImage.scaled(imageRect.size());
			p.drawImage(QPoint(-offset.x(), -offset.y()), scaledImage, scaledImage.rect());
			//p.drawImage(offset, scaledImage, scaledImage.rect());
		}
		// Basic scale.
		else
		{
			p.drawImage(rect(), _frameImage);
		}
	}

	// Bottom area.
	const int bottomAreaHeight = 30;
	const int bottomAvatarWidth = 30;
	const QRect bottomRect(rect().x(), rect().height() - bottomAreaHeight,
						   rect().width(), bottomAreaHeight);

	if (!_avatar.isNull() || !_text.isEmpty())
	{
		p.setPen(Qt::black);
		p.setOpacity(0.5);
		p.fillRect(bottomRect, Qt::SolidPattern);
		p.setOpacity(1.0);
	}

	const QRect avatarRect(bottomRect.x(), bottomRect.y(), bottomAvatarWidth,
						   bottomAreaHeight);
	if (!_avatar.isNull())
	{
		p.setOpacity(0.8);
		p.drawPixmap(avatarRect, _avatar);
		p.setOpacity(1.0);
	}

	const QRect textRect = bottomRect.adjusted(bottomAvatarWidth + 5, 0, 0, 0);
	if (!_text.isEmpty())
	{
		const QFontMetrics fmetrics = fontMetrics();
		p.setPen(Qt::white);
		p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
				   fmetrics.elidedText(_text, Qt::ElideRight, textRect.width()));
	}

	// Painter border around the entire rect.
	//const QRect borderRect = rect().adjusted(0, 0, -1, -1);
	//p.setPen(Qt::darkGray);
	//p.drawRect(borderRect);
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ...
///////////////////////////////////////////////////////////////////////////////
/*
    #ifdef WIN32
    #include <Windows.h>
    #endif
    #include "QtCore/QTime"
    #include "udp_video_frame_encoder.h"

    void test_encode_decode()
    {
    VideoFrameUdpEncoder encoder(500, 3);
    VideoFrameUdpDecoder decoder;

    QTime t1;
    t1.start();

    unsigned long long frame_time = 1;
    for (;;) {
    // Create VideoFrame.
    UdpVideoFrame frame;
    frame._id = frame_time++;
    frame._data.reserve(4535);
    for (int i = 0; i < 4535; ++i) {
      frame._data.append("A");
    }

    // Encode
    std::vector<UdpVideoFrameDatagram*> parts;
    encoder.encode(frame, parts);

    // Decode.
    int x = 0;
    for (auto i = parts.begin(); i != parts.end(); ++i) {
      if (x++ % 3 == 0) {
        delete (*i);
        (*i) = nullptr;
        continue;
      }
      decoder.add(*i);
    }

    // Insert frames with a rate of 100fps/100ms
    #ifdef WIN32
    Sleep(10);
    #endif

    // Read next frame with 15fps/66ms from decoder.
    if (t1.elapsed() < 66) {
      continue;
    }

    UdpVideoFrame *vf = nullptr;
    if ((vf = decoder.next()) == nullptr) {
      fprintf(stderr, "no frame available!\n");
      continue;
    }

    fprintf(stdout, "done frame (time_ns=%d; data.size=%d)\n", vf->_id, vf->_data.size());
    delete vf;

    t1.restart();
    }
    }*/
