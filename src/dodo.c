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

// FIXME:now we need to turn it into a function and start doing recursive
// searches

/*
 * ANSWER: It's better to have a bunch of info and trim it than less info or
 * partial
 */

// clamp size_t
size_t
clamp_lu(long x,
         long min,
         long max)
{
    if (x < min)
    {
        return (size_t)min;
    }
    if (x > max)
    {
        return max;
    }
    return (size_t)x;
}

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
                    char*  color)
{
    struct dodoList* a_ll         = malloc(sizeof(struct dodoList));
    a_ll->hl.line_start           = hl_line_start;
    a_ll->hl.line_pos_start       = hl_line_pos_start;
    a_ll->hl.line_end             = hl_line_end;
    a_ll->hl.line_pos_end         = hl_line_pos_end;
    a_ll->hl.hl_word_line_start   = hl_word_line_start;
    a_ll->hl.hl_word_line_end     = hl_word_line_end;
    a_ll->hl.hl_word_cursor_start = hl_word_cursor_start;
    a_ll->hl.hl_word_cursor_end   = hl_word_cursor_end;
    a_ll->hl.hl_color             = color;
    a_ll->next                    = nullptr;
    return a_ll;
}

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
                    char*            color)
{
    if (start == nullptr) return nullptr;
    if (start->next != nullptr)
    {
        printf("Already pointing to another element\n");
        return nullptr;
    }

    if (start->next == nullptr)
    {

        struct dodoList* new =
            dodo_ll_new_element(hl_line_start, hl_line_pos_start, hl_line_end,
                                hl_line_pos_end, hl_word_line_start, hl_word_line_end,
                                hl_word_cursor_start, hl_word_cursor_end, color);
        start->next = new;
        return new;
    }
    return nullptr;
}

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
                      char*            color)
{
    struct dodoList* a_ll = root;
    if (a_ll == nullptr) return nullptr;
    while (a_ll->next != nullptr)
    {
        a_ll = a_ll->next;
    }

    struct dodoList* new = dodo_ll_new_element(
        hl_line_start, hl_line_pos_start, hl_line_end, hl_line_pos_end, hl_word_line_start,
        hl_word_line_end, hl_word_cursor_start, hl_word_cursor_end, color);
    a_ll->next = new;
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
        printf("Cursor pos end: %lu\n", a_element->hl.line_pos_end);
        printf("Highlight word start: %lu\n", a_element->hl.hl_word_cursor_start);
        printf("Highlight word end: %lu\n", a_element->hl.hl_word_cursor_end);
        printf("\n");
        a_element = a_element->next;
    }
}

struct commentKeys
{
    char* ext;
    char* sl_key;
    char* ml_start_keys;
    char* ml_end_keys;
    bool  multiline;
    bool  multichar;
    // perhaps add an option of multikeys for multiline
};

struct commentKeys
new_key_group(char*  ext,
              char** keys)
{
    struct commentKeys ck = {
        .ext           = ext,
        .sl_key        = nullptr,
        .ml_start_keys = nullptr,
        .ml_end_keys   = nullptr,
        // if theres more than 1 char
        .multichar = true,
        .multiline = true,
    };
    size_t key_size = 0;
    while (keys[key_size] != nullptr)
    {
        if (strcmp(keys[key_size], "") == 0)
        {
            printf("Empty string as comment key...\n");
            exit(-1);
        }
        key_size++;
    }
    // so it's len, not index
    key_size++;
    if (key_size == 4)
    {
        ck.sl_key        = keys[0];
        ck.ml_start_keys = keys[1];
        ck.ml_end_keys   = keys[2];
    }
    else if (key_size == 2)
    {
        ck.sl_key = keys[0];
        if (strlen(keys[0]) == 1)
        {
            ck.multichar = false;
        }
        else
        {
            ck.multichar = true;
        }
        ck.multiline = false;
    }
    else
    {
        printf("Wrong amount of keys, given [%lu], expected [%u] or [%u]...\n", key_size,
               2, 4);
    }
    return ck;
}

