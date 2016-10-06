// simple unicode library
// most of this is from Matt Joiner (thanks)
#ifndef _UNICODE_H_
#define _UNICODE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// returns the number of utf8 code points in the buffer at s
size_t utf8len(char *s);

// returns a pointer to the beginning of the pos'th utf8 codepoint
// in the buffer at s
char *utf8index(char *s, size_t pos);

// converts codepoint indexes start and end to byte offsets in the buffer at s
void utf8slice(char *s, ssize_t *start, ssize_t *end);

// appends the utf8 string at src to dest
char *utf8cat(char *dest, char *src);

#endif
