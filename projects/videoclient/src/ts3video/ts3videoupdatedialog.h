#ifndef TS3VIDEOUPDATEDIALOGFORM_H
#define TS3VIDEOUPDATEDIALOGFORM_H

#include <QDialog>
#include <QList>
#include "ts3videoentities.h"
#include "ui_ts3videoupdatedialog.h"


class Ts3VideoUpdateDialog : public QDialog
{
	Q_OBJECT
	Ui::Ts3VideoUpdateDialogForm _ui;
	QList<VersionInfo> _versions;

public:
	Ts3VideoUpdateDialog(QWidget* parent = nullptr);
	virtual ~Ts3VideoUpdateDialog();

public slots:
	void setVersions(const QList<VersionInfo>& versions);

private slots:
	void onDownload();
};


#endif