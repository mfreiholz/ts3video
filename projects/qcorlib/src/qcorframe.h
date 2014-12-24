#ifndef QCORFRAME_HEADER
#define QCORFRAME_HEADER

#include <QObject>
#include <QSharedPointer>

/* Base class for request/response objects.
 */
class QCorFrame
{
public:
  enum Type { RequestType, ResponseType };
  enum State { TransferingState, FinishedState, ErrorState };

  QCorFrame();
  virtual ~QCorFrame();

  State state() const;
  Type type() const;
  quint32 correlationId() const { return _correlationId; }
  QByteArray data() const;

  void setState(State state) { _state = state; }
  void setType(Type type) { _type = type; }
  void setCorrelationId(quint32 id) { _correlationId = id; }
  void setData(const QByteArray &data) { _data = data; }
  void appendData(const QByteArray &data) { _data.append(data); }

private:
  State _state;
  Type _type;
  quint32 _correlationId;
  QByteArray _data; // TODO Instead of keeping the entire data, use a QIODevice-buffer or signal with new data.
};

typedef QSharedPointer<QCorFrame> QCorFrameRefPtr;
Q_DECLARE_METATYPE(QCorFrameRefPtr);

#endif