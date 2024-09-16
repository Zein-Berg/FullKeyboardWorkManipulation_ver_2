#include <windows.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>
#include <shellscalingapi.h>

#define HOTKEY_ID_SHOW_GRID 1
#define HOTKEY_ID_EXIT 2
#define HOTKEY_ID_UNHOOK 11

#define HOTKEY_ID_LEFTCLICK 3
#define HOTKEY_ID_RIGHTCLICK 4

#define HOTKEY_ID_UP 5
#define HOTKEY_ID_DOWN 6
#define HOTKEY_ID_LEFT 7
#define HOTKEY_ID_RIGHT 8

#define HOTKEY_ID_SCROLL_UP 9
#define HOTKEY_ID_SCROLL_DOWN 10

#pragma comment(lib, "Shcore.lib")
//using namespace std;

HHOOK hKeyboardHook;
bool keyStates[256] = { false };
std::vector<wchar_t> glBuffer;
bool isLeftClicking = false;

std::vector<std::vector<char>> pairs;

void SetHook();
void Unhook();
void makePairs();

double getMonitorScaler() {
    double scaler;
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    auto x = unsigned{};
    auto y = unsigned{};
    GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &x, &y);


    switch (x) {
    case 96:
        scaler = 1.04;
        break;
    case 120:
        scaler = 1.04;
        break;
    case 140:
        scaler = 1.04;
        break;
    default:
        scaler = 1.04;
        break;
    }
    return scaler;
}

// Функция для имитации нажатия левой кнопки мыши
void LeftClickDown() {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
}
// Функция для имитации отпускания левой кнопки мыши
void LeftClickUp() {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}
// Функция для имитации нажатия левой кнопки мыши
void LeftClick() {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Функция для имитации нажатия правой кнопки мыши
void RightClick() {
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Установить раскладку языка
void SetKeyboardLayout(LPCWSTR languageId) {
    HKL hkl = LoadKeyboardLayoutW(languageId, KLF_ACTIVATE);
    PostMessage(GetConsoleWindow(), WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)hkl);
}

// поправить запуск хука, так чтоб только при открытии грида, иначе блять хуйня какая-то
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    HMONITOR* pActiveMonitor = (HMONITOR*)dwData;
    if (hMonitor != *pActiveMonitor) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(hMonitor, &mi)) {
            // Переместить курсор на позицию (100, 100) неактивного монитора
            SetCursorPos(mi.rcMonitor.left + 100, mi.rcMonitor.top + 100);
        }
        return FALSE; // Остановить перечисление мониторов
    }
    return TRUE; // Продолжить перечисление мониторов
}

std::pair<int, int> getCellPosition() {
    if (glBuffer.size() != 2)
        return std::pair<int, int>{-1, -1};
    if ((int)glBuffer[0] == (int)'[' && (int)glBuffer[1] == (int)']') { // goto unactive screen
        OutputDebugString(L"___Key [ & ] was pressed simultaneously!\n");



        

        return std::pair<int, int>{-100, -100};
    }
    else
    {
        std::wstring bufferOutput;
        int pairPosition = -1;
        for (int i = 0; i < pairs.size(); i++) {
            bufferOutput = L"_Testing__ " + std::wstring(1, pairs[i][1]) + L" & " + std::wstring(1, glBuffer[1]) + L" ";
            bufferOutput += L"_int__ " + std::to_wstring((int)pairs[i][1]) + L" & " + std::to_wstring((int)glBuffer[1]) + L"| ";
            //OutputDebugString(bufferOutput.c_str());
            //OutputDebugString(L"\n");
            if ((int)pairs[i][0] == (int)glBuffer[0] && (int)pairs[i][1] == (int)glBuffer[1]) {
                pairPosition = i;
            }
        }
        if (pairPosition == -1) {
            return std::pair<int, int>{-1, -1};
        }

        double scaler = getMonitorScaler();

        const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
        const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);

        int numCells = sqrt(pairs.size());

        int cellSize = sqrt(SCREEN_HEIGHT * SCREEN_WIDTH * scaler / pairs.size());

        int cellAmountInRow = (SCREEN_WIDTH / cellSize);
        if (SCREEN_WIDTH % cellSize != 0) {
            cellAmountInRow++;
        }

        int x;
        int y = pairPosition / cellAmountInRow;
        //if (pairPosition % cellAmountInRow == 0) {
        //    y--;
        //    x = cellAmountInRow;
        //}
        //else { x = pairPosition - y * cellAmountInRow; }
        x = pairPosition - y * cellAmountInRow;
        std::wstring outputString = L"Y: " + std::to_wstring(y);
        outputString += L"\nPairPosition: " + std::to_wstring(pairPosition);
        outputString += L"\nWidthAmount: " + std::to_wstring(cellAmountInRow);
        outputString += L"\nX: " + std::to_wstring(x) + L"\n";

        OutputDebugString(outputString.c_str());

        return std::pair<int, int>{x* cellSize + cellSize / 2, y* cellSize + cellSize / 2};
    }
            
}


