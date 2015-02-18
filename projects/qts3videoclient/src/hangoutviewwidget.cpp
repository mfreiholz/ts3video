#include "hangoutviewwidget_p.h"

#include <QBoxLayout>
#include <QFrame>
#include <QScrollArea>

///////////////////////////////////////////////////////////////////////

HangoutViewWidget::HangoutViewWidget(QWidget *parent) :
  QWidget(parent),
  d(new HangoutViewWidgetPrivate(this))
{
  // Full view area.
  d->fullViewContainer = new QFrame();

  // Thumbnail area.
  d->thumbnailContainer = new QFrame();
  auto pal = d->thumbnailContainer->palette();
  pal.setColor(QPalette::Background, Qt::lightGray);
  d->thumbnailContainer->setPalette(pal);

  d->thumbnailScrollArea = new QScrollArea();
  d->thumbnailScrollArea->setWidget(d->thumbnailContainer);
  d->thumbnailScrollArea->setWidgetResizable(true);
  d->thumbnailScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  d->thumbnailScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(d->fullViewContainer, 1);
  mainLayout->addWidget(d->thumbnailScrollArea, 0);
  setLayout(mainLayout);
}

HangoutViewWidget::~HangoutViewWidget()
{
}

void HangoutViewWidget::setCameraWidget(QWidget *w)
{
  d->cameraWidget = w;
  d->doCameraLayout();
}

void HangoutViewWidget::addWidget(QWidget *widget)
{
  d->thumbnailWidgets.append(widget);
  d->doThumbnailLayout();
}

void HangoutViewWidget::removeWidget(QWidget *widget)
{
  d->thumbnailWidgets.removeAll(widget);
  widget->setVisible(false);
  widget->setParent(nullptr);

  d->doThumbnailLayout();
}

void HangoutViewWidget::setWidgets(const QList<QWidget*> &widgets)
{
  d->thumbnailWidgets = widgets;
  d->doThumbnailLayout();
}

void HangoutViewWidget::resizeEvent(QResizeEvent *ev)
{
  d->doCameraLayout();
}

void HangoutViewWidgetPrivate::doCameraLayout()
{

}

void HangoutViewWidgetPrivate::doThumbnailLayout()
{
  auto d = this;
  auto l = new QBoxLayout(QBoxLayout::LeftToRight);
  l->addStretch(1);
  foreach (auto w, d->thumbnailWidgets) {
    w->setFixedSize(160, 113);
    l->addWidget(w);
  }
  l->addStretch(1);

  // Delete old layout.
  if (d->thumbnailContainer->layout()) {
    delete d->thumbnailContainer->layout();
  }
  d->thumbnailContainer->setLayout(l);
}