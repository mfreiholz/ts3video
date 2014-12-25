#ifndef ENCRYPTEDSTOREDMESSAGEMODULE_H
#define ENCRYPTEDSTOREDMESSAGEMODULE_H

#include "shared/api/apidef.h"

#include "_legacy/modules/basemodule.h"

class EncryptedStoredMessageModule : public BaseModule
{
	DECLARE_PRIVATE(EncryptedStoredMessageModule)
	DISABLE_COPY(EncryptedStoredMessageModule)
public:
	EncryptedStoredMessageModule(StreamingServer *serverBase);
	virtual ~EncryptedStoredMessageModule();
	virtual QStringList getMethodPrefixes() const;
	virtual bool processJsonRequest(TcpSocketHandler &socket, const TcpProtocol::RequestHeader &header, const QVariant &data);
};

#endif
