#include <QCoreApplication>
#include "qcorserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCorServer server(0);
    server.listen(QHostAddress::Any, 5005);
    return a.exec();
}
