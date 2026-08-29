#include <windows.h>
#include <stdio.h>
#include "buffer.h"
#include <commdlg.h>

#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3

static void OnPaint(HWND hwnd, const NotepadState *state){
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd,&ps);
    HBITMAP hbmMem, hbmOld;
    HDC hdcMem;
    RECT rect;

    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (width <= 0 || height <= 0) {
        EndPaint(hwnd, &ps);
        return;
    }

    hdcMem = CreateCompatibleDC(hdc); //copy the hdc

    hbmMem = CreateCompatibleBitmap(hdc,
                                    rect.right-rect.left,
                                    rect.bottom-rect.top); //create a bitmap
    hbmOld = SelectObject(hdcMem, hbmMem); // use the bitmap

    FillRect(hdcMem, &rect, (HBRUSH) (COLOR_WINDOW+1)); //background

    DrawTextW(hdcMem, state->text, (int)state->length, &rect, DT_LEFT | DT_WORDBREAK | DT_EDITCONTROL); //text

    BitBlt(hdc,
           rect.left, rect.top,
           rect.right-rect.left, rect.bottom-rect.top,
           hdcMem,
           0, 0,
           SRCCOPY); //transfer to the real hdc
    
    //free memory and end paint
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    EndPaint(hwnd, &ps);
}

static void OnChar(HWND hwnd,const WPARAM wParam, NotepadState *state){
    if (wParam >= 32 && wParam != 127) { //Handle normal keyboard inputs
        Buffer_InsertChar(state,(wchar_t)wParam);
    } 
    
    else if (wParam == VK_BACK) { // Handle backspace
        Buffer_Backspace(state);
    } 
    
    else if(wParam == VK_RETURN){ // Handle enter
        Buffer_InsertChar(state,L'\n');
    }

    InvalidateRect(hwnd, NULL, FALSE);

}

static void OpenFileDialog(HWND hwnd){
    wchar_t file_path[256];

    OPENFILENAMEW  ofn = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = file_path;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = 256;
            ofn.lpstrFilter = L"Text Files\0*.txt\0\0"; //filter for only .txt
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
    if(GetOpenFileNameW(&ofn)==TRUE){
        wprintf(L"%ls\n", file_path);
        HANDLE fileHandle = CreateFileW(
            file_path,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );//creating a file handle with our new path

        if (fileHandle == INVALID_HANDLE_VALUE)
            return 1;
        
        wchar_t text_buffer[1024]; //holds text of the file
        if(ReadFile(
            fileHandle,
            text_buffer,
            sizeof(wchar_t)*1024,
            NULL,
            NULL
        )){
            wprintf(L"%ls\n", text_buffer);

        }
        CloseHandle(fileHandle);
        
    }
}


static void HandleCommands(HWND hwnd,const WPARAM wParam, NotepadState *state){
    WORD id = LOWORD(wParam);
    switch (id) {
        case IDM_FILE_NEW:
            Buffer_Clear(state);
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case IDM_FILE_OPEN:
            OpenFileDialog(hwnd);
            break;

        case IDM_FILE_QUIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
    }
    

}




static void AddMenus(HWND hwnd) {

    HMENU hMenubar;
    HMENU hMenu;

    hMenubar = CreateMenu();
    hMenu = CreatePopupMenu();

    AppendMenuW(hMenu, MF_STRING, IDM_FILE_NEW, L"&New");
    AppendMenuW(hMenu, MF_STRING, IDM_FILE_OPEN, L"&Open");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR) hMenu, L"&File");
    SetMenu(hwnd, hMenubar);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    NotepadState* state;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT *pCreate = (CREATESTRUCT*)lParam;
        state = (NotepadState*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        AddMenus(hwnd);
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

        case WM_ERASEBKGND:
            return (LRESULT)1;

        case WM_CHAR: {
            OnChar(hwnd,wParam,state);
            return 0;
        }
        
        case WM_COMMAND:
            HandleCommands(hwnd,wParam,state);
            return 0;
        case WM_DESTROY:
            Buffer_Free(state);
            PostQuitMessage(0);
            return 0;
        


        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }
}