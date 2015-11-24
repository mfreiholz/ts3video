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
  virtual void resizeEvent(QResizeEvent *);

private:
  void doLayout();
  void doGridLayout();
  void doGridLayoutAuto();

private:
  QList<QWidget*> _widgets;
  class QBoxLayout *_mainLayout;
  class QGridLayout *_gridLayout;
  class QSpinBox *_columnCountSpinBox;
  class QCheckBox *_autoDetectColumnCountCheckBox;
};

#endif // VIDEOCOLLECTIONWIDGET_H
