#include "videowidget.h"

#include <QBoxLayout>

#include "humblelogging/api.h"

#include "videowidget_cpu.h"

#if defined(OCS_INCLUDE_OPENGL)
#include "videowidget_opengl.h"
#include "videowidget_qglwidget.h"
#endif

HUMBLE_LOGGER(HL, "video");

// Private ////////////////////////////////////////////////////////////

class VideoWidget::Private
{
public:
	VideoWidget::Type type;
	QWidget* frameWidget;
	VideoWidgetI* frameWidgetImpl;
};

// VideoWidget ////////////////////////////////////////////////////////

VideoWidget::VideoWidget(Type type, QWidget* parent) :
	QWidget(parent),
	d(new Private())
{
	d->type = type;

	switch (d->type)
	{
		case CPU:
		{
			auto w = new CpuVideoWidget(this);
			d->frameWidget = w;
			d->frameWidgetImpl = w;
			break;
		}
#if defined(OCS_INCLUDE_OPENGL)
		case OpenGL:
		{
			// auto w = new YuvVideoOpenGLWidget(this);
			auto w = new VideoWidgetQGLWidget(this);
			d->frameWidget = w;
			d->frameWidgetImpl = w;
			break;
		}
#endif
		default:
			HL_FATAL(HL, "Can not create VideoWidget, invalid type.");
			break;
	}

	d->frameWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
								  QSizePolicy::MinimumExpanding);

	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(d->frameWidget);
	setLayout(mainLayout);
}

VideoWidget::~VideoWidget()
{
}

void VideoWidget::setFrame(YuvFrameRefPtr frame)
{
	d->frameWidgetImpl->setFrame(frame);
}

void VideoWidget::setFrame(const QImage& frame)
{
	d->frameWidgetImpl->setFrame(frame);
}

void VideoWidget::setAvatar(const QPixmap& pm)
{
	//switch (d->type)
	//{
	//	case CPU:
	//		if (d->cpuImageImpl)
	//		{
	//			d->cpuImageImpl->setAvatar(pm);
	//		}
	//		break;
	//}
}

void VideoWidget::setText(const QString& text)
{
	//switch (d->type)
	//{
	//	case CPU:
	//		if (d->cpuImageImpl)
	//		{
	//			d->cpuImageImpl->setText(text);
	//		}
	//		break;
	//}
}