void gotoPoint(std::pair<int, int> coordinate) {
    std::wstring glBufferOutput = L"___Key: " + std::wstring(1,glBuffer[0])
        + L" & " + std::wstring(1, glBuffer[1]) + L" was pressed simultaneously!\n";
    OutputDebugString(glBufferOutput.c_str());

    POINT pt;
    GetCursorPos(&pt);
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (coordinate.first == -100 && coordinate.second == -100) {
        EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&hMonitor);
        OutputDebugString(L"___Cursor GOTO pos 100x100 on other unactive screen!\n");
    }
    else
    {
        MONITORINFO mi = { sizeof(mi) };
        
        if (GetMonitorInfo(hMonitor, &mi)) {
            SetCursorPos(mi.rcMonitor.left + coordinate.first, mi.rcMonitor.top + coordinate.second);
            
        }
        glBufferOutput = std::to_wstring(coordinate.first) + L"x" + std::to_wstring(coordinate.second) + L"\n";
        OutputDebugString(L"___Cursor GOTO pos ");
        OutputDebugString(glBufferOutput.c_str());
    }
    HWND mainHwnd = FindWindow(L"GridWindowClass", NULL);
    
    
    SetWindowPos(mainHwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_HIDEWINDOW);

    Unhook();
   
    //SetActiveWindow(0);
    //if (HideCaret(GetForegroundWindow()) == 0)
    //    OutputDebugString(L"___Erorr caret cant be hidden\n");
    //DestroyCaret();
    //HideCaret(GetFocus());
}

