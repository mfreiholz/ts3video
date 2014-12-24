#ifndef QCORCONNECTION_HEADER
#define QCORCONNECTION_HEADER

#include <QObject>
#include <QAbstractSocket>
#include "qcorframe.h"
class QTcpSocket;
class QCorRequest;
class QCorReply;
class QCorConnectionPrivate;

class QCorConnection : public QObject
{
  Q_OBJECT

public:
  QCorConnection(QObject *parent);
  virtual ~QCorConnection();
  QAbstractSocket::SocketState state() const;
  QTcpSocket *socket() const;

public slots:
  /* Accepts the already connected socket by it's descriptor.
   */
  void connectWith(quintptr descriptor);
  void connectWith(QTcpSocket *socket);

  /* Connects to a remote host.
   * The stateChanged() signal will be emitted with every state change.
   */
  void connectTo(const QHostAddress &address, quint16 port);

  void disconnectFromHost();
  
  /* Sends an request to the remote host.
   * The returning QCorReply* emits it's finished() signal,
   * as soon as the remote host responds.
   */
  QCorReply* sendRequest(const QCorFrame &frame);

  /* Sends an response of an previously income request to the
   * remote host. The remote host will not answer to this frame.
   */
  void sendResponse(const QCorFrame &frame);

signals:
  /* Emits every time the underlying socket connection changes its state.
   */
  void stateChanged(QAbstractSocket::SocketState state);

  /* Emits for every new incoming request, which must be handled.
   * The ownership goes over to the receiver.
   */
  void newIncomingRequest(QCorFrameRefPtr frame);

private slots:
  void onSocketReadyRead();
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void doNextSendItem();
  void onCurrentSendItemDone();

private:
  friend class QCorConnectionPrivate;
  QCorConnectionPrivate *d;
};

#endif
