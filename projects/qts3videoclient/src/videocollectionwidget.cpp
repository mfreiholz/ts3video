#include <QGridLayout>
#include "videocollectionwidget.h"

VideoCollectionWidget::VideoCollectionWidget(QWidget *parent) :
  QWidget(parent),
  _columnCount(3)
{
}

void VideoCollectionWidget::setWidgets(const QList<QWidget *> widgets)
{
  _widgets = widgets;
  doGridLayout();
}

void VideoCollectionWidget::doGridLayout()
{
  auto l = new QGridLayout(this);
  l->setContentsMargins(0, 0, 0, 0);
  l->setSpacing(0);
  setLayout(l);

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
