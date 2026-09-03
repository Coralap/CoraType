#include "piece_table.h"


bool PieceTable_Init(PieceTable *table, const wchar_t *text, size_t len) {
    if(!table) return false;
    //check for possible overflow
    if(len >(SIZE_MAX/sizeof(wchar_t))-1)
        return false;

    // allocate and copy original text
    if (len > 0) {
        table->original = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (!table->original) return false;
        
        memcpy(table->original, text, len * sizeof(wchar_t));
        table->original[len] = L'\0';
    } else {
        table->original = NULL;
    }
    table->original_length = len;

    // init added buffer
    if(!Text_Init(&table->added,64)){
        free(table->original);
        table->original = NULL;
        return false;

    }

    // add the node to the linked list
    if (len > 0) {
        Node *head_node = (Node*)malloc(sizeof(Node));
        if (!head_node) {
            free(table->original);
            table->original = NULL;
            Text_Free(&table->added);
            return false;
        }

        head_node->type = BUFFER_ORIGINAL;
        head_node->start = 0;
        head_node->length = len;
        head_node->prev = NULL;
        head_node->next = NULL;

        table->head = head_node;
        table->tail = head_node;
    } else {
        table->head = NULL;
        table->tail = NULL;
    }

    return true;
}

bool PieceTable_InsertChar(PieceTable *table, const wchar_t text, size_t pos) {
    size_t add_start = table->added.length;
    Text_InsertChar(&table->added, text);

    // Empty document
    if (table->head == NULL) {
        Node *new_node = (Node*)malloc(sizeof(Node));
        if (!new_node) return false;

        new_node->type = BUFFER_ADDED;
        new_node->start = add_start;
        new_node->length = 1;
        new_node->prev = NULL;
        new_node->next = NULL;

        table->head = new_node;
        table->tail = new_node;
        return true;
    }

    Node *curr = table->head;
    size_t current_offset = 0;

    while (curr != NULL) {
        current_offset += curr->length;

        // Check if pos is inside or at the boundary of curr
        if (current_offset - curr->length <= pos && pos <= current_offset) {
            
            // Exact start of curr node
            if (pos == current_offset - curr->length) {
                Node *new_node = (Node*)malloc(sizeof(Node));
                if (!new_node) return false;

                new_node->type = BUFFER_ADDED;
                new_node->start = add_start;
                new_node->length = 1;
                
                // Link new_node between curr->prev and curr
                new_node->prev = curr->prev;
                new_node->next = curr;

                if (curr->prev != NULL) {
                    curr->prev->next = new_node;
                } else {
                    table->head = new_node;
                }
                curr->prev = new_node;

                return true;
            }

            // Exact end of curr node
            if (pos == current_offset) {
                // check for consecutive appends
                if (curr->type == BUFFER_ADDED && (curr->start + curr->length == add_start)) {
                    curr->length += 1;
                    return true;
                }

                Node *new_node = (Node*)malloc(sizeof(Node));
                if (!new_node) return false;

                new_node->type = BUFFER_ADDED;
                new_node->start = add_start;
                new_node->length = 1;

                // Link new_node between curr and curr->next
                new_node->prev = curr;
                new_node->next = curr->next;

                if (curr->next != NULL) {
                    curr->next->prev = new_node;
                } else {
                    table->tail = new_node;
                }
                curr->next = new_node;

                return true;
            }

            // middle of node
            size_t node_doc_start = current_offset - curr->length; //start of node
            size_t split_offset = pos - node_doc_start; //offset to split

            Node *right_node = (Node*)malloc(sizeof(Node));
            if (!right_node)    return false;

            right_node->type = curr->type;
            right_node->start = curr->start+split_offset;
            right_node->length = curr->length - split_offset;



            Node *middle_node = (Node*)malloc(sizeof(Node));
            if (!middle_node) {
                free(right_node);
                return false;
            }

            middle_node->type = BUFFER_ADDED;
            middle_node->start = add_start;
            middle_node->length = 1;


            right_node->next = curr->next;//insert the middle node to the linked list
            middle_node->next = right_node;
            middle_node->prev = curr;
            right_node->prev = middle_node;

            curr->length = split_offset;
            curr->next = middle_node;

            //incase the node is the last one
            if (right_node->next != NULL) {
                right_node->next->prev = right_node;
            }else {
                table->tail = right_node;
            }

            return true;
        }

        curr = curr->next;
    }

    return true;
}



bool PieceTable_DeleteChar(PieceTable *table, size_t pos) {

    // Empty document
    if (!table || table->head == NULL) {
        return false;
    }

    Node *curr = table->head;
    size_t current_offset = 0;

    while (curr != NULL) {
        current_offset += curr->length;
        size_t node_doc_start = current_offset - curr->length;

        // Check the char at pos is inside ucrr
        if (pos >= node_doc_start && pos < current_offset) {
            if(curr->length ==1)//free and relink if the node only has 1 char
            {
                if (curr->prev != NULL) {
                    curr->prev->next = curr->next;
                } else {
                    table->head = curr->next;
                }

                if (curr->next != NULL) {
                    curr->next->prev = curr->prev;
                } else {
                    table->tail = curr->prev;
                }

                free(curr);
                return true;
            }

            // Exact start of curr node
            if (pos == node_doc_start) {
                curr->start++;
                curr->length--;
                return true;
            }


            // Exact end of curr node
            if (pos == current_offset - 1) {
                curr->length--;
                return true;
            }

            // middle of node
            size_t split_offset = pos - node_doc_start; //offset to split

            Node *right_node = (Node*)malloc(sizeof(Node));
            if (!right_node)    return false;

            right_node->type = curr->type;
            right_node->start = curr->start+split_offset + 1;//skip deleted char
            right_node->length = curr->length - split_offset -1;


            //relink
            right_node->next = curr->next;
            right_node->prev = curr;

            curr->next = right_node;
            curr->length = split_offset;

            //incase the node is the last one
            if (right_node->next != NULL) {
                right_node->next->prev = right_node;
            }else {
                table->tail = right_node;
            }

            return true;
        }

        curr = curr->next;
    }

    return false; // pos was probably out of bounds
}





size_t PieceTable_GetTotalLength(const PieceTable *table){
    Node *curr = table->head;
    size_t len = 0;

    while (curr != NULL) {
        len +=curr->length;
        curr = curr->next;
    }
    return len;
}
bool PieceTable_GetText(const PieceTable* table, wchar_t *out, size_t max_len){
    if (!out || max_len == 0) return false;

    size_t dest_offset = 0;
    Node *curr = table->head;

        while (curr != NULL && dest_offset < max_len - 1) { 
        //only copy the allowed number of characters
        size_t to_copy = curr->length;
        if (dest_offset + to_copy > max_len - 1) {
            to_copy = (max_len - 1) - dest_offset;
        }

        const wchar_t *src = (curr->type == BUFFER_ORIGINAL)
                                    ? table->original + curr->start
                                    : table->added.text + curr->start; //move the pointer according to the node

        memcpy(out + dest_offset, src, to_copy * sizeof(wchar_t));
        dest_offset += to_copy;

        curr = curr->next;
    }

    out[dest_offset] = L'\0';
    return true;
}

void PieceTable_Free(PieceTable* table) {
    if (!table) return;

    free(table->original);
    table->original = NULL;
    table->original_length = 0;

    Text_Free(&table->added);

    Node *curr = table->head; //free all nodes
    while (curr != NULL) {
        Node *next = curr->next;
        free(curr);
        curr = next;
    }

    table->head = NULL;
    table->tail = NULL;
}