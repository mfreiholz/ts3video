#ifndef CLIENTCAMERAVIDEOWIDGET_H
#define CLIENTCAMERAVIDEOWIDGET_H

#include <QWidget>

class QCameraInfo;
class NetworkClient;

class ClientCameraVideoWidget : public QWidget
{
  Q_OBJECT

public:
  ClientCameraVideoWidget(NetworkClient *ts3vc, const QCameraInfo &cameraInfo, QWidget *parent);
  ~ClientCameraVideoWidget();

private:
  NetworkClient *_ts3vc;
};

#endif