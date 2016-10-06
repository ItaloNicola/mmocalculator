// simple unicode library
// most of this is from Matt Joiner (thanks)
#include "unicode.h"

// returns the number of utf8 code points in the buffer at s
size_t utf8len(char *s)
{
    size_t len = 0;
    for (; *s; ++s) if ((*s & 0xC0) != 0x80) ++len;
    return len;
}

// returns a pointer to the beginning of the pos'th utf8 codepoint
// in the buffer at s
char *utf8index(char *s, size_t pos)
{    
    ++pos;
    for (; *s; ++s) {
        if ((*s & 0xC0) != 0x80) --pos;
        if (pos == 0) return s;
    }
    return NULL;
}

// converts codepoint indexes start and end to byte offsets in the buffer at s
void utf8slice(char *s, ssize_t *start, ssize_t *end)
{
    char *p = utf8index(s, *start);
    *start = p ? p - s : -1;
    p = utf8index(s, *end);
    *end = p ? p - s : -1;
}

// appends the utf8 string at src to dest
char *utf8cat(char *dest, char *src)
{
    return strcat(dest, src);
}
