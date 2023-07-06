#pragma once

#include <iostream>
#include <mutex>

inline std::recursive_mutex output_mutex;

namespace Sentinel
{
	inline void println(const char* service, std::string_view msg)
	{
		std::lock_guard lock(output_mutex);
		std::cout << '[' << service << "] " << msg << '\n';
	}
}
