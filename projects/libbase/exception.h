#pragma once

#include <exception>
#include <string>

#include "defines.h"

OCS_NAMESPACE_BEGIN

class CoreException :
	public std::exception
{
public:
	explicit CoreException(std::string message,
						   std::string messageDetail = std::string());
	virtual ~CoreException();
	const std::string& getMessage() const;
	const std::string& getMessageDetail() const;

private:
	/* may be used as visible message for the user */
	std::string _message;

	/* detailed error details. maybe visible to advanced users */
	std::string _messageDetail;
};

OCS_NAMESPACE_END