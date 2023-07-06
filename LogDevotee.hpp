#pragma once

#include <cstdio>

namespace Sentinel
{
	class LogDevotee
	{
	private:
		inline static FILE* f = NULL;

	public:
		[[nodiscard]] static bool isGameRunning();
		static void deinit();
		static void process();
	};
}
