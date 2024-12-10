#include "utils.h"
#include "capture-screen.h"

String Ocr(HWND hwnd, WPARAM wParam) {
    TRECT rect = GetWindowAttributeRect(hwnd);

    LONG width = rect.width;
    LONG height = rect.height;
    LONG x = rect.left;
    LONG y = rect.top + height;
    if (wParam == VK_KEY_3_DOWN) {
        y = rect.top;
    }
    bool noCache = false;
    if (wParam == VK_KEY_5_DOWN) {
        noCache = true;
    }
    std::string path = CaptureScreen(x, y, width, height);

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件: " << path << std::endl;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string fileContent = oss.str();

    httplib::MultipartFormDataItems items = {
      {"img", fileContent, "image.jpg", "image/jpeg"},
      {"no_cache", (noCache ? "True" : "False"), "", "application/json"}
    };

    return Post("/ocr", items);
}

void SaveWindowPlacement(HWND hwnd) {
    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {
        HKEY hKey;
        LONG width = rect.right - rect.left;
        LONG height = rect.bottom - rect.top;
        if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            RegSetValueEx(hKey, L"WindowPosX", 0, REG_DWORD, (const BYTE*)&rect.left, sizeof(rect.left));
            RegSetValueEx(hKey, L"WindowPosY", 0, REG_DWORD, (const BYTE*)&rect.top, sizeof(rect.top));
            RegSetValueEx(hKey, L"WindowWidth", 0, REG_DWORD, (const BYTE*)&width, sizeof(int));
            RegSetValueEx(hKey, L"WindowHeight", 0, REG_DWORD, (const BYTE*)&height, sizeof(int));
            RegCloseKey(hKey);
        }
    }
}

TRECT LoadWindowPlacement() {
    HKEY hKey;
    TRECT rc = {};
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD posX, posY, width, height;
        DWORD size = sizeof(posX);
        if (RegQueryValueEx(hKey, L"WindowPosX", NULL, NULL, (LPBYTE)&posX, &size) == ERROR_SUCCESS &&
            RegQueryValueEx(hKey, L"WindowPosY", NULL, NULL, (LPBYTE)&posY, &size) == ERROR_SUCCESS &&
            RegQueryValueEx(hKey, L"WindowWidth", NULL, NULL, (LPBYTE)&width, &size) == ERROR_SUCCESS &&
            RegQueryValueEx(hKey, L"WindowHeight", NULL, NULL, (LPBYTE)&height, &size) == ERROR_SUCCESS) {

            rc.left = posX;
            rc.top = posY;

            rc.width = width;
            rc.height = height;
        }
        RegCloseKey(hKey);
    }
    return rc;
}

// 将文本放入剪贴板
bool SetClipboardText(const std::string& text) {
    // 打开剪贴板
    if (!OpenClipboard(nullptr)) {
        std::cerr << "无法打开剪贴板" << std::endl;
        return false;
    }

    // 清空剪贴板
    EmptyClipboard();

    // 分配全局内存以存放文本
    HGLOBAL glob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (glob) {
        // 将文本复制到全局内存
        memcpy(GlobalLock(glob), text.c_str(), text.size() + 1);
        GlobalUnlock(glob);

        // 设置剪贴板数据
        SetClipboardData(CF_TEXT, glob);
    }

    CloseClipboard();
    return true;
}

void SendCtrlC() {
    keybd_event(VK_CONTROL, 0, 0, 0);
    keybd_event('C', 0, 0, 0);

    keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
}

// 将文本放入剪贴板
String GetClipboardText() {
    SendCtrlC();
    Sleep(300);

    if (!OpenClipboard(nullptr)) {
        return "Failed to open clipboard";
    }

    HANDLE hData = GetClipboardData(CF_TEXT);  // CF_TEXT 表示 ANSI 文本格式
    if (hData == nullptr) {
        CloseClipboard();
        return "No clipboard data available";
    }

    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        CloseClipboard();
        return "Failed to lock clipboard data";
    }

    std::string clipboardText(pszText);

    GlobalUnlock(hData);
    CloseClipboard();

    return clipboardText;
}


