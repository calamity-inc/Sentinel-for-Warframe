#include "DataCache.hpp"

#include <soup/FileWriter.hpp>
#include <soup/string.hpp>

namespace Sentinel
{
	void DataCache::init()
	{
		std::wstring path = _wgetenv(L"localappdata");
		path.append(L"\\Sentinel for Warframe");
		if (std::filesystem::is_directory(path))
		{
			path.append(L"\\DataCache.json");
			root = soup::json::decode(soup::string::fromFilePath(path));
		}
		else
		{
			root = soup::make_unique<soup::JsonObject>();
		}
	}

	void DataCache::commit()
	{
		std::wstring path = _wgetenv(L"localappdata");
		path.append(L"\\Sentinel for Warframe");
		if (!std::filesystem::is_directory(path))
		{
			std::filesystem::create_directory(path);
		}
		path.append(L"\\DataCache.json");
		soup::FileWriter fw(path);
		auto encoded = root->encodePretty();
		fw.write(encoded.data(), encoded.size());
	}
}
