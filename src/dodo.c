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
#define SL_COMMENT 0b00000001
/// Multiline comment
#define ML_COMMENT 0b00000010
#define SKIP       0b00000100

/* Im having thinking thoughts, I dont know if I like using wchar, in reality
 * we have to ask ourselves the question:
 * "Am I supposed to care if a character is UTF or ASCII?"
 * Because in reality, if the trie can split things by 8 bits (chars)
 * then I dont really have to care about the content
 */

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
        // This is important because '\0' is gonna replace what we got at the
        // end of the destination string, and in this case
        // that is the destination string, so it needs to be initialized
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
     * UTF-8 is compatible with regular chars and thats because chars are 8 bits
     * but we never use the first bit,
     * so if a character is UTF-8 you just check the first bit
     * so the letter e is 01100101 in binary,
     * but this emoji 🧬 is 11110000 10011111 10100111 10101100
     * So when a function that operates on utf 8 sees the first bit
     * it knows if it has to treat it like a char (ASCII)
     * or a multibyte char (rune?)
     * shoutouts golang
     * "I can't believe it's not ASCII!"
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
    /*
     * So when I was trying to decide how much precision to add for the padding
     * I asked myself:
     * "How do I get the amount of 0's it takes to represent a number...?
     * Funny... I think I should know this, but I don't remember."
     * 3 seconds later... logarithm, but is it really worth it to use log
     * to get the precision so early on? Probably not but fun brain fart
     */
    // printf("%*lu |  ", -3, line_no);
    while ((x = (fgetc(test_file))) != EOF)
    {
        cursor_pos += 1;
        // TODO: Change this condition to be centered around comment bools
        //  and not chars
        if (x == '#')
        {
            sl_comment                  = 1;
            struct dodoTrieNode* a_node = cool_trie;
            while ((x = fgetc(test_file)) != EOF)
            {
                a_node = dodo_trie_find_child(a_node, x);
                if (a_node != nullptr && a_node->bottom)
                {
                    // STEP: So actually we want to later on (soon)
                    //  make it so when we find a letter we save that position
                    //  and then do what we are doing below here for bottom
                    //  save the position before fgetc
                    fsetpos(test_file, &pos);
                    x       = fgetc(test_file);
                    hl_mode = true;
                    break;
                }
                if (a_node == nullptr)
                {
                    a_node = cool_trie;
                }
                // this is useless
                if (sl_comment && x == '\n')
                {
                    sl_comment = 0;
                    break;
                }
                // STEP: If we do find the bottom, we have a word and we should
                //  print in an special mode where its colored until we end the
                //  string
            }
        }

        if (hl_mode)
        {
            printf("%c", x);
        }
        if (x == '\n')
        {
            hl_mode    = 0;
            sl_comment = 0;
        }
        // if (x == '\n')
        // {
        //     hl_mode    = 0;
        //     sl_comment = 0;
        //     line_no += 1;
        //     if (line_no == 100) break;
        //     // reset cursor
        //     cursor_pos = 0;
        //     printf("%*lu |  ", -3, line_no);
        // }
        fgetpos(test_file, &pos);
    }
    fclose(test_file);

    dodo_trie_destroy(cool_trie);
    return 0;
}