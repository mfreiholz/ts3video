#pragma once

namespace lic {
class License;

class LicenseManager
{
public:
	static bool isValid(const License& lic);
};

}
