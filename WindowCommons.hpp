#pragma once

#include <soup/RasterFont.hpp>
#include <soup/RenderTarget.hpp>
#include <soup/Rgb.hpp>
#include <soup/Vector2.hpp>
#include <soup/Window.hpp>

namespace Sentinel
{
	struct WindowCommons
	{
		bool x_focused = false;
		bool pin_focused = false;

		bool is_topmost = false;

		static void init(soup::Window w)
		{
			w.customData() = WindowCommons{};
			w.setResizable(true);
		}

		[[nodiscard]] static WindowCommons& get(soup::Window w)
		{
			return w.customData().get<WindowCommons>();
		}

		void draw(soup::RenderTarget& rt, soup::Rgb background)
		{
			rt.drawRect(rt.width - 40, 0, 40, 20, background);

			if (x_focused)
			{
				rt.drawRect(rt.width - 20, 0, 20, 20, soup::Rgb::RED);
			}
			//rt.drawText(rt.width - 13, -1, "x", soup::RasterFont::simple8(), soup::Rgb::WHITE, 2);
			rt.drawLine(soup::Vector2(rt.width - 4, 3), soup::Vector2(rt.width - 18, 17), soup::Rgb::WHITE);
			rt.drawLine(soup::Vector2(rt.width - 17, 3), soup::Vector2(rt.width - 3, 17), soup::Rgb::WHITE);

			if (pin_focused)
			{
				rt.drawRect(rt.width - 40, 0, 20, 20, soup::Rgb::GREY);
			}
			if (is_topmost)
			{
				rt.drawRect(rt.width - 34, 3, 8, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 35, 4, 10, 2, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 33, 6, 6, 3, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 34, 9, 8, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 35, 10, 10, 2, soup::Rgb::WHITE);
			}
			else
			{
				rt.drawRect(rt.width - 34, 3, 8, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 35, 4, 1, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 26, 4, 1, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 35, 5, 2, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 27, 5, 2, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 33, 6, 1, 4, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 28, 6, 1, 4, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 34, 9, 1, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 27, 9, 1, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 35, 10, 1, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 26, 10, 1, 1, soup::Rgb::WHITE);
				rt.drawRect(rt.width - 35, 11, 10, 1, soup::Rgb::WHITE);
			}
			rt.drawRect(rt.width - 31, 12, 2, 5, soup::Rgb::WHITE);
		}

		[[nodiscard]] soup::Window::on_click_t informMouse(soup::Window w, unsigned int x, unsigned int y)
		{
			bool x_focused_now = false;
			bool pin_focused_now = false;
			if (y < 20)
			{
				if (x > w.getSize().first - 20)
				{
					x_focused_now = true;
				}
				else if (x > w.getSize().first - 40)
				{
					pin_focused_now = true;
				}
			}

			if (x_focused_now != x_focused
				|| pin_focused_now != pin_focused
				)
			{
				x_focused = x_focused_now;
				pin_focused = pin_focused_now;
				w.setResizable(!x_focused && !pin_focused);
				w.redraw();
			}

			if (x_focused_now)
			{
				return [](soup::Window w, unsigned int, unsigned int)
				{
					w.close();
				};
			}
			else if (pin_focused_now)
			{
				return [](soup::Window w, unsigned int, unsigned int)
				{
					WindowCommons::get(w).is_topmost ^= 1;
					w.setTopmost(WindowCommons::get(w).is_topmost);
					w.redraw();
				};
			}
			return nullptr; 
		}
	};
}
