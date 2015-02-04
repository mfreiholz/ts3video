#include <QObject>
#include <QTimer>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include "qcorreply.h"

#include "elws.h"
#include "cliententity.h"
#include "channelentity.h"

#include "clientvideowidget.h"
#include "gridviewwidgetarranger.h"
#include "videocollectionwidget.h"
#include "ts3videoclient.h"
#include "clientapplogic.h"

QWidget* createVideoWidget()
{
  auto w = new ClientVideoWidget();
  w->setWindowFlags(Qt::FramelessWindowHint);
  w->setWindowTitle("Manuel");
  w->show();
  return w;
}

int runGuiTest(QApplication &a)
{
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

int runTestClient(QApplication &a)
{
  a.setQuitOnLastWindowClosed(false);

  QList<TS3VideoClient*> ts3vconns;
  
  auto timer = new QTimer(nullptr);
  timer->setInterval(2000);
  timer->start();

  QObject::connect(timer, &QTimer::timeout, [&ts3vconns] () {

    // Create a new connecion to the TS3VideoServer.
    auto ts3vc = new TS3VideoClient(nullptr);
    ts3vc->connectToHost(QHostAddress("127.0.0.1"), 6000);
    QObject::connect(ts3vc, &TS3VideoClient::connected, [ts3vc]() {
      // Auth.
      auto reply = ts3vc->auth("TestName");
      QObject::connect(reply, &QCorReply::finished, [ts3vc, reply]() {
        reply->deleteLater();
        qDebug() << QString(reply->frame()->data());
        // Join channel.
        auto reply2 = ts3vc->joinChannel(42);
        QObject::connect(reply2, &QCorReply::finished, [ts3vc, reply2]() {
          reply2->deleteLater();
          qDebug() << QString(reply2->frame()->data());
          // OPT We might start a timer to disconnect.
        });
      });
    });
    QObject::connect(ts3vc, &TS3VideoClient::disconnected, [ts3vc]() {
      qApp->quit();
    });

  });

  return a.exec();
}

/*!
  Runs the basic application.
  - Connects to server.
  - Authenticates with server.
  - Joins channel.
  - Sends and receives video streams.

  Command line parameters
  -----------------------
  --server-address
    Hostname or IPv4/IPv6 address of the server.
  --server-port
    Port on which the server is listen.
  --ts3-clientid
    The internal Teamspeak3 client-id (uint64) (>0)
  --ts3-channelid
    The internal Teamspeak3 channel-id (uint64) (>0)
  --username
    Visible username.
*/
int runClientAppLogic(QApplication &a)
{
  a.setApplicationName("TS3 Video Client");
  a.setApplicationDisplayName("TS3 Video Client");
  a.setApplicationVersion("1.0 ALPHA");
  a.setQuitOnLastWindowClosed(true);
  a.setOrganizationName("insaneFactory");
  a.setOrganizationDomain("http://www.insanefactory.com");

  ClientAppLogic::Options opts;
  opts.serverAddress = ELWS::getArgsValue("--server-address", "127.0.0.1").toString();
  opts.serverPort = ELWS::getArgsValue("--server-port", 6000).toUInt();
  opts.ts3clientId = ELWS::getArgsValue("--ts3-clientid", 0).toUInt();
  opts.ts3channelId = ELWS::getArgsValue("--ts3-channelid", 42).toUInt();
  opts.username = ELWS::getArgsValue("--username", ELWS::getUserName()).toString();

  ClientAppLogic logic(opts, nullptr);
  return a.exec();
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  const auto mode = ELWS::getArgsValue("--mode").toString();
  if (mode == QString("clienttest")) {
    return runTestClient(a);
  }
  return runClientAppLogic(a);
}