#ifndef VIDEOCOLLECTIONWIDGET_H
#define VIDEOCOLLECTIONWIDGET_H

#include <QWidget>

class VideoCollectionWidget : public QWidget
{
  Q_OBJECT
public:
  explicit VideoCollectionWidget(QWidget *parent = 0);
  ~VideoCollectionWidget();
  void addWidget(QWidget *widget);
  void removeWidget(QWidget *widget);
  void setWidgets(const QList<QWidget*> widgets);

protected:
  virtual void showEvent(QShowEvent *);
  virtual void closeEvent(QCloseEvent *);

private:
  void doGridLayout();
  void prepareWidget(QWidget *widget);

private:
  QList<QWidget*> _widgets;
  int _columnCount;

  class QBoxLayout *_mainLayout;
  class QGridLayout *_gridLayout;
  class QSpinBox *_columnCountSpinBox;
};

#endif // VIDEOCOLLECTIONWIDGET_H
