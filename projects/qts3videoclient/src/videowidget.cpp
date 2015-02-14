#include "videowidget_p.h"

#include <QPainter>
#include <QPaintEvent>
#include <QBoxLayout>

#include "elws.h"

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
#elif INCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT
  d->yuvWindow = nullptr;
#endif

  switch (d->type) {
    case OpenGL:
#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
      d->oglWindow = new OpenGLWindow();
      d->oglWindow->setRenderMode(OpenGLWindow::RM_CROPPED);
      d->oglWindow->setBackgroundColor(QColor(Qt::black));
      d->frameWidget = d->oglWindow->widget();
      break;
#elif INCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT
      d->yuvWindow = new YuvVideoWindowSub(nullptr);
      d->frameWidget = QWidget::createWindowContainer(d->yuvWindow, this);
      break;
#endif
    case CPU:
      d->cpuImageImpl = new VideoFrame_CpuImpl(this);
      d->frameWidget = d->cpuImageImpl;
      break;
  }

  d->frameWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

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
#elif INCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT
  delete d->yuvWindow;
#endif
}

void ClientVideoWidget::setFrame(YuvFrameRefPtr frame)
{
  switch (d->type) {
    case OpenGL:
#ifdef INCLUDE_OPENGL_VIDEOWIDGET_SUPPORT
      if (d->oglWindow) d->oglWindow->setData(frame);
      break;
#elif INCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT
      if (d->yuvWindow) d->yuvWindow->setFrame(frame);
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
      if (d->oglWindow) d->oglWindow->setData(YuvFrameRefPtr(YuvFrame::fromQImage(frame)));
      break;
#elif INCLUDE_OPENGL_VIDEOWIDGET2_SUPPORT
      if (d->yuvWindow) d->yuvWindow->setFrame(YuvFrameRefPtr(YuvFrame::fromQImage(frame)));
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
    
    // Scale and center image.
    // TODO Optimize: Only do calculation once with every resize, instead of calculating it for every image.
    if (true) {
      auto imageRect = _frameImage.rect();
      auto offset = QPoint(0, 0);
      ELWS::calcScaledAndCenterizedImageRect(rect(), imageRect, offset);
      auto scaledImage = _frameImage.scaled(imageRect.size());
      p.drawImage(offset, scaledImage, scaledImage.rect());
    }
    // Basic scale.
    else {
      p.drawImage(rect(), _frameImage);
    }
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

/*

auto surfaceRect = rect();
auto surfaceRatio = (float)surfaceRect.width() / (float)surfaceRect.height();

auto imageRect = _frameImage.rect();
auto imageRatio = (float)imageRect.width() / (float)imageRect.height();
auto scaleFactor = 1.0F;

auto x = 0, y = 0;

if (surfaceRatio < imageRatio) {
  scaleFactor = (float)surfaceRect.height() / (float)imageRect.height();
  imageRect.setWidth((float)imageRect.width() * scaleFactor);
  imageRect.setHeight((float)imageRect.height() * scaleFactor);
  x = ((float)imageRect.width() - (float)surfaceRect.width()) / 2;
} else {
  scaleFactor = (float)surfaceRect.width() / (float)imageRect.width();
  imageRect.setWidth((float)imageRect.width() * scaleFactor);
  imageRect.setHeight((float)imageRect.height() * scaleFactor);
  y = ((float)imageRect.height() - (float)surfaceRect.height()) / 2;
}

auto scaledImage = _frameImage.scaled(imageRect.size());
//p.drawImage(QPoint(-x, -y), scaledImage, scaledImage.rect());
p.drawImage(QPoint(0, 0), scaledImage, scaledImage.rect().adjusted(x, y, x, y));

*/