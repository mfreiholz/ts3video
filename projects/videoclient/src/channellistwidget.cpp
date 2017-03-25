#include "channellistwidget.h"
#include "channellistmodel.h"

ChannelListWidget::ChannelListWidget(const QSharedPointer<NetworkClient>& networkClient, QWidget* parent) :
	QDialog(parent)
{
	//_model = std::make_unique<ChannelListModel>(nullptr);
	_model.reset(new ChannelListModel(nullptr));
	_model->init(networkClient);
	_model->refresh();

	_ui.setupUi(this);
	_ui.channelsView->setModel(_model.get());
}

ChannelListWidget::~ChannelListWidget()
{
	_ui.channelsView->setModel(nullptr);
}

void ChannelListWidget::onCreate()
{

}

void ChannelListWidget::onRefresh()
{
	_model->refresh();
}

void ChannelListWidget::onJoin()
{

}

void ChannelListWidget::onCancel()
{

}
