#include "videocollectionwidget.h"

#include <limits.h>

#include <QSettings>
#include <QFrame>
#include <QSpinBox>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QBoxLayout>
#include <QGraphicsDropShadowEffect>

VideoCollectionWidget::VideoCollectionWidget(QWidget *parent) :
  QWidget(parent),
  _columnCount(1)
{
  // Top frame.
  auto topFrame = new QFrame(this);

  auto topFrameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
  topFrameLayout->setContentsMargins(3, 3, 3, 3);
  topFrameLayout->setSpacing(3);
  topFrame->setLayout(topFrameLayout);

  auto columnCountSpinBox = new QSpinBox(topFrame);
  columnCountSpinBox->setMinimum(1);
  columnCountSpinBox->setMaximum(std::numeric_limits<int>::max());
  _columnCountSpinBox = columnCountSpinBox;

  topFrameLayout->addStretch(1);
  topFrameLayout->addWidget(new QLabel(tr("Columns:")));
  topFrameLayout->addWidget(columnCountSpinBox);

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
  widget->setParent(nullptr);
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
  widget->setMinimumSize(256, 192);
}