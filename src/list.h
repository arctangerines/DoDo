#ifndef LIST_H
#define LIST_H

#include <stddef.h>

struct hlInfo
{
    size_t line_start;
    size_t line_pos_start;
    size_t line_end;
    size_t line_pos_end;
    size_t hl_word_line_start;
    size_t hl_word_line_end;
    size_t hl_word_cursor_start;
    size_t hl_word_cursor_end;
    char*  hl_color;
};

/*TODO: For linked list
 * - Add removing elements and keeping order
 * - Add inserting elements in between 2 elements
 */
// FIXME: names names names names names, theyre all ugly
// this could easily be made for arbitraryd ata types lmfao
struct dodoList
{
    struct hlInfo    hl;
    struct dodoList* next;
};

/// Contains the actual highlight info for parsing
struct dodoFileHighlights
{
    size_t           lines;
    struct dodoList* list;
};

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
                    char*  color);

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
                    char*            color);

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
                      char*            color);
/// Traverse from the root to the end of the linked list, deallocate
/// elements as we go
int
dodo_ll_destroy(struct dodoList* root);

void
dump_ll(struct dodoList* root);

#endif // LIST_H
