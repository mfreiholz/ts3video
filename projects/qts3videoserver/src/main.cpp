#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  qDebug() << QString("Starting server...");
  return a.exec();
}
