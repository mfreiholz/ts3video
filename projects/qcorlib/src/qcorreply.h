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

signals:
  void finished();

private:
  QCorFrameRefPtr _frame;
  int _elapsedMillis;
};

#define QCORREPLY_AUTODELETE(R) \
  do { if (R) { QObject::connect(R, &QCorReply::finished, R, &QCorReply::deleteLater); } } while (false)

#endif