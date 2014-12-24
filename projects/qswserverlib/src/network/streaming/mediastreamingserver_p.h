#ifndef MEDIASTREAMINGSERVERPRIVATE_HEADER
#define MEDIASTREAMINGSERVERPRIVATE_HEADER

#include "QObject"
#include "mediastreamingserver.h"
#include "mediastreamingsockethandler.h"
#include "mediastreamingmasterconnectionhandler.h"


class MediaStreamingServer::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(MediaStreamingServer *owner);
  ~Private();

public:
  MediaStreamingServer *_owner;
  MediaStreamingServer::Options _opts;
  MediaStreamingSocketHandler _handler;
  MediaStreamingMasterConnectionHandler _masterConnection;
};


#endif