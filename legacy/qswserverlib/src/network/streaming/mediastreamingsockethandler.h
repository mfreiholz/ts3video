#ifndef MEDIASTREAMINGSOCKETHANDLER_HEADER
#define MEDIASTREAMINGSOCKETHANDLER_HEADER

#include "QObject"
#include "shared/network/protocol.h"
#include "mediaserializables.h"


/*!
  Handles all incoming media data on one port and broadcasts it to other clients.
  This object is not thread-safe, but reentrant.
*/
class MediaStreamingSocketHandler :
  public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  MediaStreamingSocketHandler(QObject *parent = 0);
  virtual ~MediaStreamingSocketHandler();
  bool listen(const QHostAddress &address, quint16 port);
  void close();

public slots:
  void setReceiverMap(const UdpDataReceiverMap &map);
  void clearReceivers();

signals:
  void tokenAuthorization(const QString &token, const QHostAddress &sender, quint16 senderPort);
  void readyWrite();
  void done();
};


#endif