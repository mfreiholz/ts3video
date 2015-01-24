#include <QObject>
#include <QTimer>
#include <QApplication>
#include "clientvideowidget.h"
#include "gridviewwidgetarranger.h"
#include "videocollectionwidget.h"

QVariant getArgsValue(const QString &key, const QVariant &defaultValue = QVariant())
{
  auto pos = qApp->arguments().indexOf(key);
  if (pos < 0 || pos + 1 >= qApp->arguments().size()) {
    return defaultValue;
  }
  return qApp->arguments().at(pos + 1);
}

QWidget* createVideoWidget()
{
  auto w = new ClientVideoWidget();
  w->setWindowFlags(Qt::FramelessWindowHint);
  w->setWindowTitle("Manuel");
  w->show();
  return w;
}

void runClient()
{
  auto serverAddress = getArgsValue("--server-address", "127.0.0.1");
  auto serverPort = getArgsValue("--server-port", 6667).toUInt();
  auto ts3clientId = getArgsValue("--ts3-clientid").toString();
  auto ts3channelId = getArgsValue("--ts3-channelid").toString();
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  // Create a bunch of initial widgets.
  QList<QWidget*> widgets;
  for (auto i = 0; i < 5; ++i) {
    auto w = createVideoWidget();
    widgets.append(w);
  }

  // Update the frame of the widgets every X ms.
  auto pixmapsIndex = 0;
  QList<QPixmap> pixmaps;
  pixmaps.append(QPixmap::fromImage(QImage(QString(":/frame.jpg"))));
  pixmaps.append(QPixmap::fromImage(QImage(QString(":/frame2.jpg"))));
  pixmaps.append(QPixmap::fromImage(QImage(QString(":/frame3.jpg"))));
  QTimer t0;
  QObject::connect(&t0, &QTimer::timeout, [&pixmapsIndex, &pixmaps, &widgets] () {
    ++pixmapsIndex;
    if (pixmapsIndex >= pixmaps.size()) {
      pixmapsIndex = 0;
    }
    foreach (auto w, widgets) {
      ((ClientVideoWidget*)w)->setFrame(pixmaps.at(pixmapsIndex));
    }
  });
  t0.start(66); // 66 = ~15fps


  // Arrange widgets on desktop in a grid.
  GridViewWidgetArranger grid;
  grid.setWidgets(widgets);
  grid.setColumnCount(5);
  grid.setColumnSpacing(10);
  grid.arrange();

  // Resize the grid every X ms.
//  QTimer t1;
//  QObject::connect(&t1, &QTimer::timeout, [&grid] () {
//    grid.setColumnCount(grid.columnCount() + 1);
//    grid.arrange();
//  });
//  t1.start(2000);


  // Use a widget which groups all widgets into a single one.
//  VideoCollectionWidget coll;
//  coll.setWidgets(widgets);
//  coll.show();


  // Create more widgets over time.
//  QTimer tWidgetCreator;
//  QObject::connect(&tWidgetCreator, &QTimer::timeout, [&widgets, &grid] () {
//    auto w = createVideoWidget();
//    widgets.append(w);
//    grid.setWidgets(widgets);
//    grid.arrange();
//  });
//  tWidgetCreator.start(2000);

  return a.exec();
}
