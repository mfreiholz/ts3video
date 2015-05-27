#include "movablewidgetcontainer.h"

#include "QDebug"

#include "QtCore/QSequentialAnimationGroup"
#include "QtCore/QParallelAnimationGroup"
#include "QtCore/QPropertyAnimation"

#include "QtWidgets/QLayout"
#include "QtGui/QMouseEvent"

#include "flowlayout.h"

MovableWidgetContainer::MovableWidgetContainer(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f), tmp_widget_(nullptr), drag_widget_(nullptr), drag_widget_index_(-1),
  enable_animations_(false), show_temp_widget_anim_(nullptr), hide_temp_widget_anim_(nullptr)
{
  layout_ = new FlowLayout();
  setLayout(layout_);
}

MovableWidgetContainer::~MovableWidgetContainer()
{
}

QWidget* MovableWidgetContainer::childAt(const QPoint &point) const
{
  QWidget *child_widget = QWidget::childAt(point);
  if (child_widget == nullptr)
    return nullptr;

  while (true) {
    if (child_widget->parentWidget() == this)
      return child_widget;
    if (child_widget->parentWidget() == nullptr)
      return nullptr;
    child_widget = child_widget->parentWidget();
  }
  return nullptr;
}

void MovableWidgetContainer::mousePressEvent(QMouseEvent *e)
{
  if (e->button() != Qt::LeftButton) {
    QWidget::mousePressEvent(e);
    return;
  }

  QWidget *child_widget = childAt(e->pos());
  if (child_widget == nullptr || layout() == nullptr || child_widget == tmp_widget_)
    return;

  const int index = layout()->indexOf(child_widget);
  if (index == -1)
    return;

  e->accept();

  drag_widget_ = child_widget;
  drag_widget_offset_ = drag_widget_->mapFrom(this, e->pos());
  drag_widget_index_ = index;
}

void MovableWidgetContainer::mouseMoveEvent(QMouseEvent *e)
{
  if (!drag_widget_ || !layout())
    return;

  e->accept();

  if (layout()->indexOf(drag_widget_) != -1) {
    // Remove from layout.
    layout_->removeWidget(drag_widget_);
    drag_widget_->setVisible(true);

    // Drop indicator widget.
    tmp_widget_ = new QWidget(this);
    //tmp_widget_->setStyleSheet("background-color: lightblue; border: 1px solid blue;");
    tmp_widget_->setAttribute(Qt::WA_ShowWithoutActivating);
    tmp_widget_->setFocusPolicy(Qt::NoFocus);
    tmp_widget_->setMinimumSize(drag_widget_->size());
    tmp_widget_->setMaximumSize(drag_widget_->size());
    tmp_widget_->stackUnder(drag_widget_);
    tmp_widget_->setVisible(true);
    layout_->insertWidget(drag_widget_index_, tmp_widget_);
    layout_->update();
  }

  drag_widget_->move(e->pos() - drag_widget_offset_);
  drag_widget_->raise();

  // Drop indicator widget.
  const int drop_location_index = currentDropLocation(e->pos());
  if (drop_location_index == layout_->indexOf(tmp_widget_)) {
    return;
  } else if (drop_location_index == -1) {
    layout_->removeWidget(tmp_widget_);
    layout_->insertWidget(drag_widget_index_, tmp_widget_);
    tmp_widget_->setVisible(true);
    layout_->update();
    return;
  }

  if (!enable_animations_) {
    layout_->removeWidget(tmp_widget_);
    layout_->insertWidget(drop_location_index, tmp_widget_);
    tmp_widget_->setVisible(true);
    layout_->update();
  } else if (enable_animations_ && show_temp_widget_anim_ == nullptr && hide_temp_widget_anim_ == nullptr) {
    QSequentialAnimationGroup *seq_anim_group = new QSequentialAnimationGroup(this);
    seq_anim_group->connect(seq_anim_group, SIGNAL(finished()), SLOT(deleteLater()));
    seq_anim_group->addAnimation(createTempWidgetHideAnimation(drop_location_index));
    seq_anim_group->addAnimation(createTempWidgetShowAnimation(drop_location_index));
    seq_anim_group->start();
  }
}

