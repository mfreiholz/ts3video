#ifndef CHANNELLISTWIDGET_H
#define CHANNELLISTWIDGET_H

#include <memory>

#include <QSharedPointer>
#include <QDialog>

#include "ui_channellistwidget.h"

class NetworkClient;
class ChannelListModel;

class ChannelListWidget : public QDialog
{
	Q_OBJECT

public:
	ChannelListWidget(const QSharedPointer<NetworkClient>& networkClient, QWidget* parent = nullptr);
	virtual ~ChannelListWidget();

private slots:
	void onCreate();
	void onRefresh();
	void onJoin();
	void onCancel();

private:
	Ui::ChannelListWidgetForm _ui;
	std::unique_ptr<ChannelListModel> _model;
};

#endif