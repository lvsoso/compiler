#include "rvcc.h"

// formated string
char *format(char *Fmt, ...) {
    char *Buf;
    size_t BufLen;
    // create stream from string memory
    FILE *Out = open_memstream(&Buf, &BufLen);

    va_list VA;
    va_start(VA, Fmt);

    // write data
    vfprintf(Out, Fmt, VA);
    va_end(VA);

    fclose(Out);
    return Buf;
}