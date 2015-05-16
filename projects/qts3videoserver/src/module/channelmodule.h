#ifndef CHANNELMODULE_H
#define CHANNELMODULE_H

#include "abstractmodulebase.h"

class ChannelModule : public AbstractModuleBase
{
public:
  QSet<QString> actions() const;
  int processActionRequest(const ActionRequest &req);
};

#endif
