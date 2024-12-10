#include <string>
#include <vector>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <chrono>
#include <ctime>
#include <locale>
#include <codecvt>
#include <iomanip>
#include <sstream>
#include <thread>
#include <filesystem>
#include <algorithm>
#include <shellapi.h>
#include <dwmapi.h>
#include <httplib.h>
#include "http-requests.h"
#include <curl/curl.h>


#ifndef UTILS_H_
#define UTILS_H_

typedef unsigned long u32;
typedef unsigned long long u64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef std::string String;
typedef std::wstring WString;
template <typename T>
using Vector = std::vector<T>;

String Utf16ToLocalCP(const WString& wstr);
String Utf16ToLocalCP(const wchar_t* wstr);

String LocalCPToUtf8(const String& str);
String LocalCPToUtf8(const char* str);

String Utf16ToUtf8(const WString& wstr);
String Utf16ToUtf8(const wchar_t* wstr);

WString Utf8ToUtf16(const String& str);
WString Utf8ToUtf16(const char* str);

String Utf8ToLocalCP(const String& str);
String Utf8ToLocalCP(const char* str);

typedef struct _TRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    LONG width;
    LONG height;
} TRECT, *PTRECT;

const std::wstring REGISTRY_KEY = L"hkcu\\MagicImage";

constexpr auto F4_KEY_DOWN = 1;
constexpr auto VK_KEY_2_DOWN = 2;
constexpr auto VK_KEY_3_DOWN = 3;
constexpr auto VK_KEY_4_DOWN = 4;
constexpr auto VK_KEY_5_DOWN = 5;
constexpr auto VK_KEY_F12_DOWN = 6;

constexpr auto VK_KEY_2 = '2';
constexpr auto VK_KEY_3 = '3';
constexpr auto VK_KEY_4 = '4';
constexpr auto VK_KEY_5 = '5';

String Ocr(HWND hwnd, WPARAM wParam);

bool SetClipboardText(const std::string& text);

String GetClipboardText();

void DrawRect(HWND hwnd, HDC hdc, COLORREF solidColor = 0);

void SaveWindowPlacement(HWND hwnd);

TRECT LoadWindowPlacement();

void CheckAndCreateRegistryKey();

void HotKeyChangeWindowSize(HWND hwnd, WPARAM wParam);

void HotKeyMoveWindow(HWND hwnd, WPARAM wParam);

void HotKeyWindowTop(HWND hwnd, WPARAM wParam);

void SetWindowTop(HWND hwnd);

void SetWindowUnTop(HWND hwnd);

void SetupConsole();

void DestroyConsole();

void PaintText(HDC hdc, std::string text, int windowWidth, bool center = false, bool newline = false);

TRECT GetWindowAttributeRect(HWND hwnd);

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

HWND GetHwndFromProcess(HANDLE hProcess);

#define FN_ADDRESS(func) \
    std::cout << "Function address of " #func ": " << "0x" << (void*)func << std::endl;

#define NOT_NULL(value)                                 \
    do {                                                \
        if (value == NULL) {                            \
            throw std::runtime_error("Handle is NULL"); \
        }                                               \
    } while (0)

#endif  // !UTILS_H_
