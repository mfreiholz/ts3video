#ifndef QCORCONNECTIONPRIVATE_HEADER
#define QCORCONNECTIONPRIVATE_HEADER

#include <QObject>
#include <QQueue>
#include <QHash>
#include <QDateTime>
#include "corprotocol.h"
#include "qcorframe.h"
class QTcpSocket;
class QCorConnection;
class QCorResponse;
class SendQueueItem;
class ReplyItem;

/*
 */
class QCorConnectionPrivate : public QObject
{
  Q_OBJECT

public:
  QCorConnectionPrivate(QCorConnection *owner);
  static int onParserFrameBegin(cor_parser *parser);
  static int onParserFrameHeaderBegin(cor_parser *parser);
  static int onParserFrameHeaderEnd(cor_parser *parser);
  static int onParserFrameBodyData(cor_parser *parser, const uint8_t *data, size_t length);
  static int onParserFrameEnd(cor_parser *parser);

public:
  QCorConnection *owner;
  QTcpSocket *socket;

  QByteArray buffer; ///< Incoming data buffer.
  QCorFrameRefPtr frame; ///< Current incoming frame.

  cor_parser_settings corSettings;
  cor_parser *corParser;
  cor_frame::correlation_t nextCorrelationId;

  QQueue<SendQueueItem*> sendQueue; ///< Outgoing item queue.
  SendQueueItem *sendQueueCurrent; ///< Current outgoing item.

  QHash<cor_frame::correlation_t, ReplyItem*> responseMap; ///< Holds response objects of outstanding replies from servers.
};

/*
 */
class SendQueueItem
{
public:
  SendQueueItem() {}

public:
  cor_frame frame;
  QByteArray data;
};

/*
 */
class ReplyItem
{
public:
  QCorResponse *res;
  QDateTime dtEnqueued;
  QDateTime dtReceived;
};

#endif