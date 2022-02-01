#include <Windows.h>
#include <array>
#include <strsafe.h>
#include <sstream>

import MineSweeperGame;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    // Open the config file stored in the AppData folder
    wchar_t configFilePath[256] = { 0 };
    ExpandEnvironmentStringsW(L"%USERPROFILE%", configFilePath, 32);
    StringCchCatW(configFilePath, 256, L"\\AppData\\Local\\MineSweeper\\config.txt");
    bool madeNewFile = false;

    HANDLE hConfigFile = CreateFileW(configFilePath,
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     nullptr,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);

    DWORD dwError = GetLastError();

    // If the configFile failed to open (probably because the file does not exist, create it.
    if (hConfigFile == INVALID_HANDLE_VALUE)
    {
        // Ensure that AppData\Local\MineSweeper directory exists.
        if (dwError == ERROR_PATH_NOT_FOUND)
        {
            wchar_t configFileDir[256] = { 0 };
            ExpandEnvironmentStringsW(L"%USERPROFILE%", configFileDir, 32);
            StringCchCatW(configFileDir, 256, L"\\AppData\\Local\\MineSweeper");
            CreateDirectoryW(configFileDir, nullptr);
        }
        hConfigFile = CreateFileW(configFilePath,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  nullptr,
                                  CREATE_NEW,
                                  FILE_ATTRIBUTE_NORMAL,
                                  nullptr);
        madeNewFile = true;
    }

    char configBuffer[16] = { 0 };
    int width, height, mines;
    bool wasParseSuccessful = true;

    // If the file exists, read from it and parse it to initialize widht, height, mines.
    if (!madeNewFile)
    {
        ReadFile(hConfigFile, configBuffer, sizeof(configBuffer), nullptr, nullptr);
        std::stringstream ss(configBuffer);
        wasParseSuccessful = static_cast<bool>(ss >> width >> height >> mines);
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
    CloseHandle(hConfigFile);
    hConfigFile = CreateFileW(configFilePath,
                              GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr,
                              TRUNCATE_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);

    ZeroMemory(configBuffer, sizeof(configBuffer));
    StringCbPrintfA(configBuffer, sizeof(configBuffer), "%d %d %d", finalWidth, finalHeight, finalMines);

    DWORD dwWritten;
    WriteFile(hConfigFile, configBuffer, sizeof(configBuffer), &dwWritten, nullptr);
    CloseHandle(hConfigFile);

    return 0;
}

