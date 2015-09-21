#include "startupwidget.h"
#include "ui_startupwidget.h"

#include <QCameraInfo>
#include <QSettings>

///////////////////////////////////////////////////////////////////////

class StartupDialogPrivate
{
public:
	StartupDialogPrivate(StartupDialog* o) : owner(o) {}
	bool validateUi();

public:
	StartupDialog* owner;
	Ui::StartupWidgetForm ui;
	ClientAppLogic::Options opts;
};

///////////////////////////////////////////////////////////////////////

StartupDialog::StartupDialog(QWidget* parent) :
	QDialog(parent),
	d(new StartupDialogPrivate(this))
{
	d->ui.setupUi(this);

	d->ui.cameraComboBox->clear();

	auto cameraInfos = QCameraInfo::availableCameras();
	foreach (const auto& ci, cameraInfos)
	{
		d->ui.cameraComboBox->addItem(ci.description(), ci.deviceName());
	}
	d->ui.cameraComboBox->addItem(tr("No camera (Viewer mode)"));

	adjustSize();

	QObject::connect(d->ui.cameraComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(validate()));
	QObject::connect(d->ui.okButton, &QPushButton::clicked, this, &StartupDialog::onAccept);
	QObject::connect(d->ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	// Restore from config.
	QSettings conf;
	const auto camID = conf.value("UI/StartupDialog-CameraDeviceID");
	for (auto i = 0; i < d->ui.cameraComboBox->count(); ++i)
	{
		if (d->ui.cameraComboBox->itemData(i).toString() == camID)
		{
			d->ui.cameraComboBox->setCurrentIndex(i);
			break;
		}
	}
	d->ui.cameraAutoEnable->setChecked(conf.value("UI/StartupDialog-CameraAutoEnable").toBool());
}

StartupDialog::~StartupDialog()
{
	QSettings conf;
	auto v = values();
	conf.setValue("UI/StartupDialog-CameraDeviceID", v.cameraDeviceId);
	conf.setValue("UI/StartupDialog-CameraAutoEnable", v.cameraAutoEnable);
}

ClientAppLogic::Options StartupDialog::values() const
{
	auto v = d->opts;
	v.cameraDeviceId = d->ui.cameraComboBox->currentData().toString();
	v.cameraAutoEnable = d->ui.cameraAutoEnable->isEnabled() && d->ui.cameraAutoEnable->isChecked();
	return v;
}

void StartupDialog::setValues(const ClientAppLogic::Options& v)
{
	d->opts = v;

	const auto index = d->ui.cameraComboBox->findData(v.cameraDeviceId);
	if (index >= 0) d->ui.cameraComboBox->setCurrentIndex(index);

	d->ui.cameraAutoEnable->setChecked(v.cameraAutoEnable);

	d->validateUi();
}

void StartupDialog::showEvent(QShowEvent* ev)
{
	d->validateUi();
}

void StartupDialog::onAccept()
{
	if (!d->validateUi())
	{
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

	if (d->ui.cameraComboBox->currentData().isNull())
		d->ui.cameraAutoEnable->setEnabled(false);
	else
		d->ui.cameraAutoEnable->setEnabled(true);

	d->ui.okButton->setEnabled(valid);
	return valid;
}