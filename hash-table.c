#include "hash-table.h"
#include "memory-utils.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int hash_table_init( HashTable *table, size_t size ) {
    table->entries = malloc( sizeof *table->entries * size );

    if ( !table->entries )
        return 0;

    int i;
    for ( i = 0; i < size; ++i ) {
        table->entries[ i ] = malloc( sizeof *table->entries[ i ] );
        table->entries[ i ]->next = NULL;
        table->entries[ i ]->data = NULL;
    }

    table->num_entries = size;

    return 1;
}

int hash_table_resize( HashTable *table, size_t size ) {
    table->entries = realloc( table->entries, sizeof *table->entries * size );

    if ( !table->entries )
        return 0;

    table->num_entries = size;

    return 1;
}

void hash_table_destroy( HashTable *table ) {
    int i;

    for ( i = 0; i < table->num_entries; ++i ) {
        HashEntry *entry = table->entries[ i ];
        while( entry ) {
            entry = entry->next->next;

            if ( entry->next )
                free_ptr( entry->next );
        }
    }

    free_ptr( table->entries );

    table->num_entries = 0;
}

int hash_table_insert( HashTable *table,
                       unsigned char *key,
                       void *data,
                       HashFunc hash_func ) {
    const int index = hash_func( key ) % table->num_entries;
    HashEntry *entry = table->entries[ index ];

    if ( entry->key && entry->data ) {
        while ( entry->next ) {
            entry = entry->next;
        }

        entry->next = malloc( sizeof *entry->next );

        if ( !entry->next )
            return 0;

        entry->next->next = NULL;
        entry->next->data = data;
        entry->next->key = key;
    } else {
        entry->next = NULL;
        entry->data = data;
        entry->key = key;
    }

    return 1;
}

void *hash_table_lookup( HashTable *table,
                         unsigned char *key,
                         HashFunc hash_func ) {
    const unsigned long index = hash_func( key ) % table->num_entries;
    static HashEntry *next_entry = NULL;
    static int count = 0;

    if ( count == 0 )
        next_entry = table->entries[ index ];
    else if ( next_entry || next_entry->key )
        next_entry = next_entry->next;

    ++count;

    if ( next_entry && next_entry->data ) {
        if ( !strcmp( next_entry->key, key ) ) {
            return next_entry->data;
        }
    }

    next_entry = NULL;
    count = 0;
    return NULL;
}

void hash_table_remove( HashTable *table ) {

}

unsigned long HASH_DJB2( unsigned char *str ) {
    unsigned long hash = 5381;
    int c;

    while ( ( c = *str++ ) ) {
        hash = ( ( hash << 5 ) + hash ) + c;
    }

    return hash;
}