void CheckAndCreateRegistryKey() {
    const wchar_t* keyPath = REGISTRY_KEY.c_str();  // 注册表路径
    HKEY hKey;

    // 尝试打开注册表键
    LONG result = RegOpenKeyEx(
      HKEY_CURRENT_USER,  // 根键
      keyPath,            // 键路径
      0,                  // 保留值，通常为0
      KEY_READ,           // 访问权限
      &hKey               // 输出的键句柄
    );

    if (result == ERROR_SUCCESS) {
        std::wcout << L"Registry key already exists." << std::endl;
        RegCloseKey(hKey);  // 关闭句柄
    } else if (result == ERROR_FILE_NOT_FOUND) {
        // 键不存在，创建它
        LONG createResult = RegCreateKeyEx(
          HKEY_CURRENT_USER,  // 根键
          keyPath,            // 键路径
          0,                  // 保留值，通常为0
          NULL,               // 类名，通常为NULL
          0,                  // 创建选项，通常为0
          KEY_WRITE,          // 访问权限
          NULL,               // 安全属性
          &hKey,              // 输出的键句柄
          NULL                // 可选的创建标志
        );

        if (createResult == ERROR_SUCCESS) {
            std::wcout << L"Registry key created successfully." << std::endl;

            // 在此可以设置值，如果需要的话
            const wchar_t* valueName = L"MyValue";
            const wchar_t* valueData = L"Hello, Registry!";
            RegSetValueEx(
              hKey,
              valueName,
              0,
              REG_SZ,
              (const BYTE*)valueData,
              (wcslen(valueData) + 1) * sizeof(wchar_t)
            );

            RegCloseKey(hKey);  // 关闭句柄
        } else {
            std::cerr << "Failed to create registry key. Error code: " << createResult << std::endl;
        }
    } else {
        std::cerr << "Failed to open registry key. Error code: " << result << std::endl;
    }
}

void HotKeyChangeWindowSize(HWND hwnd, WPARAM wParam) {
    int size = 4;
    std::string direction = std::string() + (char)wParam;
    RECT rect;
    GetWindowRect(hwnd, &rect);

    LONG width = rect.right - rect.left;
    LONG height = rect.bottom - rect.top;
    width = width < 50 ? 50 : width;
    height = height < 50 ? 50 : height;
    if (direction == "W") {
        rect.right = width;
        rect.bottom = height + size;
        rect.top -= size;
    }
    if (direction == "S") {
        rect.right = width;
        rect.bottom = height - size;
        rect.top += size;
    }
    if (direction == "A") {
        rect.bottom = height;
        rect.right = width - size;
    }
    if (direction == "D") {
        rect.bottom = height;
        rect.right = width + size;
    }

    if (wParam == 'W' || wParam == 'S' || wParam == 'A' || wParam == 'D') {
        SetWindowPos(
          hwnd,
          NULL,
          rect.left, rect.top, rect.right, rect.bottom,
          SWP_NOZORDER
        );
    }
}

void HotKeyMoveWindow(HWND hwnd, WPARAM wParam) {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    LONG width = rect.right - rect.left;
    LONG height = rect.bottom - rect.top;
    int size = 3;

    if (wParam == VK_UP) {
        rect.right = width;
        rect.bottom = height;
        rect.top -= size;
    }
    if (wParam == VK_DOWN) {
        rect.right = width;
        rect.bottom = height;
        rect.top += size;
    }
    if (wParam == VK_LEFT) {
        rect.right = width;
        rect.bottom = height;
        rect.left -= size;
    }
    if (wParam == VK_RIGHT) {
        rect.right = width;
        rect.bottom = height;
        rect.left += size;
    }

    if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT) {
        SetWindowPos(
          hwnd,
          NULL,
          rect.left, rect.top, rect.right, rect.bottom,
          SWP_NOZORDER
        );
    }
}

