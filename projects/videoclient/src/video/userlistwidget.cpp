#include "userlistwidget.h"

#include <QtCore/QPoint>
#include <QtCore/QSortFilterProxyModel>

#include <QtGui/QIcon>

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMenu>

#include "videolib/src/cliententity.h"

#include "networkclient/clientlistmodel.h"
#include "networkclient/networkclient.h"

#include "video/conferencevideowindow.h"

UserListWidget::UserListWidget(ConferenceVideoWindow* window, QWidget* parent) :
	QFrame(parent),
	_window(window),
	_listView(nullptr)
{
	setWindowTitle(tr("Participants"));

	auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	//mainLayout->setContentsMargins(0, 0, 0, 0);
	//mainLayout->setSpacing(0);
	setLayout(mainLayout);

	auto filterEdit = new QLineEdit();
	filterEdit->setPlaceholderText(tr("Filter..."));
	filterEdit->setClearButtonEnabled(true);
	mainLayout->addWidget(filterEdit);
	connect(filterEdit, &QLineEdit::textChanged, this, &UserListWidget::onFilterTextChanged);

	_listView = new QListView();
	_listView->setContextMenuPolicy(Qt::CustomContextMenu);
	mainLayout->addWidget(_listView);
	connect(_listView, &QWidget::customContextMenuRequested, this, &UserListWidget::onCustomContextMenuRequested);

	// Update data of list view
	// Use sorted model.
	auto proxyModel = new SortFilterClientListProxyModel(this);
	proxyModel->setSourceModel(_window->networkClient()->clientModel());
	proxyModel->sort(0, Qt::AscendingOrder);
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	_listView->setModel(proxyModel);
}

void UserListWidget::onFilterTextChanged(const QString& text)
{
	auto m = _listView->model();
	if (!m)
		return;
	auto pm = qobject_cast<QSortFilterProxyModel*>(m);
	if (!pm)
		return;
	pm->setFilterWildcard(text);
}

void UserListWidget::onCustomContextMenuRequested(const QPoint& point)
{
	const auto mi = _listView->indexAt(point);
	if (!mi.isValid())
	{
		return;
	}
	const auto ci = mi.data(ClientListModel::ClientEntityRole).value<ClientEntity>();
	const auto nc = _window->networkClient();

	// Create context menu.
	QMenu menu;

	// Admin actions.
	if (nc->isAdmin() && !nc->isSelf(ci))
	{
		// Kick client.
		auto kickAction = menu.addAction(QIcon(), tr("Kick client"));
		QObject::connect(kickAction, &QAction::triggered, [this, ci, nc]()
		{
			const auto reply = nc->kickClient(ci.id, false);
			QCORREPLY_AUTODELETE(reply);
		});
		// Ban client.
		auto banAction = menu.addAction(QIcon(), tr("Ban client"));
		QObject::connect(banAction, &QAction::triggered, [this, ci, nc]()
		{
			const auto reply = nc->kickClient(ci.id, true);
			QCORREPLY_AUTODELETE(reply);
		});
	}
	if (menu.actions().isEmpty())
	{
		auto a = menu.addAction(tr("No actions available."));
		a->setEnabled(false);
	}
	menu.exec(_listView->mapToGlobal(point));
}