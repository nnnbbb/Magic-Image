#include "utils.h"

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

RECT LoadWindowPlacement() {
    HKEY hKey;
    RECT rc{};
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD posX, posY, width, height;
        DWORD size = sizeof(posX);
        if (RegQueryValueEx(hKey, L"WindowPosX", NULL, NULL, (LPBYTE)&posX, &size) == ERROR_SUCCESS &&
            RegQueryValueEx(hKey, L"WindowPosY", NULL, NULL, (LPBYTE)&posY, &size) == ERROR_SUCCESS &&
            RegQueryValueEx(hKey, L"WindowWidth", NULL, NULL, (LPBYTE)&width, &size) == ERROR_SUCCESS &&
            RegQueryValueEx(hKey, L"WindowHeight", NULL, NULL, (LPBYTE)&height, &size) == ERROR_SUCCESS) {

            rc.left = posX;
            rc.top = posY;

            rc.right = width;
            rc.bottom = height;
        }
        RegCloseKey(hKey);
    }
    return rc;
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
            RegSetValueEx(hKey, valueName, 0, REG_SZ, (const BYTE*)valueData, (wcslen(valueData) + 1) * sizeof(wchar_t));

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