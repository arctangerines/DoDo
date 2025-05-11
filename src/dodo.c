#include "term_colors.h"
#include <trie.h>

#include <errno.h>
#include <locale.h>
#include <silky.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <wchar.h>
#include <wordexp.h>

/*
 *XXX: As it is, the program saves starting from the ToDo of the coment,
 * to the end.
 * Perhaps it should save comment blocks only if the comment block has a todo
 *REBUTTAL: Although, this could be prettier on terminal, simply saving todo
 * leads to embracing a "just plain text™️" approach, so we can pass it
 * to other programs
 */

size_t
utf8_length(const uint8_t c)
{
    // practically saying if theres no set bit at the start ([1]0000000), cancel
    if (c < 0x80)
    {
        // It's held in 1 byte but we dont need to check anything
        return 1;
    }
    // inverse of the bits
    uint8_t m = 0b0100'0000;
    // suppse c 0b1011'1010
    // supps ~c 0b0100'0101
    // So what we are doing is basically comparing that witht he mask, we are
    // searching for the 0 that indicates the amount of bytes the utf8 char has
    // we do it with a 1 inversed
    // 0b0100'0000
    // 0b0100'0101
    //-----------
    //=0b0100'0000

    uint8_t x = (~c) & m;
    // shift size IS the byte size, because
    uint8_t shift_size = 1;
    // If it's not 1 byte, we start shifting to the right
    // until we find our 0 (which is a 1 when we !)
    while (x != m)
    {
        shift_size++;
        m = m >> 1;
        x = (~c) & m;
        if (shift_size > 4)
        {
            return 0;
        };
    }
    return shift_size;
}

struct hlInfo
{
    size_t line_start;
    size_t line_pos_start;
    size_t line_end;
    size_t line_pos_end;
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

/// Make an arbitrary element and return it, good for starting a linked list
/// @param hl_line_start: Line to start the highlight from
struct dodoList*
dodo_ll_new_element(size_t hl_line_start,
                    size_t hl_line_pos_start,
                    size_t hl_line_end,
                    size_t hl_line_pos_end)
{
    struct dodoList* a_ll   = malloc(sizeof(struct dodoList));
    a_ll->hl.line_start     = hl_line_start;
    a_ll->hl.line_pos_start = hl_line_pos_start;
    a_ll->hl.line_end       = hl_line_end;
    a_ll->hl.line_pos_end   = hl_line_pos_end;
    a_ll->next              = nullptr;
    return a_ll;
}

struct dodoList*
dodo_ll_add_element(struct dodoList* start,
                    size_t           hl_line_start,
                    size_t           hl_line_pos_start,
                    size_t           hl_line_end,
                    size_t           hl_line_pos_end)
{
    if (start == nullptr) return nullptr;
    if (start->next != nullptr)
    {
        printf("Already pointing to another element\n");
        return nullptr;
    }

    struct dodoList* new = dodo_ll_new_element(hl_line_start, hl_line_pos_start,
                                               hl_line_end, hl_line_pos_end);
    if (start->next == nullptr)
    {
        start->next = new;
        return new;
    }
}

// FIXME: Maybe this should be the default way to add elements,
// we add a pointer to the end at the start? Now that im writing, it seems bad
struct dodoList*
dodo_ll_add_from_root(struct dodoList* root,
                      size_t           hl_line_start,
                      size_t           hl_line_pos_start,
                      size_t           hl_line_end,
                      size_t           hl_line_pos_end)
{
    struct dodoList* a_ll = root;
    if (a_ll == nullptr) return nullptr;
    while (a_ll->next != nullptr)
    {
        a_ll = a_ll->next;
    }

    struct dodoList* new = dodo_ll_new_element(hl_line_start, hl_line_pos_start,
                                               hl_line_end, hl_line_pos_end);
    a_ll->next           = new;
    return new;
}

/// Traverse from the root to the end of the linked list, deallocate
/// elements as we go
int
dodo_ll_destroy(struct dodoList* root)
{
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
        printf("Cursor post end: %lu\n", a_element->hl.line_pos_end);
        printf("\n");
        a_element = a_element->next;
    }
}

struct commentKeys
{
    char** exts;
    char** keys;
};

void
ck_add_key(struct commentKeys* ck)
{
}

/// @param c: char we are looking for
bool
simple_look_ahead(char    c,
                  FILE*   f,
                  fpos_t* pos)
{
    fgetpos(f, pos);
    // L for lookahead
    int l = fgetc(f);
    if (l == c)
    {
        fsetpos(f, pos);
        return true;
    }
    fsetpos(f, pos);
    return false;
}

