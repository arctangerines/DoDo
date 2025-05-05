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

#define COMMENT 0b00000001
#define SKIP    0b00000010

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
    FILE* config_file = fopen("./testfig/dodofile", "a+");

    /// Portable locale lolz
    setlocale(LC_ALL, "");

    // TODO: Config file and its integration

    bool only_comments;
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
    // TODO: Change this to use the bit flags and the var name to be 'flags'
    uint8_t comment = 0;
    fpos_t  pos;
    /// This variable is going to tell us how many letters to skip
    size_t skip = 0;
    while ((x = (fgetc(test_file))) != EOF)
    {
        /* The way we implement this should be that when we
         * finally find a letter we are looking for we save the position, so we
         * can check.
         * Perhaps this should be filetype compatible thonk
         * so if its py it knows what comments to use
         */
        if (x == '#')
        {
            comment = 1;
        }
        // STEP: We check if a char is the first char of a multiline comment,
        //  if it is a multiline we lookahead, then if it is multiline,
        //  we set the flag
        if (x == '/')
        {
            fgetpos(test_file, &pos);
            x = fgetc(test_file);
            if (x == '*')
            {
                comment = 1;
            }
            fsetpos(test_file, &pos);
        }
        // STEP: We can repeat this with every char in the comment until
        //  multiline is over but doing it the other way
        if (!comment && !only_comments)
        {
            printf("%c", x);
        }
        if (comment && skip == 0)
        {
            if (x == '*')
            {
                fgetpos(test_file, &pos);
                x = fgetc(test_file);
                if (x == '/')
                {
                    comment = 0;
                }
                fsetpos(test_file, &pos);
            }
            struct dodoTrieNode* starting_point =
                dodo_trie_find_child(cool_trie, x);
            if (starting_point != nullptr)
            {
                while ((x = fgetc(test_file)) != EOF)
                {
                    starting_point = dodo_trie_find_child(starting_point, x);
                    if (starting_point == nullptr)
                    {
                        break;
                    }
                    if (starting_point->bottom == true)
                    {
                        // Go back to beginning of keyword
                        fsetpos(test_file, &pos);
                        skip = starting_point->depth;
                        break;
                    }
                    // if (starting_point == nullptr&&)
                }
            }
            else
                printf("%c", x);
            fgetpos(test_file, &pos);
        }
        else if (comment && skip != 0)
        {
            printf(MAGHB "%c" COLOR_RESET, x);
            skip -= 1;
        }

        if (comment && x == '\n') comment = 0;
    }
    fclose(test_file);

    dodo_trie_destroy(cool_trie);
    return 0;
}