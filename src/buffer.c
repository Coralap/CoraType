#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_BUFFER_ELEMENTS (SIZE_MAX / sizeof(wchar_t))

bool State_Init(NotepadState *state, const wchar_t *initial_text, size_t len) {
    if (!state) return false;

    
    if (len > (MAX_BUFFER_ELEMENTS / 2) - 1) {
        return false;
    }

    // init table
    if (!PieceTable_Init(&state->piece_table, initial_text ? initial_text : L"", len)) {
        return false;
    }

    state->cursor_pos = len;// position cursor

    size_t candidate_cap = (len + 1) * 2;
    state->render_capacity = (candidate_cap > BASE_BUFFER_LEN) ? candidate_cap : BASE_BUFFER_LEN;
    //check for overflow
    if (state->render_capacity > MAX_BUFFER_ELEMENTS) {
        PieceTable_Free(&state->piece_table);
        return false;
    }

    state->render_buffer = (wchar_t*)malloc(state->render_capacity * sizeof(wchar_t));
    if (!state->render_buffer) {
        PieceTable_Free(&state->piece_table);
        state->render_capacity = 0;
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

bool State_EnsureRenderCapacity(NotepadState *state, size_t needed_len) {
    if (!state) return false;

    if (needed_len + 1 > state->render_capacity) {
        if (needed_len > (MAX_BUFFER_ELEMENTS / 2) - 1) {
            return false;
        }

        size_t new_cap = (needed_len + 1) * 2;
        if (new_cap < BASE_BUFFER_LEN) {
            new_cap = BASE_BUFFER_LEN;
        }

        wchar_t *new_buf = (wchar_t*)realloc(state->render_buffer, new_cap * sizeof(wchar_t));
        if (!new_buf) {
            return false;
        }

        state->render_buffer = new_buf;
        state->render_capacity = new_cap;
    }
    return true;
}

void State_Free(NotepadState *state) {
    if (!state) return;

    PieceTable_Free(&state->piece_table);
    
    free(state->render_buffer);
    state->render_buffer = NULL;
    state->render_capacity = 0;
    state->cursor_pos = 0;
}

void State_Rebase(NotepadState *state, size_t total_len) {
    if (!state) return;

    size_t saved_cursor = state->cursor_pos;
    wchar_t saved_path[MAX_PATH];
    wcsncpy(saved_path, state->current_file_path, MAX_PATH - 1);
    saved_path[MAX_PATH - 1] = L'\0';

    // free all nodes
    PieceTable_Free(&state->piece_table);

    // make a new buffer with all the text
    PieceTable_Init(&state->piece_table, state->render_buffer, total_len);

    // restore editor data
    state->cursor_pos = saved_cursor;
    wcsncpy(state->current_file_path, saved_path, MAX_PATH - 1);
    state->current_file_path[MAX_PATH - 1] = L'\0';
    state->is_dirty = false;
}