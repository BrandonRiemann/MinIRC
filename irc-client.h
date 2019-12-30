#ifndef IRC_CLIENT_H__
#define IRC_CLIENT_H__

#include "irc-types.h"
#include "hash-table.h"
#include <pthread.h>

#ifdef IRC_SHOWOUTPUT
    #define IRC_PRINT( x )  printf x
#else
    #define IRC_PRINT( x )  do { } while( 0 )
#endif

struct IrcContext;

typedef int ( IrcCallback )( struct IrcContext*, char**, char* );

typedef struct {
    unsigned char *param;
    int type;
    IrcCallback *callback;
} IrcEvent;

typedef struct IrcContext_ {
    int socket;
    pthread_mutex_t state;
    pthread_t thread;
    unsigned int num_channels;
    unsigned int num_events;
    char *host;
    char *port;
    char **channels;
    char *username;
    char *password;
    IrcEvent *events;
    HashTable *events_hash;
} IrcContext;

void irc_init_context( IrcContext* );
void irc_destroy_context( IrcContext* );
void irc_auth( IrcContext*, IrcCallback*, const char*, const char* );
void irc_join( IrcContext*, IrcCallback*, const char* );
void irc_connect( IrcContext*, IrcCallback*, const char*, const char* );
void *irc_read_loop( void* );
void irc_wait( IrcContext* );
void irc_signal( IrcContext* );
int irc_send_raw( IrcContext*, const char* );
int irc_say( IrcContext*, const char*, const unsigned int );
void irc_hook_reply( IrcContext*, IrcCallback*, const char* );
void irc_unhook_reply( IrcContext*, IrcCallback*, const char* );
void irc_hook_command( IrcContext*, IrcCallback*, const char* );
void irc_unhook_command( IrcContext*, IrcCallback*, const char* );

#endif
