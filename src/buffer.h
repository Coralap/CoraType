
#pragma once
#define BASE_BUFFER_LEN 16
#include <stdlib.h>
#include <stdbool.h>
typedef struct {
    wchar_t *text;
    size_t length;
    size_t capacity;
    wchar_t current_file_path[_MAX_PATH];
    bool is_dirty;
    bool is_rtl;
} NotepadState;

int Buffer_Init(NotepadState *state, size_t initial_capacity);
void Buffer_InsertChar(NotepadState *state, wchar_t ch);
void Buffer_Backspace(NotepadState *state);
void Buffer_Clear(NotepadState *state);
void Buffer_Free(NotepadState *state);
int Buffer_Resize(NotepadState *state,size_t new_size);