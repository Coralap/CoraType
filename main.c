
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

#define MAX_BUFFER_LEN 256

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


typedef struct {
    wchar_t text[256];
    int index;
} NotepadState;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
    (void)hPrevInstance;
    (void)pCmdLine;
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        _setmode(_fileno(stdout), _O_U16TEXT);
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
        L"CoraType",    // Window text
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
        if(!state)
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd,&ps);
            RECT rect;
            GetClientRect(hwnd, &rect);

            FillRect(hdc, &rect, (HBRUSH) (COLOR_WINDOW+1));
        
            DrawTextW(hdc, state->text, state->index, &rect, DT_LEFT);
            EndPaint(hwnd, &ps);
            
        }
        return 0;

        case WM_CHAR: {

            if (wParam >= 32 && wParam != 127) {
                if (state->index < MAX_BUFFER_LEN - 1) {
                    state->text[state->index++] = (wchar_t)wParam;
                    state->text[state->index] = '\0';
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == VK_BACK && state->index > 0) { // Handle backspace
                state->index--;
                state->text[state->index] = '\0';
                InvalidateRect(hwnd, NULL, FALSE);
            } else if(wParam == VK_RETURN && state->index < MAX_BUFFER_LEN - 1){
                    state->text[state->index++] = '\n';
                    state->text[state->index] = '\0';
                    InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }
}