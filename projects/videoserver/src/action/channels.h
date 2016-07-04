#pragma once

#include "actionbase.h"

class GetChannelListAction : public ActionBase
{
public:
	QString name() const
	{
		return QString("GetChannelList");
	}
	void run(const ActionData& req);
};

class JoinChannelAction : public ActionBase
{
public:
	QString name() const
	{
		return QString("joinchannel");
	}
	void run(const ActionData& req);
};

class JoinChannel2Action : public JoinChannelAction
{
public:
	QString name() const
	{
		return QString("joinchannelbyidentifier");
	}
	void run(const ActionData& req)
	{
		JoinChannelAction::run(req);
	}
};

class LeaveChannelAction : public ActionBase
{
public:
	QString name() const
	{
		return QString("leavechannel");
	}
	void run(const ActionData& req);
};