void MovableWidgetContainer::mouseReleaseEvent(QMouseEvent *e)
{
  if (drag_widget_ == nullptr || layout() == nullptr || layout()->indexOf(drag_widget_) != -1)
    return;

  e->accept();

  // Move drag_widget_ on top of the tmp_widget_ with animation.
  if (true) {
    QPropertyAnimation *move_anim = new QPropertyAnimation(drag_widget_, "pos", this);
    connect(move_anim, SIGNAL(finished()), SLOT(onMoveDragWidgetOnTempWidgetAnimationFinished()));
    move_anim->setStartValue(drag_widget_->pos());
    move_anim->setEndValue(tmp_widget_->pos());
    move_anim->setDuration(200);
    move_anim->start();
    return;
  }

  const int drop_location_index = currentDropLocation(e->pos());
  if (drop_location_index == -1)
    layout_->insertWidget(drag_widget_index_, drag_widget_);
  else
    layout_->insertWidget(drop_location_index, drag_widget_);

  if (tmp_widget_) {
    layout_->removeWidget(tmp_widget_);
    delete tmp_widget_;
    tmp_widget_ = nullptr;
  }

  layout()->update();

  drag_widget_ = nullptr;
  drag_widget_offset_ = QPoint();
  drag_widget_index_ = -1;
}

int MovableWidgetContainer::currentDropLocation(const QPoint &p)
{
  int drop_location_index = -1;
  for (int i = 0; i < layout()->count(); ++i) {
    QLayoutItem *item = layout()->itemAt(i);
    QWidget *widget = item->widget();
    if (widget) {
      if (widget->frameGeometry().contains(p)) {
        drop_location_index = i;
        break;
      }
    }
  }
  return drop_location_index;
}

QAbstractAnimation* MovableWidgetContainer::createTempWidgetShowAnimation(int index)
{
  if (index == -1)
    index = drag_widget_index_;

  layout_->insertWidget(index, tmp_widget_);
  tmp_widget_->setMaximumSize(QSize(0, 0));
  tmp_widget_->setMinimumSize(QSize(0, 0));
  tmp_widget_->setVisible(true);
  layout_->update();

  QParallelAnimationGroup *anim_group = new QParallelAnimationGroup(this);
  connect(anim_group, SIGNAL(finished()), SLOT(onShowTempWidgetAnimationFinished()));

  QPropertyAnimation *max_size_anim = new QPropertyAnimation(tmp_widget_, "maximumSize", this);
  max_size_anim->setDuration(150);
  max_size_anim->setStartValue(QSize(0, 0));
  max_size_anim->setEndValue(drag_widget_->maximumSize());
  anim_group->addAnimation(max_size_anim);

  QPropertyAnimation *min_size_anim = new QPropertyAnimation(tmp_widget_, "minimumSize", this);
  min_size_anim->setDuration(150);
  min_size_anim->setStartValue(QSize(0, 0));
  min_size_anim->setEndValue(drag_widget_->minimumSize());
  anim_group->addAnimation(min_size_anim);

  show_temp_widget_anim_ = anim_group;
  return anim_group;
}

QAbstractAnimation* MovableWidgetContainer::createTempWidgetHideAnimation(int index)
{
  QParallelAnimationGroup *anim_group = new QParallelAnimationGroup(this);
  connect(anim_group, SIGNAL(finished()), SLOT(onHideTempWidgetAnimationFinished()));

  QPropertyAnimation *max_size_anim = new QPropertyAnimation(tmp_widget_, "maximumSize", this);
  max_size_anim->setDuration(150);
  max_size_anim->setStartValue(drag_widget_->maximumSize());
  max_size_anim->setEndValue(QSize(0, 0));
  anim_group->addAnimation(max_size_anim);

  QPropertyAnimation *min_size_anim = new QPropertyAnimation(tmp_widget_, "minimumSize", this);
  min_size_anim->setDuration(150);
  min_size_anim->setStartValue(drag_widget_->minimumSize());
  min_size_anim->setEndValue(QSize(0, 0));
  anim_group->addAnimation(min_size_anim);

  hide_temp_widget_anim_ = anim_group;
  return anim_group;
}

void MovableWidgetContainer::onShowTempWidgetAnimationFinished()
{
  layout_->update();
  show_temp_widget_anim_->deleteLater();
  show_temp_widget_anim_ = nullptr;
}

void MovableWidgetContainer::onHideTempWidgetAnimationFinished()
{
  layout_->removeWidget(tmp_widget_);
  tmp_widget_->setVisible(false);
  layout_->update();
  hide_temp_widget_anim_->deleteLater();
  hide_temp_widget_anim_ = nullptr;
}

void MovableWidgetContainer::onMoveDragWidgetOnTempWidgetAnimationFinished()
{
  const int drop_index = layout_->indexOf(tmp_widget_);
  layout_->removeWidget(tmp_widget_);
  layout_->insertWidget(drop_index, drag_widget_);

  delete tmp_widget_;
  tmp_widget_ = nullptr;

  layout_->update();

  drag_widget_ = nullptr;
  drag_widget_offset_ = QPoint();
  drag_widget_index_ = -1;

  sender()->deleteLater();
}
