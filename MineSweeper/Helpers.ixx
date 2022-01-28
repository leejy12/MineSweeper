module;

#include <Windows.h>
#include <strsafe.h>

export module Helpers;

export void DisplayError(const wchar_t* function)
{
    wchar_t* pErrorMsg = nullptr;
    wchar_t errorDisplayMsg[256] = { 0 };
    DWORD dwError = GetLastError();

    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL,
                   dwError,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   pErrorMsg,
                   0,
                   NULL);

    StringCbPrintfW(
        errorDisplayMsg, sizeof(errorDisplayMsg), L"%ls failed with code %u: %ls", function, dwError, pErrorMsg);

    MessageBoxW(nullptr, errorDisplayMsg, L"Error", MB_OK | MB_ICONERROR);

    LocalFree(pErrorMsg);
}