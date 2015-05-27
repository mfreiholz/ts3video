#include "aboutwidget.h"

#include "ts3video.h"

///////////////////////////////////////////////////////////////////////

AboutWidget::AboutWidget(QWidget *parent, Qt::WindowFlags f) :
  QWidget(parent, f)
{
  setAttribute(Qt::WA_DeleteOnClose);
  ui.setupUi(this);
  ui.versionLabel->setText(IFVS_SOFTWARE_VERSION_QSTRING);
}