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
#include "clientlistmodel.h"

class QCorConnection;
class MediaSocket;

class NetworkClientPrivate : public QObject
{
  Q_OBJECT
public:
  NetworkClientPrivate(NetworkClient *o) :
    QObject(o),
    owner(o),
    corSocket(nullptr),
    mediaSocket(nullptr),
    goodbye(false),
    videoStreamingEnabled(false),
    isAdmin(false),
    useMediaSocket(true),
    clientModel(o)
  {}
  NetworkClientPrivate(const NetworkClientPrivate &);
  void reset();

public slots:
  void onAuthFinished();
  void onJoinChannelFinished();

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
  ClientListModel clientModel;

  // DEV
  bool useMediaSocket; ///< TODO Remove me (DEV ONLY)
};

#endif
