#ifndef PARSING_IO_H
#define PARSING_IO_H
#include <list.h>
#include <stddef.h>

#include <ck.h>
#include <stdio.h>
#include <trie.h>

/// @param c: char we are looking for
/// @param f: file we are working with
/// @param pos: pos_t var we are using to preserve our position
bool
simple_look_ahead(int     c,
                  FILE*   f,
                  fpos_t* pos);

/// If n < k_len then we will start at index len - n
/// Its like saying how many n last characters we want to read
/// Useful because we might have read
bool
n_look_ahead(size_t  n,
             char*   k,
             FILE*   f,
             fpos_t* pos);

// FIXME: Error handling and cleanup
/// This functions takes in a trie with the keywords and generates and return
/// a linked list with the info related to the lines and cursor position of the highlights
struct dodoFileHighlights*
dodo_gen_todo_data(FILE*                f,
                   struct commentKeys   ck,
                   struct dodoTrieNode* trie_keywords);

/// This function takes in a wrapper that contains the linked list with highlight data
/// and other file info and prints to terminal
void
dodo_print_todos(FILE*                      f,
                 const char*                filename,
                 struct dodoFileHighlights* file_hl,
                 size_t                     extra_lines);

#endif // PARSING_IO_H
