#include "Overlay.hpp"

#include <thread>

#include <soup/country_names.hpp>
#include <soup/format.hpp>
#include <soup/RasterFont.hpp>
#include <soup/Rgb.hpp>
#include <soup/RenderTarget.hpp>
#include <soup/string.hpp>
#include <soup/Vector2.hpp>
#include <soup/Window.hpp>

#include "data.hpp"
#include "DuviriTarot.hpp"
#include "GeoIpService.hpp"
#include "LogDevotee.hpp"
#include "mission.hpp"
#include "ProcessWatcher.hpp"
#include "squad.hpp"

#define OCR_POC false

#if OCR_POC
#include <soup/Canvas.hpp>
#include <soup/FileReader.hpp>
#include <soup/FileWriter.hpp>
#include <soup/unicode.hpp>
#include "screenshot.hpp"
#endif

namespace Sentinel
{
	static bool inited = false;
	static soup::Window w{};
	static std::thread t{};

	bool Overlay::isInited()
	{
		return inited;
	}

	void Overlay::init()
	{
		// If window was closed, we would've de-inited, but never joined the thread.
		if (t.joinable())
		{
			t.join();
		}

		inited = true;
		t = std::thread([]
		{
			using namespace Sentinel;
			using namespace soup;

			auto pos = ProcessWatcher::getWindowPos();
			auto size = ProcessWatcher::getWindowSize();

			w = soup::Window::create("Sentinel Overlay", size.first, size.second);
			w.setPos(pos.first, pos.second);
			w.setInvisibleColour(Rgb::BLACK);
			w.setTopmost(true);
			w.setDrawFunc([](Window w, RenderTarget& rt)
			{
				rt.fill(Rgb::BLACK);

#if false // Proof of concept door visualisation for CorpusOutpostRescue
				rt.drawLine(Vector2((1920 / 2) - (250 / 2), 10), Vector2((1920 / 2) - (250 / 2), 10 + 250), Rgb::WHITE);
				rt.drawLine(Vector2((1920 / 2) - (250 / 2), 10), Vector2((1920 / 2) + (250 / 2), 10), Rgb::WHITE);
				rt.drawLine(Vector2((1920 / 2) + (250 / 2), 10), Vector2((1920 / 2) + (250 / 2), 10 + 250), Rgb::WHITE);

				constexpr auto door_spacer = 22;
				constexpr auto door_size = 55;

				rt.drawLine(Vector2((1920 / 2) - (250 / 2) + 1, 10 + door_spacer), Vector2((1920 / 2) - (250 / 2) + 1, 10 + door_spacer + door_size), Rgb::GREEN);
				rt.drawLine(Vector2((1920 / 2) - (250 / 2) + 1, 10 + door_spacer + door_size + door_spacer + door_size + 25), Vector2((1920 / 2) - (250 / 2) + 1, 10 + door_spacer + door_size + door_spacer + door_size + door_spacer + door_size), Rgb::RED);
#endif

#if OCR_POC
				static bool once = false;
				bool capture_screenshots = false;
				if (!once)
				{
					once = false;
					capture_screenshots = true;
				}
				for (int y = 0; y != 4; ++y)
				{
					for (int x = 0; x != 6; ++x)
					{
						//rt.drawPixel(99 + (211 * x), 201 + (200 * y), Rgb::RED);
					
						if (!capture_screenshots)
						{
							rt.drawHollowRect(
								99 + (211 * x), 201 + (200 * y),
								166, 165,
								Rgb::GRAY
							);
						}

						for (int line = 0; line != 3; ++line)
						{
							if (capture_screenshots)
							{
								std::string basename = soup::format("C:/Users/Sainan/Desktop/Sentinel/Item {}.{} Line {}", x, y, line);
								std::string bmp = basename; bmp += ".bmp";
								std::string png = basename; png += ".png";
								std::wstring wBmp = unicode::utf8_to_utf16(bmp);
								captureScreenshot(
									wBmp.c_str(),
									165, 22,
									99 + (211 * x), 201 + (200 * y) + 140 - (22 * line)
								);
								soup::FileReader fr(bmp);
								auto c = soup::Canvas::fromBmp(fr);
								soup::FileWriter fw(png);
								c.toPng(fw);
							}

							if (!capture_screenshots)
							{
								rt.drawHollowRect(
									99 + (211 * x), 201 + (200 * y) + 140 - (22 * line),
									165, 22,
									Rgb::WHITE
								);
							}
						}
					}
				}
#endif

				unsigned int x = w.getSize().first / 2;
				unsigned int y = 10;
				constexpr auto text_scale = 3;

#if false // just to see if the overlay is working
				rt.drawCentredText(x, y, "Sentinel for Warframe", RasterFont::simple8(), Rgb::WHITE, text_scale);
				y += 40;
#endif

				{
					std::lock_guard lock(squad_members_mtx);
					for (const auto& member : squad_members)
					{
						std::vector<std::string> trivia{};
						if (member.getName() == host_name)
						{
							trivia.emplace_back("Host");
						}
						if (!member.ip.empty())
						{
							if (GeoIpService::available)
							{
								if (auto loc = GeoIpService::get().getLocationByIpv4(IpAddr(member.ip).getV4NativeEndian()))
								{
									trivia.emplace_back(getCountryName(loc->country_code.c_str()));
								}
								if (auto as = GeoIpService::get().getAsByIpv4(IpAddr(member.ip).getV4NativeEndian()))
								{
									trivia.emplace_back(as->name);
									if (as->isHosting())
									{
										trivia.emplace_back("VPN");
									}
								}
							}
						}
						if (!trivia.empty())
						{
							rt.drawCentredText(x, y, soup::format("{}: {}", member.getName(), string::join(trivia, ", ")), RasterFont::simple8(), Rgb::WHITE, text_scale);
							y += 40;
						}
					}
				}
				if (amHost()
					&& current_missionType == "MT_RESCUE"
					&& mission_stage == 1
					)
				{
					rt.drawCentredText(x, y, soup::format("{}, cell {}", current_levelOverride, hostage_cell), RasterFont::simple8(), Rgb::WHITE, text_scale);
					y += 40;
				}
				if (background_region == "/Lotus/Levels/Hub/SolarMapDuviri.level")
				{
					rt.drawCentredText(x, y, "Your cave options in Duviri will be as follows:", RasterFont::simple8(), Rgb::WHITE, text_scale);
					y += 40;
					std::lock_guard lock(duviri_items_mtx);
					for (const auto& item : duviri_items)
					{
						if (item.category == IC_POWERSUIT)
						{
							rt.drawCentredText(x, y, soup::format("Warframe: {}", codename_to_english(item.codename)), RasterFont::simple8(), Rgb::WHITE, text_scale);
							y += 40;
						}
					}
					for (const auto& item : duviri_items)
					{
						if (item.category == IC_PRIMARY)
						{
							rt.drawCentredText(x, y, soup::format("Primary Weapon: {}", codename_to_english(item.codename)), RasterFont::simple8(), Rgb::WHITE, text_scale);
							y += 40;
						}
					}
					for (const auto& item : duviri_items)
					{
						if (item.category == IC_SECONDARY)
						{
							rt.drawCentredText(x, y, soup::format("Secondary Weapon: {}", codename_to_english(item.codename)), RasterFont::simple8(), Rgb::WHITE, text_scale);
							y += 40;
						}
					}
					for (const auto& item : duviri_items)
					{
						if (item.category == IC_MELEE)
						{
							rt.drawCentredText(x, y, soup::format("Melee Weapon: {}", codename_to_english(item.codename)), RasterFont::simple8(), Rgb::WHITE, text_scale);
							y += 40;
						}
					}
				}
			});
			w.setMouseInformer([](soup::Window w, unsigned int x, unsigned int y) -> soup::Window::on_click_t
			{
				// By default, Soup allows the user to drag the window, but we'd rather do nothing when the window is clicked.
				return [](soup::Window w, unsigned int x, unsigned int y)
				{
				};
			});
			w.runMessageLoop();
			inited = false;
		});
	}

	void Overlay::deinit()
	{
		if (isInited())
		{
			// isInited is true as soon as init is called, but this does not mean the window is already created, so we have to wait for that
			// before we can close the window to cause the thread to leave the message loop.
			while (!w)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			w.close();
			w.reset();
			t.join();
		}
	}

	void Overlay::redraw()
	{
		if (w)
		{
			if (ProcessWatcher::isGameFocused())
			{
				if (auto pos = ProcessWatcher::getWindowPos(); pos != w.getPos())
				{
					w.setPos(pos.first, pos.second);
				}
				if (auto size = ProcessWatcher::getWindowSize(); size != w.getSize())
				{
					w.setSize(size.first, size.second);
				}
			}
			w.redraw();
		}
	}
}
