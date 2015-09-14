#include "ts3videoupdatedialog.h"
#include <QDesktopServices>

Ts3VideoUpdateDialog::Ts3VideoUpdateDialog(QWidget* parent) :
	QDialog(parent)
{
	_ui.setupUi(this);
	connect(_ui.download, &QPushButton::clicked, this, &Ts3VideoUpdateDialog::onDownload);
	connect(_ui.skip, &QPushButton::clicked, this, &Ts3VideoUpdateDialog::reject);
}

Ts3VideoUpdateDialog::~Ts3VideoUpdateDialog()
{
}

void Ts3VideoUpdateDialog::setVersions(const QList<VersionInfo>& versions)
{
	QString html;
	for (auto i = 0; i < versions.size(); ++i)
	{
		auto& v = versions[i];
		html.append(QString("<h1>Version %1</h1>").arg(v.version));
		html.append(QString("<p>Released on %2</p>").arg(v.releasedOn));
		if (!v.message.isEmpty())
		{
			html.append(QString("<p>%1</p>").arg(v.message));
		}
	}
	_ui.text->setHtml(html);
	_versions = versions;
}

void Ts3VideoUpdateDialog::onDownload()
{
	if (!_versions.isEmpty())
	{
		QDesktopServices::openUrl(_versions.last().homepageUrl);
	}
	accept();
}