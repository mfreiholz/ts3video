#include <QCoreApplication>
#include <QTimer>
#include "mytestobject.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyTestObject o(0);
    
    //o.clientConnect();
    //o.clientConnect();
    //o.clientConnect();

    QTimer t;
    QObject::connect(&t, SIGNAL(timeout()), &o, SLOT(clientConnect()));
    t.start(50);

    return a.exec();
}
