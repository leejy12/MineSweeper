#include <Windows.h>

import MineSweeperGame;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    MineSweeperGame game(20, 18, 20);
    return static_cast<int>(game.Run());
}
