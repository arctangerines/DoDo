#include "list.h"
#include <stdio.h>
#include <stdlib.h>

/// Make an arbitrary element and return it, good for starting a linked list
/// @param hl_line_start: Line to start the highlight from
/// FIXME: Names names names
struct dodoList*
dodo_ll_new_element(size_t hl_line_start,
                    size_t hl_line_pos_start,
                    size_t hl_line_end,
                    size_t hl_line_pos_end,
                    size_t hl_word_line_start,
                    size_t hl_word_line_end,
                    size_t hl_word_cursor_start,
                    size_t hl_word_cursor_end,
                    char*  color)
{
    struct dodoList* a_ll         = malloc(sizeof(struct dodoList));
    a_ll->hl.line_start           = hl_line_start;
    a_ll->hl.line_pos_start       = hl_line_pos_start;
    a_ll->hl.line_end             = hl_line_end;
    a_ll->hl.line_pos_end         = hl_line_pos_end;
    a_ll->hl.hl_word_line_start   = hl_word_line_start;
    a_ll->hl.hl_word_line_end     = hl_word_line_end;
    a_ll->hl.hl_word_cursor_start = hl_word_cursor_start;
    a_ll->hl.hl_word_cursor_end   = hl_word_cursor_end;
    a_ll->hl.hl_color             = color;
    a_ll->next                    = nullptr;
    return a_ll;
}

struct dodoList*
dodo_ll_add_element(struct dodoList* start,
                    size_t           hl_line_start,
                    size_t           hl_line_pos_start,
                    size_t           hl_line_end,
                    size_t           hl_line_pos_end,
                    size_t           hl_word_line_start,
                    size_t           hl_word_line_end,
                    size_t           hl_word_cursor_start,
                    size_t           hl_word_cursor_end,
                    char*            color)
{
    if (start == nullptr) return nullptr;
    if (start->next != nullptr)
    {
        printf("Already pointing to another element\n");
        return nullptr;
    }

    if (start->next == nullptr)
    {

        struct dodoList* new =
            dodo_ll_new_element(hl_line_start, hl_line_pos_start, hl_line_end,
                                hl_line_pos_end, hl_word_line_start, hl_word_line_end,
                                hl_word_cursor_start, hl_word_cursor_end, color);
        start->next = new;
        return new;
    }
    return nullptr;
}

// FIXME: Maybe this should be the default way to add elements,
// we add a pointer to the end at the start? Now that im writing, it seems bad
struct dodoList*
dodo_ll_add_from_root(struct dodoList* root,
                      size_t           hl_line_start,
                      size_t           hl_line_pos_start,
                      size_t           hl_line_end,
                      size_t           hl_line_pos_end,
                      size_t           hl_word_line_start,
                      size_t           hl_word_line_end,
                      size_t           hl_word_cursor_start,
                      size_t           hl_word_cursor_end,
                      char*            color)
{
    struct dodoList* a_ll = root;
    if (a_ll == nullptr) return nullptr;
    while (a_ll->next != nullptr)
    {
        a_ll = a_ll->next;
    }

    struct dodoList* new = dodo_ll_new_element(
        hl_line_start, hl_line_pos_start, hl_line_end, hl_line_pos_end, hl_word_line_start,
        hl_word_line_end, hl_word_cursor_start, hl_word_cursor_end, color);
    a_ll->next = new;
    return new;
}

/// Traverse from the root to the end of the linked list, deallocate
/// elements as we go
int
dodo_ll_destroy(struct dodoList* root)
{
    if (root == nullptr)
    {
        return -1;
    }
    // NOTE: why did i say recursive, all i needed was a loop
    struct dodoList* current_element = root;
    struct dodoList* next_element    = root->next;
    if (current_element == nullptr) return -1;
    while (next_element != nullptr)
    {
        free(current_element);
        current_element = next_element;
        next_element    = current_element->next;
    }
    free(current_element);
    return 0;
}

void
dump_ll(struct dodoList* root)
{
    if (root == nullptr)
    {
        printf("\nNull root.\n");
        return;
    }
    struct dodoList* a_element = root;
    while (a_element != nullptr)
    {
        printf("\n");
        printf("Line start: %lu\n", a_element->hl.line_start);
        printf("Cursor pos start: %lu\n", a_element->hl.line_pos_start);
        printf("Line end: %lu\n", a_element->hl.line_end);
        printf("Cursor pos end: %lu\n", a_element->hl.line_pos_end);
        printf("Highlight word start: %lu\n", a_element->hl.hl_word_cursor_start);
        printf("Highlight word end: %lu\n", a_element->hl.hl_word_cursor_end);
        printf("\n");
        a_element = a_element->next;
    }
}
