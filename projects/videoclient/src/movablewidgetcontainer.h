#ifndef MOVABLEWIDGETCONTAINER_HEADER
#define MOVABLEWIDGETCONTAINER_HEADER

#include "QtCore/QObject"
#include "QtCore/QPoint"

#include "QtWidgets/QWidget"

class QMouseEvent;
class QAbstractAnimation;
class QParallelAnimationGroup;
class FlowLayout;

class MovableWidgetContainer : public QWidget
{
  Q_OBJECT
public:
  MovableWidgetContainer(QWidget *parent, Qt::WindowFlags f);
  ~MovableWidgetContainer();
  QWidget* childAt(const QPoint &point) const;
protected:
  virtual void mousePressEvent(QMouseEvent *e);
  virtual void mouseMoveEvent(QMouseEvent *e);
  virtual void mouseReleaseEvent(QMouseEvent *e);
private:
  int currentDropLocation(const QPoint &p);
  QAbstractAnimation* createTempWidgetShowAnimation(int index);
  QAbstractAnimation* createTempWidgetHideAnimation(int index);
private slots:
  void onShowTempWidgetAnimationFinished();
  void onHideTempWidgetAnimationFinished();
  void onMoveDragWidgetOnTempWidgetAnimationFinished();
private:
  FlowLayout *layout_;
  QWidget *tmp_widget_;
  QWidget *drag_widget_;
  QPoint drag_widget_offset_;
  int drag_widget_index_;

  bool enable_animations_;
  QAbstractAnimation *show_temp_widget_anim_;
  QAbstractAnimation *hide_temp_widget_anim_;
};

#endif
