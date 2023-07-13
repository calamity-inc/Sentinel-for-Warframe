#pragma once

#include <cstdio>
#include <string>

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
	private:
		static void processMissionJson(const std::string& missionJson);
	};
}
