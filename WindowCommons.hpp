#pragma once

#include <soup/RasterFont.hpp>
#include <soup/RenderTarget.hpp>
#include <soup/Rgb.hpp>
#include <soup/Window.hpp>

namespace Sentinel
{
	struct WindowCommons
	{
		bool x_focused = false;

		static void init(soup::Window w)
		{
			w.customData() = WindowCommons{};
			w.setResizable(true);
		}

		[[nodiscard]] static WindowCommons& get(soup::Window w)
		{
			return w.customData().get<WindowCommons>();
		}

		void draw(soup::RenderTarget& rt)
		{
			if (x_focused)
			{
				rt.drawRect(rt.width - 20, 0, 20, 20, soup::Rgb::RED);
			}
			rt.drawText(rt.width - 13, -1, "x", soup::RasterFont::simple8(), soup::Rgb::WHITE, 2);
		}

		[[nodiscard]] soup::Window::on_click_t informMouse(soup::Window w, unsigned int x, unsigned int y)
		{
			bool x_focused_now = false;
			if (y < 20)
			{
				if (x > w.getSize().first - 20)
				{
					x_focused_now = true;
				}
			}
			
			if (x_focused_now != x_focused)
			{
				x_focused = x_focused_now;
				w.setResizable(!x_focused);
				w.redraw();
			}

			if (x_focused_now)
			{
				return [](soup::Window w, unsigned int, unsigned int)
				{
					w.close();
				};
			}
			return nullptr; 
		}
	};
}
