#ifndef HANGOUTVIEWWIDGET_H
#define HANGOUTVIEWWIDGET_H

#include <QScopedPointer>
#include <QWidget>

class HangoutViewWidgetPrivate;
class HangoutViewWidget : public QWidget
{
  Q_OBJECT
  QScopedPointer<HangoutViewWidgetPrivate> d;

public:
  HangoutViewWidget(QWidget *parent);
  virtual ~HangoutViewWidget();
  
  void setCameraWidget(QWidget *w);
  
  void addWidget(QWidget *widget);
  void removeWidget(QWidget *widget);
  void setWidgets(const QList<QWidget*> &widgets);

protected:
  void resizeEvent(QResizeEvent *);
};

#endif