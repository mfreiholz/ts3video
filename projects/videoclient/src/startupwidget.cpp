#include "startupwidget.h"
#include "ui_startupwidget.h"

#include <QCameraInfo>
#include <QSettings>

///////////////////////////////////////////////////////////////////////

class StartupDialogPrivate
{
public:
  StartupDialogPrivate(StartupDialog *o) : owner(o) {}
  bool validateUi();

public:
  StartupDialog *owner;
  Ui::StartupWidgetForm ui;
  ClientAppLogic::Options opts;
};

///////////////////////////////////////////////////////////////////////

StartupDialog::StartupDialog(QWidget *parent) :
  QDialog(parent),
  d(new StartupDialogPrivate(this))
{
  d->ui.setupUi(this);

  d->ui.cameraComboBox->clear();
  d->ui.channelIdentifierLineEdit->clear();
  d->ui.usernameLineEdit->clear();
  d->ui.channelPasswordLineEdit->clear();
  d->ui.serverAddress->clear();

  auto cameraInfos = QCameraInfo::availableCameras();
  foreach (const auto &ci, cameraInfos) {
    d->ui.cameraComboBox->addItem(ci.description(), ci.deviceName());
  }
  d->ui.cameraComboBox->addItem(tr("No camera (Viewer mode)"));

  adjustSize();

  QObject::connect(d->ui.cameraComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(validate()));
  QObject::connect(d->ui.channelIdentifierLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(validate()));
  QObject::connect(d->ui.usernameLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(validate()));
  QObject::connect(d->ui.serverAddress, SIGNAL(editTextChanged(const QString&)), this, SLOT(validate()));
  QObject::connect(d->ui.okButton, &QPushButton::clicked, this, &StartupDialog::onAccept);
  QObject::connect(d->ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);

  // Restore from config.
  QSettings conf;
  const auto camID = conf.value("UI/StartupDialog-CameraDeviceID");
  for (auto i = 0; i < d->ui.cameraComboBox->count(); ++i) {
    if (d->ui.cameraComboBox->itemData(i).toString() == camID) {
      d->ui.cameraComboBox->setCurrentIndex(i);
      break;
    }
  }
  const auto addresses = conf.value("UI/StartupDialog-Addresses").toString().split(",", QString::SkipEmptyParts);
  for (auto i = 0; i < addresses.count(); ++i) {
    d->ui.serverAddress->addItem(addresses[i]);
  }
}

StartupDialog::~StartupDialog()
{
  QSettings conf;
  auto v = values();

  conf.setValue("UI/StartupDialog-CameraDeviceID", v.cameraDeviceId);

  QStringList addresses;
  for (auto i = 0; i < d->ui.serverAddress->count(); ++i)
    addresses.append(d->ui.serverAddress->itemText(i));
  if (!addresses.contains(d->ui.serverAddress->currentText()))
    addresses.prepend(d->ui.serverAddress->currentText());
  conf.setValue("UI/StartupDialog-Addresses", addresses.join(","));
}

ClientAppLogic::Options StartupDialog::values() const
{
  auto v = d->opts;

  v.cameraDeviceId = d->ui.cameraComboBox->currentData().toString();
  v.username = d->ui.usernameLineEdit->text();

  auto addr = d->ui.serverAddress->currentText().split(":", QString::SkipEmptyParts);
  if (addr.size() >= 1) {
    v.serverAddress = addr[0];
  }
  if (addr.size() >= 2) {
    bool ok = false;
    auto port = addr[1].toUInt(&ok);
    if (port > 0 && ok)
      v.serverPort = port;
  }
  v.serverPassword = d->ui.serverPasswordLineEdit->text();

  v.channelIdentifier = d->ui.channelIdentifierLineEdit->text();
  v.channelPassword = d->ui.channelPasswordLineEdit->text();

  // Handle special format of channel ident => ID=789456
  const auto parts = v.channelIdentifier.split('=', QString::SkipEmptyParts);
  if (parts.size() == 2 && parts[0].compare("ID") == 0) {
    bool isNum = false;
    const auto channelId = parts[1].trimmed().toInt(&isNum);
    if (channelId != 0 && isNum) {
      v.channelIdentifier.clear();
      v.channelId = channelId;
    }
  }

  return v;
}

void StartupDialog::setValues(const ClientAppLogic::Options &v)
{
  d->opts = v;

  const auto index = d->ui.cameraComboBox->findData(v.cameraDeviceId);
  if (index >= 0) d->ui.cameraComboBox->setCurrentIndex(index);
  d->ui.usernameLineEdit->setText(v.username);

  d->ui.serverAddress->setCurrentText(v.serverAddress + QString(":") + QString::number(v.serverPort));
  d->ui.serverPasswordLineEdit->setText(v.serverPassword);

  d->ui.channelIdentifierLineEdit->setText(v.channelIdentifier);
  d->ui.channelPasswordLineEdit->setText(v.channelPassword);

  // Handle special channel-ident format => ID=123456
  if (v.channelId != 0) {
    d->ui.channelIdentifierLineEdit->setText(QString("ID=%1").arg(v.channelId));
  }

  d->validateUi();
}

void StartupDialog::showEvent(QShowEvent *ev)
{
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

  if (d->ui.usernameLineEdit->text().isEmpty()) {
    valid = false;
  }
  if (d->ui.serverAddress->currentText().isEmpty()) {
    valid = false;
  }
  if (d->ui.channelIdentifierLineEdit->text().isEmpty()) {
    valid = false;
  }

  d->ui.okButton->setEnabled(valid);
  return valid;
}