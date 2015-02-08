#include "videowidget_p.h"

#include <QPainter>
#include <QPaintEvent>
#include <QBoxLayout>

///////////////////////////////////////////////////////////////////////

ClientVideoWidget::ClientVideoWidget(Type type, QWidget *parent) :
  QWidget(parent),
  d(new VideoWidgetPrivate(this))
{
  d->type = type;
  d->frameWidget = nullptr;
  d->cpuImageImpl = nullptr;
#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
  d->oglWindow = nullptr;
#endif

  switch (d->type) {
    case OpenGL:
#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
      d->oglWindow = new OpenGLWindow();
      d->oglWindow->setRenderMode(OpenGLWindow::RM_CROPPED);
      d->oglWindow->setBackgroundColor(QColor(Qt::black));
      d->frameWidget = d->oglWindow->widget();
      break;
#endif
    case CPU:
      d->cpuImageImpl = new VideoFrame_CpuImpl(this);
      d->frameWidget = d->cpuImageImpl;
      break;
  }

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(d->frameWidget);
  setLayout(mainLayout);
}

ClientVideoWidget::~ClientVideoWidget()
{
#if INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
  delete d->oglWindow;
#endif
}

void ClientVideoWidget::setFrame(YuvFrameRefPtr frame)
{
  switch (d->type) {
    case OpenGL:
#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
      d->oglWindow->setData(frame);
      break;
#endif
    case CPU:
      if (d->cpuImageImpl) {
        auto image = frame->toQImage();
        d->cpuImageImpl->setFrame(image);
      }
      break;
  }
}

void ClientVideoWidget::setFrame(const QImage &frame)
{
  switch (d->type) {
    case OpenGL:
#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
      d->oglWindow->setData(YuvFrameRefPtr(YuvFrame::fromQImage(frame)));
      break;
#endif
    case CPU:
      if (d->cpuImageImpl) {
        d->cpuImageImpl->setFrame(frame);
      }
      break;
  }
}

void ClientVideoWidget::setAvatar(const QPixmap &pm)
{
  if (d->cpuImageImpl) {
    d->cpuImageImpl->setAvatar(pm);
  }
}

void ClientVideoWidget::setText(const QString &text)
{
  if (d->cpuImageImpl) {
    d->cpuImageImpl->setText(text);
  }
}

///////////////////////////////////////////////////////////////////////

VideoFrame_CpuImpl::VideoFrame_CpuImpl(QWidget *parent) :
  QWidget(parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
}

void VideoFrame_CpuImpl::setFrame(const QImage &image)
{
  _frameImage = image;
  update();
}

void VideoFrame_CpuImpl::setAvatar(const QPixmap &avatar)
{
  _avatar = avatar;
  update();
}

void VideoFrame_CpuImpl::setText(const QString &text)
{
  _text = text;
  update();
}

void VideoFrame_CpuImpl::paintEvent(QPaintEvent *)
{
  QPainter p(this);

  // Paint background.
  if (_frameImage.isNull()) {
    p.setPen(Qt::black);
    p.fillRect(rect(), Qt::SolidPattern);
  }

  // Paint frame.
  if (!_frameImage.isNull()) {
    auto scaledImage = _frameImage.scaled(rect().size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    auto pixmap = QPixmap::fromImage(scaledImage);
    p.drawPixmap(pixmap.rect(), pixmap);
  }

  // Bottom area.
  const int bottomAreaHeight = 30;
  const int bottomAvatarWidth = 30;
  const QRect bottomRect(rect().x(), rect().height() - bottomAreaHeight, rect().width(), bottomAreaHeight);

  if (!_avatar.isNull() || !_text.isEmpty()) {
    p.setPen(Qt::black);
    p.setOpacity(0.5);
    p.fillRect(bottomRect, Qt::SolidPattern);
    p.setOpacity(1.0);
  }

  const QRect avatarRect(bottomRect.x(), bottomRect.y(), bottomAvatarWidth, bottomAreaHeight);
  if (!_avatar.isNull()) {
    p.setOpacity(0.8);
    p.drawPixmap(avatarRect, _avatar);
    p.setOpacity(1.0);
  }

  const QRect textRect = bottomRect.adjusted(bottomAvatarWidth + 5, 0, 0, 0);
  if (!_text.isEmpty()) {
    const QFontMetrics fmetrics = fontMetrics();
    p.setPen(Qt::white);
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, fmetrics.elidedText(_text, Qt::ElideRight, textRect.width()));
  }

  // Painter border around the entire rect.
  //const QRect borderRect = rect().adjusted(0, 0, -1, -1);
  //p.setPen(Qt::darkGray);
  //p.drawRect(borderRect);
}
