#include "parsing_io.h"
#include <ck.h>
#include <term_colors.h>
#include <trie.h>
#include <utility.h>

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

// FIXME: Error handling and cleanup
/// This functions takes in a trie with the keywords and generates and return
/// a linked list with the info related to the lines and cursor position of the highlights
struct dodoFileHighlights*
dodo_gen_todo_data(FILE*                f,
                   struct commentKeys   ck,
                   struct dodoTrieNode* trie_keywords)
{
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
    // based on cursor
    size_t hl_word_cursor_start = 0;
    size_t hl_word_cursor_end   = 0;
    size_t hl_word_line_start   = 0;
    size_t hl_word_line_end     = 0;
    bool   keyword_found        = false;
    char*  keyword_color        = nullptr;
    // Line number we are at
    size_t line_no = 1;
    // Position of cursor
    size_t           cursor_pos      = 0;
    size_t           absolute_lines  = 1;
    size_t           hl_line_start   = line_no;
    size_t           hl_cursor_start = cursor_pos;
    size_t           hl_line_end     = line_no;
    size_t           hl_cursor_end   = cursor_pos;
    struct dodoList* root            = nullptr;
    struct dodoList* last            = nullptr;

    /// Single line comment
    bool sl_comment = false;
    /// Multiline comment
    bool   ml_comment = false;
    fpos_t pos;
    // FIXME: annotate
    while ((x = (fgetc(f))) != EOF)
    {
        // FIXME: these conditions can be made more readable by using struts
        if (x == ck.sl_key[0] && !ml_comment && !sl_comment)
        {
            size_t key_len = strlen(ck.sl_key);
            // The idea here is to add offsets of +- 1 since we already consumed
            // a char
            sl_comment =
                (key_len > 1) ? n_look_ahead(key_len - 1, ck.sl_key + 1, f, &pos) : true;
            if (sl_comment)
            {
                hl_line_start   = line_no;
                hl_cursor_start = cursor_pos;
            }
        }
        if (ck.multiline && x == ck.ml_start_keys[0] && !sl_comment && !ml_comment)
        {
            size_t key_len = strlen(ck.ml_start_keys);
            ml_comment     = (key_len > 1)
                                 ? n_look_ahead(key_len - 1, ck.ml_start_keys + 1, f, &pos)
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
                x = fgetc(f);
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
            struct dodoTrieNode* a_node = dodo_trie_find_child(trie_keywords, x);
            // if (a_node != nullptr && !keyword_found)
            if (a_node != nullptr)
            {
                fgetpos(f, &pos);
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
                while ((m = fgetc(f)) != EOF)
                {
                    a_node = dodo_trie_find_child(a_node, m);
                    hl_word_cursor_end++;
                    if (a_node == nullptr)
                    {
                        // So we can start searching again starting at next
                        // letter cuz there will be another x = fgetc(...) at
                        // the end of while loop
                        fsetpos(f, &pos);
                        break;
                    }
                    // FIXME: implement another level of lookahead
                    // cuz as it is, longer words are not working properly
                    if (a_node->word)
                    {
                        // STEP: continue if lookahead finds another word
                        keyword_found = true;
                        fsetpos(f, &pos);
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
            ml_comment = (key_len > 1)
                             ? !(n_look_ahead(key_len - 1, ck.ml_end_keys + 1, f, &pos))
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
    struct dodoFileHighlights* file_hl = malloc(sizeof(struct dodoFileHighlights));
    file_hl->list                      = root;
    file_hl->lines                     = absolute_lines;
    return file_hl;
}

/// This function takes in a wrapper that contains the linked list with highlight data
/// and other file info and prints to terminal
void
dodo_print_todos(FILE*                      f,
                 const char*                filename,
                 struct dodoFileHighlights* file_hl,
                 size_t                     extra_lines)
{
    if (file_hl == nullptr)
    {
        printf("No ToDo's here\n");
        return;
    }
    if (file_hl->lines == 0)
    {
        printf("No lines read.\n");
        return;
    }
    if (file_hl->list == nullptr)
    {
        printf("Empty list of elements, hence, no ToDo's\n");
        return;
    }
    int              x;
    struct dodoList* a_list         = file_hl->list;
    size_t           absolute_lines = file_hl->lines - 1;
    // dump_ll(root);
    // exit(0);
    // reset file position
    rewind(f);
    size_t cursor_pos = -1;
    size_t line_no    = 1;
    // we are printing chars
    bool printing = false;
    bool coloring = false;

    // skip whitespace
    bool   skip_ws   = false;
    int    prev_char = 0;
    fpos_t pos2;
    size_t reset_line = 0;
    bool   reset_bool = false;
    // this works with both a value of 0 and an n value
    while (1)
    {
        /*
         * I didn't want the control of my function to depend on the while loop
         * I wanted to handle EOF myself but at the end i didn't need it
         */
        x = fgetc(f);
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
            if (line_no == clamp_lu((long)a_list->hl.line_start - (long)extra_lines, 1,
                                    (long)a_list->hl.line_start))
            {
                // so fgetpos doesnt get called a billion times
                // it could perhaps be a '\n'(?)
                // if (!reset_bool)
                if (!reset_bool)
                {
                    fgetpos(f, &pos2);
                    reset_bool = true;
                    // we save the line info
                    reset_line = line_no;
                }
                printing = true;
                printf(FILEPATH "%s:%lu:%lu\n" CRESET, filename,
                       a_list->hl.hl_word_line_start, a_list->hl.hl_word_cursor_start + 1);
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
        if (line_no - 1 == clamp_lu((long)a_list->hl.line_end + (long)extra_lines,
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
            fsetpos(f, &pos2);
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
            if ((prev_char == ' ' || prev_char == '\t') && (x == ' ' || prev_char == '\t'))
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
}
