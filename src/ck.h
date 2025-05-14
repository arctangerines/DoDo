#ifndef DODO_CK_H
#define DODO_CK_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
              char** keys);

#endif // DODO_CK_H
