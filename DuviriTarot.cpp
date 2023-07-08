#include "DuviriTarot.hpp"

#include "DataCache.hpp"

namespace Sentinel
{
	void DuviriTarot::readFromCache()
	{
		if (auto data_node = DataCache::root->asObj().find("duviri_items"))
		{
			for (const auto& raw_item : data_node->asArr())
			{
				Item item;
				item.codename = raw_item.asObj().at("codename").asStr();
				item.category = raw_item.asObj().at("category").asInt();
				// readFromCache is only called during init, so we're fine without locking the mutex here.
				duviri_items.emplace_back(std::move(item));
			}
		}
	}

	void DuviriTarot::writeToCache()
	{
		soup::JsonNode* data_node = DataCache::root->asObj().find("duviri_items");
		if (data_node)
		{
			data_node->asArr().clear();
		}
		else
		{
			auto up = soup::make_unique<soup::JsonArray>();
			data_node = up.get();
			DataCache::root->asObj().add("duviri_items", std::move(up));
		}
		// LogDevotee is the only writer of duviri_items and the only caller of writeToCache, so we're fine without locking the mutex here.
		for (const auto& item : duviri_items)
		{
			auto obj = soup::make_unique<soup::JsonObject>();
			obj->add("codename", item.codename);
			obj->add("category", item.category);
			data_node->asArr().children.emplace_back(std::move(obj));
		}
		DataCache::commit();
	}
}
