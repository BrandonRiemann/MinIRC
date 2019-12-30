#include "memory-utils.h"
#include <stdlib.h>

void free_ptr( void *ptr ) {
    if ( ptr )
        free( ptr );

    ptr = NULL;
}
