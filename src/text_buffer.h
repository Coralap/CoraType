#pragma once
#include <stdlib.h>
#include <stdbool.h>

typedef struct{
    wchar_t *text;
    size_t length;
    size_t capacity;
} TextBuffer;

bool Text_Init(TextBuffer* buffer, size_t initial_capacity);
void Text_InsertChar(TextBuffer* buffer, wchar_t ch);
void Text_Backspace(TextBuffer* buffer);
void Text_Clear(TextBuffer* buffer);
bool Text_Resize(TextBuffer* buffer,size_t new_size);
void Text_Free(TextBuffer* buffer);
