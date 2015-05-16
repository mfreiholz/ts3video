#include "channelmodule.h"


QSet<QString> ChannelModule::actions() const
{
  QSet<QString> set;
  set.insert("joinchannel");
  set.insert("joinchannelbyidentifier");
  set.insert("leavechannel");
  return set;
}

int ChannelModule::processActionRequest(const AbstractModuleBase::ActionRequest &req)
{
  throw new ActionRequestException(req);
}
