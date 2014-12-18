#ifndef QCORCONNECTION_HEADER
#define QCORCONNECTION_HEADER

#include <QObject>
#include <QAbstractSocket>
#include "qcorframe.h"
class QCorResponse;

class QCorConnection : public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  QCorConnection(QObject *parent);
  virtual ~QCorConnection();
  QAbstractSocket::SocketState state() const;

public slots:
  void connectWith(quintptr descriptor);
  void connectTo(const QHostAddress &address, quint16 port);
  void sendTestRequest();
  void sendResponse(QCorResponse *res, QIODevice *dev);

private slots:
  void onSocketReadyRead();
  void onSocketStateChanged(QAbstractSocket::SocketState state);

  void doNextSendItem();
  void onCurrentSendItemDone();

signals:
  /* Emits every time the underlying socket connection changes its state.
   */
  void stateChanged(QAbstractSocket::SocketState state);

  /* Emits for every new frame, independent of the frame's type.
   * Note: The frame has to be deleted by receiver with deleteLater().
   * It doesn't have a parent object.
   */
  void newFrame(QCorFrameRefPtr frame);
};

#endif