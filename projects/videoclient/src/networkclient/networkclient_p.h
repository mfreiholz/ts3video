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
  NetworkClientPrivate(NetworkClient *o) :
    owner(o),
    corSocket(nullptr),
    mediaSocket(nullptr),
    goodbye(false),
    videoStreamingEnabled(false),
    isAdmin(false),
    useMediaSocket(true)
  {}
  NetworkClientPrivate(const NetworkClientPrivate &);
  void reset();

public:
  NetworkClient *owner;

  // Connection objects.
  QCorConnection *corSocket;
  MediaSocket *mediaSocket;
  QTimer heartbeatTimer;
  bool goodbye;

  // Data about self.
  ClientEntity clientEntity;
  bool videoStreamingEnabled;
  bool isAdmin;

  // Data about others.
  // TODO Hold and manage ClientListModel here.

  // DEV
  bool useMediaSocket; ///< TODO Remove me (DEV ONLY)
};

#endif
