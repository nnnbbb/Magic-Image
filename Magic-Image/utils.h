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

const std::wstring REGISTRY_KEY = L"hkcu\\MagicImage";

constexpr auto F4_KEY_DOWN = 1;

void SaveWindowPlacement(HWND hwnd);

RECT LoadWindowPlacement();

void CheckAndCreateRegistryKey();

void HotKeyChangeWindowSize(HWND hwnd, WPARAM wParam);

void HotKeyMoveWindow(HWND hwnd, WPARAM wParam);

void HotKeyWindowTop(HWND hwnd, WPARAM wParam);

void SetWindowTop(HWND hwnd);

void SetWindowUnTop(HWND hwnd);

void SetupConsole();

void DestroyConsole();