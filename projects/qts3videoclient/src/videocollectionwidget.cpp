#include <QGridLayout>
#include "videocollectionwidget.h"

VideoCollectionWidget::VideoCollectionWidget(QWidget *parent) :
  QWidget(parent),
  _columnCount(3)
{
  QPalette pal(palette());
  pal.setColor(QPalette::Background, Qt::black);
  setAutoFillBackground(true);
  setPalette(pal);
}

VideoCollectionWidget::~VideoCollectionWidget()
{
  // We don't want to delete the widgets, so we remove the parent reference.
  for (auto i = 0; i < _widgets.size(); ++i) {
    _widgets[i]->setParent(nullptr);
  }
}

void VideoCollectionWidget::addWidget(QWidget *widget)
{
  _widgets.append(widget);
  doGridLayout();
}

void VideoCollectionWidget::removeWidget(QWidget *widget)
{
  _widgets.removeAll(widget);
  doGridLayout();
}

void VideoCollectionWidget::setWidgets(const QList<QWidget *> widgets)
{
  _widgets = widgets;
  doGridLayout();
}

void VideoCollectionWidget::doGridLayout()
{
  // Prepare layout.
  auto baseLayout = layout();
  if (!baseLayout) {
    baseLayout = new QGridLayout(this);
    baseLayout->setContentsMargins(6, 6, 6, 6);
    baseLayout->setSpacing(6);
    setLayout(baseLayout);
  }
  auto l = dynamic_cast<QGridLayout*>(baseLayout);

  // Remove existing widgets from layout.
  for (auto i = 0; i < l->count(); ++i) {
    auto li = l->itemAt(i);
    auto w = li->widget();
    w->setParent(nullptr);
    l->removeWidget(w);
  }

  // Add new widgets to layout.
  auto ri = 0;
  auto ci = 0;
  for (auto i = 0; i < _widgets.size(); ++i) {
    auto w = _widgets.at(i);
    l->addWidget(w, ri, ci);

    // Calc next cell index.
    ci++;
    if (ci >= _columnCount) {
      ri++;
      ci = 0;
    }
  }
}