// Проверка, открыто ли окно
bool IsWindowOpen(HWND hwnd)
{
    return IsWindow(hwnd) && IsWindowVisible(hwnd);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        bool alt;
        bool f2;
        bool isCtrlAltPressed;
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyBoard->vkCode;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            
            if (!keyStates[vkCode]) {
                keyStates[vkCode] = true;
                f2 = HIWORD(GetAsyncKeyState(VK_F2)) != 0;
                //alt = HIWORD(GetKeyState(VK_MENU)) != 0;
                //isCtrlAltPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_MENU) & 0x8000);
                //if (alt) {
                //    OutputDebugString(L"_Hook__ Alt btn was pressed!\n");
                //}
                //if (isCtrlAltPressed) {
                //    POINT pt;
                //    GetCursorPos(&pt);
                //
                //    switch (vkCode) {
                //    case VK_UP:
                //        pt.y -= 10; // Двигаем курсор вверх
                //        break;
                //    case VK_DOWN:
                //        pt.y += 10; // Двигаем курсор вниз
                //        break;
                //    case VK_LEFT:
                //        pt.x -= 10; // Двигаем курсор влево
                //        break;
                //    case VK_RIGHT:
                //        pt.x += 10; // Двигаем курсор вправо
                //        break;
                //    default:
                //        break;
                //    }
                //    SetCursorPos(pt.x, pt.y);
                //}

                wchar_t buffer[3];
                buffer[0] = (wchar_t)vkCode;
                buffer[1] = '\n';
                buffer[2] = '\0';
                glBuffer.push_back(MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR));
                OutputDebugString(L"___KEY WAS pressed: ");
                OutputDebugString(buffer);
                
                //std::wstring output = std::wstring(1, _pair[0]) + L" " + std::wstring(1, _pair[1]) + L"\n";
                //OutputDebugString(output.c_str());
                std::wstring glBufferOutput;
                OutputDebugString(L"___Buffer is: ");
                for (auto i : glBuffer) {
                    glBufferOutput += std::wstring(1, i) + L" " + std::to_wstring((int)i) + L" |";
                }
                glBufferOutput += L"\n";
                OutputDebugString(glBufferOutput.c_str());

                if (glBuffer.size() == 2) {
                    std::pair<int,int> posCoord = getCellPosition();
                    if (posCoord.first == -1)
                        OutputDebugString(L"___Error in glBuffer\n");
                    else
                        gotoPoint(posCoord);
                    // if (IsWindowOpen(FindWindow(L"GridWindowClass", NULL)))
                }

                if (keyStates[0x51] && keyStates[0x50]) { // 0x51 : 'Q' | 0x50 : 'P'
                    OutputDebugString(L"___Key Q & P was pressed simultaneously!\n");
                    //SetCursorPos(100, 100);
                    POINT pt;
                    GetCursorPos(&pt);
                    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

                    MONITORINFO mi = { sizeof(mi) };
                    if (GetMonitorInfo(hMonitor, &mi)) {
                        SetCursorPos(mi.rcMonitor.left + 100, mi.rcMonitor.top + 100);
                    }
                    OutputDebugString(L"___Cursor GOTO pos 100x100!\n");

                    HWND mainHwnd = FindWindow(L"GridWindowClass", NULL);
                    
                    ShowWindow(mainHwnd, SW_HIDE);
                    //if (hKeyboardHook) {
                    //    UnhookWindowsHookEx(hKeyboardHook);
                    //    hKeyboardHook = NULL;
                    //}
                    //return CallNextHookEx(hKeyboardHook, nCode, wParam, 
                }
                else if (keyStates[0x50] && keyStates[0x41]) { // 0x41 : 'A' | goto unactive screen
                    OutputDebugString(L"___Key P & A was pressed simultaneously!\n");
                    //SetCursorPos(100, 100);
                    POINT pt;
                    GetCursorPos(&pt);
                    HMONITOR hActiveMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

                    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&hActiveMonitor);
                    OutputDebugString(L"___Cursor GOTO pos 100x100 on other unactive screen!\n");


                    HWND mainHwnd = FindWindow(L"GridWindowClass", NULL);
                    ShowWindow(mainHwnd, SW_HIDE);
                    //if (hKeyboardHook) {
                    //    UnhookWindowsHookEx(hKeyboardHook);
                    //    hKeyboardHook = NULL;
                    //}
                }
            }

        }
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            //keyStates[vkCode] = false;    
            // Добавить глобальный буфер, куда записываются нажатия по порядку
            // когда длина буфера = 2 -> очищаем его. Это чтобы можно было обрабатывать 
            // Нажатия одного символа дважды и чтобы нажатие двух разных символов зависили
            // от порядка нажатия 
            std::fill(keyStates, keyStates + 256, false);
            glBuffer.clear();
            //alt = HIWORD(GetKeyState(VK_MENU)) != 0;
            f2 = HIWORD(GetAsyncKeyState(VK_F2)) != 0;
            if (f2 && isLeftClicking) {
                Unhook();
                LeftClickUp();
                isLeftClicking = false;
                
                OutputDebugString(L"_Hook__ left click realeased\n");
            }
        }
        
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

void MoveCursor(int dx, int dy) {
    POINT pt;
    GetCursorPos(&pt);
    SetCursorPos(pt.x + dx, pt.y + dy);
}

void ScrollMouse(int direction) {
    //mouse_event(MOUSEEVENTF_WHEEL, 0, 0, direction, 0);
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = direction;
    
    SendInput(1, &input, sizeof(INPUT));
}


