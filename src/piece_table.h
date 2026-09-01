#pragma once
#include <windows.h>
#include "text_buffer.h"
#include <stdlib.h>
#include <stdbool.h>
typedef enum {
    BUFFER_ORIGINAL,
    BUFFER_ADDED
} NodeType;

typedef struct Node {
    NodeType type;
    size_t start;
    size_t length;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct {
    wchar_t *original;
    size_t original_length;

    TextBuffer added;

    Node *head;
    Node *tail;
} PieceTable;


bool PieceTable_Init(PieceTable *table, const wchar_t *text, size_t len);
bool PieceTable_InsertChar(PieceTable *table, const wchar_t text, size_t pos);
bool PieceTable_DeleteChar(PieceTable *table, size_t pos);
size_t PieceTable_GetTotalLength(const PieceTable *table);
bool PieceTable_GetText(const PieceTable* table, wchar_t *out, size_t max_len);
void PieceTable_Free(PieceTable* table);