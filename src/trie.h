//
// Created by infrared on 21/04/25.
//

#ifndef TRIE_H
#define TRIE_H
#include <stdio.h>

// TODO: We could alloc a default array of n length for @children
//  so the count is exclusively for the amount of nodes and
//  we add another variable that keeps track of the actual length of the
//  array allowing us to allocate less
// XXX: A space optimized version of a trie
// would be with pointers instead of char?
/// Our node data type for the trie
struct dodoTrieNode
{
    /// My data!
    char c;
    /// Having this variable allows for checking if the nodes have been removed
    /// but the array hasn't been dealloced
    bool _children_alloced;
    /// Pointer(array) to a pointer (data) of all our children nodes
    struct dodoTrieNode** children;
    /// Amount of nodes.
    size_t count;
    /// End of the word
    bool bottom;
    /// How long is the word. (It's just len but in a different direction)
    size_t depth;
};

void
mem_error_handling(void* p, const char h);

struct dodoTrieNode*
dodo_make_trie();

struct dodoTrieNode*
dodo_make_tnode(const char c, size_t depth);

struct dodoTrieNode*
dodo_trie_find_child(struct dodoTrieNode* node, const char c);

struct dodoTrieNode*
dodo_trie_insert(struct dodoTrieNode* node, const char c);

void
dodo_trie_add_keyword(struct dodoTrieNode* node, const char* keyword);

void
dodo_trie_destroy_node(struct dodoTrieNode* node);

void
dodo_trie_destroy(struct dodoTrieNode* node);

#endif // TRIE_H
