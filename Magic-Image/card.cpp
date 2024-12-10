#include "card.h"

std::vector<std::unique_ptr<Card>> Card::s_instances;

void MakeCard(const std::string text) {
    auto card = std::make_unique<Card>();
    card->SetText(text);
    Card::s_instances.push_back(std::move(card));
}

Card::Card() : m_hInstance(GetModuleHandle(NULL)) {

    WNDCLASS wc = {0};
    wc.lpfnWndProc = Card::StaticWndProc;
    wc.hInstance = m_hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = m_className.c_str();

    if (!GetClassInfo(m_hInstance, m_className.c_str(), &wc)) {
        wc.lpfnWndProc = Card::StaticWndProc;
        wc.hInstance = m_hInstance;
        wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
        wc.lpszClassName = m_className.c_str();

        if (!RegisterClass(&wc)) {
            DWORD error = GetLastError();
            wchar_t errorMessage[256];
            swprintf_s(errorMessage, L"窗口类注册失败，错误代码: %lu", error);
            MessageBox(nullptr, errorMessage, L"错误", MB_OK | MB_ICONERROR);
            throw std::runtime_error("窗口类注册失败");
        }
    }

    m_hwnd = CreateWindowExW(
      WS_EX_TOOLWINDOW,
      m_className.c_str(),
      L"My Card Window",
      WS_POPUP | WS_SIZEBOX | WS_VISIBLE,
      10, 10,
      800, 130, nullptr, nullptr, m_hInstance, this
    );

    if (m_hwnd == nullptr) {
        MessageBox(nullptr, L"窗口创建失败！", L"错误", MB_OK | MB_ICONERROR);
        throw std::runtime_error("窗口创建失败");
    }
    SetWindowTop(m_hwnd);

    s_instances.emplace_back(this);
}

Card::~Card() {
    auto it = std::find_if(s_instances.begin(), s_instances.end(), [this](const std::unique_ptr<Card>& instance) { return instance.get() == this; });
    if (it != s_instances.end()) {
        s_instances.erase(it);
    }
    UnregisterClass(m_className.c_str(), m_hInstance);
}

void Card::Show(int nCmdShow) {
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
}


void Card::SetText(const std::string& word) {
    m_word = word + "\r\n" + GetWordSuggest(word);
    if (m_hwnd) {
        InvalidateRect(m_hwnd, nullptr, TRUE);
    }
}

// 静态窗口过程
LRESULT CALLBACK Card::StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Card* pThis = nullptr;

    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<Card*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<Card*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {

        return pThis->WndProc(hwnd, message, wParam, lParam);
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}


LRESULT CALLBACK Card::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static bool isDragging = false;
    static POINT pt{};
    static RECT borderThickness;
    switch (message) {
        case WM_KEYDOWN: {

            // 检测 ESC 键
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
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
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc = ps.rcPaint;

            HBRUSH hBrush = CreateSolidBrush(RGB(24, 53, 94));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);

            PaintText(hdc, m_word, rc.right - rc.left, false, true);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, message, wParam, lParam);
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
            lParam = -1;
            break;
        }
        case WM_DESTROY: {
            // PostQuitMessage(0);
            break;
        }
        case WM_NCCALCSIZE: {
            if (wParam) {
                WINDOWPLACEMENT wPos;
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
                    // sz->rgrc[0].top += 7;
                    sz->rgrc[0].left += borderThickness.left;
                    sz->rgrc[0].right -= borderThickness.right;
                    sz->rgrc[0].bottom -= borderThickness.bottom;
                    return 0;
                }
            }
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}