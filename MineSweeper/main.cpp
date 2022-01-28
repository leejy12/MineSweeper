#include <Windows.h>

import MineSweeperGame;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    MineSweeperGame game(30, 16, 99);
    return static_cast<int>(game.Run());
}
