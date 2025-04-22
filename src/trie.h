//
// Created by infrared on 21/04/25.
//

#ifndef TRIE_H
#define TRIE_H
#include <wchar.h>

struct dodoTrieNode;

void
mem_error_handling(void* p, const wchar_t h);

struct dodoTrieNode*
dodo_make_trie();

struct dodoTrieNode*
dodo_make_tnode(const wchar_t c);

struct dodoTrieNode*
dodo_trie_find_child(struct dodoTrieNode* node, const wchar_t c);

struct dodoTrieNode*
dodo_trie_insert(struct dodoTrieNode* node, const wchar_t c);

void
dodo_trie_add_keyword(struct dodoTrieNode* node, const wchar_t* keyword);

void
dodo_trie_destroy_node(struct dodoTrieNode* node);

void
dodo_trie_destroy(struct dodoTrieNode* node);

#endif // TRIE_H
