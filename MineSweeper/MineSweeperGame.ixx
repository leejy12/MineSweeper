module;

#include <vector>
#include <Windows.h>
#include <strsafe.h>
#include <gdiplus.h>
#include <string>

#include "resource.h"

export module MineSweeperGame;

import Helpers;
import MineField;

constexpr int IDT_GAME_TIMER = 12345;
constexpr int BLOCK_SIZE = 40;
constexpr int MARGIN = 100;
constexpr int OFFSET = 1;

INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
RECT CalculateNewWindowSize(Difficulty diff);

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

        // Brush and pen to draw flags
        Gdiplus::SolidBrush flagBrush(Gdiplus::Color::DarkRed);
        Gdiplus::Pen flagPolePen(Gdiplus::Color::Black, 2.0f);

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
        for (int x = 0; x < fieldWidth; x++)
        {
            for (int y = 0; y < fieldHeight; y++)
            {
                const auto cell = _mineField.GetCellInfo(x, y);
                if (cell.explored)
                {
                    graphics.FillRectangle(&exploredBrush,
                                           MARGIN + OFFSET + x * BLOCK_SIZE,
                                           MARGIN + OFFSET + y * BLOCK_SIZE,
                                           BLOCK_SIZE - OFFSET,
                                           BLOCK_SIZE - OFFSET);

                    Gdiplus::PointF point((MARGIN + (x + 0.25f) * BLOCK_SIZE), (MARGIN + (y + 0.2f) * BLOCK_SIZE));
                    if (cell.adjacentMines > 0)
                        graphics.DrawString(std::to_wstring(cell.adjacentMines).c_str(),
                                            -1,
                                            &font,
                                            point,
                                            &mineBrush[cell.adjacentMines - 1]);
                }
                else
                {
                    graphics.FillRectangle(&unexploredBrush,
                                           MARGIN + OFFSET + x * BLOCK_SIZE,
                                           MARGIN + OFFSET + y * BLOCK_SIZE,
                                           BLOCK_SIZE - OFFSET,
                                           BLOCK_SIZE - OFFSET);
                    if (cell.hasFlag)
                    {
                        Gdiplus::PointF trianglePoints[3] = {
                            Gdiplus::PointF(MARGIN + (x + 0.4f) * BLOCK_SIZE, MARGIN + (y + 0.2f) * BLOCK_SIZE),
                            Gdiplus::PointF(MARGIN + (x + 0.4f) * BLOCK_SIZE, MARGIN + (y + 0.6f) * BLOCK_SIZE),
                            Gdiplus::PointF(MARGIN + (x + 0.8f) * BLOCK_SIZE, MARGIN + (y + 0.4f) * BLOCK_SIZE)
                        };

                        graphics.FillPolygon(&flagBrush, trianglePoints, 3);
                        graphics.DrawLine(&flagPolePen,
                                          MARGIN + (x + 0.4f) * BLOCK_SIZE,
                                          MARGIN + (y + 0.2f) * BLOCK_SIZE,
                                          MARGIN + (x + 0.4f) * BLOCK_SIZE,
                                          MARGIN + (y + 0.8f) * BLOCK_SIZE);
                    }
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

    void _ForceRedraw(HWND hWnd, BOOL bEraseBackground)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        InvalidateRect(hWnd, &rc, bEraseBackground);
        UpdateWindow(hWnd);
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
        _wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);
        _wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

        RegisterClassExW(&_wc);

        RECT rc = {
            .left = 0,
            .top = 0,
            .right = 2 * MARGIN + BLOCK_SIZE * (width),
            .bottom = 2 * MARGIN + BLOCK_SIZE * (height),
        };
        AdjustWindowRect(&rc, WS_CAPTION, TRUE);

        _hWnd = CreateWindowExW(0,
                                _wc.lpszClassName,
                                L"MineSweeper",
                                WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                rc.right - rc.left,
                                rc.bottom - rc.top,
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
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case ID_GAME_EXIT:
                DestroyWindow(hWnd);
                break;

            case ID_HELP_ABOUT:
            {
                auto ret = DialogBoxW(pGame->_wc.hInstance, MAKEINTRESOURCEW(IDD_GAMEABOUTBOX), hWnd, About);
                if (ret == -1)
                    DisplayError(L"DialogBoxW");
                break;
            }

            case ID_GAME_NEWGAME:
            {
                pGame->_mineField.Reset(
                    pGame->_mineField.GetWidth(), pGame->_mineField.GetHeight(), pGame->_mineField.GetNumMines());
                pGame->_gameStarted = false;
                pGame->_ForceRedraw(hWnd, FALSE);
                break;
            }

            case ID_SETDIFFICULTY_EASY:
            {
                const auto rc = CalculateNewWindowSize(Difficulty::EASY);
                pGame->_mineField.Reset(Difficulty::EASY);
                pGame->_gameStarted = false;
                SetWindowPos(pGame->_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_SHOWWINDOW);
                pGame->_ForceRedraw(hWnd, TRUE);
                break;
            }

            case ID_SETDIFFICULTY_MEDIUM:
            {
                const auto rc = CalculateNewWindowSize(Difficulty::MEDIUM);
                pGame->_mineField.Reset(Difficulty::MEDIUM);
                pGame->_gameStarted = false;
                SetWindowPos(pGame->_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_SHOWWINDOW);
                pGame->_ForceRedraw(hWnd, TRUE);
                break;
            }

            case ID_SETDIFFICULTY_HARD:
            {
                const auto rc = CalculateNewWindowSize(Difficulty::HARD);
                pGame->_mineField.Reset(Difficulty::HARD);
                pGame->_gameStarted = false;
                SetWindowPos(pGame->_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_SHOWWINDOW);
                pGame->_ForceRedraw(hWnd, TRUE);
                break;
            }

            default:
                return DefWindowProcW(hWnd, msg, wParam, lParam);
            }

            return 0;
        }

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
                const auto cell = pGame->_mineField.GetCellInfo(x, y);

                if (cell.hasFlag)
                {
                    return 0;
                }
                if (cell.hasMine)
                {
                    MessageBoxW(hWnd, L"BOOM!", L"BOOM!", MB_OK | MB_ICONEXCLAMATION);
                    pGame->_mineField.Reset(
                        pGame->_mineField.GetWidth(), pGame->_mineField.GetHeight(), pGame->_mineField.GetNumMines());
                    pGame->_gameStarted = false;
                    pGame->_ForceRedraw(hWnd, FALSE);
                    return 0;
                }
            }

            if (pGame->_mineField.StepOn(x, y) == 1)
            {
                pGame->_ForceRedraw(hWnd, FALSE);
                return 0;
            }
        }

        case WM_RBUTTONDOWN:
        {
            int x = (LOWORD(lParam) - MARGIN) / BLOCK_SIZE;
            int y = (HIWORD(lParam) - MARGIN) / BLOCK_SIZE;

            if (x < 0 || x >= pGame->_mineField.GetWidth() || y < 0 || y >= pGame->_mineField.GetHeight())
                return 0;

            pGame->_mineField.ToggleFlag(x, y);
            pGame->_ForceRedraw(hWnd, FALSE);
            return 0;
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

INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }

    return FALSE;
}

RECT CalculateNewWindowSize(Difficulty diff)
{
    RECT rc;
    rc.left = 0;
    rc.top = 0;
    switch (diff)
    {
    case Difficulty::EASY:
        rc.right = 2 * MARGIN + BLOCK_SIZE * 9;
        rc.bottom = 2 * MARGIN + BLOCK_SIZE * 9;
        break;
    case Difficulty::MEDIUM:
        rc.right = 2 * MARGIN + BLOCK_SIZE * 16;
        rc.bottom = 2 * MARGIN + BLOCK_SIZE * 16;
        break;
    case Difficulty::HARD:
        rc.right = 2 * MARGIN + BLOCK_SIZE * 30;
        rc.bottom = 2 * MARGIN + BLOCK_SIZE * 16;
        break;
    }
    AdjustWindowRect(&rc, WS_CAPTION, TRUE);
    return rc;
}
