#include <QCoreApplication>
#include <QTimer>
#include <QHostAddress>
#include "mytestobject.h"
#include "qtestclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    const QStringList args = a.arguments();

    // Server test.
    MyTestObject *server = 0;
    if (args.contains("--server")) {
      server = new MyTestObject(0);
      server->startServer();
    }

    // Client test.
    MyTestObject *client = 0;
    QTimer clientTimer;
    if (args.contains("--client")) {
      //client = new MyTestObject(0);
      //QObject::connect(&clientTimer, SIGNAL(timeout()), client, SLOT(clientConnect()));
      //clientTimer.setSingleShot(false);
      //clientTimer.start(250);

      QTestClient *testClient = new QTestClient(&a);
      testClient->connectToHost(QHostAddress("127.0.0.1"), 5005);
    }

    return a.exec();
}
