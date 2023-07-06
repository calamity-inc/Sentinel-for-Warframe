#pragma once

#include <utility>

namespace Sentinel
{
	struct ProcessWatcher
	{
		[[nodiscard]] static bool isGameRunning();
		[[nodiscard]] static bool isGameFocused();
		[[nodiscard]] static std::pair<unsigned int, unsigned int> getWindowPos();
		[[nodiscard]] static std::pair<unsigned int, unsigned int> getWindowSize();
	};
}
