#ifndef QCORRESPONSE_HEADER
#define QCORRESPONSE_HEADER

#include <QIODevice>

class QCorResponse : public QIODevice
{
  Q_OBJECT

public:
  QCorResponse(QObject *parent = 0);

protected:
  virtual qint64 readData(char *data, qint64 maxSize);
  virtual qint64 writeData(const char *data, qint64 maxSize);
};

#endif