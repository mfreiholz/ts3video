#include "clientvideowidget.h"
#include "ui_clientvideowidget.h"

#include <QPainter>
#include <QPaintEvent>

ClientVideoWidget::ClientVideoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ClientVideoWidget)
{
  ui->setupUi(this);
  ui->videoToggleButton->setVisible(false);
  ui->fullscreenButton->setVisible(false);
  ui->exitButton->setVisible(false);
  setAttribute(Qt::WA_OpaquePaintEvent);
}

ClientVideoWidget::~ClientVideoWidget()
{
  delete ui;
}

void ClientVideoWidget::setFrame(const QPixmap &pm)
{
  _frame = pm;
  update();
}

void ClientVideoWidget::setImage(const QImage &image)
{
  _frame = QPixmap::fromImage(image);
  update();
}

void ClientVideoWidget::setAvatar(const QPixmap &pm)
{
  _avatar = pm;
  update();
}

void ClientVideoWidget::setText(const QString &text)
{
  _text = text;
  update();
}

void ClientVideoWidget::paintEvent(QPaintEvent *)
{
  QPainter p(this);

  // Paint background.
  if (_frame.isNull()) {
    p.setPen(Qt::black);
    p.fillRect(rect(), Qt::SolidPattern);
  }

  // Paint frame.
  if (!_frame.isNull()) {
    if (true) {
      const QPixmap pm = _frame.scaled(rect().size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
      p.drawPixmap(pm.rect(), pm);
    } else {
      p.drawPixmap(rect(), _frame);
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
