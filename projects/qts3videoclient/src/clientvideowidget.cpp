#include <QDebug>
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include "clientvideowidget.h"
#include "ui_clientvideowidget.h"

ClientVideoWidget::ClientVideoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ClientVideoWidget)
{
  ui->setupUi(this);
  //_frame = QPixmap::fromImage(QImage(QString(":/frame.jpg")));
  _avatar = QPixmap::fromImage(QImage(QString(":/avatar.jpg")));
  _text = QString("Manuel wir brauchen hier einen längeren Text!");

  // Handle fullscreen toggle.
  connect(ui->fullscreenButton, &QPushButton::clicked, [this] (bool checked) {
    if (checked) {
      this->showFullScreen();
    } else {
      this->showNormal();
    }
  });

  // Simple actions.
  connect(ui->exitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
}

ClientVideoWidget::~ClientVideoWidget()
{
  delete ui;
}

void ClientVideoWidget::setFrame(const QPixmap &pm)
{
  //qDebug() << QString("New pixmap (w=%1; h=%2").arg(pm.width()).arg(pm.height());
  _frame = pm;
  update();
}

void ClientVideoWidget::setImage(const QImage &image)
{
  _frame = QPixmap::fromImage(image);
  update();
}

void ClientVideoWidget::paintEvent(QPaintEvent *)
{
  QPainter p(this);

  // Paint background.
  p.setPen(Qt::black);
  p.fillRect(rect(), Qt::SolidPattern);

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

  p.setPen(Qt::black);
  p.setOpacity(0.5);
  p.fillRect(bottomRect, Qt::SolidPattern);
  p.setOpacity(1.0);

  const QRect avatarRect(bottomRect.x(), bottomRect.y(), bottomAvatarWidth, bottomAreaHeight);
  p.setOpacity(0.8);
  p.drawPixmap(avatarRect, _avatar);
  p.setOpacity(1.0);

  const QRect textRect = bottomRect.adjusted(bottomAvatarWidth + 5, 0, 0, 0);
  const QFontMetrics fmetrics = fontMetrics();
  p.setPen(Qt::white);
  p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, fmetrics.elidedText(_text, Qt::ElideRight, textRect.width()));
}
