#include "ProcessWatcher.hpp"

#include <soup/Process.hpp>
#include <soup/time.hpp>
#include <soup/Window.hpp>

namespace Sentinel
{
    static soup::UniquePtr<soup::Process> game_process{};
    static bool game_process_is_game = false;

    bool ProcessWatcher::isGameRunning()
    {
        if (!game_process)
        {
			game_process = soup::Process::get("Warframe.x64.exe");
            if (!game_process)
            {
                return false;
            }
        }
        SOUP_IF_LIKELY (soup::Process::get(game_process->id))
        {
			// Warframe.x64.exe is running, that's nice, but it doesn't have to mean _the game_ is running...
			if (game_process_is_game)
            {
                return true;
            }
			std::wstring path = _wgetenv(L"localappdata");
			path.append(L"\\Warframe\\EE.log");
            if (soup::time::unixSecondsSince(soup::time::unixFromFile(std::filesystem::last_write_time(path))) < 10)
            {
                game_process_is_game = true;
                return true;
            }
            return false;
        }
        game_process.reset();
        game_process_is_game = false;
        return false;
    }

    bool ProcessWatcher::isGameFocused()
    {
        return game_process
            && soup::Window::getFocused().getOwnerPid() == game_process->id
            ;
    }

    [[nodiscard]] static soup::Window getWindow()
    {
        auto w = soup::Window::getFocused();
        SOUP_ASSERT(w.getOwnerPid() == game_process->id);
        return w;
    }

    [[nodiscard]] std::pair<unsigned int, unsigned int> ProcessWatcher::getWindowPos()
    {
        return getWindow().getPos();
    }

    [[nodiscard]] std::pair<unsigned int, unsigned int> ProcessWatcher::getWindowSize()
	{
        return getWindow().getSize();
	}
}