/// @param c: char we are looking for
/// @param f: file we are working with
/// @param pos: pos_t var we are using to preserve our position
bool
simple_look_ahead(int     c,
                  FILE*   f,
                  fpos_t* pos)
{
    fgetpos(f, pos);
    // L for lookahead
    int l = fgetc(f);
    if (l == c)
    {
        // needs to be consumed, was causing issues duhhh
        fsetpos(f, pos);
        return true;
    }
    fsetpos(f, pos);
    return false;
}
/// If n < k_len then we will start at index len - n
/// Its like saying how many n last characters we want to read
/// Useful because we might have read
bool
n_look_ahead(size_t  n,
             char*   k,
             FILE*   f,
             fpos_t* pos)
{
    fgetpos(f, pos);
    // l for lookahead;
    int    l;
    size_t len = strlen(k);
    if (n > len)
    {
        printf("Trying to read more than available\n");
    }
    size_t i = 0;
    while ((l = fgetc(f)) != EOF)
    {
        if (l != k[i])
        {
            // if we dont find the char we are looking for, go back
            fsetpos(f, pos);
            return false;
        }
        i++;
        if (i == n)
        {
            break;
        }
    }
    fsetpos(f, pos);

    return true;
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
/*
 * TODO: Implement recursively search for files in a folder
 * TODO: Implement argument for doc functions?
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
    if (argc < 2)
    {
        printf("No file.\n");
        exit(-1);
    }
    size_t n_count = 0;
    if (argc == 3)
    {
        n_count = strtol(argv[2], nullptr, 10);
    }

    struct dodoTrieNode* cool_trie = dodo_make_trie();
    dodo_trie_add_keyword(cool_trie, "TODO", GOLD);
    dodo_trie_add_keyword(cool_trie, "FIXME", REDRUM);
    dodo_trie_add_keyword(cool_trie, "XXX", SCARYORANGE);
    dodo_trie_add_keyword(cool_trie, "BUG", REDRUM);
    dodo_trie_add_keyword(cool_trie, "THINK", THINKING);
    dodo_trie_add_keyword(cool_trie, "WARNING", BEWAREOFDOGS);
    dodo_trie_add_keyword(cool_trie, "MORSEL", STEELBLUE);
    dodo_trie_add_keyword(cool_trie, "NOTE", NOTESGREEN);
    dodo_trie_add_keyword(cool_trie, "STEP", BOLDTERM SKY);
    // dodo_trie_add_keyword(cool_trie, "🧬");

    FILE* test_file = fopen(argv[1], "r");
    if (!test_file)
    {
        printf("Error [%i]: [%s] when opening file...\n", errno, strerror(errno));
    }
    char*              ext = strrchr(argv[1], '.');
    struct commentKeys ck;
    char*              c_keys_temp[]  = {"//", "/*", "*/", nullptr};
    char*              py_keys_temp[] = {"#", "\"\"\"", "\"\"\"", nullptr};
    // printf("ext: %s\n", ext);
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0 || strcmp(ext, ".cxx") == 0)
    {
        ck = new_key_group(ext, c_keys_temp);
    }
    else if (strcmp(ext, ".py") == 0)
    {
        ck = new_key_group(ext, py_keys_temp);
    }
    else
    {
        printf("Unsupported filetype");
        exit(-1);
    }

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
    fpos_t pos;

    // Line number we are at
    size_t line_no         = 1;
    size_t absolute_lines  = 1;
    size_t absolute_cursor = 0;
    // Position of cursor
    size_t           cursor_pos = 0;
    struct dodoList* root       = nullptr;
    struct dodoList* last       = nullptr;

    size_t hl_line_start        = line_no;
    size_t hl_cursor_start      = cursor_pos;
    size_t hl_line_end          = line_no;
    size_t hl_cursor_end        = cursor_pos;
    // based on cursor
    size_t hl_word_cursor_start = 0;
    size_t hl_word_cursor_end   = 0;
    size_t hl_word_line_start   = 0;
    size_t hl_word_line_end     = 0;
    bool   keyword_found        = false;
    char*  keyword_color        = nullptr;

    // FIXME: annotate
    while ((x = (fgetc(test_file))) != EOF)
    {
        // FIXME: these conditions can be made more readable by using struts
        if (x == ck.sl_key[0] && !ml_comment && !sl_comment)
        {
            size_t key_len = strlen(ck.sl_key);
            // The idea here is to add offsets of +- 1 since we already consumed
            // a char
            sl_comment = (key_len > 1)
                             ? n_look_ahead(key_len - 1, ck.sl_key + 1, test_file, &pos)
                             : true;
            if (sl_comment)
            {
                hl_line_start   = line_no;
                hl_cursor_start = cursor_pos;
            }
        }
        if (ck.multiline && x == ck.ml_start_keys[0] && !sl_comment && !ml_comment)
        {
            size_t key_len = strlen(ck.ml_start_keys);
            ml_comment = (key_len > 1) ? n_look_ahead(key_len - 1, ck.ml_start_keys + 1,
                                                      test_file, &pos)
                                       : true;
            if (ml_comment)
            {
                // offset by how much we moved
                // cursor_pos += key_len - 1;
                hl_line_start   = line_no;
                hl_cursor_start = cursor_pos;
                // only needed for multiline because it might have to look for
                // the same characters so we dont want it to enable it and
                // disable it instantly since x didnt move
                // it's like nudging a little
                // FIXME: Candidate for reimplementation
                x = fgetc(test_file);
                // no idea why this would happen
                if (x == '\n')
                {
                    line_no++;
                    cursor_pos = 0;
                }
                cursor_pos++;
            }
        }
        if ((sl_comment || ml_comment) && !keyword_found)
        {
            struct dodoTrieNode* a_node = dodo_trie_find_child(cool_trie, x);
            // if (a_node != nullptr && !keyword_found)
            if (a_node != nullptr)
            {
                fgetpos(test_file, &pos);
                hl_word_cursor_start = cursor_pos;
                hl_word_cursor_end   = cursor_pos;
                hl_word_line_start   = line_no;
                hl_word_line_end     = line_no;
                // cursor_pos++;
                // It was causing a bug where if the last cahracter was in the
                // trie then it would search the next would find a newline
                // so it would break but \n was saved on x so it would then
                // then execute our newline path so now below
                int m;
                while ((m = fgetc(test_file)) != EOF)
                {
                    a_node = dodo_trie_find_child(a_node, m);
                    hl_word_cursor_end++;
                    if (a_node == nullptr)
                    {
                        // So we can start searching again starting at next
                        // letter cuz there will be another x = fgetc(...) at
                        // the end of while loop
                        fsetpos(test_file, &pos);
                        break;
                    }
                    // FIXME: implement another level of lookahead
                    // cuz as it is, longer words are not working properly
                    if (a_node->word)
                    {
                        // STEP: continue if lookahead finds another word
                        keyword_found = true;
                        fsetpos(test_file, &pos);
                        keyword_color = a_node->color;
                        break;
                    }
                }
            }
        }

        if (ml_comment && x == ck.ml_end_keys[0])
        {
            size_t key_len = strlen(ck.ml_end_keys);
            // inverse because if we dont find the char ahead, we want to keep
            // it on but if we do find it we want to turn off this
            ml_comment =
                (key_len > 1)
                    ? !(n_look_ahead(key_len - 1, ck.ml_end_keys + 1, test_file, &pos))
                    : false;
            if (!ml_comment)
            {
                hl_line_end = line_no;
                // we looked ahead and it ends on key_len - 1 chars
                // aka if its 2, it ends on the next char
                hl_cursor_end = cursor_pos + (key_len - 1);
            }
        }
        if (x == '\n')
        {
            if (sl_comment)
            {
                sl_comment    = false;
                hl_line_end   = line_no;
                hl_cursor_end = cursor_pos - 1;
            }
            line_no++;
            absolute_lines++;
            cursor_pos = 0;
        }
        else
        {
            cursor_pos++;
            absolute_cursor = cursor_pos;
        }

        if (keyword_found && !(sl_comment || ml_comment))
        {
            last          = (last == nullptr)
                                ? (root = dodo_ll_new_element(
                              hl_line_start, hl_cursor_start, hl_line_end, hl_cursor_end,
                              hl_word_line_start, hl_word_line_end, hl_word_cursor_start,
                              hl_word_cursor_end, keyword_color))
                                : dodo_ll_add_element(
                             last, hl_line_start, hl_cursor_start, hl_line_end,
                             hl_cursor_end, hl_word_line_start, hl_word_line_end,
                             hl_word_cursor_start, hl_word_cursor_end, keyword_color);
            keyword_found = false;
        }
    }
    // it gets added to 1 last time
    absolute_lines -= 1;
    struct dodoList* a_list = root;
    if (a_list == nullptr)
    {
        printf("No ToDo's here...!\n");
    }
    else
    {
        dump_ll(root);
        // exit(0);
        // reset file position
        rewind(test_file);
        cursor_pos = -1;
        line_no    = 1;
        // we are printing chars
        bool printing = false;
        bool coloring = false;
        // skip whitespace
        bool   skip_ws   = false;
        int    prev_char = 0;
        fpos_t pos1;
        fpos_t pos2;
        size_t print_line_end = 0;
        size_t reset_line     = 0;
        size_t reset_pos      = 0;
        size_t next_cursor    = 0;
        bool   reset_bool     = false;
        // this works with both a value of 0 and an n value
        while (1)
        {
            /*
             * I didn't want the control of my function to depend on the while loop
             * I wanted to handle EOF myself but at the end i didn't need it
             */
            x = fgetc(test_file);
            cursor_pos++;
            if (x == EOF)
            {
                printf("eof");
                break;
            }
            /*
             * The rationale for this is that I asked myself how would a method that
             * parses the file line by line would update the line, would it get \n
             * operate on it and then update the line number, or get \n and update line
             * My answer was the latter, because we want atomicity(at least the idea of
             * it) So in the spirit of atomicity, we gotta update the line as soon as
             * we get it This goes in hand with how fgetc works, fgetc when you save
             * the position in the char that\n, it will get you the character in the
             * NEXT line so we restore at this \n simplified, if we are on line 78 at
             * the last char \n and we want to print at line 79, we save on said line
             * 79 @ last char \n, because when we call fgetc again and we print it, it
             * will give us the first char of 79 this is the logic of [A]
             */
            if (x == '\n')
            {
                /*
                 * So if we are on line 78 and we get \n, we behave like we are already
                 * at line 79 [A]
                 */
                line_no++;
                // unsigned integer overflow is defined behaviour :)
                // it has to be -1 because we are setting it literally -1 of the start
                cursor_pos = -1;
            }
            if (!printing)
            {
                // [A] our next char is the next line since we on \n,
                // we save our position and consider it saved so we can rewind to it
                // later reset_bool is so we dont save it every time we in this line,
                // the reset position is always the start of the "previous" highlight
                if (line_no == clamp_lu((long)a_list->hl.line_start - (long)n_count, 1,
                                        (long)a_list->hl.line_start))
                {
                    // so fgetpos doesnt get called a billion times
                    // it could perhaps be a '\n'(?)
                    // if (!reset_bool)
                    if (!reset_bool)
                    {
                        fgetpos(test_file, &pos2);
                        reset_bool = true;
                        // we save the line info
                        reset_line = line_no;
                    }
                    printing = true;
                    printf(FILEPATH "%s:%lu:%lu\n" CRESET, argv[1],
                           a_list->hl.hl_word_line_start,
                           a_list->hl.hl_word_cursor_start + 1);
                }
            }
            /*
             * [B] This is the other key point that makes this work
             * Say we finish printing at 80, but we dont want to stop printing
             * right when we get to 80 (which would be at \n) of 79
             * So following the same logic, if are at line 81 but it's '\n', that
             * means that we are right at the beginning of it (which is 80 at \n)
             * (maybe we can rewrite our parser to have similar behaviour
             */
            if (line_no - 1 == clamp_lu((long)a_list->hl.line_end + (long)n_count,
                                        (long)a_list->hl.line_end, (long)absolute_lines) &&
                x == '\n')
            {
                printing = false;
                // we save our next element
                a_list = a_list->next;
                if (a_list == nullptr)
                {
                    // This works actually because all POSIX files end with
                    // a newline Soooooo, we basically can print with no
                    // worries, in theory we cant have more elements than
                    // there are
                    // TODOs, so if a ToDo that is an element of our list
                    // and starts any line aftyer the last line, cannot
                    // exist hence why POSIX files ending in a newline works
                    // we will each newline before we reach actual EOF
                    // and by the time we reach the last \n, this will be
                    // set to null and the function will break
                    // this printf is to simulate we actually printed the
                    // last newline
                    printf("\n");
                    break;
                }
                fsetpos(test_file, &pos2);
                reset_bool = false;
                line_no    = reset_line;
                cursor_pos = -1;
                // To add some breathing room for the messages
                printf("\n\n");
            }
            if (printing)
            {
                if (line_no == a_list->hl.hl_word_line_start &&
                    cursor_pos == a_list->hl.hl_word_cursor_start)
                {
                    coloring = true;
                }
                if (coloring)
                {
                    printf(a_list->hl.hl_color);
                }
                // printf("%c", x);
                // FIXME: Might delete this feature or make it a flag----------------------
                if ((prev_char == ' ' || prev_char == '\t') &&
                    (x == ' ' || prev_char == '\t'))
                {
                    skip_ws = true;
                }
                if (!skip_ws)
                {
                    printf("%c", x);
                }
                if (skip_ws && !(x == ' ' || x == '\t'))
                {
                    skip_ws = false;
                    printf("%c", x);
                }
                //-------------------------------------------------------------------------

                if (line_no == a_list->hl.hl_word_line_end &&
                    cursor_pos == a_list->hl.hl_word_cursor_end)
                {
                    printf(CRESET);
                    coloring = false;
                }
                if (x == '\n' && line_no <= absolute_lines)
                {
                    printf("%*lu|  ", -3, line_no);
                    // add by option
                    // printf("%*lu/%*lu|  ", -3, line_no, 3,
                    // absolute_lines);
                }
            }
            prev_char = x;
        }
        // dump_ll(root);
        dodo_ll_destroy(root);
    }
    fclose(test_file);
    dodo_trie_destroy(cool_trie);
    return 0;
}