// TODO: Look at error codes in the C/GNU way thinking emoji
// FIXME: Document this function;
int
create_config_files()
{
    // TODO: Get environment variable for XDG_CONFIG_HOME
    char* conf_dir_env_path = getenv("XDG_CONFIG_HOME");
    if (conf_dir_env_path)
    {
        char* conf_dodo = "/dodo";
        char* conf_file = "/dodofile";

        struct stat conf_dir_env_stat;
        if (stat(conf_dir_env_path, &conf_dir_env_stat) == -1)
        {
            if (errno == ENOENT)
            {
                mkdir(conf_dir_env_path, 0700);
                wprintf(L"Creating XDG config directory...\n");
            }
        }
        size_t env_path_len = strlen(conf_dir_env_path);

        // +1 for our '\0'
        size_t n = strlen(conf_dodo) + env_path_len + strlen(conf_file) + 1;
        char*  conf_dodo_path;
        conf_dodo_path = malloc(sizeof(char) * n);
        // This is important because '\0' is gonna replace what we got at
        // the end of the destination string, and in this case that is the
        // destination string, so it needs to be initialized
        conf_dodo_path[0] = '\0';
        strncat(conf_dodo_path, conf_dir_env_path, env_path_len);
        strncat(conf_dodo_path, conf_dodo, strlen(conf_dodo));

        if (stat(conf_dodo_path, &conf_dir_env_stat) == -1)
        {
            if (errno == ENOENT)
            {
                mkdir(conf_dodo_path, 0700);
                wprintf(L"Creating dodo folder in XDG config directory...\n");
            }
        }
        strncat(conf_dodo_path, conf_file, strlen(conf_file));
        if (stat(conf_dodo_path, &conf_dir_env_stat) == -1)
        {
            if (errno == ENOENT)
            {
                fclose(fopen(conf_dodo_path, "a+"));
                wprintf(L"Creating dodofile\n");
            }
        }

        free(conf_dodo_path);
        return 0;
    }
    // SECTION: Without XDG_CONFIG_HOME
    // FIXME: Please stop using so many variables, fix this NEOW
    wordexp_t p;
    char*     conf_dir_tilde = "~/.config";
    if (wordexp(conf_dir_tilde, &p, 0) == -1)
    {
        wprintf(L"We couldn't expand\n");
        return -1;
    }
    char* conf_dir;
    conf_dir = p.we_wordv[0];

    struct stat conf_dir_stat;
    if (stat(conf_dir, &conf_dir_stat) == -1)
    {
        wprintf(L"Couldn't stat user config directory.\n");
        if (errno == ENOENT)
        {
            mkdir(conf_dir, 0700);
            wprintf(L"Creating home config directory...\n");
        }
        else
            return -1;
    }

    wordexp_t q;
    char*     conf_dir_full_tilde = "~/.config/dodo";
    if (wordexp(conf_dir_full_tilde, &q, 0) == -1)
    {
        wprintf(L"We couldn't expand\n");
        return -1;
    }
    char* conf_dir_full;
    conf_dir_full = q.we_wordv[0];
    struct stat conf_dir_full_stat;
    if (stat(conf_dir_full, &conf_dir_full_stat) == -1)
    {
        if (errno == ENOENT)
        {
            mkdir(conf_dir_full, 0700);
            wprintf(L"Creating...\n");
        }
        else
            return -1;
    }

    wordexp_t w;
    char*     conf_file_path = "~/.config/dodo/dodofile";
    if (wordexp(conf_file_path, &w, 0) == -1)
    {
        wprintf(L"We couldn't expand\n");
        return -1;
    }
    char*       conf_file = w.we_wordv[0];
    struct stat conf_file_stat;
    if (stat(conf_file, &conf_file_stat) == -1)
    {
        if (errno == ENOENT)
        {
            // This is really really really bad but it makes me laugh
            fclose(fopen(conf_file, "a+"));
        }
        else
            return -1;
    }
    return 0;
}

/*WARNING: I'm going to be leaking memory all over the place because
 * initial focus will be on design correctness
 * (which you can argue includes memory management but I promise I won't
 * forget)
 */
