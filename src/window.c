#include <windows.h>
#include <stdio.h>
#include "buffer.h"
#include <commdlg.h>
#include <stringapiset.h>
#include <winuser.h>

#define IDM_FILE_NEW 1
#define IDM_FILE_OPEN 2
#define IDM_FILE_QUIT 3
#define IDM_FILE_SAVE 4
#define IDM_FILE_SAVE_AS 5

#define MAX_REASONABLE_SIZE 1500000000

static void UpdateCaretPos(HDC hdc, const NotepadState *state,RECT rect);
static void InitCaret(HWND hwnd);

static void OnPaint(HWND hwnd, NotepadState *state){
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
    
    size_t total_len = PieceTable_GetTotalLength(&state->piece_table); //get len of string
    State_EnsureRenderCapacity(state, total_len); //check the text buffer is long enough
    PieceTable_GetText(&state->piece_table, state->render_buffer, state->render_capacity); //write the text.
    DrawTextW(hdcMem, state->render_buffer, (int)total_len, &rect, DT_LEFT | DT_EDITCONTROL); //text

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
    UpdateCaretPos(hdc,state,rect);

    EndPaint(hwnd, &ps);

}

static void OnChar(HWND hwnd,const WPARAM wParam, NotepadState *state){
    if (wParam >= 32 && wParam != 127) { //Handle normal keyboard inputs
        State_InsertChar(state,(wchar_t)wParam);
    } 
    else if(wParam == VK_RETURN){ // Handle enter
        State_InsertChar(state,L'\n');
    }

    InvalidateRect(hwnd, NULL, false);

}

static void HandleCommands(HWND hwnd,const WPARAM wParam, NotepadState *state){
    WORD id = LOWORD(wParam);
    switch (id) {
        case IDM_FILE_NEW:
            State_Free(state);
            State_Init(state, L"", 0);
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case IDM_FILE_OPEN:
            //File_Open(hwnd,state); needs adapting
            break;

        case IDM_FILE_SAVE:
            //File_Save(hwnd,state); needs adapting
            break;

        case IDM_FILE_SAVE_AS:
            //File_Save_As(hwnd,state);
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
    AppendMenuW(hMenu, MF_STRING, IDM_FILE_SAVE, L"&Save");
    AppendMenuW(hMenu, MF_STRING, IDM_FILE_SAVE_AS, L"&Save As");

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

        case WM_SETFOCUS:
            InitCaret(hwnd);
            return 0;

        
        case WM_KILLFOCUS:
            DestroyCaret();
            return 0;

        case WM_DESTROY:
            State_Free(state);

            PostQuitMessage(0);
            return 0;
        


        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

    }
}

static void InitCaret(HWND hwnd){
    HDC hdc = GetDC(hwnd);
            TEXTMETRICW tm;
            GetTextMetricsW(hdc,&tm);
            int lineHeight = tm.tmExternalLeading + tm.tmHeight; // calculate the line height using font and other stuff.
            ReleaseDC(hwnd, hdc);
            
            CreateCaret(
                hwnd,
                (HBITMAP) 0,
                2,
                lineHeight
            );
    ShowCaret(hwnd);



}

static void UpdateCaretPos(HDC hdc, const NotepadState *state,RECT rect){

    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    int lineHeight = tm.tmHeight + tm.tmExternalLeading;
    int lineCount = 0;
    size_t lineStart = 0;

    for (size_t i = 0; i < state->cursor_pos; i++) {//count lines and find the last one's index
        if (state->render_buffer[i] == L'\n') {
            lineCount++;
            lineStart = i + 1; // Start of the next line
        }
    }

    size_t currentLineLength = state->cursor_pos - lineStart;
    SIZE lineSize = {0};

    if (currentLineLength > 0) {
        GetTextExtentPoint32W(hdc, &state->render_buffer[lineStart], (int)currentLineLength, &lineSize); //find info about the text from the last line
    }

    //calculate and set the caret pose
    int caretX = state->is_rtl ? rect.right - lineSize.cx : lineSize.cx;
    int caretY = lineCount * lineHeight;
    SetCaretPos(caretX, caretY);


}