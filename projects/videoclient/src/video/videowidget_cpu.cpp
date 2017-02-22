#include "videowidget_cpu.h"

#include <QPainter>

#include "videolib/elws.h"

class CpuVideoWidget::Private
{
public:
	YuvFrameRefPtr yuvFrame;
	QImage rgbImage;
};

CpuVideoWidget::CpuVideoWidget(QWidget* parent) :
	QWidget(parent),
	VideoWidgetI(),
	d(new Private())
{
	setAttribute(Qt::WA_OpaquePaintEvent);
}

void
CpuVideoWidget::setFrame(YuvFrameRefPtr frame)
{
	d->yuvFrame = frame;
	update();
}

void
CpuVideoWidget::setFrame(const QImage& frame)
{
	d->rgbImage = frame;
	update();
}

void
CpuVideoWidget::paintEvent(QPaintEvent*)
{
	if (d->yuvFrame)
	{
		d->rgbImage = d->yuvFrame->toQImage();
		d->yuvFrame.clear();
	}

	QPainter p(this);

	// Paint background.
	if (d->rgbImage.isNull())
	{
		p.setPen(Qt::black);
		p.fillRect(rect(), Qt::SolidPattern);
	}

	// Paint frame.
	if (!d->rgbImage.isNull())
	{
		// Scale and center image.
		// TODO Optimize: Only do calculation once with every resize, instead of calculating it for every image.
		if (true)
		{
			auto imageRect = d->rgbImage.rect();
			auto offset = QPoint(0, 0);
			ELWS::calcScaledAndCenterizedImageRect(rect(), imageRect, offset);
			auto scaledImage = d->rgbImage.scaled(imageRect.size());
			p.drawImage(QPoint(-offset.x(), -offset.y()), scaledImage, scaledImage.rect());
			//p.drawImage(offset, scaledImage, scaledImage.rect());
		}
		// Basic scale.
		else
		{
			p.drawImage(rect(), d->rgbImage);
		}
	}

	//// Bottom area.
	//const int bottomAreaHeight = 30;
	//const int bottomAvatarWidth = 30;
	//const QRect bottomRect(rect().x(), rect().height() - bottomAreaHeight,
	//					   rect().width(), bottomAreaHeight);

	//if (!_avatar.isNull() || !_text.isEmpty())
	//{
	//	p.setPen(Qt::black);
	//	p.setOpacity(0.5);
	//	p.fillRect(bottomRect, Qt::SolidPattern);
	//	p.setOpacity(1.0);
	//}

	//const QRect avatarRect(bottomRect.x(), bottomRect.y(), bottomAvatarWidth,
	//					   bottomAreaHeight);
	//if (!_avatar.isNull())
	//{
	//	p.setOpacity(0.8);
	//	p.drawPixmap(avatarRect, _avatar);
	//	p.setOpacity(1.0);
	//}

	//const QRect textRect = bottomRect.adjusted(bottomAvatarWidth + 5, 0, 0, 0);
	//if (!_text.isEmpty())
	//{
	//	const QFontMetrics fmetrics = fontMetrics();
	//	p.setPen(Qt::white);
	//	p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
	//			   fmetrics.elidedText(_text, Qt::ElideRight, textRect.width()));
	//}

	// Painter border around the entire rect.
	//const QRect borderRect = rect().adjusted(0, 0, -1, -1);
	//p.setPen(Qt::darkGray);
	//p.drawRect(borderRect);
}