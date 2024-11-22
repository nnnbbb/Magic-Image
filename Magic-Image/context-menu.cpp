#include "context-menu.h"

MenuContext::MenuContext() : hMenu(NULL), isChecked(false), pythonTerminalHwnd(0) {}
MenuContext::~MenuContext() {
    if (hMenu) {
        DestroyMenu(hMenu);
    }
}

void MenuContext::ToggleCheckState() {
    isChecked = !isChecked;
}

HMENU MenuContext::CreateMenu() {
    hMenu = CreatePopupMenu();
    AppendMenu(
      hMenu,
      (MF_STRING | (isChecked ? MF_CHECKED : MF_UNCHECKED)),
      MENU_ID1,
      L"显示Python终端"
    );
    // AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, MENU_ID2, L"退出");
    return hMenu;
}

void MenuContext::UpdateMenuItemCheckState() {
    if (hMenu) {

        CheckMenuItem(hMenu, MENU_ID1, isChecked ? MF_CHECKED : MF_UNCHECKED);
        ShowWindow(this->pythonTerminalHwnd, isChecked ? SW_SHOW : SW_HIDE);
    }
}

void MenuContext::HandleMenuCommand(int command) {
    switch (command) {
        case MENU_ID1:
            ToggleCheckState();
            UpdateMenuItemCheckState();
            break;
        case MENU_ID2:
            PostQuitMessage(0);
            break;
        default:
            break;
    }
}

HMENU MenuContext::GetMenuHandle() const {
    return hMenu;
}
