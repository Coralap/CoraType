#include <windows.h>
#include <stdio.h>
#include "buffer.h"

void OnPaint(HWND hwnd, const NotepadState *state){
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd,&ps);
    RECT rect;
    GetClientRect(hwnd, &rect);

    FillRect(hdc, &rect, (HBRUSH) (COLOR_WINDOW+1));

    DrawTextW(hdc, state->text, state->length, &rect, DT_LEFT|DT_WORDBREAK|DT_EDITCONTROL );
    EndPaint(hwnd, &ps);
}

void OnChar(HWND hwnd,const WPARAM wParam, NotepadState *state){
    if (wParam >= 32 && wParam != 127) {
        Buffer_InsertChar(state,(wchar_t)wParam);
    } 
    
    else if (wParam == VK_BACK) { // Handle backspace
        Buffer_Backspace(state);
    } 
    
    else if(wParam == VK_RETURN){
        Buffer_InsertChar(state,L'\n');
    }

    InvalidateRect(hwnd, NULL, FALSE);

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
            Buffer_Free(state);
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }
}