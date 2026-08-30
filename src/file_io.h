#pragma once
#include <windows.h>
#include <stdbool.h>
#include "buffer.h"

//bool File_Open(HWND hwnd, NotepadState *state);
bool File_Open(HWND hwnd, NotepadState *state);
bool File_Save(HWND hwnd,NotepadState *state);
bool File_Save_As(HWND hwnd, NotepadState *state);
