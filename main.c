#include "stdio.h"
#include "windows.h"
#include <d2d1.h>
#include <dwrite.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


struct NotepadState {
    char text[256];
    int index;
};
typedef struct NotepadState NotepadState;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
    (void)hPrevInstance;
    (void)pCmdLine;
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Coral Window Class";

    WNDCLASS wc= {0};

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    NotepadState state = {0};
    state.index = 0;



    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Textalap",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        &state        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if(SUCCEEDED(hr)){
    }else{
        return 0;
    }
    
    
    
    
    
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    NotepadState* state;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT *pCreate = (CREATESTRUCT*)lParam;
        state = (NotepadState*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
        state = (NotepadState*)ptr;
    }
    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd,&ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            LPCSTR text = state->text;
            DrawTextA(hdc, text, state->index, &ps.rcPaint, DT_CENTER | DT_VCENTER);
            EndPaint(hwnd, &ps);
            
        }
        return 0;

        case WM_CHAR: {
            if (wParam >= 32 && wParam <= 126) {
                if (state->index < 255) {
                    state->text[state->index++] = (char)wParam;
                    state->text[state->index] = '\0';
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == VK_BACK && state->index > 0) { // Handle backspace
                state->index--;
                state->text[state->index] = '\0';
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        
        case WM_DESTROY:
            CoUninitialize();
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }
}