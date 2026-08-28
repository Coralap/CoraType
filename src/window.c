#include <windows.h>
#include <stdio.h>
#include "buffer.h"

void OnPaint(HWND hwnd, const NotepadState *state){
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd,&ps);
    RECT rect;
    GetClientRect(hwnd, &rect);

    FillRect(hdc, &rect, (HBRUSH) (COLOR_WINDOW+1));

    DrawTextW(hdc, state->text, state->index, &rect, DT_LEFT);
    EndPaint(hwnd, &ps);
}

void OnChar(HWND hwnd,const WPARAM wParam, NotepadState *state){
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
            OnPaint(hwnd,state);
            return 0;

        case WM_CHAR: {
            OnChar(hwnd,wParam,state);
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }
}