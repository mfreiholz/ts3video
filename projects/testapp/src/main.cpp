#include <QApplication>
#include "gui/CameraTestWidget.h"

int main(int argc, char** argv)
{
	QApplication qtapp(argc, argv);
	qtapp.setQuitOnLastWindowClosed(true);

	CameraTestWidget ctwidget(nullptr, Qt::WindowFlags());
	ctwidget.setVisible(true);

	return qtapp.exec();
}