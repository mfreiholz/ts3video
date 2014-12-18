#ifndef QCORFRAME_HEADER
#define QCORFRAME_HEADER

#include <QObject>
#include <QSharedPointer>
class QCorConnection;

class QCorFrame
{
public:
  enum Type { RequestType, ResponseType };
  enum State { TransferingState, FinishedState, ErrorState };

  QCorFrame(QCorConnection *connection);
  virtual ~QCorFrame();

  QCorConnection* connection() const;
  State state() const;
  Type type() const;
  QByteArray data() const;

private:
  QCorConnection *_connection;
  State _state;
  Type _type;
  QByteArray _data; // TODO Instead of keeping the entire data, use a QIODevice-buffer or signal with new data.

  friend class QCorConnection;
};

typedef QSharedPointer<QCorFrame> QCorFrameRefPtr;
Q_DECLARE_METATYPE(QCorFrameRefPtr);

#endif