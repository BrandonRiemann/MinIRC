#ifndef HASH_TABLE__
#define HASH_TABLE__

#include <unistd.h>

typedef unsigned long ( HashFunc )( unsigned char* );

typedef struct HashEntry_ {
    struct HashEntry_ *next;
    unsigned char *key;
    void *data;
} HashEntry;

typedef struct {
    HashEntry **entries;
    int num_entries;
} HashTable;

int hash_table_init( HashTable*, size_t );
int hash_table_resize( HashTable*, size_t );
void hash_table_destroy( HashTable* );
int hash_table_insert( HashTable*, unsigned char*, void*, HashFunc );
void *hash_table_lookup( HashTable*, unsigned char*, HashFunc );
void hash_table_remove( HashTable* );
unsigned long HASH_DJB2( unsigned char* );

#endif
