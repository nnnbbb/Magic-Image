#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <dwmapi.h>
#include <string>
#include <png.h>
#include <filesystem>
#include <cstdio>
#include "utils.h"
#include "http-requests.h"
#include "capture-screen.h"
#include "context-menu.h"
#include "card.h"


static std::string text = "Hello, Windows";
static MenuContext menuContext;
static bool shortcutKey = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    static bool isDragging = false;
    static POINT pt{};
    static bool visible = true;
    static RECT borderThickness;

    switch (uMsg) {
        case WM_HOTKEY: {
            if (wParam == VK_KEY_F12_DOWN) {
                shortcutKey = !shortcutKey;
            }
            if (shortcutKey && VK_KEY_2_DOWN) {
                String w = GetClipboardText();
                MakeCard(w);
            }
            if (
              shortcutKey &&
              (wParam == VK_KEY_3_DOWN ||
               wParam == VK_KEY_4_DOWN ||
               wParam == VK_KEY_5_DOWN)
            ) {
                visible = !visible;
                ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);
                visible ? SetWindowTop(hwnd) : SetWindowUnTop(hwnd);

                if (visible == false) {
                    text = Ocr(hwnd, wParam);
                    if (!text.empty()) {
                        SetClipboardText(text);
                    }
                }
            }
            break;
        }
        case WM_KEYDOWN: {
            HotKeyChangeWindowSize(hwnd, wParam);
            HotKeyMoveWindow(hwnd, wParam);
            HotKeyWindowTop(hwnd, wParam);

            // 检测 ESC 键
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
                return 0;
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            isDragging = true;
            SetCapture(hwnd);
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (isDragging) {
                POINT currentPt;
                GetCursorPos(&currentPt);
                ScreenToClient(hwnd, &currentPt);

                RECT rect;
                GetWindowRect(hwnd, &rect);
                SetWindowPos(
                  hwnd,
                  NULL,
                  rect.left + currentPt.x - pt.x,
                  rect.top + currentPt.y - pt.y,
                  0, 0, SWP_NOSIZE | SWP_NOZORDER
                );
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            isDragging = false;
            ReleaseCapture();
            return 0;
        }
        case WM_RBUTTONUP: {
            // 获取鼠标位置
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = menuContext.CreateMenu();
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

            DestroyMenu(hMenu);
            break;
        }
        case WM_COMMAND: {
            menuContext.HandleMenuCommand(wParam);
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc = ps.rcPaint;

            HBRUSH hBrush = CreateSolidBrush(RGB(24, 53, 94));  // 创建黑色画刷
            FillRect(hdc, &rc, hBrush);                         // 用黑色填充窗口
            DeleteObject(hBrush);

            PaintText(hdc, text, rc.right - rc.left, true);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) {
                pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                ScreenToClient(hwnd, &pt);

                RECT rc;
                GetClientRect(hwnd, &rc);

                if (pt.x < rc.left + 5 ||
                    pt.x >= rc.right - 5 ||
                    pt.y < rc.top + 5 ||
                    pt.y >= rc.bottom - 5) {
                    return HTBORDER;  // 鼠标在窗口边缘
                }
            }

            return hit;
        }
        case WM_NCACTIVATE: {
            /* Returning 0 from this message disable the window from receiving activate events which is not
            desirable. However When a visual style is not active (?) for this window, "lParam" is a handle to an
            optional update region for the nonclient area of the window. If this parameter is set to -1,
            DefWindowProc does not repaint the nonclient area to reflect the state change. */
            lParam = -1;
            break;
        }

        case WM_NCCALCSIZE: {
            if (wParam) {
                /* Detect whether window is maximized or not. We don't need to change the resize border when win is
                 *  maximized because all resize borders are gone automatically */
                WINDOWPLACEMENT wPos;
                // GetWindowPlacement fail if this member is not set correctly.
                wPos.length = sizeof(wPos);
                GetWindowPlacement(hwnd, &wPos);
                if (wPos.showCmd != SW_SHOWMAXIMIZED) {
                    SetRectEmpty(&borderThickness);
                    AdjustWindowRectEx(
                      &borderThickness,
                      GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_CAPTION,
                      FALSE,
                      NULL
                    );
                    borderThickness.left *= -1;
                    borderThickness.top *= -1;
                    NCCALCSIZE_PARAMS* sz = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                    // Add 1 pixel to the top border to make the window resizable from the top border
                    // sz->rgrc[0].top += 7;
                    sz->rgrc[0].left += borderThickness.left;
                    sz->rgrc[0].right -= borderThickness.right;
                    sz->rgrc[0].bottom -= borderThickness.bottom;
                    return 0;
                }
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HANDLE hProcess;
DWORD WINAPI ExecutePythonScript(LPVOID lpParam) {
    std::wstring pythonPath = L"py.exe";
    std::wstring scriptPath = L"D:/code/Magic-Image/Magic-Image/api/python/gemma_api.py";

    std::filesystem::path path(scriptPath);
    std::wstring directory = path.parent_path().wstring();

    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;  // 允许在执行后获得进程句柄
    sei.hwnd = NULL;
    sei.lpVerb = NULL;                      // 默认操作
    sei.lpFile = pythonPath.c_str();        // 可执行文件
    sei.lpParameters = scriptPath.c_str();  // 传递的参数
    sei.lpDirectory = directory.c_str();    // 目录
    sei.nShow = SW_HIDE /* SW_SHOW */;      // 窗口显示方式
    sei.hInstApp = NULL;

    if (ShellExecuteEx(&sei)) {

        hProcess = sei.hProcess;
        WaitForSingleObject(hProcess, INFINITE);
        CloseHandle(hProcess);
    } else {
        std::cerr << "ShellExecuteEx failed (" << GetLastError() << ")." << std::endl;
        return 1;
    }

    return 0;
}

HWND CreateMainWindow(const wchar_t CLASS_NAME[] = L"Sample Window Class") {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class." << std::endl;
        return 0;  // 确保类注册成功
    }
    CheckAndCreateRegistryKey();
    TRECT rc = LoadWindowPlacement();

    HWND hwnd = CreateWindowEx(
      WS_EX_TOOLWINDOW,
      CLASS_NAME,
      L"",
      WS_POPUP | WS_SIZEBOX | WS_VISIBLE,  // 修改窗口样式以允许调整大小但没有标题栏
      rc.left, rc.top, rc.width, rc.height,
      NULL,
      NULL,
      wc.hInstance,
      NULL
    );

    if (hwnd == NULL) {
        std::cerr << "Failed to create window." << std::endl;
        return 0;
    }

    // 置顶
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    if (
      !RegisterHotKey(hwnd, VK_KEY_2_DOWN, 0, VK_KEY_2) ||  // MOD_ALT
      !RegisterHotKey(hwnd, VK_KEY_3_DOWN, 0, VK_KEY_3) ||
      !RegisterHotKey(hwnd, VK_KEY_4_DOWN, 0, VK_KEY_4) ||
      !RegisterHotKey(hwnd, VK_KEY_5_DOWN, 0, VK_KEY_5) ||
      !RegisterHotKey(hwnd, VK_KEY_F12_DOWN, 0, VK_F11)
    ) {
        MessageBox(NULL, L"Hotkey Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    return hwnd;
}


void SendSigint(DWORD pid) {
    AttachConsole(pid);
    SetConsoleCtrlHandler(NULL, TRUE);
    GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
    FreeConsole();
}

int WINAPI WinMain(
  _In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPSTR lpCmdLine,
  _In_ int nShowCmd
) {

#ifdef _DEBUG
    SetupConsole();
#endif
    HWND hwnd = CreateMainWindow();

    HANDLE hThread = CreateThread(NULL, 0, ExecutePythonScript, NULL, 0, NULL);

    if (hThread == NULL) {
        std::cerr << "CreateThread failed (" << GetLastError() << ")." << std::endl;
        return 1;
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (hProcess && menuContext.pythonTerminalHwnd == 0) {
            menuContext.pythonTerminalHwnd = GetHwndFromProcess(hProcess);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

#ifdef _DEBUG
    DestroyConsole();
#endif

    if (hProcess) {
        DWORD processId = GetProcessId(hProcess);
        SendSigint(processId);
    }

    CloseHandle(hThread);
    if (hwnd) {
        SaveWindowPlacement(hwnd);
        UnregisterHotKey(hwnd, VK_KEY_2_DOWN);
        UnregisterHotKey(hwnd, VK_KEY_3_DOWN);
        UnregisterHotKey(hwnd, VK_KEY_4_DOWN);
        UnregisterHotKey(hwnd, VK_KEY_5_DOWN);
        UnregisterHotKey(hwnd, VK_KEY_F12_DOWN);
    }
    return 0;
}
