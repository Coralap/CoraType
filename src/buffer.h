
#pragma once
#define MAX_BUFFER_LEN 256

typedef struct {
    wchar_t text[MAX_BUFFER_LEN];
    int index;
} NotepadState;

