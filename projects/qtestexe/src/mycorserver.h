#ifndef MYCORSERVER_HEADER
#define MYCORSERVER_HEADER

#include <QObject>
class QCorServer;
class QCorFrame;

class MyCorServer : public QObject
{
  Q_OBJECT

public:
  MyCorServer(QObject *parent);

private slots:
  void onNewFrame(QCorFrame *frame);

private:
  QCorServer *_server;
};


#endif