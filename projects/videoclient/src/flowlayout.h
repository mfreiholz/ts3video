#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QtCore/QRect>
#include <QtWidgets/QLayout>
#include <QtWidgets/QStyle>

class FlowLayout : public QLayout
{
public:
  explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
  explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
  ~FlowLayout();

  void addItem(QLayoutItem *item);
  void insertItem(int index, QLayoutItem *item);
  void insertWidget(int index, QWidget *widget);
  int horizontalSpacing() const;
  int verticalSpacing() const;
  Qt::Orientations expandingDirections() const;
  bool hasHeightForWidth() const;
  int heightForWidth(int) const;
  int count() const;
  QLayoutItem *itemAt(int index) const;
  QSize minimumSize() const;
  void setGeometry(const QRect &rect);
  QSize sizeHint() const;
  QLayoutItem *takeAt(int index);

private:
  int doLayout(const QRect &rect, bool testOnly) const;
  int smartSpacing(QStyle::PixelMetric pm) const;

  QList<QLayoutItem *> itemList;
  int m_hSpace;
  int m_vSpace;
};

#endif // FLOWLAYOUT_H
