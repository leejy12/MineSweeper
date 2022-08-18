#include <Windows.h>
#include <array>
#include <fstream>
#include <filesystem>

import MineSweeperGame;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    // Open the config file stored in the AppData folder
    wchar_t configFilePath[256] = { 0 };
    wchar_t configFileDir[256] = { 0 };
    ExpandEnvironmentStringsW(L"%USERPROFILE%\\AppData\\Local\\MineSweeper\\config.txt", configFilePath, 256);
    ExpandEnvironmentStringsW(L"%USERPROFILE%\\AppData\\Local\\MineSweeper", configFileDir, 256);

    std::ifstream inConfigFile(configFilePath);
    std::ofstream outConfigFile;
    int width, height, mines;
    bool wasParseSuccessful = true, madeNewFile = false;

    if (inConfigFile.is_open())
    {
        wasParseSuccessful = static_cast<bool>(inConfigFile >> width >> height >> mines);
        inConfigFile.close();
    }
    else
    {
        if (!std::filesystem::exists(configFileDir))
        {
            std::filesystem::create_directory(configFileDir);
        }
        outConfigFile.open(configFilePath);
        madeNewFile = true;
        outConfigFile.close();
    }

    // If the parsing was unsuccessful or we created a new file (empty), then initialize
    // width, height, mines to Easy difficulty values.
    if (!wasParseSuccessful || madeNewFile)
    {
        width = 10;
        height = 10;
        mines = 9;
    }

    MineSweeperGame game(width, height, mines);

    // finalGameConfig is an array of 3 integers, that stores information of the mineField when the game is exited.
    // The format of finalGameConfig is { width, height, mines }
    const auto [finalWidth, finalHeight, finalMines] = game.Run();

    // Close and reopen the config file, but in write mode this time, to erase the contents.
    // Then, write to the file the final game configurations.
    outConfigFile.open(configFilePath);
    outConfigFile << finalWidth << ' ' << finalHeight << ' ' << finalMines;

    return 0;
}

