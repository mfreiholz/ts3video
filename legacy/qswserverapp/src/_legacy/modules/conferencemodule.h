#ifndef CONFERENCEMODULE_H
#define CONFERENCEMODULE_H

#include "shared/api/apidef.h"

#include "_legacy/modules/basemodule.h"

class ConferenceModule : public BaseModule
{
	DECLARE_PRIVATE(ConferenceModule)
	DISABLE_COPY(ConferenceModule)
public:
	ConferenceModule(StreamingServer *serverBase);
	virtual ~ConferenceModule();
	virtual QStringList getMethodPrefixes() const;
	virtual bool processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
};

#endif
