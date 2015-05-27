#include "adminauthwidget.h"
#include <QMessageBox>
#include "networkclient/networkclient.h"

AdminAuthWidget::AdminAuthWidget(NetworkClient *networkClient, QWidget *parent) :
  QDialog(parent),
  _networkClient(networkClient)
{
  _ui.setupUi(this);
  QObject::connect(_ui.loginButton, &QPushButton::clicked, this, &AdminAuthWidget::onLogin);
  QObject::connect(_ui.cancelButton, &QPushButton::clicked, this, &AdminAuthWidget::onCancel);
}

void AdminAuthWidget::onLogin()
{
  _ui.loginButton->setEnabled(false);

  auto reply = _networkClient->authAsAdmin(_ui.passwordLineEdit->text());
  QObject::connect(reply, &QCorReply::finished, [this, reply]()
  {
    _ui.loginButton->setEnabled(true);

    reply->deleteLater();
    if (!_networkClient->isAdmin()) {
      auto mb = new QMessageBox(this);
      mb->setIcon(QMessageBox::Warning);
      mb->setText(tr("Login failed!"));
      mb->setDetailedText(QString::fromUtf8(reply->frame()->data()));
      mb->setStandardButtons(QMessageBox::Ok);
      mb->show();
      return;
    }
    accept();
  });
}

void AdminAuthWidget::onCancel()
{
  reject();
}