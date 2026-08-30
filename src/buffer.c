#include "buffer.h"
#include <stdlib.h>

int Buffer_Init(NotepadState *state, size_t initial_capacity) {
    if(initial_capacity<16)
        return -1;
    state->text = (wchar_t*)malloc(sizeof(wchar_t)*initial_capacity); //alocate mem
    if(!state->text) // check for failure of allocation
        return -1;
    state->length = 0;
    state->text[0] = L'\0';
    state->capacity = initial_capacity;
    return 0;

}

void Buffer_InsertChar(NotepadState *state, wchar_t ch) {
    if (state->length + 1 >= state->capacity) { // check for overflow
        size_t new_capacity = state->capacity*2;
        wchar_t* new_text = (wchar_t*)realloc(state->text,sizeof(wchar_t)*new_capacity);
        if(!new_text){return;}

        state->text = new_text;
        state->capacity = new_capacity;

    }
    state->text[state->length++] = (wchar_t)ch;
    state->text[state->length] = '\0'; //null terminator
}

void Buffer_Backspace(NotepadState *state) {
    if (state->length > 0) {
        state->length--;
        state->text[state->length] = L'\0';
    }
}

void Buffer_Clear(NotepadState *state) {
    state->length = 0;
    if (state->text) {
        state->text[0] = L'\0';
    }
    
    state->current_file_path[0] = L'\0';
    
}


int Buffer_Resize(NotepadState *state,size_t new_size) {
        wchar_t* new_text = (wchar_t*)realloc(state->text,sizeof(wchar_t)*new_size);
        if(!new_text){return -1;}

        state->text = new_text;
        state->capacity = new_size;
        return 1;
}



void Buffer_Free(NotepadState *state) {
    free(state->text);
    state->text = NULL;
    state->length = 0;
    state->capacity = 0;
}