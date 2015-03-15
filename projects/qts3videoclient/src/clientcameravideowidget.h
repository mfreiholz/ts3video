#ifndef CLIENTCAMERAVIDEOWIDGET_H
#define CLIENTCAMERAVIDEOWIDGET_H

#include <QWidget>

class QCameraInfo;
class TS3VideoClient;

class ClientCameraVideoWidget : public QWidget
{
  Q_OBJECT

public:
  ClientCameraVideoWidget(TS3VideoClient *ts3vc, const QCameraInfo &cameraInfo, QWidget *parent);
  ~ClientCameraVideoWidget();

private:
  TS3VideoClient *_ts3vc;
};

#endif