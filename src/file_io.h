#pragma once
#include <windows.h>
#include <stdbool.h>
#include "buffer.h"

//bool File_Open(HWND hwnd, NotepadState *state);
bool File_Open(HWND hwnd, NotepadState *state);
bool SavePathedFile(NotepadState *state);