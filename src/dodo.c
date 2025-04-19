#include <errno.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
// TODO: make a clang tidy lmao
// TODO: Look at error codes in the C/GNU way thinking emoji

// XXX: We could alloc a default array of n length for @children
//  so the count is exclusively for the amount of nodes and
//  we add another variable that keeps track of the actual length of the
//  array allowing us to allocate less
/// Our node data type for the trie
struct dodoTrieNode
{
    /// My data!
    wchar_t c;
    /// Pointer(array) to a pointer (data) of all our children nodes
    struct dodoTrieNode** children;
    /// Amount of nodes.
    size_t count;
};

// TODO: Maybe add a system to track nodes on the whole tree

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
    /// STX control character because I think \0 is confusing
    node->c        = 0x02;
    node->children = nullptr;
    node->count    = 0;

    return node;
};

struct dodoTrieNode*
dodo_make_tnode(const wchar_t c)
{
    struct dodoTrieNode* node = malloc(sizeof(struct dodoTrieNode));
    /// STX control character because I think \0 is confusing
    node->c = c;
    // XXX: Look at the XXX on top of the dodoTrieNode definition
    node->children = nullptr;
    node->count    = 0;

    return node;
}

// MORSEL: The function strstr from libc's <string.h> header takes 2 parameters:
//  a const char* called haystack and a const char* called needle
/// Find the children node with the character we are looking for
struct dodoTrieNode*
dodo_trie_find_child(struct dodoTrieNode* node, const wchar_t c)
{
    if (node->count == 0)
    {
        return nullptr;
    }
    for (size_t i = 0; i < node->count; i++)
    {
        // recursive?
        if (node->children[i]->c == c) return node->children[i];
    }
    // If we don't find anything
    return nullptr;
}

// Simple operation (?)
struct dodoTrieNode*
dodo_trie_insert(struct dodoTrieNode* node, const wchar_t c)
{
    struct dodoTrieNode* a_node = node;
    struct dodoTrieNode* b_node = nullptr;
    if (a_node->count != 0)
    {
        b_node = dodo_trie_find_child(a_node, c);
        if (b_node == nullptr)
        {
            // Expand the size of the array
            a_node->count += 1;
            struct dodoTrieNode** a_child =
                realloc(a_node->children,
                        sizeof(struct dodoTrieNode**) * (a_node->count));
            if (a_child == NULL)
            {
                printf("Error %i: %s", errno, strerror(errno));
                exit(errno);
            }
            a_node->children                    = a_child;
            struct dodoTrieNode* c_node         = dodo_make_tnode(c);
            a_node->children[a_node->count - 1] = c_node;
            return c_node;
        }
        return b_node;
    }
    // stupid
    a_node->children    = malloc(sizeof(struct dodoTrieNode**));
    a_node->children[0] = dodo_make_tnode(c);
    return a_node->children[0];
}

void
dodo_trie_add_keyword(struct dodoTrieNode* node, const wchar_t* keyword)
{
    // Arbitrary node, on this one we will store the node on top of the one we
    // will insert
    struct dodoTrieNode* a_node = node;
    for (size_t j = 0; keyword[j] != '\0'; j++)
    {
        // redundant? lol
        a_node = dodo_trie_insert(a_node, keyword[j]);
    }
}

void
dodo_destroy_children(struct dodoTrieNode* node)
{
    if (node->count != 0)
    {
        for (size_t j = 0; j < node->count; j--)
        {
            free(node->children[j]);
        }
        free(node->children);
    }
    free(node);
}

void
dodo_destroy_trie(struct dodoTrieNode* node)
{
    struct dodoTrieNode* a_node = node;
    while (a_node->count != 0)
    {
        for (size_t j = 0; j < a_node->count; j++)
        {
            //... I think I need to implement a dig function thats recursive that
            // just digs and digs
        }
    }
}

/*WARNING: I'm going to be leaking memory all over the place because
 * initial focus will be on design correctness
 * (which you can argue includes memory management but I promise I won't
 * forget)
 */
int
main(int argc, char** argv)
{
    /// Portable locale lolz
    setlocale(LC_ALL, "");
    wchar_t* x = L"Hello";
    wprintf(L"Stop it: %ls", x);
    return 0;
    FILE* f = fopen("./test.py", "r");
    /*NOTE: Cool thing about wchars
     * UTF-8 is compatible with regular chars
     * thats because chars are 8 bits
     * but we never use the first bit
     * so if a character is UTF-8 you just check the first bit
     * so the letter e is 01100101 in binary
     * but this emoji 🧬 is 11110000 10011111 10100111 10101100
     * So when a function that operates on utf 8 sees the first bit
     * it knows if said char,or rune, shoutouts golang,
     * needs to be handled like a multibyte sequence or like ASCII
     * "I can't believe it's not ASCII!"
     */
    wint_t c;
    while ((c = fgetwc(f)) != WEOF)
    {
        wprintf(L"%lc", c);
        if (c == 'T')
        {
            wprintf(L"\nBlam!\n");
            /*TODO: So when we find a letter that we want (like the T in TODO,
             * we want to start checking what comes after it
             * so we need to save our fseek position and check the next few
             * letters so we check for an O, D and O. But what if we want to do
             * this for other keywords, like
             * FIXME, NOTE, BUG, XXX
             * we need a structure that can hold every letter we need in an
             * ordered way and that we can go down of as we match and discard oh
             * wait a second, that's a trie lmao 🧠
             */
            // XXX: For now this will break when we find the word, but
            //  later it will break when we find the end of the tree (?)
        }
        // outer:
    }
    fclose(f);
}