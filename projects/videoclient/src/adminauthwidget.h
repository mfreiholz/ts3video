#ifndef ADMINAUTHWIDGET_H
#define ADMINAUTHWIDGET_H

#include "ui_adminauthwidget.h"
class NetworkClient;

class AdminAuthWidget : public QDialog
{
  Q_OBJECT

public:
  AdminAuthWidget(NetworkClient *networkClient, QWidget *parent);

private slots:
  void onLogin();
  void onCancel();

private:
  Ui::AdminAuthDialogForm _ui;
  NetworkClient *_networkClient;
};

#endif