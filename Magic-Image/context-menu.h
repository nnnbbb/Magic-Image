#include <windows.h>

constexpr auto MENU_ID1 = 1;
constexpr auto MENU_ID2 = 2;

class MenuContext {
   public:
    HWND pythonTerminalHwnd;
    MenuContext();
    ~MenuContext();

    void ToggleCheckState();
    HMENU CreateMenu();
    void UpdateMenuItemCheckState();
    void HandleMenuCommand(int command);
    HMENU GetMenuHandle() const;

   private:
    HMENU hMenu;
    bool isChecked;
};