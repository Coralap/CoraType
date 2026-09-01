
#pragma once
#define BASE_BUFFER_LEN 16
#include <stdlib.h>
#include <stdbool.h>
#include "piece_table.h"
typedef struct {
    PieceTable piece_table;
    size_t cursor_pos;       // character offset

    // buffer for rendering without reallocating every frame
    wchar_t *render_buffer;
    size_t render_capacity;

    int line_height;

    wchar_t current_file_path[_MAX_PATH];
    bool is_dirty;
    bool is_rtl;
} NotepadState;

bool State_Init(NotepadState *state, const wchar_t *initial_text, size_t len);
bool State_InsertChar(NotepadState *state, wchar_t ch);
bool State_Backspace(NotepadState *state);
void State_EnsureRenderCapacity(NotepadState *state, size_t needed_len);
void State_Free(NotepadState *state);
void State_Rebase(NotepadState *state, size_t total_len);