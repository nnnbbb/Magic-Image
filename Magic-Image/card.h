#include <windows.h>
#include <windowsx.h>
#include <stdexcept>
#include "utils.h"

class Card {
   public:
    Card();
    ~Card();
    void Show(int nCmdShow);
    void SetText(const std::string& text);

    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static std::vector<std::unique_ptr<Card>> s_instances;

   private:
    HINSTANCE m_hInstance;
    WString m_className = L"CardWindowClass";

    HWND m_hwnd;
    String m_word;
};

void MakeCard(const std::string text);