#pragma once

#include "actionbase.h"

class EnableVideoAction : public ActionBase
{
public:
	QString name() const
	{
		return QString("clientenablevideo");
	}
	void run(const ActionData& req);
};

class DisableVideoAction : public ActionBase
{
public:
	QString name() const
	{
		return QString("clientdisablevideo");
	}
	void run(const ActionData& req);
};