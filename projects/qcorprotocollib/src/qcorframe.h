#ifndef QCORFRAME_HEADER
#define QCORFRAME_HEADER

#include <QObject>
class QCorConnection;

class QCorFrame : public QObject
{
  Q_OBJECT
  friend class QCorConnection;

public:
  enum Type { RequestType, ResponseType };
  enum State { TransferingState, FinishedState, ErrorState };

  QCorFrame(QCorConnection *connection, QObject *parent);
  QCorConnection* connection() const;
  State state() const;
  Type type() const;

private:
  void setState(State s);
  void setType(Type t);

signals:
  void newBodyData(const QByteArray &data);
  void end();

private:
  QCorConnection *_connection;
  State _state; ///< Indicates the state of this frame. Managed by QCorConnection, before end() signal.
  Type _type;
};

#endif