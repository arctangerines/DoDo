#include "term_colors.h"
#include <errno.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <trie.h>

// TODO: Consider renaming functions we dont want user (me?) to call like
// dodo_trie_destroy_children

/// @param p: Pointer to the allocated memory
/// @param h: If set to 'q' we panic because of the error,
/// if set to 'c' we continue
void
mem_error_handling(void* p, const char h)
{
    if (p == NULL)
    {

        printf("Error %i: %hs\n", errno, strerror(errno));
        // I think that 2 if statements looks better but switch literally
        // made for these situations
        switch (h)
        {
        case 'c':
        {
            printf("Continuing...\n");
            break;
        }
        case 'q':
        {
            exit(errno);
            break;
        }
        default:
            printf("Continuing...\n");
        }
    }
}

/*MORSEL: In C and other languages, I guess, we have this concept of the comma
 * operator, its an operator, just like the other operators (lol) when we
 * separate expressions (1,2,3) or (a,b,c), it always returns the last
 * expression, in this case 3 or c, respectively BUT, all expressions are
 * evaluated
 */

/// Returns a starting node for our trie,
/// the root node always has the value 0x02
struct dodoTrieNode*
dodo_make_trie()
{
    struct dodoTrieNode* node = malloc(sizeof(struct dodoTrieNode));
    mem_error_handling(node, 'q');
    /// STX control character because I think \0 is confusing
    node->c                 = 0x02;
    node->_children_alloced = false;
    node->children          = nullptr;
    node->count             = 0;
    node->bottom            = false;
    node->depth             = 0;

    return node;
}

/// Function to make a node with a char we specify
struct dodoTrieNode*
dodo_make_tnode(const char c, size_t depth)
{
    struct dodoTrieNode* node = malloc(sizeof(struct dodoTrieNode));
    mem_error_handling(node, 'q');
    node->c = c;
    // XXX: Look at the XXX on top of the dodoTrieNode definition
    node->_children_alloced = false;
    node->children          = nullptr;
    node->count             = 0;
    node->bottom            = false;
    node->depth             = depth;

    return node;
}

// MORSEL: The function strstr from libc's <string.h> header takes 2 parameters:
//  a const char* called haystack and a const char* called needle
/// Find the children node with the character we are looking for
struct dodoTrieNode*
dodo_trie_find_child(struct dodoTrieNode* node, const char c)
{
    // If its empty, theres no children
    if (node->count == 0)
    {
        return nullptr;
    }
    // Loop through our children nodes
    for (size_t i = 0; i < node->count; i++)
    {
        if (node->children[i]->c == c) return node->children[i];
    }
    // If we don't find anything
    return nullptr;
}

/// This functions inserts 1 char into the nodes below the one we pass,
/// and then returns said node we just inserted
struct dodoTrieNode*
dodo_trie_insert(struct dodoTrieNode* node, const char c)
{
    // not a valid character to insert since it's our character for root of the
    // trie
    if (c == 0x02)
    {
        printf("Error: 0x02 is not a valid character for keywords.\n");
        exit(-1);
    }
    // Arbitrary node
    struct dodoTrieNode* a_node = node;
    // Backup node so we dont shadow a_node if we were to use it
    struct dodoTrieNode* b_node = nullptr;
    // We check if there are children nodes
    if (a_node->count != 0)
    {
        // First we try to check that the node is not there already
        b_node = dodo_trie_find_child(a_node, c);
        if (b_node != nullptr)
        {
            return b_node;
        }

        // No children node has our char so we need to add it:
        // Expand the size of the array
        a_node->count += 1;
        // Realloc to expand, but this is up
        struct dodoTrieNode** a_child = realloc(
            a_node->children, sizeof(struct dodoTrieNode**) * (a_node->count));
        // Error handling
        // Should we add a perror handling function?
        mem_error_handling(a_child, 'q');
        a_node->children = a_child;
        // We add +1 because we adding an element downwards
        struct dodoTrieNode* c_node = dodo_make_tnode(c, a_node->depth + 1);
        a_node->children[a_node->count - 1] = c_node;
        return c_node;
    }
    // stupid
    a_node->children = malloc(sizeof(struct dodoTrieNode**));
    mem_error_handling(a_node->children, 'q');
    a_node->children[0]       = dodo_make_tnode(c, a_node->depth + 1);
    a_node->_children_alloced = true;
    a_node->count += 1;
    if (a_node->bottom == true)
    {
        a_node->bottom = false;
    }
    return a_node->children[0];
}

/// This function lets us add a keyword to the tree
void
dodo_trie_add_keyword(struct dodoTrieNode* node, const char* keyword)
{
    // Arbitrary node, on this one we will store the node on top of the one we
    // will insert
    struct dodoTrieNode* a_node = node;
    for (size_t j = 0; keyword[j] != '\0'; j++)
    {
        // Function returns the node that we have to continue inserting from
        a_node = dodo_trie_insert(a_node, keyword[j]);
    }
    a_node->bottom = true;
}

/// This function destroys the children nodes and the node itself we passed
/// No questions asked, it will just free everything basically
void
dodo_trie_destroy_node(struct dodoTrieNode* node)
{
    // Check for children
    if (node->count != 0)
    {
        // Loop through them destroying
        for (size_t j = 0; j < node->count; j--)
        {
            free(node->children[j]);
        }
        // Free the array
        free(node->children);
    }
    free(node);
}

/// Here we find the deepest node and then start returning from that
/// Return the deepest node that contains nodes that don't have any other
/// subnodes
/// Should I be lazy and make dig delete things or just return things
void
dodo_trie_destroy(struct dodoTrieNode* node)
{
    // copy of our count so we can subtract from node->count
    size_t count = node->count;
    // Check if there are any children
    if (node->count != 0)
    {
        for (size_t j = 0; j < count; j++)
        {
            // If the child node has children we need to keep digging
            if (node->children[j]->count != 0)
            {
                dodo_trie_destroy(node->children[j]);
            }
            // finally reduce and free
            node->count -= 1;
            free(node->children[j]);
        }
        // Since the value of count gets updated recursively
        // we need to check again so we can free the array
        if (node->count == 0 && node->_children_alloced) free(node->children);
    }
    //  Checking if NOW we can delete root
    if (node->c == 0x02 && node->count == 0)
    {
        free(node);
    }
}
