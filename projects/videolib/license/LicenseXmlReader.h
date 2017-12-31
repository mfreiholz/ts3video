#pragma once

#include <memory>
#include <QString>

namespace lic {
class License;

class LicenseXmlReader
{
public:
	std::shared_ptr<License> loadFromFile(const QString& filePath) const;
};

}
