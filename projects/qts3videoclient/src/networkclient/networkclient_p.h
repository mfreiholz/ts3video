#ifndef NetworkClient_P_H
#define NetworkClient_P_H

#include <QHash>
#include <QPair>
#include <QUdpSocket>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QTime>
#include <QTimer>

#include "cliententity.h"
#include "channelentity.h"
#include "networkusageentity.h"
#include "jsonprotocolhelper.h"

#include "vp8frame.h"
#include "yuvframe.h"

#include "networkclient.h"

class QCorConnection;
class MediaSocket;

class NetworkClientPrivate
{
public:
  NetworkClientPrivate(NetworkClient *o) : owner(o), corSocket(nullptr), mediaSocket(nullptr), videoStreamingEnabled(false), useMediaSocket(true) {}
  NetworkClientPrivate(const NetworkClientPrivate &);

public:
  NetworkClient *owner;

  // Connection objects.
  QCorConnection *corSocket;
  MediaSocket *mediaSocket;
  QTimer heartbeatTimer;

  ClientEntity clientEntity;
  bool videoStreamingEnabled;

  bool useMediaSocket; ///< TODO Remove me (DEV ONLY)
};

#endif // NetworkClient_P_H
