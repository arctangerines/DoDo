#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
// TODO: make a clang tidy lmao

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

/// Returns a new node for our trie
struct dodoTrieNode*
dodo_trie_init()
{
    struct dodoTrieNode* node = malloc(sizeof(struct dodoTrieNode));
    /// What the hell, sure
    node->c        = '\0';
    node->children = nullptr;
    node->count    = 0;

    return node;
};

/// Find the children node with the character we are looking for
// MORSEL: The function strstr from libc <string.h> header takes 2 parameters:
//  a const char* called haystack and a const char* called needle
struct dodoTrieNode*
dodo_find_child(struct dodoTrieNode* node, const wchar_t c)
{
    if (node->count == 0)
    {
        return nullptr;
    }
    for (size_t i = 0; i < node->count; i++)
    {
    }
}

// XXX: Less verbose?
/// Add a keyword to the trie
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
        if (keyword[j] == a_node->c)
        {
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