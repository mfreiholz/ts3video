#ifndef REQUESTACTIONHANDLERBASE_H
#define REQUESTACTIONHANDLERBASE_H

class ActionRequestHandlerI
{
public:
  class RequestData
  {
  public:
    ClientConnectionHandler connection;
    QCorFrameRefPtr frame;
    QString action;
    QJsonObject params;
  };  

  /*!
    Provides a list of unique actions the implementation of this handler supports.
    \return
  */
  virtual QSet<QString> actions() const = 0;
  
  /*!
    TODO
  */
  virtual int processRequest(const RequestData &data);
};

#endif