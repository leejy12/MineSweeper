module;

#include <vector>
#include <Windows.h>
#include <strsafe.h>
#include <gdiplus.h>
#include <string>

export module MineSweeperGame;

import Helpers;
import MineField;

constexpr int IDT_GAME_TIMER = 12345;
constexpr int BLOCK_SIZE = 40;
constexpr int MARGIN = 100;
constexpr int OFFSET = 1;

export class MineSweeperGame
{
private:
    WNDCLASSEXW _wc{ 0 };
    HWND _hWnd;

    Gdiplus::GdiplusStartupInput _gdiplusStartupInput;
    ULONG_PTR _gdiplusToken;
    bool _shouldRedrawGrid = false;

    MineField _mineField;
    bool _gameStarted;

    void _OnPaint(HDC hdc)
    {
        Gdiplus::Graphics graphics(hdc);

        Gdiplus::Pen gridPen(Gdiplus::Color::Black, 1);

        Gdiplus::SolidBrush unexploredBrush(Gdiplus::Color::DarkGray);
        Gdiplus::SolidBrush exploredBrush(Gdiplus::Color::LightGray);

        Gdiplus::SolidBrush textBrush(Gdiplus::Color::Red);
        Gdiplus::FontFamily fontFamily(L"Times New Roman");
        Gdiplus::Font font(&fontFamily, 24, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

        // Brushes to draw numbers on cells
        Gdiplus::SolidBrush mineBrush[8] = {
            Gdiplus::SolidBrush(Gdiplus::Color::Blue),      // 1
            Gdiplus::SolidBrush(Gdiplus::Color::Green),     // 2
            Gdiplus::SolidBrush(Gdiplus::Color::Red),       // 3
            Gdiplus::SolidBrush(Gdiplus::Color::Navy),      // 4
            Gdiplus::SolidBrush(Gdiplus::Color::Brown),     // 5
            Gdiplus::SolidBrush(Gdiplus::Color::Turquoise), // 6
            Gdiplus::SolidBrush(Gdiplus::Color::Black),     // 7
            Gdiplus::SolidBrush(Gdiplus::Color::Gray)       // 8
        };

        const int fieldWidth = _mineField.GetWidth();
        const int fieldHeight = _mineField.GetHeight();

        // Show information about game status
        Gdiplus::PointF statusLocation(static_cast<Gdiplus::REAL>(MARGIN), static_cast<Gdiplus::REAL>(MARGIN / 2));
        wchar_t statusMsg[128] = { 0 };
        StringCbPrintfW(statusMsg,
                        sizeof(statusMsg),
                        L"Dimension: %d X %d\t\tMines: %d",
                        fieldWidth,
                        fieldHeight,
                        _mineField.GetNumMines());
        graphics.DrawString(statusMsg, -1, &font, statusLocation, &textBrush);

        // Draw minefield
        auto& cells = _mineField.GetCells();
        for (int x = 0; x < fieldWidth; x++)
        {
            for (int y = 0; y < fieldHeight; y++)
            {
                if (cells[x][y].explored)
                {
                    graphics.FillRectangle(&exploredBrush,
                                           MARGIN + OFFSET + x * BLOCK_SIZE,
                                           MARGIN + OFFSET + y * BLOCK_SIZE,
                                           BLOCK_SIZE - OFFSET,
                                           BLOCK_SIZE - OFFSET);

                    Gdiplus::PointF point(static_cast<Gdiplus::REAL>(MARGIN + x * BLOCK_SIZE),
                                          static_cast<Gdiplus::REAL>(MARGIN + y * BLOCK_SIZE));
                    if (cells[x][y].adjacentMines > 0)
                        graphics.DrawString(
                            std::to_wstring(cells[x][y].adjacentMines).c_str(), -1, &font, point, &mineBrush[cells[x][y].adjacentMines - 1]);
                }
                else
                {
                    graphics.FillRectangle(&unexploredBrush,
                                           MARGIN + OFFSET + x * BLOCK_SIZE,
                                           MARGIN + OFFSET + y * BLOCK_SIZE,
                                           BLOCK_SIZE - OFFSET,
                                           BLOCK_SIZE - OFFSET);
                }
            }
        }

        // Draw grids
        if (!_gameStarted || _shouldRedrawGrid)
        {
            for (int x = 0; x < fieldWidth + 1; x++)
            {
                graphics.DrawLine(&gridPen,
                                  MARGIN + x * BLOCK_SIZE,
                                  MARGIN,
                                  MARGIN + x * BLOCK_SIZE,
                                  MARGIN + fieldHeight * BLOCK_SIZE);
            }
            for (int y = 0; y < fieldHeight + 1; y++)
            {
                graphics.DrawLine(&gridPen,
                                  MARGIN,
                                  MARGIN + y * BLOCK_SIZE,
                                  MARGIN + fieldWidth * BLOCK_SIZE,
                                  MARGIN + y * BLOCK_SIZE);
            }
            _shouldRedrawGrid = false;
        }
    }

public:
    MineSweeperGame(int width, int height, int mines) : _mineField(width, height, mines), _gameStarted(false)
    {
        _wc.cbSize = sizeof(_wc);
        _wc.hInstance = GetModuleHandleW(nullptr);
        _wc.lpfnWndProc = WndProcInit;
        _wc.style = CS_VREDRAW | CS_HREDRAW;
        _wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
        _wc.lpszClassName = L"MINESWEEPER_GAME_WINDOW_CLASS";
        _wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

        RegisterClassExW(&_wc);

        _hWnd = CreateWindowExW(0,
                                _wc.lpszClassName,
                                L"MineSweeper",
                                WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                2 * MARGIN + BLOCK_SIZE * (width),
                                2 * MARGIN + BLOCK_SIZE * (height),
                                nullptr,
                                nullptr,
                                _wc.hInstance,
                                this);

        if (!_hWnd)
        {
            DisplayError(L"CreateWindowExW");
            ExitProcess(EXIT_FAILURE);
        }

        Gdiplus::GdiplusStartup(&_gdiplusToken, &_gdiplusStartupInput, nullptr);

        ShowWindow(_hWnd, SW_SHOWDEFAULT);
        UpdateWindow(_hWnd);
    }

    UINT_PTR Run()
    {
        MSG msg;
        while (GetMessageW(&msg, _hWnd, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        return msg.wParam;
    }

    static LRESULT CALLBACK WndProcInit(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_NCCREATE)
        {
            const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            MineSweeperGame* pGame = reinterpret_cast<MineSweeperGame*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pGame));
            SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&MineSweeperGame::WndProc));
            return WndProc(hWnd, msg, wParam, lParam);
        }

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        MineSweeperGame* pGame = reinterpret_cast<MineSweeperGame*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

        switch (msg)
        {
        case WM_DESTROY:
        {
            Gdiplus::GdiplusShutdown(pGame->_gdiplusToken);
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            pGame->_OnPaint(hdc);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            int x = (LOWORD(lParam) - MARGIN) / BLOCK_SIZE;
            int y = (HIWORD(lParam) - MARGIN) / BLOCK_SIZE;

            if (x < 0 || x >= pGame->_mineField.GetWidth() || y < 0 || y >= pGame->_mineField.GetHeight())
                return 0;

            if (!pGame->_gameStarted)
            {
                pGame->_mineField.PlaceMines(x, y);
                pGame->_gameStarted = true;
            }
            else
            {
                // Player clicked a mine
                if (pGame->_mineField.HasMine(x, y))
                {
                    MessageBoxW(hWnd, L"BOOM!", L"BOOM!", MB_OK | MB_ICONEXCLAMATION);
                    return 0;
                }
            }

            if (pGame->_mineField.StepOn(x, y) == 1)
            {
                RECT rc;
                GetClientRect(hWnd, &rc);
                InvalidateRect(hWnd, &rc, FALSE);
                UpdateWindow(hWnd);
                return 0;
            }
        }

        case WM_SIZE:
        {
            pGame->_shouldRedrawGrid = true;
            return 0;
        }
        }

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
};
