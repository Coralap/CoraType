#include "buffer.h"
#include <stdlib.h>
#include <string.h>

bool State_Init(NotepadState *state, const wchar_t *initial_text, size_t len) {
    if (!state) return false;

    // init table
    if (!PieceTable_Init(&state->piece_table, initial_text ? initial_text : L"", len)) {
        return false;
    }

    state->cursor_pos = len;// position cursor

    // add render cache
    state->render_capacity = (len + 1 > BASE_BUFFER_LEN) ? (len + 1) * 2 : BASE_BUFFER_LEN;
    state->render_buffer = (wchar_t*)malloc(state->render_capacity * sizeof(wchar_t));
    if (!state->render_buffer) {
        PieceTable_Free(&state->piece_table);
        return false;
    }
    state->render_buffer[0] = L'\0';

    // set defaults
    state->current_file_path[0] = L'\0';
    state->is_dirty = false;
    state->is_rtl = false;
    state->line_height = 0;

    return true;
}

bool State_InsertChar(NotepadState *state, wchar_t ch) {
    if (!state) return false;

    if (PieceTable_InsertChar(&state->piece_table, ch, state->cursor_pos)) {
        state->cursor_pos++;
        state->is_dirty = true;
        return true;
    }
    return false;
}

bool State_Backspace(NotepadState *state) {
    if (!state || state->cursor_pos == 0) return false;

    if (PieceTable_DeleteChar(&state->piece_table, state->cursor_pos - 1)) {
        state->cursor_pos--;
        state->is_dirty = true;
        return true;
    }
    return false;
}

void State_EnsureRenderCapacity(NotepadState *state, size_t needed_len) {
    if (needed_len + 1 > state->render_capacity) {
        size_t new_cap = (needed_len + 1) * 2;
        wchar_t *new_buf = (wchar_t*)realloc(state->render_buffer, new_cap * sizeof(wchar_t));
        if (new_buf) {
            state->render_buffer = new_buf;
            state->render_capacity = new_cap;
        }
    }
}

void State_Free(NotepadState *state) {
    if (!state) return;

    PieceTable_Free(&state->piece_table);
    
    free(state->render_buffer);
    state->render_buffer = NULL;
    state->render_capacity = 0;
    state->cursor_pos = 0;
}