#include "videowidget_p.h"

#include <QPainter>
#include <QPaintEvent>
#include <QBoxLayout>

#include "elws.h"

///////////////////////////////////////////////////////////////////////

VideoWidget::VideoWidget(Type type, QWidget *parent) :
  QWidget(parent),
  d(new VideoWidgetPrivate(this))
{
  d->type = type;

  switch (d->type) {
  case CPU:
    d->cpuImageImpl = new VideoFrame_CpuImpl(this);
    d->frameWidget = d->cpuImageImpl;
    break;
  case OpenGL_ImageWidget:
    d->glImageImpl = new VideoFrame_OpenGL(this);
    d->frameWidget = d->glImageImpl;
    break;
  case OpenGL_RenderThread:
    d->oglWindow = new OpenGLWindow(nullptr);
    d->oglWindow->setRenderMode(OpenGLWindow::RM_CROPPED);
    d->oglWindow->setBackgroundColor(QColor(Qt::black));
    d->frameWidget = d->oglWindow->widget();
    break;
  case OpenGL_WindowSurface:
    d->yuvWindow = new YuvVideoWindowSub(nullptr);
    d->frameWidget = QWidget::createWindowContainer(d->yuvWindow, this);
    break;
  case OpenGL_YuvWidget:
    d->glVideoWidget = new GLVideoWidget(this);
    d->frameWidget = d->glVideoWidget;
    break;
  }

  if (!d->frameWidget) {
    return;
  }

  d->frameWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(d->frameWidget);
  setLayout(mainLayout);
}

VideoWidget::~VideoWidget()
{
  delete d->cpuImageImpl;
  delete d->glImageImpl;
  delete d->oglWindow;
  delete d->yuvWindow;
  delete d->glVideoWidget;
}

void VideoWidget::setFrame(YuvFrameRefPtr frame)
{
  switch (d->type) {
  case CPU:
    if (d->cpuImageImpl) {
      QImage image;
      if (!frame.isNull())
        image = frame->toQImage();
      d->cpuImageImpl->setFrame(image);
    }
    break;
  case OpenGL_ImageWidget:
    if (d->glImageImpl) {
      QImage image;
      if (!frame.isNull())
        image = frame->toQImage();
      d->glImageImpl->setFrame(image);
    }
    break;
  case OpenGL_RenderThread:
    if (d->oglWindow) {
      d->oglWindow->setData(frame);
    }
    break;
  case OpenGL_WindowSurface:
    if (d->yuvWindow) {
      d->yuvWindow->setFrame(frame);
    }
    break;
  case OpenGL_YuvWidget:
    if (d->glVideoWidget){
      d->glVideoWidget->setFrame(frame);
    }
    break;
  }
}

void VideoWidget::setFrame(const QImage &frame)
{
  switch (d->type) {
  case CPU:
    if (d->cpuImageImpl) {
      d->cpuImageImpl->setFrame(frame);
    }
    break;
  case OpenGL_ImageWidget:
    if (d->glImageImpl) {
      d->glImageImpl->setFrame(frame);
    }
    break;
  case OpenGL_RenderThread:
    if (d->oglWindow) {
      d->oglWindow->setData(YuvFrameRefPtr(YuvFrame::fromQImage(frame)));
    }
    break;
  case OpenGL_WindowSurface:
    if (d->yuvWindow) {
      d->yuvWindow->setFrame(YuvFrameRefPtr(YuvFrame::fromQImage(frame)));
    }
    break;
  case OpenGL_YuvWidget:
    if (d->glVideoWidget) {
      d->glVideoWidget->setFrame(YuvFrameRefPtr(YuvFrame::fromQImage(frame)));
    }
    break;
  }
}

void VideoWidget::setAvatar(const QPixmap &pm)
{
  switch (d->type) {
  case CPU:
    if (d->cpuImageImpl) {
      d->cpuImageImpl->setAvatar(pm);
    }
    break;
  }
}

void VideoWidget::setText(const QString &text)
{
  switch (d->type) {
  case CPU:
    if (d->cpuImageImpl) {
      d->cpuImageImpl->setText(text);
    }
    break;
  }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
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
      p.drawImage(QPoint(-offset.x(), -offset.y()), scaledImage, scaledImage.rect());
      //p.drawImage(offset, scaledImage, scaledImage.rect());
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

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

VideoFrame_OpenGL::VideoFrame_OpenGL(QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags f) :
  QGLWidget(parent, shareWidget, f)
{
}

void VideoFrame_OpenGL::setFrame(const QImage &image)
{
  _image = image;
  update();
}

void VideoFrame_OpenGL::paintEvent(QPaintEvent *)
{
  QPainter p(this);

  // Paint background.
  if (_image.isNull()) {
    p.setPen(Qt::black);
    p.fillRect(rect(), Qt::SolidPattern);
  }

  // Paint frame.
  if (!_image.isNull()) {
    // Scale and center image.
    if (true) {
      auto imageRect = _image.rect();
      auto offset = QPoint(0, 0);
      ELWS::calcScaledAndCenterizedImageRect(rect(), imageRect, offset);
      auto scaledImage = _image.scaled(imageRect.size()); ///< TODO This does cost performance on CPU!
      p.drawImage(QPoint(-offset.x(), -offset.y()), scaledImage, scaledImage.rect());
    }
    // Basic scale.
    else {
      p.drawImage(rect(), _image);
    }
  }

  p.end();
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