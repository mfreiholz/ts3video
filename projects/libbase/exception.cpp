#include "exception.h"

OCS_NAMESPACE_BEGIN

CoreException::CoreException(std::string message, std::string messageDetail) :
	std::exception(),
	_message(std::move(message)),
	_messageDetail(std::move(messageDetail))
{}

CoreException::~CoreException()
{}

const std::string& CoreException::getMessage() const
{
	return _message;
}

const std::string& CoreException::getMessageDetail() const
{
	return _messageDetail;
}

OCS_NAMESPACE_END