#include "aboutwidget.h"

#include "videolib/ts3video.h"

///////////////////////////////////////////////////////////////////////

AboutWidget::AboutWidget(QWidget* parent, Qt::WindowFlags f) :
	QDialog(parent, f)
{
	ui.setupUi(this);
	ui.versionLabel->setText(IFVS_SOFTWARE_VERSION_QSTRING);
}
