#include "text_buffer.h"


bool Text_Init(TextBuffer* buffer, size_t initial_capacity) {
if (initial_capacity == 0) {
        initial_capacity = 64; // default min.
    }

    buffer->text = (wchar_t*)malloc(sizeof(wchar_t)*initial_capacity); //alocate mem
    if(!buffer->text) // check for failure of allocation
        return false;
        
    buffer->length = 0;
    buffer->text[0] = L'\0';
    buffer->capacity = initial_capacity;
    return true;
}

void Text_InsertChar(TextBuffer* buffer, wchar_t ch) {
    if (buffer->length + 1 >= buffer->capacity) { // check for overflow
        size_t new_capacity = buffer->capacity*2;
        wchar_t* new_text = (wchar_t*)realloc(buffer->text,sizeof(wchar_t)*new_capacity);
        if(!new_text){return;}

        buffer->text = new_text;
        buffer->capacity = new_capacity;

    }
    buffer->text[buffer->length++] = (wchar_t)ch;
    buffer->text[buffer->length] = '\0'; //null terminator
}

void Text_Backspace(TextBuffer* buffer) {
    if (buffer->length > 0) {
        buffer->length--;
        buffer->text[buffer->length] = L'\0';
    }
}

void Text_Clear(TextBuffer* buffer) {
    buffer->length = 0;
    if (buffer->text) {
        buffer->text[0] = L'\0';
    }
    
    
}

bool Text_Resize(TextBuffer* buffer,size_t new_size) {
        wchar_t* new_text = (wchar_t*)realloc(buffer->text,sizeof(wchar_t)*new_size);
        if(!new_text){return false;}

        buffer->text = new_text;
        buffer->capacity = new_size;
        return true;
}



void Text_Free(TextBuffer* buffer) {
    free(buffer->text);
    buffer->text = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}