int
main(int    argc,
     char** argv)
{
    // FILE* config_file = fopen("./testfig/dodofile", "a+");

    // if (create_config_files() == -1)
    // {
    //     wprintf(L"Error %i: %hs", errno, strerror(errno));
    //     exit(errno);
    // }

    /// Portable locale lolz
    setlocale(LC_ALL, "");

    // TODO: Config file and its integration

    bool only_comments = false;
    if (argc > 1 && strcmp(argv[1], "-c") == 0)
    {
        only_comments = true;
    }

    struct dodoTrieNode* cool_trie = dodo_make_trie();
    dodo_trie_add_keyword(cool_trie, "TODO");
    dodo_trie_add_keyword(cool_trie, "FIXME");
    dodo_trie_add_keyword(cool_trie, "XXX");
    dodo_trie_add_keyword(cool_trie, "BUG");
    dodo_trie_add_keyword(cool_trie, "THINK");
    // dodo_trie_add_keyword(cool_trie, "🧬");

    FILE* test_file = fopen("test.py", "r");
    char* ext       = strrchr("test.py", '.');

    /*MORSEL: Cool thing about unicode
     * UTF-8 is compatible with regular chars and thats because chars are 8
     * bits but we never use the first bit, so if a character is UTF-8 you
     * just check the first bit so the letter e is 01100101 in binary, but
     * this emoji 🧬 is 11110000 10011111 10100111 10101100 So when a
     * function that operates on utf 8 sees the first bit it knows if it has
     * to treat it like a char (ASCII) or a multibyte char (rune?) shoutouts
     * golang "I can't believe it's not ASCII!"
     */
    int x;
    /// Single line comment
    bool sl_comment = false;
    /// Multiline comment
    bool   ml_comment = false;
    bool   hl_mode    = false;
    fpos_t pos;
    /// This variable is going to tell us how many letters to skip
    size_t skip     = 0;
    size_t inc_skip = 0;

    // Line number we are at
    size_t line_no = 1;
    // Position of cursor
    size_t cursor_pos = 0;
    // last value of the cursor in previous line
    size_t cursor_pos_prev = 0;
    // printf("%*lu |  ", -3, line_no);
    /* It should have a lookahead concept but not for a word or a char
     * but for comment lines, also I think this should be done in 2 passes,
     * one stores where the todo starts and the second pass prints
     */
    // printf("%*lu |  ", -3, line_no);
    struct dodoList* root  = nullptr;
    struct dodoList* last  = nullptr;

    size_t hl_line_start   = line_no;
    size_t hl_cursor_start = cursor_pos;
    // size_t hl_line_end     = line_no;
    // size_t hl_cursor_end   = cursor_pos;
    bool keyword_found = false;
    while ((x = (fgetc(test_file))) != EOF)
    {
        printf("%c", x);
        if (x == '#' && !ml_comment)
        {
            sl_comment = 1;
        }
        if (x == '/' && !sl_comment)
        {
            if (simple_look_ahead('*', test_file, &pos))
            {
                ml_comment = true;
            }
        }
        if (sl_comment || ml_comment)
        {
            // Current node
            struct dodoTrieNode* c_node = dodo_trie_find_child(cool_trie, x);
            if (c_node != nullptr && !keyword_found)
            {
                fgetpos(test_file, &pos);
                hl_line_start   = line_no;
                hl_cursor_start = cursor_pos;
                // hl_line_end     = line_no;
                // hl_cursor_end   = cursor_pos;
                while ((x = fgetc(test_file)) != EOF)
                {
                    c_node = dodo_trie_find_child(c_node, x);
                    // we can put it on top because it's not the first value
                    cursor_pos++;
                    // In theory no keyword should contain the char '\n'
                    // if (x =='\n') line_no++; cursor_pos=0;
                    if (c_node == nullptr)
                    {
                        fsetpos(test_file, &pos);
                        break;
                    }
                    if (c_node->bottom == true)
                    {
                        keyword_found = true;
                        break;
                    }
                }
            }
        }

        if (x == '\n')
        {
            if (sl_comment)
            {
                sl_comment = 0;
                if (keyword_found)
                {
                    last          = (last == nullptr)
                                        ? (root = dodo_ll_new_element(
                                      hl_line_start, hl_cursor_start, line_no,
                                      cursor_pos))
                                        : dodo_ll_add_element(last, hl_line_start,
                                                              hl_cursor_start, line_no,
                                                              cursor_pos);

                    keyword_found = false;
                }
            }
            line_no += 1;
            cursor_pos_prev = cursor_pos;
            cursor_pos      = 0;
        }
        else if (x == '*')
        {
            if (ml_comment)
            {
                if (simple_look_ahead('/', test_file, &pos))
                {
                    ml_comment = false;
                }
                if (keyword_found)
                {
                    // we make this variable so we dont have to touch cursor pos
                    // since multiline comments end in */, the last value of
                    // them will be the position before the *
                    // BUG? What if user leaves a bunch of whitespace before
                    // */ instead of a newline
                    size_t hl_cursor_end;
                    // multiline commend ends at the start of a newline
                    if (cursor_pos == 0)
                    {
                        hl_cursor_end = cursor_pos_prev;
                    }
                    else
                    {
                        hl_cursor_end = cursor_pos--;
                    }
                    last          = (last == nullptr)
                                        ? (root = dodo_ll_new_element(
                                      hl_line_start, hl_cursor_start, line_no,
                                      cursor_pos))
                                        : dodo_ll_add_element(last, hl_line_start,
                                                              hl_cursor_start, line_no,
                                                              hl_cursor_end);

                    keyword_found = false;
                }
            }
        }
        else
        {
            cursor_pos += 1;
        }
        // if (x == '\n') printf("%*lu |  ", -3, line_no);
    }
    dump_ll(root);
    dodo_ll_destroy(root);
    fclose(test_file);

    dodo_trie_destroy(cool_trie);
    return 0;
}