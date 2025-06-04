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
    uint8_t m = 0b0010'0000;
    // suppse c 0b1101'1010
    // supps ~c 0b0010'0101
    // So what we are doing is basically comparing that witht he mask, we are
    // searching for the 0 that indicates the amount of bytes the utf8 char has
    // we do it with a 1 inversed
    // 0b0010'0000
    // 0b0010'0101
    //-----------
    //=0b0010'0000

    uint8_t x = (~c) & m;
    // shift size IS the byte size, because
    uint8_t shift_size = 2;
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
        return false;
    }
    // start at 1 to skip first parameter
    for (int i = 1; i < argc; i++)
    {
        // we skip whatever is not a flag
        if (argv[i][0] != '-') continue;
        size_t arg_len = strlen(argv[i]);
        for (int j = 0; (size_t)j < arg_len; j++)
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
          size_t def_val,
          char*  error_str)
{
    char* digits = nullptr;
    if (argc == 1)
    {
        return def_val;
    }
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-') continue;
        size_t arg_len = strlen(argv[i]);
        // we already checked the 0 position before
        for (int j = 1; (size_t)j < arg_len; j++)
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
                // give us the value at next starting point
                digits = argv[i] + j + 1;
                // i love strtol since it discards everything we need discarded here
                return strtol(digits, nullptr, 10);
            }
        }
    }
    return def_val;
}

struct fileGroup
{
    size_t count;
    char** files;
};

struct fileGroup*
collect_files(int    argc,
              char** argv)
{
    // starting point of index files up till the end
    size_t idx_f = 0;
    // skip first file
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            idx_f = (size_t)i;
            break;
        }
    }
    if (idx_f == 0)
    {
        printf("No files provided.");
        return nullptr;
    }
    size_t            count     = (size_t)argc - idx_f;
    struct fileGroup* file_list = malloc(sizeof(struct fileGroup));
    file_list->count            = count;
    file_list->files            = malloc(sizeof(char*) * count);
    for (size_t j = 0; j < count; j++)
    {
        file_list->files[j] = argv[j + idx_f];
    }
    // print file list
    for (size_t k = 0; k < file_list->count; k++)
    {
        printf("%s\n", file_list->files[k]);
    }
    return file_list;
}

void
destroy_filegroup(struct fileGroup* fg)
{
    if (fg == nullptr)
    {
        return;
    }
    if (fg->files != nullptr)
    {
        free(fg->files);
    }
    free(fg);
}

/// @param line_pad: the amount of lines to add above and below the hl comment
void
gen_todo_from_filegroup(struct fileGroup*    fg,
                        struct dodoTrieNode* trie_root,
                        size_t               line_pad)
{
    struct commentKeys ck;
    char*              c_keys_temp[]  = {"//", "/*", "*/", nullptr};
    char*              py_keys_temp[] = {"#", "\"\"\"", "\"\"\"", nullptr};
    for (size_t i = 0; i < fg->count; i++)
    {
        // arbitrary file
        FILE* a_file = fopen(fg->files[i], "r");
        if (!a_file)
        {
            printf("Error [%i]: [%s] when opening file [%s]...\n", errno, strerror(errno),
                   fg->files[i]);
            break;
        }
        char* ext = strrchr(fg->files[i], '.');
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
            printf("Unsupported filetype, defaulting to c...");
            ck = new_key_group(ext, c_keys_temp);
        }

        // some file highlight
        struct dodoFileHighlights* a_file_hl = dodo_gen_todo_data(a_file, ck, trie_root);
        dodo_print_todos(a_file, fg->files[i], a_file_hl, line_pad);
        dodo_ll_destroy(a_file_hl->list);
        free(a_file_hl);
        fclose(a_file);
    }
}

int
main(int    argc,
     char** argv)
{
    /// flag for piping to less
    const bool        less         = flag_bool(argc, argv, 'l');
    const size_t      line_padding = flag_uint(argc, argv, 'n', 4, nullptr);
    struct fileGroup* myfiles      = collect_files(argc, argv);
    if (argc == 1)
    {
        printf("No file or options provided.\n");
        exit(EXIT_FAILURE);
    }

    /*
     * TODO: Implement recursively search for files in a folder
     * TODO: Implement argument for doc functions?
     * TODO: Support multiple todos in 1 comment block
     */

    // TODO: Config file and its integration
    // FIXME: Handling all files in a directory
    // FIXME: Argument handling

    /// Portable locale lolz
    setlocale(LC_ALL, "");

    struct dodoTrieNode* cool_trie = dodo_make_trie();
    dodo_trie_add_keyword(cool_trie, "TODO", GOLD);
    dodo_trie_add_keyword(cool_trie, "NOTE", NOTESGREEN);
    dodo_trie_add_keyword(cool_trie, "XXX", SCARYORANGE);
    dodo_trie_add_keyword(cool_trie, "BUG", REDRUM);
    dodo_trie_add_keyword(cool_trie, "THINK", THINKING);
    dodo_trie_add_keyword(cool_trie, "FIXME", REDRUM);
    dodo_trie_add_keyword(cool_trie, "REVIEW", SKY);
    dodo_trie_add_keyword(cool_trie, "WARNING", BEWAREOFDOGS);
    dodo_trie_add_keyword(cool_trie, "MORSEL", STEELBLUE);
    dodo_trie_add_keyword(cool_trie, "STEP", BOLDTERM SKY);
    dodo_trie_add_keyword(cool_trie, "REBUTTAL", SKY);
    dodo_trie_add_keyword(cool_trie, "BUG?", PINKISH);

    if (less)
    {
        // file descriptors are an abstraction for an I/O stream used by linux
        // each file has a fd, theres a fd table used by the kernel
        // the way we treat fd's depends on the context, for pipes its very straightforward
        // but they are an opaque way to deal with I/O stream with the kernel
        // FILE* is the same
        // file descriptors have calls like open, read, lseek associated with them.
        // more interesting is that processes get their own file descriptors,
        // stdin and stdout represent the read and the write end respectively
        // its canonical to know that stdin, stdout and stderr are fd that
        // every program gets by default and they point to the io of the console
        // you can extract info about a file descriptor with different functions like
        // isatty(), for example 0 will be a tty because thats the stdin of our terminal
        // when you split a process with fork(), you're giving each process a
        // stdin/stdout/stderr

        // 0 is the read and 1 is write end
        int pipe_fds[2];
        if (pipe(pipe_fds) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        // for forking
        // we need this because we are trying to pipe different processes
        // from our program to less
        pid_t pid;

        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            // The fd for stdout, changes so its the same as the
            // write end of the pipe aka
            // printing to stdout sends to the pipe
            // aka stdout and write end of the pipe are the same
            dup2(pipe_fds[1], STDOUT_FILENO);
            close(pipe_fds[0]);
            gen_todo_from_filegroup(myfiles, cool_trie, line_padding);
        }
        else
        {
            // the fd for stdin, becomes the same as the read end
            // of the pipe, so we can send to the pipe
            // and it will be treated as stdin
            dup2(pipe_fds[0], STDIN_FILENO);
            close(pipe_fds[1]);
            // execlp("less", "less", NULL);
        }
    }
    else
    {
        gen_todo_from_filegroup(myfiles, cool_trie, line_padding);
    }
    destroy_filegroup(myfiles);
    dodo_trie_destroy(cool_trie);
    return 0;
}
