#ifndef GRIDVIEWWIDGETARRANGER_H
#define GRIDVIEWWIDGETARRANGER_H

#include <QObject>
#include <QList>
#include <QRect>
class QWidget;

class GridViewWidgetArranger : public QObject
{
  Q_OBJECT
  Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount)
  Q_PROPERTY(int columnSpacing READ columnSpacing WRITE setColumnSpacing)

public:
  explicit GridViewWidgetArranger(QObject *parent = 0);

  int columnCount() const { return _columnCount; }
  void setColumnCount(int count) { _columnCount = count; }

  int columnSpacing() const { return _columnSpacing; }
  void setColumnSpacing(int spacing) { _columnSpacing = spacing; }

  void setWidgets(const QList<QWidget*> &widgets) { _widgets = widgets; }
  void setBaseRec(const QRect &rect) { _rect = rect; }

public slots:
  void arrange();

private:
  QList<QWidget*> _widgets;
  QRect _rect;

  bool _animationsEnabled;
  int _columnCount;
  int _columnSpacing;
};

#endif // GRIDVIEWWIDGETARRANGER_H
