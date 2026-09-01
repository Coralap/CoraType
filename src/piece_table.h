#pragma once
#include <windows.h>
#include "text_buffer.h"
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