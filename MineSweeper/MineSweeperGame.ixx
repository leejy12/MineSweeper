module;

#include <Windows.h>
#include <strsafe.h>
#include <gdiplus.h>
#include <string>
#include <array>
#include <sstream>

#include "resource.h"

export module MineSweeperGame;

import Helpers;
import MineField;

constexpr int BLOCK_SIZE = 40;
constexpr int MARGIN = 100;
constexpr int OFFSET = 1;

INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Custom(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
RECT CalculateNewWindowSize(int width, int height);

export class MineSweeperGame
{
private:
    WNDCLASSEXW _wc{ 0 };
    HWND _hWnd;

    Gdiplus::GdiplusStartupInput _gdiplusStartupInput;
    ULONG_PTR _gdiplusToken;
    bool _shouldRedrawGrid;

    MineField _mineField;
    bool _gameStarted, _kaBoom;
    int _numExploredCells;

    void _OnPaint(HDC hdc)
    {
        Gdiplus::Graphics graphics(hdc);

        // Pen to draw grids
        Gdiplus::Pen gridPen(Gdiplus::Color::Black, 1);

        // Brushes to draw cells
        Gdiplus::SolidBrush unexploredBrush(Gdiplus::Color::DarkGray);
        Gdiplus::SolidBrush exploredBrush(Gdiplus::Color::LightGray);

        Gdiplus::SolidBrush textBrush(Gdiplus::Color::Red);
        Gdiplus::FontFamily fontFamily(L"Consolas");
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
        Gdiplus::PointF statusLocation(static_cast<Gdiplus::REAL>(MARGIN), static_cast<Gdiplus::REAL>(MARGIN / 3));
        wchar_t statusMsg[128] = { 0 };
        StringCbPrintfW(statusMsg,
                        sizeof(statusMsg),
                        L"Dimension: %d X %d\nMines: %d",
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
                Gdiplus::PointF point((MARGIN + (x + 0.25f) * BLOCK_SIZE), (MARGIN + (y + 0.2f) * BLOCK_SIZE));
                if (cell.explored)
                {
                    graphics.FillRectangle(&exploredBrush,
                                           MARGIN + OFFSET + x * BLOCK_SIZE,
                                           MARGIN + OFFSET + y * BLOCK_SIZE,
                                           BLOCK_SIZE - OFFSET,
                                           BLOCK_SIZE - OFFSET);

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

                if (cell.hasMine && _kaBoom)
                {
                    point.X -= 0.15f * BLOCK_SIZE;
                    graphics.DrawString(L"\u2739", 1, &font, point, &mineBrush[2]);
                }
            }
        }
        _kaBoom = false;

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

    void _ResetGame(int newWidth, int newHeight, int newMines, bool bEraseBackground)
    {
        _mineField.Reset(newWidth, newHeight, newMines);
        _gameStarted = false;
        _numExploredCells = 0;
        const RECT rc = CalculateNewWindowSize(newWidth, newHeight);
        SetWindowPos(_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_SHOWWINDOW);
        _ForceRedraw(_hWnd, bEraseBackground);
    }

public:
    MineSweeperGame(int width, int height, int mines) :
        _mineField(width, height, mines), _gameStarted(false), _numExploredCells(0), _shouldRedrawGrid(false)
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

        const RECT rc = CalculateNewWindowSize(width, height);

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
        auto& mf = pGame->_mineField;

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
                DialogBoxW(pGame->_wc.hInstance, MAKEINTRESOURCEW(IDD_ABOUT), pGame->_hWnd, About);
                break;
            }

            case ID_GAME_NEWGAME:
            {
                pGame->_ResetGame(mf.GetWidth(), mf.GetHeight(), mf.GetNumMines(), true);
                break;
            }

            case ID_SETDIFFICULTY_EASY:
            {
                pGame->_ResetGame(9, 9, 10, true);
                break;
            }

            case ID_SETDIFFICULTY_MEDIUM:
            {
                pGame->_ResetGame(16, 16, 40, true);
                break;
            }

            case ID_SETDIFFICULTY_HARD:
            {
                pGame->_ResetGame(30, 16, 99, true);
                break;
            }

            case ID_SETDIFFICULTY_CUSTOM:
            {
                std::array<int, 3> customInfo{};
                DialogBoxParamW(pGame->_wc.hInstance,
                                MAKEINTRESOURCEW(IDD_CUSTOM),
                                pGame->_hWnd,
                                Custom,
                                reinterpret_cast<LPARAM>(&customInfo));
                pGame->_ResetGame(customInfo[0], customInfo[1], customInfo[2], true);
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

            if (x < 0 || x >= mf.GetWidth() || y < 0 || y >= mf.GetHeight())
                return 0;

            if (!pGame->_gameStarted)
            {
                mf.PlaceMines(x, y);
                pGame->_gameStarted = true;
            }
            else
            {
                const auto cell = mf.GetCellInfo(x, y);

                if (cell.hasFlag)
                {
                    return 0;
                }
                if (cell.hasMine)
                {
                    pGame->_kaBoom = true;
                    pGame->_ForceRedraw(pGame->_hWnd, false);
                    MessageBoxW(hWnd, L"BOOM!", L"BOOM!", MB_OK | MB_ICONEXCLAMATION);
                    pGame->_ResetGame(mf.GetWidth(), mf.GetHeight(), mf.GetNumMines(), false);
                    return 0;
                }
            }

            const auto newlyExploredCells = mf.StepOn(x, y);
            pGame->_numExploredCells += newlyExploredCells;
            if (newlyExploredCells > 0)
            {
                pGame->_ForceRedraw(hWnd, FALSE);
            }

            if (pGame->_numExploredCells >= mf.GetWidth() * mf.GetHeight() - mf.GetNumMines())
            {
                MessageBoxW(pGame->_hWnd, L"You win!", L"You win!", MB_OK);
                pGame->_ResetGame(mf.GetWidth(), mf.GetHeight(), mf.GetNumMines(), true);
            }

            return 0;
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

INT_PTR About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(lParam));
            return TRUE;
        }
    }
    return FALSE;
}

