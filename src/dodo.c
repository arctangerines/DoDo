#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
// TODO: make a clang tidy lmao
// TODO: Look at error codes in the C/GNU way thinking emoji

/// Our node data type for the trie
struct dodoTrieNode
{
    /// My data!
    wchar_t c;
    /// Pointer(array) to a pointer (data) of all our children nodes
    struct dodoTrieNode** children;
    /// Amount of nodes
    size_t count;
};

// TODO: Maybe add a system to track nodes on the whole tree

/*MORSEL: So we have this concept of the comma operator, its an operator,
 * just like the other operators (lol)
 * when we separate expressions (1,2,3) or (a,b,c), it always returns the
 * last expression, in this case 3 or c, respectively
 * BUT, all expressions are evaluated
 */

/*
 * Soooo, these 2 macros work together, leveraging the variadic macros,
 * to create a default value for the funciton parameters
 * So the dodo_trie_init macro, if its empty it'll expand to:
 * dodo_trie_init(leading_var(0x02))
 * And leading_var always returns the first parameter so it's just 0x02
 * but if there is a value, it expandes to
 * dodo_trie_init(leading_var(x, 0x02))
 * and leading_var always return the first value, we can put in as many values
 * as we want but it will always return the first value in the (a,b) comma
 * separated expression
 */
/// Selects the first variable and discards the rest
#define leading_var(x, ...) x
/// @param c: Char we want to initialize the node with, if empty it'll create a
/// root node, and we do not want that unless we starting a new tree
#define dodo_trie_init(...)                                                    \
    dodo_trie_init(leading_var(__VA_ARGS__ __VA_OPT__(, ) 0x02))
/// Returns a new node for our trie
struct dodoTrieNode*
dodo_trie_init(const wchar_t c)
{
    struct dodoTrieNode* node = malloc(sizeof(struct dodoTrieNode));
    /// STX control character because I think \0 is confusing
    node->c        = c;
    node->children = nullptr;
    node->count    = 0;

    return node;
};

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

// XXX: Less verbose?
/// Add a keyword to the trie
/// @param node: We NEED the node parameter to be the root of the tree
/// @param keyword: Our string
/// @param keyword_len: Pointers decay
// THINK: Change node var to root?
void
dodo_trie_add(struct dodoTrieNode* node, const wchar_t* keyword,
              const size_t keyword_len)
{
    /// Arbitrary node, we initialize it with the node we passed
    struct dodoTrieNode* a_node = node;
    // Checking children first of the root node
    // THINK: Add a variable called 'empty children'? That way no need to check
    // these directly. Problem is that maybe we will need to access them anyways
    // What happens if one of the two is not set right? lol
    for (size_t j = 0; keyword[j] != '\0'; j++)
    {
        if (keyword[j] == a_node->c && keyword[j + 1] != '\0')
        {
            // Backup node?
            struct dodoTrieNode* b_node =
                dodo_trie_find_child(a_node, keyword[j + 1]);
            // Actually gotta stop and think cuz I need to organize this search
            // I could do keyword[j+1] or work with conditionals/loops better
            if (b_node != nullptr)
            {
                //... Then we continue from that node
                a_node = b_node;
            }
            // We try to find the children, and we assign a_node to this new
            // pointer
            else
            {
                //... Then we insert making a new branch
                // NOTE: This function needs to insert only looking down at the
                // tree
            }
        }
        else if (keyword[j] != a_node->c)
        {
            //... Then we make a new branch for our tree, easy
        }
    }
    for (size_t i = 0; i < keyword_len; i++)
    {
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