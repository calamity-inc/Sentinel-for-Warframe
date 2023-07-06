#pragma once

namespace Sentinel
{
	struct Overlay
	{
		[[nodiscard]] static bool isInited();
		static void init();
		static void deinit();

		static void redraw();
	};
}
