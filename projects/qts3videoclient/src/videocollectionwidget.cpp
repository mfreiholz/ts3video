#include "videocollectionwidget.h"

#include <limits.h>

#include <QSettings>

#include <QGridLayout>
#include <QBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

VideoCollectionWidget::VideoCollectionWidget(QWidget *parent) :
  QWidget(parent),
  _columnCount(1)
{
  auto columnCountSpinBox = new QSpinBox(this);
  columnCountSpinBox->setMinimum(1);
  columnCountSpinBox->setMaximum(std::numeric_limits<int>::max());
  _columnCountSpinBox = columnCountSpinBox;

  auto topLayout = new QBoxLayout(QBoxLayout::LeftToRight);
  topLayout->addStretch(1);
  topLayout->addWidget(new QLabel(tr("Columns:")));
  topLayout->addWidget(columnCountSpinBox);

  auto gridLayout = new QGridLayout();
  gridLayout->setContentsMargins(0, 0, 0, 0);
  gridLayout->setSpacing(9);
  _gridLayout = gridLayout;

  _mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  _mainLayout->addLayout(topLayout, 0);
  _mainLayout->addLayout(gridLayout, 1);
  setLayout(_mainLayout);

  QObject::connect(columnCountSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this, columnCountSpinBox] (int value) {
    _columnCount = columnCountSpinBox->value();
    doGridLayout();
  });
}

VideoCollectionWidget::~VideoCollectionWidget()
{
  // We don't want to delete the widgets, so we remove the parent reference.
  for (auto i = 0; i < _widgets.size(); ++i) {
    _widgets[i]->setVisible(false);
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
  widget->setVisible(false);
  doGridLayout();
}

void VideoCollectionWidget::setWidgets(const QList<QWidget *> widgets)
{
  _widgets = widgets;
  doGridLayout();
}

void VideoCollectionWidget::showEvent(QShowEvent *)
{
  QSettings settings;
  restoreGeometry(settings.value("UI/VideoCollectionWidget-Geometry").toByteArray());
  _columnCountSpinBox->setValue(settings.value("UI/VideoCollectionWidget-ColumnCount").toUInt());
}

void VideoCollectionWidget::closeEvent(QCloseEvent *)
{
  QSettings settings;
  settings.setValue("UI/VideoCollectionWidget-Geometry", saveGeometry());
  settings.setValue("UI/VideoCollectionWidget-ColumnCount", _columnCount);
}

void VideoCollectionWidget::doGridLayout()
{
  auto l = _gridLayout;

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
    prepareWidget(w);
    w->setVisible(true);

    // Calc next cell index.
    ci++;
    if (ci >= _columnCount) {
      ri++;
      ci = 0;
    }
  }
}

void VideoCollectionWidget::prepareWidget(QWidget *widget)
{
  // Note: Drop shadow causes flickering during resize events.
  //if (!widget->graphicsEffect()) {
  //  auto dse = new QGraphicsDropShadowEffect(this);
  //  dse->setColor(QColor(Qt::black));
  //  dse->setOffset(0);
  //  dse->setBlurRadius(5);
  //  widget->setGraphicsEffect(dse);
  //}
  //widget->setMinimumSize(480, 270);
}