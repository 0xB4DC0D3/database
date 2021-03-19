#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include "DataBase/DataBase.h"

int wmain() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    HWND console{ GetConsoleWindow() };
    RECT r;
    GetWindowRect(console, &r); 
    MoveWindow(console, r.left, r.top, 1100, 500, TRUE);

    HANDLE hOut{ GetStdHandle(STD_OUTPUT_HANDLE) };
    DWORD dwMode{};
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    try {
        DataBase test1{};
    } catch (std::wstring& e) {
        std::wcout << L"Помилка: " << e << L"\nПерезапустіть програму!" << std::endl;
    } catch (web::json::json_exception& e) {
        std::wcout << L"Помилка: " << e.what() << L"\nПерезапустіть програму!" << std::endl;
    }
}