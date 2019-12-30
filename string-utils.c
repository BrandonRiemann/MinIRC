#include "string-utils.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

char *format_string( int size, const char *str, ... ) {
    char *buffer = malloc( sizeof *buffer * size );

    va_list args;
    va_start( args, str );
    vsnprintf( buffer, size, str, args );
    va_end( args );

    return buffer;
}
