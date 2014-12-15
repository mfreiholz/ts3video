#include <QCoreApplication>
#include "mytestobject.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyTestObject o(0);
    o.clientConnect();
    return a.exec();
}
