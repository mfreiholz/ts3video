#ifndef PROFILEMODULE_H
#define PROFILEMODULE_H

#include "shared/api/apidef.h"

#include "_legacy/modules/basemodule.h"


class ProfileModule : public BaseModule
{
	DECLARE_PRIVATE(ProfileModule)
	DISABLE_COPY(ProfileModule)
public:
	ProfileModule(StreamingServer *serverBase);
	virtual ~ProfileModule();
	virtual QStringList getMethodPrefixes() const;
	virtual bool processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
};

#endif
