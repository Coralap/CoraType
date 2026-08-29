
#pragma once
#define BASE_BUFFER_LEN 16
#include <stdlib.h>

typedef struct {
    wchar_t *text;
    size_t length;
    size_t capacity;
} NotepadState;

int Buffer_Init(NotepadState *state, size_t initial_capacity);
void Buffer_InsertChar(NotepadState *state, wchar_t ch);
void Buffer_Backspace(NotepadState *state);
void Buffer_Clear(NotepadState *state);
void Buffer_Free(NotepadState *state);
