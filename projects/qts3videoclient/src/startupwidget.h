#ifndef STARTUPWIDGET_H
#define STARTUPWIDGET_H

#include <QScopedPointer>
#include <QString>
#include <QDialog>

class StartupDialogValues
{
public:
  QString cameraDeviceName;
  QString username;
  QString serverAddress;
};

class StartupDialogPrivate;
class StartupDialog : public QDialog
{
  Q_OBJECT
  QScopedPointer<StartupDialogPrivate> d;

public:
  StartupDialog(QWidget *parent);
  virtual ~StartupDialog();
  
  StartupDialogValues values() const;
  void setValues(const StartupDialogValues &values);

private slots:
  void onAccept();
  void validate();
};

#endif