INT_PTR Custom(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static std::array<int, 3>* pCustomInfo;
    switch (msg)
    {
    case WM_INITDIALOG:
        pCustomInfo = reinterpret_cast<std::array<int, 3>*>(lParam);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(lParam));
            return TRUE;
        }
        else if (LOWORD(wParam) == IDOK)
        {
            std::wstringstream wss;
            int width = 0, height = 0, mines = 0;
            wchar_t widthBuffer[4] = { 0 };
            wchar_t heightBuffer[4] = { 0 };
            wchar_t minesBuffer[4] = { 0 };

            GetDlgItemTextW(hDlg, IDC_EDIT_WIDTH, widthBuffer, 4);
            GetDlgItemTextW(hDlg, IDC_EDIT_HEIGHT, heightBuffer, 4);
            GetDlgItemTextW(hDlg, IDC_EDIT_MINES, minesBuffer, 4);
            wss << widthBuffer;
            wss >> width;
            wss.clear();
            wss << heightBuffer;
            wss >> height;
            wss.clear();
            wss << minesBuffer;
            wss >> mines;

            if (width == 0 || height == 0 || mines == 0 || width > 40 || height > 40 || mines > 1000 ||
                mines > width * height - 1)
            {
                MessageBoxW(nullptr, L"Please check your input.", L"Error", MB_OK | MB_ICONEXCLAMATION);
            }
            else
            {
                (*pCustomInfo)[0] = width;
                (*pCustomInfo)[1] = height;
                (*pCustomInfo)[2] = mines;
                EndDialog(hDlg, LOWORD(lParam));
            }

            return TRUE;
        }
    }
    return FALSE;
}

RECT CalculateNewWindowSize(int width, int height)
{
    RECT rc;
    rc.left = 0;
    rc.top = 0;
    rc.right = 2 * MARGIN + BLOCK_SIZE * width;
    rc.bottom = 2 * MARGIN + BLOCK_SIZE * height;
    AdjustWindowRect(&rc, WS_CAPTION, TRUE);
    return rc;
}
