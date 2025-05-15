#include "term_colors.h"
#include <ck.h>
#include <list.h>
#include <parsing_io.h>
#include <trie.h>

#include <errno.h>
#include <locale.h>
#include <silky.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>
#include <wordexp.h>

// FIXME:now we need to turn it into a function and start doing recursive
// searches

/*
 * ANSWER: It's better to have a bunch of info and trim it than less info or
 * partial
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

bool
flag_bool(int    argc,
          char** argv,
          char   c)
{
    if (argc == 1)
    {
        printf("No arguments provided");
        return false;
    }
    // start at 1 to skip first parameter
    for (int i = 1; i < argc; i++)
    {
        // we skip whatever is not a flag
        if (argv[i][0] != '-') continue;
        size_t arg_len = strlen(argv[i]);
        for (int j = 0; j < arg_len; j++)
        {
            if (argv[i][j] == c) return true;
        }
    }
    return false;
}

size_t
flag_uint(int    argc,
          char** argv,
          char   fl,
          char*  error_str)
{
    char* digits = nullptr;
    bool  found  = false;
    if (argc == 1)
    {
        printf("No arguments provided");
        return 0;
    }
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-') continue;
        size_t arg_len = strlen(argv[i]);
        // we already checked the 0 position before
        for (int j = 1; j < arg_len; j++)
        {
            // probably dont need the for loop above, if its not at the end
            // or alone, it doesnt work
            if (argv[i][1] == fl)
            {
                // start where we at at count what comes after
                if (strlen(argv[i] + j) < 2)
                {
                    // gave us nothing after the letter, so it does nothing
                    return 0;
                }
                found = true;
                // give us the value at next starting point
                digits = argv[i] + j + 1;
                printf("Digits: [%s]\n", digits);
                // i love strtol since it discards everything we need discarded here
                return strtol(digits, nullptr, 10);
                break;
            }
        }
    }
    return 0;
}

char**
collect_files();

// TODO: Accept piping
int
main(int    argc,
     char** argv)
{
    // TODO: Config file and its integration
    // FIXME: Handling all files in a directory
    // FIXME: Argument handling

    /// Portable locale lolz
    setlocale(LC_ALL, "");

    if (argc < 2)
    {
        printf("No file.\n");
        exit(-1);
    }
    size_t extra_lines_arg = 4;
    bool   less            = false;
    // if we got 4 arguments check which one has the --less flag
    if (argc == 4)
    {
        if (strcmp(argv[2], "--less") == 0 || strcmp(argv[2], "-l") == 0)
        {
            less = true;
            // the next argument is the line number
            extra_lines_arg = strtol(argv[3], nullptr, 10);
        }
        else if (strcmp(argv[3], "--less") == 0 || strcmp(argv[3], "-l") == 0)
        {
            less = true;
            // the previous argument is the line number
            extra_lines_arg = strtol(argv[2], nullptr, 10);
        }
    }
    if (argc == 3)
    {
        if (strcmp(argv[2], "--less") == 0 || strcmp(argv[2], "-l") == 0)
        {
            less = true;
        }
        else
        {
            // the next argument is the line number
            extra_lines_arg = strtol(argv[3], nullptr, 10);
        }
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

    struct dodoFileHighlights* file_hl = dodo_gen_todo_data(test_file, ck, cool_trie);

    // If we pipe to less

    if (less)
    {
        // 0 is read and 1 is write end
        int pipe_fds[2];
        if (pipe(pipe_fds) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        // for forking
        // we need this because we are trying to pipe different processes
        // from our program to less
        // so the scond process will assume the role of less and first one will print
        pid_t pid;

        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            // STDOUT now refers to the write end
            dup2(pipe_fds[1], STDOUT_FILENO);
            close(pipe_fds[0]);
            dodo_print_todos(test_file, argv[1], file_hl, extra_lines_arg);
        }
        else
        {
            // just like a pipe, stdin is the read end of the pipe
            // less is going to read from stdin
            dup2(pipe_fds[0], STDIN_FILENO);
            close(pipe_fds[1]);
            execlp("less", "less", NULL);
        }
    }
    else
    {

        dodo_print_todos(test_file, argv[1], file_hl, extra_lines_arg);
    }
    dodo_ll_destroy(file_hl->list);
    free(file_hl);

    fclose(test_file);
    dodo_trie_destroy(cool_trie);
    return 0;
}