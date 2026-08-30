#include "file_io.h"
#include <commdlg.h>
#include <stdlib.h>

#define MAX_REASONABLE_SIZE 1500000000 // 1.5 GB

static bool ShowOpenFileDialog(HWND hwnd,wchar_t *out_path,size_t max_len,NotepadState* state){
    OPENFILENAMEW  ofn = { 0 };
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = out_path;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = max_len;
            ofn.lpstrFilter = L"Text Files\0*.txt\0\0"; //filter for only .txt
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileNameW(&ofn)==TRUE){
        wcscpy(state->current_file_path, out_path);
        return TRUE;
    }
    return FALSE;
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
    if (byte_count == 0) {
        Buffer_Clear(state);
        return true;
    }

    if (Buffer_Resize(state, byte_count + 1) == -1) { //check if the resize worked
        return false;
    }

    int chars_written = MultiByteToWideChar(CP_UTF8, 0, utf8_data, -1, state->text, (int)state->capacity); //turn into utf
    if (chars_written <= 0) { //return false incase of a fail
        return false;
    }

    state->length = (size_t)(chars_written - 1);
    state->text[state->length] = L'\0';
    return true;
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
        InvalidateRect(hwnd, NULL, FALSE);
    } else {
        MessageBoxW(hwnd, L"Failed to parse file text.", L"Error", MB_ICONERROR | MB_OK);
    }

    return success;
}


bool SavePathedFile(NotepadState *state){
    if(state->current_file_path[0] == L'\0')
        return FALSE;

    HANDLE fileHandle = CreateFileW(
        state->current_file_path,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );//create a file handle with a write request
    //check if we got the handle
    if (fileHandle == INVALID_HANDLE_VALUE)
        return FALSE;

    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, state->text, (int)state->length, NULL, 0, NULL, NULL); // get the size of the text
    if (utf8_size <= 0) {
        CloseHandle(fileHandle);
        return FALSE;
    }

    // use the size to allocate mem to a temporary buffer
    char *utf8_data = (char*)malloc(utf8_size);
    if (!utf8_data) {
        CloseHandle(fileHandle);
        return FALSE;
    }

    WideCharToMultiByte(CP_UTF8, 0, state->text, (int)state->length, utf8_data, utf8_size, NULL, NULL);

    // write to disk
    DWORD bytesWritten = 0;
    BOOL success = WriteFile(fileHandle, utf8_data, (DWORD)utf8_size, &bytesWritten, NULL);

    free(utf8_data);
    CloseHandle(fileHandle);

    return (success && bytesWritten == (DWORD)utf8_size);
}
