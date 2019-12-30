#include "input-utils.h"
#include <stdio.h>
#include <stdlib.h>

/*
    getline_stdin

    Gets a line (delimited by \n) from stdin such that it does not require any
    predefined buffer size. It is expected that the memory allocated is freed
    somewhere outside of this function. If NULL is supplied as a parameter,
    it is expected that the return value is given to a char pointer elsewhere.
*/

char *getline_stdin( char *buffer ) {
    char ch;
    int i = 0;

    do {
        ch = fgetc( stdin );
        buffer = realloc( buffer, sizeof *buffer * ++i );

        if ( !buffer )
            return NULL;

        buffer[ i - 1 ] = ch;
    } while ( ch != '\n' );

    buffer[ i - 1 ] = '\0';

    return buffer;
}
