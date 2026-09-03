#include "text_buffer.h"


bool Text_Init(TextBuffer* buffer, size_t initial_capacity) {
    if (!buffer) return false;

    if (initial_capacity == 0) {
        initial_capacity = 64;
    }

    // Check for integer overflow
    if (initial_capacity > (SIZE_MAX / sizeof(wchar_t))) {
        return false;
    }

    buffer->text = (wchar_t*)malloc(initial_capacity * sizeof(wchar_t));
    if (!buffer->text) {
        buffer->capacity = 0;
        buffer->length = 0;
        return false;
    }

    buffer->capacity = initial_capacity;
    buffer->length = 0;
    buffer->text[0] = L'\0';
    return true;
}

void Text_InsertChar(TextBuffer* buffer, wchar_t ch) {
    if (!buffer) return;

    // Check if there is enough space for character and null terminator
    if (buffer->length + 1 >= buffer->capacity) {
        if (buffer->capacity > (SIZE_MAX / sizeof(wchar_t)) / 2) {
            return;
        }

        size_t new_capacity = buffer->capacity ? buffer->capacity * 2 : 64;
        wchar_t* new_text = (wchar_t*)realloc(buffer->text, new_capacity * sizeof(wchar_t));
        if (!new_text) return;

        buffer->text = new_text;
        buffer->capacity = new_capacity;
    }

    buffer->text[buffer->length++] = ch;
    buffer->text[buffer->length] = L'\0';
}

void Text_Backspace(TextBuffer* buffer) {
    if (buffer && buffer->length > 0) {
        buffer->length--;
        buffer->text[buffer->length] = L'\0';
    }
}

void Text_Clear(TextBuffer* buffer) {
    if (!buffer) return;
    buffer->length = 0;
    if (buffer->text) {
        buffer->text[0] = L'\0';
    }
    
    
}

bool Text_Resize(TextBuffer* buffer, size_t new_size) {
    if (!buffer) return false;
    if (new_size > (SIZE_MAX / sizeof(wchar_t))) return false;

    wchar_t* new_text = (wchar_t*)realloc(buffer->text, new_size * sizeof(wchar_t));
    if (!new_text) return false;

    buffer->text = new_text;
    buffer->capacity = new_size;
    return true;
}

void Text_Free(TextBuffer* buffer) {
    if (!buffer) return;
    free(buffer->text);
    buffer->text = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}