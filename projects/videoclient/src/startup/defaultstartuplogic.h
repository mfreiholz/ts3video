#pragma once

#include "startuplogic.h"

class DefaultStartupLogic :
	public AbstractStartupLogic
{
public:
	explicit DefaultStartupLogic(QApplication* a);
	virtual ~DefaultStartupLogic();
	virtual int exec();
};