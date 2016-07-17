#include "lookup_client.h"

class LookupServiceClient::Private
{
public:
	Private() {}

public:

};

LookupServiceClient::LookupServiceClient(QObject* parent) :
	QObject(parent),
	d(std::make_unique<Private>())
{}

LookupServiceClient::~LookupServiceClient()
{}