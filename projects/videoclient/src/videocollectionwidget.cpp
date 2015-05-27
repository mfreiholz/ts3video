#include "videocollectionwidget.h"

#include <limits.h>

#include <QSettings>
#include <QFrame>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QBoxLayout>
#include <QGraphicsDropShadowEffect>

VideoCollectionWidget::VideoCollectionWidget(QWidget *parent) :
  QWidget(parent)
{
  // Top frame.
  auto topFrame = new QFrame(this);

  auto topFrameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
  topFrameLayout->setContentsMargins(3, 3, 3, 3);
  topFrameLayout->setSpacing(3);
  topFrame->setLayout(topFrameLayout);

  _columnCountSpinBox = new QSpinBox(topFrame);
  _columnCountSpinBox->setMinimum(1);
  _columnCountSpinBox->setMaximum(std::numeric_limits<int>::max());
  _columnCountSpinBox->setValue(1);

  _autoDetectColumnCountCheckBox = new QCheckBox(topFrame);
  _autoDetectColumnCountCheckBox->setText(tr("Auto columns"));
  _autoDetectColumnCountCheckBox->setCheckable(true);
  _autoDetectColumnCountCheckBox->setChecked(false);

  topFrameLayout->addWidget(new QLabel(tr("Columns:")));
  topFrameLayout->addWidget(_columnCountSpinBox);
  topFrameLayout->addWidget(_autoDetectColumnCountCheckBox);
  topFrameLayout->addStretch(1);

  // Content area.
  auto contentAreaWidget = new QWidget();

  auto contentAreaWidgetLayout = new QGridLayout();
  contentAreaWidgetLayout->setContentsMargins(0, 0, 0, 0);
  contentAreaWidgetLayout->setSpacing(0);
  contentAreaWidget->setLayout(contentAreaWidgetLayout);

  auto contentScrollArea = new QScrollArea(this);
  contentScrollArea->setBackgroundRole(QPalette::Dark);
  contentScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  contentScrollArea->setWidget(contentAreaWidget);
  contentScrollArea->setWidgetResizable(true);

  _gridLayout = contentAreaWidgetLayout;

  // Layout elements.
  _mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  _mainLayout->setContentsMargins(0, 0, 0, 0);
  _mainLayout->setSpacing(0);
  _mainLayout->addWidget(topFrame, 0, Qt::AlignTop);
  _mainLayout->addWidget(contentScrollArea, 1);
  setLayout(_mainLayout);

  QObject::connect(_columnCountSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this] (int value) {
    doLayout();
  });

  QObject::connect(_autoDetectColumnCountCheckBox, &QCheckBox::stateChanged, [this] (int state) {
    _columnCountSpinBox->setEnabled(state != Qt::Checked);
    doLayout();
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
  doLayout();
}

void VideoCollectionWidget::removeWidget(QWidget *widget)
{
  _widgets.removeAll(widget);
  widget->setVisible(false);
  widget->setParent(nullptr);
  doLayout();
}

void VideoCollectionWidget::setWidgets(const QList<QWidget *> widgets)
{
  _widgets = widgets;
  doLayout();
}

void VideoCollectionWidget::showEvent(QShowEvent *)
{
  QSettings settings;
  restoreGeometry(settings.value("UI/VideoCollectionWidget-Geometry").toByteArray());
  _columnCountSpinBox->setValue(settings.value("UI/VideoCollectionWidget-ColumnCount").toUInt());
  _autoDetectColumnCountCheckBox->setChecked(settings.value("UI/VideoCollectionWidget-ColumnCountAuto").toBool());
}

void VideoCollectionWidget::closeEvent(QCloseEvent *)
{
  QSettings settings;
  settings.setValue("UI/VideoCollectionWidget-Geometry", saveGeometry());
  settings.setValue("UI/VideoCollectionWidget-ColumnCount", _columnCountSpinBox->value());
  settings.setValue("UI/VideoCollectionWidget-ColumnCountAuto", _autoDetectColumnCountCheckBox->isChecked());
}

void VideoCollectionWidget::resizeEvent(QResizeEvent *)
{
  if (_autoDetectColumnCountCheckBox->isChecked()) {
    doLayout();
  }
}

void VideoCollectionWidget::doLayout()
{
  if (_autoDetectColumnCountCheckBox->isChecked()) {
    doGridLayoutAuto();
  } else {
    doGridLayout();
  }
}

void VideoCollectionWidget::doGridLayout()
{
  auto l = _gridLayout;
  auto columnCount = _columnCountSpinBox->value();
  auto minWidgetSize = QSize(256, 192);

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
    w->setMinimumSize(minWidgetSize);
    w->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    w->setVisible(true);

    // Calc next cell index.
    ci++;
    if (ci >= columnCount) {
      ri++;
      ci = 0;
    }
  }
}

void VideoCollectionWidget::doGridLayoutAuto()
{
  auto l = _gridLayout;
  auto columnCount = 1;
  auto minWidgetSize = QSize(256, 192);
  auto fixWidgetSize = minWidgetSize;
  auto surfaceRect = rect();
  
  // Calculate number of required columns based on "minWidgetSize.width()".
  auto itemsPerRowCount = (int) ((float)surfaceRect.width() / (float)minWidgetSize.width());
  itemsPerRowCount = qMin(itemsPerRowCount, _widgets.count());
  if (itemsPerRowCount <= 0) {
    itemsPerRowCount = 1;
    columnCount = 1;
  } else {
    columnCount = itemsPerRowCount;
  }
  // Add free space to the widget size.
  auto widthFree = (float)surfaceRect.width() - ((float)minWidgetSize.width() * (float)columnCount);
  widthFree = (float)widthFree / (float)columnCount;
  widthFree -= 1;

  // Set fixed height to keep aspect ratio.
  if (widthFree > 0) {
    fixWidgetSize.scale(fixWidgetSize.width() + widthFree, fixWidgetSize.height() + widthFree, Qt::KeepAspectRatio);
    foreach (auto w, _widgets) {
      w->setFixedSize(fixWidgetSize);
    }
  }

  // Only re-layout, if the column count changes.
  //if (columnCount == _columnCount) {
  //  return;
  //}
  //_columnCount = columnCount;

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
    l->addWidget(w, ri, ci, Qt::AlignCenter | Qt::AlignVCenter);
    w->setFixedSize(fixWidgetSize);
    w->setVisible(true);

    // Calc next cell index.
    ci++;
    if (ci >= columnCount) {
      ri++;
      ci = 0;
    }
  }
}