#include <ck.h>

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