void SetWindowTop(HWND hwnd) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void SetWindowUnTop(HWND hwnd) {
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void HotKeyWindowTop(HWND hwnd, WPARAM wParam) {
    static bool isTop = true;
    if (wParam == 'T') {  // 按下 'T' 键时将窗口置顶
        if (isTop == true) {
            SetWindowTop(hwnd);
            isTop = false;
        } else {
            SetWindowUnTop(hwnd);
            isTop = true;
        }
    }
}

void PaintText(HDC hdc, std::string text, int windowWidth, bool center, bool newline) {
    int y = 30, height = 40;  // 初始绘制位置
    HFONT font = CreateFont(
      height,                         // 字体高度
      0,                              // 字体宽度
      0,                              // 字体倾斜角度
      0,                              // 字体倾斜方向
      FW_NORMAL,                      // 字体重量
      FALSE,                          // 斜体
      FALSE,                          // 下划线
      FALSE,                          // 删除线
      DEFAULT_CHARSET,                // 字符集
      OUT_PS_ONLY_PRECIS,             // 输出精度
      CLIP_STROKE_PRECIS,             // 剪辑精度
      PROOF_QUALITY,                  // 输出质量
      DEFAULT_PITCH | FF_SWISS,       // 字体类别
      L"CodeNewRoman Nerd Font Mono"  // 字体
    );

    SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);  // 透明背景
    SetTextColor(hdc, RGB(255, 255, 255));

    // 获取文本的宽度
    SIZE textSize;
    GetTextExtentPoint32A(hdc, text.c_str(), text.length(), &textSize);

    int x = 10;
    if (center) {
        x = (windowWidth - textSize.cx) / 2;
    }

    if (newline) {
        std::stringstream ss(text);
        std::string line;
        int i = 0;
        while (std::getline(ss, line)) {
            TextOutA(hdc, x, y + i * height, line.c_str(), line.length());
            i++;
        }
    } else {
        TextOutA(hdc, x, y, text.c_str(), text.length());
    }
    DeleteObject(font);
}

TRECT GetWindowAttributeRect(HWND hwnd) {
    RECT rect;

    DwmGetWindowAttribute(
      hwnd,
      DWMWA_EXTENDED_FRAME_BOUNDS,
      &rect,
      sizeof(rect)
    );
    LONG width = rect.right - rect.left;
    LONG height = rect.bottom - rect.top;
    LONG left = rect.left;
    LONG top = rect.top;
    LONG right = rect.right;
    LONG bottom = rect.bottom;

    TRECT trect = {left, top, right, bottom, width, height};
    return trect;
}

void DrawRect(HWND hwnd, HDC hdc, COLORREF solidColor) {
    // 设置边框颜色
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));  // 边框为红色
    TRECT trect = GetWindowAttributeRect(hwnd);
    RECT rect = {trect.left, trect.top + trect.height, trect.right, trect.bottom};

    // 绘制矩形边框
    FrameRect(hdc, &rect, hBrush);

    // 删除笔刷
    DeleteObject(hBrush);
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    auto targetProcessData = reinterpret_cast<std::pair<DWORD, HWND*>*>(lParam);

    if (processId == targetProcessData->first) {
        *(targetProcessData->second) = hwnd;
        return FALSE;  // 停止枚举
    }

    return TRUE;
}

HWND GetHwndFromProcess(HANDLE hProcess) {
    DWORD processId = GetProcessId(hProcess);
    if (processId == 0) {
        std::cerr << "Failed to get process ID." << std::endl;
        return nullptr;
    }

    HWND hwnd = nullptr;

    // 将目标进程 ID 和句柄指针打包成一个结构
    std::pair<DWORD, HWND*> processData = {processId, &hwnd};

    // 枚举所有顶层窗口
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&processData));

    return hwnd;
}

FILE* stdinNew = nullptr;
FILE* stdoutNew = nullptr;
FILE* stderrNew = nullptr;


void SetupConsole() {  // 分配控制台
    AllocConsole();
    freopen_s(&stdinNew, "CONIN$", "r+", stdin);
    freopen_s(&stdoutNew, "CONOUT$", "w+", stdout);
    freopen_s(&stderrNew, "CONOUT$", "w+", stderr);
    SetConsoleTitle(L"Console");
    HWND hConsole = GetConsoleWindow();
    if (hConsole) {
        // 获取当前窗口样式
        LONG style = GetWindowLong(hConsole, GWL_STYLE);
        // 移除关闭按钮样式
        style &= ~WS_SYSMENU;
        // 设置新的窗口样式
        SetWindowLong(hConsole, GWL_STYLE, style);
    }
    printf("Hello World！\n");
}

void DestroyConsole() {

    if (stdinNew) {
        fclose(stdinNew);
    }
    if (stdoutNew) {
        fclose(stdoutNew);
    }

    if (stderrNew) {
        fclose(stderrNew);
    }
    FreeConsole();
}



void Setup() {
}

void Destroy() {
}