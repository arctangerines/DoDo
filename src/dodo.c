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

// TODO: Start counting line and position in line

/// For bit flags idea but probably wont use it
/// Single line comment
#define SL_COMMENT  0b00000001
/// Multiline comment
#define ML_COMMENT  0b00000010
#define SKIP        0b00000100

// UTF characteristics (thanks to the Unicode spec)
#define UTF8_4BYTES 0b11110000
#define UTF8_3BYTES 0b11100000
#define UTF8_2BYTES 0b11000000
#define UTF8_1BYTE  0b00000000

// TODO: Add a funciton to determine the length of a UTF-8 char,
//  this should return the number of bytes to ignore for colored output

size_t
utf8_length(uint32_t c)
{
    // xor the bits
}

struct hlInfo
{
    size_t line_start;
    size_t char_pos_start;
    size_t line_end;
    size_t char_pos_end;
};

/*TODO: For linked list
 * - Add removing elements and keeping order
 * - Add inserting elements in between 2 elements
 */
// FIXME: names names names names names, theyre all ugly
// this could easily be made for arbitraryd ata types lmfao
struct dodo_linked_list
{
    struct hlInfo            hl;
    struct dodo_linked_list* next;
};

struct dodo_linked_list*
dodo_ll_new_node(size_t hl_line_start, size_t hl_char_pos_start,
                 size_t hl_line_end, size_t hl_char_pos_end)
{
    struct dodo_linked_list* a_ll = malloc(sizeof(struct dodo_linked_list));
    a_ll->hl.line_start           = hl_line_start;
    a_ll->hl.char_pos_start       = hl_char_pos_start;
    a_ll->hl.line_end             = hl_line_end;
    a_ll->hl.char_pos_end         = hl_char_pos_end;
    a_ll->next                    = nullptr;
    return a_ll;
}
struct dodo_linked_list*
dodo_ll_add_element(struct dodo_linked_list* start, size_t hl_line_start,
                    size_t hl_char_pos_start, size_t hl_line_end,
                    size_t hl_char_pos_end)
{
    if (start == nullptr) return nullptr;
    if (start->next != nullptr)
    {
        printf("Already pointing to another element\n");
        return nullptr;
    }

    struct dodo_linked_list* new = dodo_ll_new_node(
        hl_line_start, hl_char_pos_start, hl_line_end, hl_char_pos_end);

    if (start->next == nullptr)
    {
        start->next = new;
        return new;
    }
}

struct dodo_linked_list*
dodo_ll_add_from_root(struct dodo_linked_list* root, size_t hl_line_start,
                      size_t hl_char_pos_start, size_t hl_line_end,
                      size_t hl_char_pos_end)
{
    struct dodo_linked_list* a_ll = root;
    if (a_ll == nullptr) return nullptr;
    while (a_ll->next != nullptr)
    {
        a_ll = a_ll->next;
    }

    struct dodo_linked_list* new = dodo_ll_new_node(
        hl_line_start, hl_char_pos_start, hl_line_end, hl_char_pos_end);
    a_ll->next = new;
    return new;
}
int
dodo_ll_destroy(struct dodo_linked_list* root)
{
    // NOTE: why did i say recursive, all i needed was a loop
    struct dodo_linked_list* current_element = root;
    struct dodo_linked_list* next_element    = root->next;
    if (current_element == nullptr) return -1;
    while (next_element != nullptr)
    {
        free(current_element);
        current_element = next_element;
        next_element    = current_element->next;
    }
    free(current_element);
}

void
traverse_ll_and_dump(struct dodo_linked_list* root)
{
    if (root == nullptr)
    {
        printf("\nNull root.\n");
        return;
    }
    struct dodo_linked_list* a_element = root;
    while (a_element != nullptr)
    {
        printf("\n");
        printf("Line start: %lu\n", a_element->hl.line_start);
        printf("Cursor pos start: %lu\n", a_element->hl.char_pos_start);
        printf("Line end: %lu\n", a_element->hl.line_end);
        printf("Cursor post end: %lu\n", a_element->hl.char_pos_end);
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
simple_look_ahead(char c, FILE* f, fpos_t* pos)
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
main(int argc, char** argv)
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
    // printf("%*lu |  ", -3, line_no);
    /* It should have a lookahead concept but not for a word or a char
     * but for comment lines, also I think this should be done in 2 passes,
     * one stores where the todo starts and the second pass prints
     */
    // printf("%*lu |  ", -3, line_no);
    struct dodo_linked_list* root = nullptr;
    struct dodo_linked_list* last = nullptr;
    while ((x = (fgetc(test_file))) != EOF)
    {
        printf("%c", x);
        if (x == '#')
        {
            sl_comment = 1;
        }
        if ((sl_comment || ml_comment))
        {
            // Current node
            struct dodoTrieNode* c_node = dodo_trie_find_child(cool_trie, x);
            if (c_node != nullptr)
            {
                fgetpos(test_file, &pos);
                size_t hl_line       = line_no;
                size_t hl_cursor     = cursor_pos;
                size_t hl_line_end   = line_no;
                size_t hl_cursor_end = cursor_pos;
                while ((x = fgetc(test_file)) != EOF)
                {
                    c_node = dodo_trie_find_child(c_node, x);
                    if (c_node == nullptr)
                    {
                        // fsetpos(test_file, &pos);
                        break;
                    }
                    if (c_node->bottom == true)
                    {
                        if (root == nullptr)
                        {
                            root = dodo_ll_new_node(line_no, cursor_pos,
                                                    hl_line_end, hl_cursor_end);
                            last = root;
                            fsetpos(test_file, &pos);
                            break;
                        }
                        last = dodo_ll_add_element(last, line_no, cursor_pos,
                                                   hl_line_end, hl_cursor_end);

                        fsetpos(test_file, &pos);
                        break;
                    }

                    hl_cursor_end += 1;
                    if (x == '\n')
                    {
                        hl_line_end += 1;
                        hl_cursor_end = 0;
                    }
                }
            }
        }
        cursor_pos += 1;
        if (x == '\n')
        {
            line_no += 1;
            cursor_pos = 0;
            sl_comment = 0;
        }
        // if (x == '\n') printf("%*lu |  ", -3, line_no);
    }
    traverse_ll_and_dump(root);
    dodo_ll_destroy(root);
    fclose(test_file);

    dodo_trie_destroy(cool_trie);
    return 0;
}