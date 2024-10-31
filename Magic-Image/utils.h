#include <windows.h>
#include <string>
#include <iostream>

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