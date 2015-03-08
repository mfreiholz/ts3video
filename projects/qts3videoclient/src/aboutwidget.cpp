#include "aboutwidget.h"

///////////////////////////////////////////////////////////////////////

AboutWidget::AboutWidget(QWidget *parent, Qt::WindowFlags f) :
QWidget(parent, f)
{
  setAttribute(Qt::WA_DeleteOnClose);
  ui.setupUi(this);
}