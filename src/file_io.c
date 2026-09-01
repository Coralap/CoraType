#include "file_io.h"
#include <commdlg.h>
#include <stdlib.h>

#define MAX_REASONABLE_SIZE 1500000000 // 1.5 GB

static bool ShowOpenFileDialog(HWND hwnd,wchar_t *out_path,size_t max_len,NotepadState* state){
    OPENFILENAMEW  ofn = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = out_path;
            ofn.lpstrFile[0] = L'\0';
            ofn.nMaxFile = (DWORD)max_len;
            ofn.lpstrFilter = L"Text Files\0*.txt\0\0"; //filter for only .txt
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileNameW(&ofn)==true){
        wcscpy(state->current_file_path, out_path);
        return true;
    }
    return false;
}

bool ShowSaveFileDialog(HWND hwnd,wchar_t *out_path,size_t max_len){
    OPENFILENAMEW  ofn = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = out_path;
            ofn.lpstrFile[0] = L'\0';
            ofn.nMaxFile = (DWORD)max_len;
            ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0"; // filter for txt and other files
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrDefExt = L"txt";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    return GetSaveFileNameW(&ofn)==true;

}





static char* ReadRawFileBytes(const wchar_t *file_path,DWORD* out_size){
        HANDLE fileHandle = CreateFileW(
            file_path,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );//creating a file handle with our new path
        //check if we got the handle
        if (fileHandle == INVALID_HANDLE_VALUE)
            return NULL;

        LARGE_INTEGER fileSize;
        //check for size
        if(GetFileSizeEx(fileHandle,&fileSize)==0||fileSize.QuadPart > MAX_REASONABLE_SIZE){
            CloseHandle(fileHandle);
            return NULL;
        }
        //check for empty file
        if (fileSize.QuadPart == 0) {
            CloseHandle(fileHandle);
            *out_size = 0;
            char *empty = (char*)malloc(1);
            if (empty) empty[0] = '\0';
            return empty;
        }
        
        //alocate space for the new text with and extra byte for a null terminator
        char *bytes = (char*)malloc(fileSize.QuadPart + 1);
            if (!bytes) {
                CloseHandle(fileHandle);
                return NULL;
        }
        
        DWORD bytesRead = 0;
        if (!ReadFile(fileHandle, bytes, (DWORD)fileSize.QuadPart, &bytesRead, NULL)) {
            free(bytes);
            CloseHandle(fileHandle);
            return NULL;
        }

        bytes[bytesRead] = '\0';
        *out_size = bytesRead;

        CloseHandle(fileHandle);
        return bytes;
}


static bool LoadUtf8IntoState(const char *utf8_data, DWORD byte_count, NotepadState *state) {
    if(!state) return false;
    State_Free(state);

    if (byte_count == 0) {
        return State_Init(state, L"" ,0);
    }

    State_EnsureRenderCapacity(state, byte_count + 1);

    // Find required wide character count (without null terminator)
    int wide_count = MultiByteToWideChar(CP_UTF8, 0, utf8_data, (int)byte_count, NULL, 0);
    if (wide_count <= 0) {
        State_Init(state, L"", 0);
        return false;
    }

    // Allocate temporary buffer to hold converted text
    wchar_t *wide_text = (wchar_t*)malloc((wide_count + 1) * sizeof(wchar_t));
    if (!wide_text) {
        State_Init(state, L"", 0);
        return false;
    }

    // convert text
    MultiByteToWideChar(CP_UTF8, 0, utf8_data, (int)byte_count, wide_text, wide_count);
    wide_text[wide_count] = L'\0';

    // init piece table and update the original buffer
    bool ok = State_Init(state, wide_text, (size_t)wide_count);
    free(wide_text);

    return ok;
}

bool File_Open(HWND hwnd, NotepadState *state) {
    wchar_t filePath[MAX_PATH];
    if (!ShowOpenFileDialog(hwnd, filePath, MAX_PATH,state)) {
        return false; // clicked cancel
    }

    DWORD byteCount = 0;
    char *rawBytes = ReadRawFileBytes(filePath, &byteCount);
    if (!rawBytes) {
        MessageBoxW(hwnd, L"Failed to read file.", L"Error", MB_ICONERROR | MB_OK);
        return false; //add error
    }

    bool success = LoadUtf8IntoState(rawBytes, byteCount, state);
    free(rawBytes); //free unused memory

    if (success) {
        InvalidateRect(hwnd, NULL, false);
    } else {
        MessageBoxW(hwnd, L"Failed to parse file text.", L"Error", MB_ICONERROR | MB_OK);
    }

    return success;
}



static bool SavePathedFile(NotepadState *state){
    if (!state || state->current_file_path[0] == L'\0') return false;

    HANDLE fileHandle = CreateFileW(
        state->current_file_path,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS, // Creates new file or overwrites an existing file
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );//create a file handle with a write request

    //check if we got the handle
    if (fileHandle == INVALID_HANDLE_VALUE)
        return false;

    //make sure the buffer is updated
    size_t total_len = PieceTable_GetTotalLength(&state->piece_table);
    State_EnsureRenderCapacity(state, total_len);
    PieceTable_GetText(&state->piece_table, state->render_buffer, state->render_capacity);

    if (total_len == 0) {
        CloseHandle(fileHandle);
        state->is_dirty = false;
        return true;
    }

    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, state->render_buffer,(int)total_len, NULL, 0, NULL, NULL); // get the size of the text
    if (utf8_size <= 0) {
        CloseHandle(fileHandle);
        return false;
    }

    // use the size to allocate mem to a temporary buffer
    char *utf8_data = (char*)malloc(utf8_size);
    if (!utf8_data) {
        CloseHandle(fileHandle);
        return false;
    }

    WideCharToMultiByte(CP_UTF8, 0, state->render_buffer,(int)total_len, utf8_data, utf8_size, NULL, NULL);

    // write to disk
    DWORD bytesWritten = 0;
    BOOL success = WriteFile(fileHandle, utf8_data, (DWORD)utf8_size, &bytesWritten, NULL);

    free(utf8_data);
    CloseHandle(fileHandle);

    if (success && bytesWritten == (DWORD)utf8_size) {
        State_Rebase(state, total_len);
        return true;
    }


    return (success && bytesWritten == (DWORD)utf8_size);
}
bool File_Save(HWND hwnd, NotepadState *state) {
    if (state->current_file_path[0] == L'\0') {
        if (!ShowSaveFileDialog(hwnd, state->current_file_path, MAX_PATH)) {
            return false; // User cancelled
        }
    }
    return SavePathedFile(state);
}

bool File_Save_As(HWND hwnd, NotepadState *state) {
    wchar_t newPath[MAX_PATH] = { 0 };
    if (!ShowSaveFileDialog(hwnd, newPath, MAX_PATH)) {
        return false; // user cancled.
    }

    wcscpy(state->current_file_path, newPath);
    return SavePathedFile(state);
}