#ifndef QCORRESPONSE_HEADER
#define QCORRESPONSE_HEADER

#include <QObject>
#include "qcorframe.h"

class QCorReply : public QObject
{
  Q_OBJECT

public:
  QCorReply(QObject *parent = 0);

  QCorFrameRefPtr frame() const;
  int elapsedMillis() const { return _elapsedMillis; }

  void setFrame(QCorFrameRefPtr frame) { _frame = frame; }
  void setElapsedMillis(int ms) { _elapsedMillis = ms; }

  static void autoDelete(QCorReply* reply);

signals:
  void finished();

private:
  QCorFrameRefPtr _frame;
  int _elapsedMillis;
};

#define QCORREPLY_AUTODELETE(R) QCorReply::autoDelete(R);

#endif