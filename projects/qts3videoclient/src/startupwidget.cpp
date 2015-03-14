#include "startupwidget.h"
#include "ui_startupwidget.h"

#include <QCameraInfo>

///////////////////////////////////////////////////////////////////////

class StartupDialogPrivate
{
public:
  StartupDialogPrivate(StartupDialog *o) : owner(o) {}
  bool validateUi();

public:
  StartupDialog *owner;
  Ui::StartupWidgetForm ui;
};

///////////////////////////////////////////////////////////////////////

StartupDialog::StartupDialog(QWidget *parent) :
  QDialog(parent),
  d(new StartupDialogPrivate(this))
{
  d->ui.setupUi(this);

  d->ui.cameraComboBox->clear();
  d->ui.usernameLineEdit->clear();
  d->ui.serverAddress->clear();
  d->ui.serverPort->clear();

  auto cameraInfos = QCameraInfo::availableCameras();
  foreach (auto ci, cameraInfos) {
    d->ui.cameraComboBox->addItem(ci.description(), ci.deviceName());
  }
  d->validateUi();
  adjustSize();

  QObject::connect(d->ui.cameraComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(validate()));
  QObject::connect(d->ui.usernameLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(validate()));
  QObject::connect(d->ui.serverAddress, SIGNAL(textEdited(const QString&)), this, SLOT(validate()));
  QObject::connect(d->ui.serverPort, SIGNAL(textEdited(const QString&)), this, SLOT(validate()));
  QObject::connect(d->ui.okButton, &QPushButton::clicked, this, &StartupDialog::onAccept);
  QObject::connect(d->ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

StartupDialog::~StartupDialog()
{

}

StartupDialogValues StartupDialog::values() const
{
  StartupDialogValues v;
  v.cameraDeviceName = d->ui.cameraComboBox->currentData().toString();
  v.username = d->ui.usernameLineEdit->text();
  v.serverAddress = d->ui.serverAddress->text();
  v.serverPort = d->ui.serverPort->text().toUInt();
  return v;
}

void StartupDialog::setValues(const StartupDialogValues &v)
{
  auto index = d->ui.cameraComboBox->findData(v.cameraDeviceName);
  if (index >= 0) {
    d->ui.cameraComboBox->setCurrentIndex(index);
  }
  d->ui.usernameLineEdit->setText(v.username);
  d->ui.serverAddress->setText(v.serverAddress);
  d->ui.serverPort->setText(QString::number(v.serverPort));
  d->validateUi();
}

void StartupDialog::onAccept()
{
  if (!d->validateUi()) {
    return;
  }
  accept();
}

void StartupDialog::validate()
{
  d->validateUi();
}

///////////////////////////////////////////////////////////////////////

bool StartupDialogPrivate::validateUi()
{
  auto d = this;
  auto valid = true;
  
  if (d->ui.cameraComboBox->currentData().toString().isEmpty()) {
    valid = false;
  }
  if (d->ui.usernameLineEdit->text().isEmpty()) {
    valid = false;
  }
  if (d->ui.serverAddress->text().isEmpty()) {
    valid = false;
  }
  if (d->ui.serverPort->text().isEmpty() || d->ui.serverPort->text().toUInt() <= 0) {
    valid = false;
  }

  d->ui.okButton->setEnabled(valid);
  return valid;
}