#ifndef QCORRESPONSE_HEADER
#define QCORRESPONSE_HEADER

#include <QObject>
#include "qcorframe.h"

class QCorResponse : public QObject
{
  Q_OBJECT

public:
  QCorResponse(QObject *parent = 0);
  QCorFrameRefPtr frame() const;
  int elapsedMillis() const { return _elapsedMillis; }

  void setFrame(QCorFrameRefPtr frame) { _frame = frame; }
  void setElapsedMillis(int ms) { _elapsedMillis = ms; }

signals:
  void finished();

private:
  QCorFrameRefPtr _frame;
  int _elapsedMillis;
};

#endif