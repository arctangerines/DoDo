#include "utility.h"

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