LRESULT CALLBACK HotkeyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_HOTKEY:
        if (wParam == HOTKEY_ID_SHOW_GRID) {
            /// Определить активный монитор по положению курсора
            // соответственно можно прожимать комбу, чтобы перекинуть курсор на нужный моник
            // и потом брать сетку уже по необходимому
            POINT pt;
            GetCursorPos(&pt);
            HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

            MONITORINFO mi = { sizeof(mi) };
            if (GetMonitorInfo(hMonitor, &mi)) {
                // Показать основное окно с сеткой на активном мониторе
                

                HWND mainHwnd = FindWindow(L"GridWindowClass", NULL);
                if (mainHwnd) {
                    OutputDebugString(L"Hotkey pressed, showing grid window.\n");
                    DWORD dwCurrentThread = GetCurrentThreadId();
                    DWORD dwFGThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

                    //HWND activeWindow = GetForegroundWindow();
                    //if (activeWindow == NULL)
                    //    OutputDebugString(L"___Erorr active window cant be gotter\n");
                    //if (HideCaret(activeWindow) == 0)
                    //    OutputDebugString(L"___Erorr caret cant be hidden\n");
                    //DestroyCaret();

                    AttachThreadInput(dwCurrentThread, dwFGThread, TRUE);

                    SetWindowPos(mainHwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_SHOWWINDOW);

                    SetForegroundWindow(mainHwnd);
                    SetCapture(mainHwnd);
                    SetFocus(mainHwnd);
                    SetActiveWindow(mainHwnd);
                    EnableWindow(mainHwnd, TRUE);

                    UpdateWindow(mainHwnd);
                    

                    AttachThreadInput(dwCurrentThread, dwFGThread, FALSE);
                    // Смена на английский (United States-English)
                    SetKeyboardLayout(L"00000409");
                    // Установить хук на клавиатуру
                    //hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
                    SetHook();
                    //if (HideCaret(GetForegroundWindow()) == 0)
                    //    OutputDebugString(L"___Erorr caret cant be hidden\n");
                    //DestroyCaret();
                }
                else {
                    OutputDebugString(L"Main window not found.\n");
                }
            }
        }
        else if (wParam == HOTKEY_ID_EXIT) {
            //DestroyWindow(hwnd);
            OutputDebugString(L"___HOTKEY EXIT pressed");
            OutputDebugString(L"___Programm destroied.\n");
            Unhook();
            PostQuitMessage(0);
            return 0L;
        }


        if (wParam == HOTKEY_ID_LEFTCLICK) {
            if (!isLeftClicking) {
                SetHook();
                LeftClickDown();
                isLeftClicking = true;
                OutputDebugString(L"___F2 pressed: Left Click Down!\n");
            }
        }                                    
        //else if (isLeftClicking) {               
        //    LeftClickUp();
        //    isLeftClicking = false;
        //    OutputDebugString(L"___Alt+F1 realeased: Left Click Up!\n");
        //}
        if (wParam == HOTKEY_ID_RIGHTCLICK) {
            RightClick();
            OutputDebugString(L"___Alt+F2 pressed: Right Click!\n");
        }
        
        switch (wParam) {
            case HOTKEY_ID_UP:
                MoveCursor(0, -10); // Двигаем курсор вверх
                break;
            case HOTKEY_ID_DOWN:
                MoveCursor(0, 10); // Двигаем курсор вниз
                break;
            case HOTKEY_ID_LEFT:
                MoveCursor(-10, 0); // Двигаем курсор влево
                break;
            case HOTKEY_ID_RIGHT:
                MoveCursor(10, 0); // Двигаем курсор вправо
                break;
            case HOTKEY_ID_SCROLL_UP:
                ScrollMouse(120); // Прокрутка вверх
                break;
            case HOTKEY_ID_SCROLL_DOWN:
                ScrollMouse(-120); // Прокрутка вниз
                break;
        }
        

        break;
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Установка прозрачного фона
        SetBkMode(hdc, TRANSPARENT);
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Установка черного фона для текста
        HBRUSH backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
        SetTextColor(hdc, RGB(255, 255, 255)); // Белый цвет текста
        SetBkMode(hdc, OPAQUE); // Установка непрозрачного фона для текста
        SetBkColor(hdc, RGB(0, 0, 0)); // Черный цвет фона для текста


        const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
        const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
        //std::wstring outputDebugMessage = L"Screen Height = " + std::to_wstring(SCREEN_HEIGHT) + L" Screen Width = " + std::to_wstring(SCREEN_WIDTH);
        //MessageBox(NULL, outputDebugMessage.c_str(), L"Внимание", MB_OK | MB_ICONERROR);

        
        //int cellWidth = (SCREEN_WIDTH / numCells) - 5;
        //int cellHeight = (SCREEN_HEIGHT / numCells) - 10;
        double scaler = getMonitorScaler();
       
        //std::wstring outputDebugString = L"scaler is " + std::to_wstring(scaler);
        //MessageBox(NULL, outputDebugString.c_str(), L"Внимание", MB_OK | MB_ICONERROR);
        int cellSize = sqrt(SCREEN_HEIGHT * SCREEN_WIDTH * scaler / pairs.size());
        int numCellsInRow = (SCREEN_WIDTH / cellSize);

        
        int i, j, charPos;
        for (int y = 0; y < rect.bottom; y += cellSize) {
            for (int x = 0; x < rect.right; x += cellSize) {
                RECT cellRect = { x, y, x + cellSize, y + cellSize };
                RECT textRect = cellRect;

                i = y / cellSize;
                j = x / cellSize;
                charPos = i * numCellsInRow + (i==0?j+i:j+i);
                if (charPos >= pairs.size()) {
                    OutputDebugString(L"___PairsSize error out of range\n");
                }
                else {
                    std::wstring output = std::wstring(1, pairs[charPos][0]) + L" " + std::wstring(1, pairs[charPos][1]) + L"\n";
                    // Рисование текста
                    DrawText(hdc, output.c_str(), -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
        }
        
        DeleteObject(backgroundBrush);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_SETFOCUS:
        SetCapture(hwnd);
        break;
    case WM_KILLFOCUS:
        // Скрыть окно при потере фокуса and remove hook
        //Unhook();
        //ShowWindow(hwnd, SW_HIDE);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
        ReleaseCapture();
        
       
        //PostQuitMessage(0);
        
        //if (hKeyboardHook) {
        //    UnhookWindowsHookEx(hKeyboardHook);
        //    hKeyboardHook = NULL;
        //}
        return 0;
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


// make keys pairs for mouse jump
void makePairs() {
    //Sleep(10);
    const char* leftHand = "qwertasdfgzxcvb"; // 15 chars
    const char* leftHandNumbers = "QWERTASDFGZXCVB123456"; // 21 chars
    const char* rightHand = "yuiop[]hjkl;'nm,./";  // 17 chars
    const char* rightHandNumbers = "YUIOP[]HJKL;'NM,./7890-"; // 22 chars

    // filling vector<vector<char>> pairs
    std::vector<char> pair;
    for (int i = 0; leftHandNumbers[i] != '\0'; i++) {
        for (int j = 0; j < rightHandNumbers[j] != '\0'; j++) {
            pair = { leftHandNumbers[i], rightHandNumbers[j] };
            pairs.push_back(pair);
            pair.clear();
        }
    }
    for (int i = 0; rightHandNumbers[i] != '\0'; i++) {
        for (int j = 0; j < leftHandNumbers[j] != '\0'; j++) {
            pair = { rightHandNumbers[i], leftHandNumbers[j] };
            pairs.push_back(pair);
            pair.clear();
        }
    }
    //pairs.push_back({'=', 'Q'});
    //pairs.push_back({ '=', 'W' });
    
    // print all gotten pairs
    for (const auto& _pair : pairs) {
        std::wstring output = std::wstring(1, _pair[0]) + L" " + std::wstring(1, _pair[1]) + L"\n";
        OutputDebugString(output.c_str());
    }

}

bool hotkeysRegistration(HWND hotkeyHwnd) {
    // Регистрация горячей клавиши (ctrl + ~)
    if (!RegisterHotKey(hotkeyHwnd, HOTKEY_ID_SHOW_GRID, MOD_CONTROL, VK_OEM_3)) { // VK_OEM_3 - код клавиши '~'
        MessageBox(NULL, L"Не удалось зарегистрировать горячую клавишу для показа сетки", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }
    else {
        OutputDebugString(L"___Hotkey for showing grid registered successfully.\n");
    }

    // Регистрация горячей клавиши (Ctrl + Alt + Q) для завершения программы
    if (!RegisterHotKey(hotkeyHwnd, HOTKEY_ID_EXIT, MOD_ALT | MOD_CONTROL, 0x51)) { // 0x51 - код клавиши 'Q'
        MessageBox(NULL, L"Не удалось зарегистрировать горячую клавишу для выхода", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }
    else {
        OutputDebugString(L"___Hotkey for exiting registered successfully.\n");
    }

    // Регистрация горячей клавиши(F2) для имитации левого клика мыши
    if (!RegisterHotKey(hotkeyHwnd, HOTKEY_ID_LEFTCLICK, 0, VK_F2)) { // replace with 0, VK_F2. Change alt processing 
        MessageBox(NULL, L"Не удалось зарегистрировать горячую клавишу Левой кнопки мыши", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }
    else {
        OutputDebugString(L"___Hotkey for left click registered successfully.\n");
    }
    // Регистрация горячей клавиши(Alt + F2) для имитации правого клика мыши
    if (!RegisterHotKey(hotkeyHwnd, HOTKEY_ID_RIGHTCLICK, MOD_ALT, VK_F2)) {
        MessageBox(NULL, L"Не удалось зарегистрировать горячую клавишу Правой кнопки мыши", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }
    else {
        OutputDebugString(L"___Hotkey for right click registered successfully.\n");
    }

    RegisterHotKey(hotkeyHwnd, HOTKEY_ID_UP, MOD_CONTROL | MOD_ALT, VK_UP);
    RegisterHotKey(hotkeyHwnd, HOTKEY_ID_DOWN, MOD_CONTROL | MOD_ALT, VK_DOWN);
    RegisterHotKey(hotkeyHwnd, HOTKEY_ID_LEFT, MOD_CONTROL | MOD_ALT, VK_LEFT);
    RegisterHotKey(hotkeyHwnd, HOTKEY_ID_RIGHT, MOD_CONTROL | MOD_ALT, VK_RIGHT);

    RegisterHotKey(hotkeyHwnd, HOTKEY_ID_SCROLL_UP, MOD_ALT, VK_HOME);
    RegisterHotKey(hotkeyHwnd, HOTKEY_ID_SCROLL_DOWN, MOD_ALT, VK_END);
}

void SetHook() {
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hKeyboardHook == NULL) {
        OutputDebugString(L"_HookError__ failed to set hook\n");
    }
    else {
        OutputDebugString(L"_Hook__ keyboard hook was setted successfully\n");
    }
}

void Unhook() {
    if (hKeyboardHook)
    {
        UnhookWindowsHookEx(hKeyboardHook);
        OutputDebugString(L"_Hook__ unHook successfully\n");
    }
    glBuffer.clear();
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Создать пары символов для сетки
    makePairs();
    // Создание окна для обработки горячих клавиш
    WNDCLASS hotkeyWc = { 0 };
    hotkeyWc.lpfnWndProc = HotkeyWindowProc;
    hotkeyWc.hInstance = hInstance;
    hotkeyWc.lpszClassName = L"HotkeyWindowClass";
    RegisterClass(&hotkeyWc);

    HWND hotkeyHwnd = CreateWindowEx(0, L"HotkeyWindowClass", L"Hotkey Window", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    hotkeysRegistration(hotkeyHwnd);

    // Создание основного окна
    WNDCLASS mainWc = { 0 };
    mainWc.lpfnWndProc = MainWindowProc;
    mainWc.hInstance = hInstance;
    mainWc.lpszClassName = L"GridWindowClass";
    RegisterClass(&mainWc);

    // prev first arg is WS_EX_LAYERED | WS_EX_TRANSPARENT
    HWND mainHwnd = CreateWindowEx(0, L"GridWindowClass", L"Grid Window",
        WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);

    // Установка прозрачности окна
    SetLayeredWindowAttributes(mainHwnd, RGB(0, 0, 0), 128, LWA_ALPHA);

    bool isAltPressed = false;
    bool isF1Pressed = false;
    //SetHook();
    // Основной цикл обработки сообщений
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
    
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Sleep(10);
    }

    Unhook();
    // Отмена регистрации горячих клавиш
    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_SHOW_GRID);
    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_EXIT);
    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_LEFTCLICK);
    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_RIGHTCLICK);

    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_UP);
    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_DOWN);
    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_LEFT);
    UnregisterHotKey(hotkeyHwnd, HOTKEY_ID_RIGHT);

    UnregisterHotKey(NULL, HOTKEY_ID_SCROLL_UP);
    UnregisterHotKey(NULL, HOTKEY_ID_SCROLL_DOWN);

    // Удаление хука перед выходом
    UnhookWindowsHookEx(hKeyboardHook);

    return 0;
}

