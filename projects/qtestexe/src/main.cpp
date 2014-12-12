#include <QCoreApplication>
#include "mycorserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyCorServer server(0);
    return a.exec();
}
