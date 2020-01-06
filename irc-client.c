#include "irc-client.h"
#include "input-utils.h"
#include "string-utils.h"
#include "memory-utils.h"
#include "hash-table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <curses.h>

void irc_init_context( IrcContext *irc ) {
    pthread_mutex_init( &irc->state, NULL );
    irc->socket = 0;
    irc->num_channels = 0;
    irc->num_events = 0;
    irc->channels = NULL;
    irc->host = NULL;
    irc->password = NULL;
    irc->port = NULL;
    irc->username = NULL;
    irc->events = NULL;
    irc->events_hash = malloc( sizeof *irc->events_hash );
    irc->events = malloc( sizeof *irc->events * IRC_MAX_EVENTS );

    hash_table_init( irc->events_hash, IRC_MAX_EVENTS );

    int i;
    for ( i = 0; i < IRC_MAX_EVENTS; ++i )
        irc->events->param = irc->events->callback = NULL;
}

void irc_destroy_context( IrcContext *irc ) {
    int i;

    pthread_mutex_destroy( &irc->state );

    for ( i = 0; i < ( int ) irc->num_channels; ++i ) {
        free_ptr( irc->channels[ i ] );
    }

    for ( i = 0; i < IRC_MAX_EVENTS; ++i ) {
        free_ptr( irc->events[ i ].param );
    }

    free_ptr( irc->channels );
    free_ptr( irc->events );
    free_ptr( irc->host );
    free_ptr( irc->password );
    free_ptr( irc->port );
    free_ptr( irc->username );

    hash_table_destroy( irc->events_hash );
}

static int irc_welcome_callback( IrcContext *irc, char **argv, char *line ) {
    irc_signal( irc );

    IRC_PRINT( line );

    return IRC_OK;
}

static int irc_join_callback( IrcContext *irc, char **argv, char *line ) {
    if ( argv[ 0 ] && !strcmp( argv[ 0 ], irc->username ) ) {
        irc_signal( irc );
    }

    // TODO: make volatile -- unhook this event callback at this point

    IRC_PRINT( line );

    return IRC_OK;
}

static int irc_pong( IrcContext *irc, char **argv, char *line ) {
    if ( line ) {
        char *server = NULL;
        char line_orig[ 513 ];
        strncpy( line_orig, line, 512 );
        line_orig[ 512 ] = '\0';
        int i;

        for ( i = 0; i < strlen( line_orig ); ++i ) {
            if ( line_orig[ i ] == ' ' || line_orig[ i ] == ':' ) {
                server = line_orig + i + 1;
                break;
            }
        }

        if ( server ) {
            char *raw = format_string( strlen( "PONG " ) +
                                       strlen( server ) + 3,
                                       "PONG %s\r\n",
                                       server );

            irc_send_raw( irc, raw );
            free_ptr( raw );
        }
    }

    return IRC_OK;
}

void irc_auth( IrcContext *irc, IrcCallback* ready,
               const char *username, const char *password ) {
    /* Username */
    if ( !username ) {
        IRC_PRINT( ( "Username: " ) );
        irc->username = getline_stdin( NULL );
    } else {
        irc->username = malloc( sizeof *username * strlen( username ) );
        strcpy( irc->username, username );
    }

    if ( !irc->username ) {
        IRC_PRINT( ( "irc_auth(): malloc error\n") );
        exit( EXIT_FAILURE );
    }

    /* Password */
    if ( !password ) {
        IRC_PRINT( ( "Password: ") );
        irc->password = getline_stdin( NULL );
    } else {
        irc->password = malloc( sizeof *password * strlen( password ) );
        strcpy( irc->password, password );
    }

    if ( !irc->password ) {
        IRC_PRINT( ( "irc_auth(): malloc error\n" ) );
        exit( EXIT_FAILURE );
    }

    if ( ready )
        ready( irc, NULL, NULL );

    char *raw;

    raw = format_string( strlen( irc->password ) + 9,
                         "PASS %s\r\n",
                         irc->password );

    irc_send_raw( irc, raw );
    free_ptr( raw );
    
    raw = format_string( strlen( irc->username ) + 9,
                         "NICK %s\r\n",
                         irc->username );
    irc_send_raw( irc, raw );
    free_ptr( raw );

    raw = format_string( strlen( irc->username ) * 3
                         + strlen(irc->host) + 16,
                         "USER %s %s %s :%s\r\n",
                         irc->username,
                         irc->username,
                         irc->host,
                         irc->username);
    irc_send_raw( irc, raw );
    free_ptr( raw );

    irc_wait( irc );

    /* Start receiving data from the server */
    irc_hook_reply( irc, &irc_welcome_callback, IRC_RPL_WELCOME );
    irc_hook_command( irc, &irc_join_callback, "JOIN" );
    irc_hook_command( irc, &irc_pong, "PING" );

    pthread_create( &irc->thread, NULL, irc_read_loop, irc );
}

void irc_signal( IrcContext *irc ) {
    pthread_mutex_unlock( &irc->state );
}

void irc_wait( IrcContext *irc ) {
    pthread_mutex_lock( &irc->state );
}

void *irc_read_loop( void *args ) {
    IrcContext *irc = ( IrcContext* ) args;
    ssize_t bytes;
    char buffer[ 512 ];
    char line_orig[ 512 ];
    char **argv = NULL;
    char *line, *unit;
    unsigned int i, j;
    int argc = 0;

    memset( line_orig, '\0', 512 );

    while ( ( bytes = read( irc->socket, buffer, 512 ) ) ) {
        line = strtok( buffer, "\n" );
        if ( line ) {
            strncpy( line_orig, line, strlen( line ) );
            line_orig[ strlen( line ) ] = '\0';
        }

        while ( line ) {
            for ( j = 1; j < strlen( line ); ++j ) {
                if ( line[ j ] == '!' ) {
                    argv = realloc( argv, sizeof *argv *
                                    ( argc + 1 ) );
                    if ( !argv ) {
                        IRC_PRINT( ( "irc_read_loop(): " \
                                     "realloc error\n" ) );
                        exit( EXIT_FAILURE );
                    }
                    argv[ argc ] = malloc( sizeof
                                           *argv[ argc ] *
                                           j );
                    if ( !argv[ argc ] ) {
                        IRC_PRINT( ( "irc_read_loop(): " \
                                     "malloc error\n" ) );
                        exit( EXIT_FAILURE );
                    }
                    strncpy( argv[ argc ],
                             line + 1,
                             j - 1 );
                    argv[ argc ][ j - 1 ] = '\0';
                    ++argc;
                    break;
                }
            }

            unit = strtok( line, " " );
            while ( unit ) {
                if ( !strcmp( unit, "PRIVMSG" ) ) {
                    char *next_unit = strtok( NULL, " " );
                    if ( next_unit ) {
                        const int start = ( next_unit - buffer ) +
                                        strlen( next_unit ) + 2;
                        const int len = strlen( line_orig ) - start;
                        argv = realloc( argv, sizeof *argv *
                                       ( argc + 1 ) );
                        if ( !argv ) {
                            IRC_PRINT( ( "irc_read_loop(): " \
                                         "realloc error\n" ) );
                            exit( EXIT_FAILURE );
                        }
                        argv[ argc ] = malloc( sizeof
                                               *argv[ argc ] *
                                               ( len + 1 ) );
                        if ( !argv[ argc ] ) {
                            IRC_PRINT( ( "irc_read_loop(): " \
                                         "malloc error\n" ) );
                            exit( EXIT_FAILURE );
                        }
                        strncpy( argv[ argc ], line + start, len );
                        argv[ argc ][ len ] = '\0';
                        ++argc;
                    }
                }

                IrcEvent *event;
                while ( ( event = hash_table_lookup( irc->events_hash,
                          unit, HASH_DJB2 ) ) ) {
                    switch ( event->type ) {
                        case IRC_TYPE_COMMAND:
                        case IRC_TYPE_REPLY:
                            if ( !strcmp( unit, event->param ) )
                                event->callback( irc, argv, line_orig );
                            break;
                        default:
                            event->callback( irc, argv, line_orig );
                    }
                }

                unit = strtok( NULL, " " );
            }

            for ( j = 0; j < argc; ++j ) {
                free_ptr( argv[ j ] );
            }

            free_ptr( argv );
            argv = NULL;
            argc = 0;

            line = strtok( NULL, "\n" );
            if ( line ) {
                strncpy( line_orig, line, strlen( line ) );
                line_orig[ strlen( line ) ] = '\0';
            }
        }
    }

    pthread_detach( irc->thread );

    return NULL;
}

void irc_join( IrcContext *irc, IrcCallback* ready, const char *channel ) {
    irc_wait( irc );

    irc->channels = realloc( irc->channels,
                             sizeof *irc->channels * ( ++irc->num_channels ) );

    if ( !irc->channels ) {
        IRC_PRINT( ( "irc_join(): realloc error\n" ) );
        exit( EXIT_FAILURE );
    }

    if ( !channel ) {
        IRC_PRINT( ( "IRC channel: " ) );
        irc->channels[ irc->num_channels - 1 ] = getline_stdin( NULL );
    } else {
        irc->channels[ irc->num_channels - 1 ] = malloc( sizeof *channel *
                                                         strlen( channel ) );
        strcpy( irc->channels[ irc->num_channels - 1 ], channel );
    }

    if ( !irc->channels[ irc->num_channels - 1 ] ) {
        IRC_PRINT( ( "irc_join(): malloc error\n" ) );
        exit( EXIT_FAILURE );
    }

    if ( ready )
        ready( irc, NULL, NULL );

    char *raw;
    raw = format_string( strlen( irc->channels[ irc->num_channels - 1 ] ) + 9,
                         "JOIN #%s\r\n",
                         irc->channels[ irc->num_channels - 1 ] );
    irc_send_raw( irc, raw );
    free_ptr( raw );

    irc_wait( irc );
}

void irc_connect( IrcContext *irc, IrcCallback* ready,
                  const char *host, const char *port ) {
    /* Host */
    if ( !host ) {
        IRC_PRINT( ( "IRC server: " ) );
        irc->host = getline_stdin( NULL );
    } else {
        irc->host = malloc( sizeof *host * strlen( host ) );
        strcpy( irc->host, host );
    }

    if ( !irc->host ) {
        IRC_PRINT( ( "irc_connect(): malloc error\n" ) );
        exit( EXIT_FAILURE );
    }

    /* Port */
    if ( !port ) {
        IRC_PRINT( ( "IRC port: " ) );
        irc->port = getline_stdin( NULL );
    } else {
        irc->port = malloc( sizeof *port * strlen( port ) );
        strcpy( irc->port, port );
    }

    if ( !irc->port ) {
        IRC_PRINT( ( "irc_connect(): malloc error\n" ) );
        exit( EXIT_FAILURE );
    }

    if ( ready )
        ready( irc, NULL, NULL );

    struct addrinfo hints, *res;

    memset( &hints, 0, sizeof hints );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo( irc->host, irc->port, &hints, &res );
    if ( ret != 0 ) {
        IRC_PRINT( ( "irc_connect(): %s\n", gai_strerror( ret ) ) );
        exit( EXIT_FAILURE );
    }

    irc->socket = socket( res->ai_family, res->ai_socktype, res->ai_protocol );

    if ( irc->socket == -1 ) {
        IRC_PRINT( ( "irc_connect(): socket error\n" ) );
        exit( EXIT_FAILURE );
    }

    if ( connect( irc->socket, res->ai_addr, res->ai_addrlen ) == -1 ) {
        IRC_PRINT( ( "irc_connect(): connect error\n" ) );
        exit( EXIT_FAILURE );
    }
}

int irc_send_raw( IrcContext *irc, const char *str ) {
    if ( !irc->socket ) {
        IRC_PRINT( ( "irc_send_raw(): socket error\n" ) );
        return IRC_SOCK_ERROR;
    }

    ssize_t bytes = write( irc->socket, str, strlen( str ) );

    if ( bytes <= 0 ) {
        IRC_PRINT( ( "irc_send_raw(): write failed\n" ) );
        return IRC_SOCK_ERROR;
    }

    return IRC_OK;
}

int irc_say( IrcContext *irc, const char *str, const unsigned int channel ) {
    if ( channel >= irc->num_channels ) {
        IRC_PRINT( ( "irc_say(): invalid channel\n" ) );
        return IRC_ERR;
    }

    char *raw = format_string( strlen( str ) +
                               strlen( irc->channels[ channel ] ) + 15,
                               "PRIVMSG #%s :%s\r\n",
                               irc->channels[ channel ],
                               str );
    int ret = irc_send_raw( irc, raw );
    free_ptr( raw );

    return ret;
}

void irc_hook_reply( IrcContext *irc,
                     IrcCallback* callback,
                     const char *reply ) {
    ++irc->num_events;

    irc->events[ irc->num_events - 1 ].param = ( unsigned char* ) malloc(
        strlen( reply ) + 1
    );

    if ( !irc->events[ irc->num_events - 1 ].param ) {
        exit( EXIT_FAILURE );
    }

    strcpy( irc->events[ irc->num_events - 1 ].param, reply );
    irc->events[ irc->num_events - 1 ].param[ strlen( reply ) ] = '\0';

    irc->events[ irc->num_events - 1 ].callback = callback;
    irc->events[ irc->num_events - 1 ].type = IRC_TYPE_REPLY;

    hash_table_insert( irc->events_hash,
                       reply,
                       &irc->events[ irc->num_events - 1 ],
                       HASH_DJB2 );
}

void irc_unhook_reply( IrcContext *irc,
                       IrcCallback* callback,
                       const char *reply ) {

}

void irc_hook_command( IrcContext *irc,
                       IrcCallback* callback,
                       const char *command ) {
    ++irc->num_events;

    irc->events[ irc->num_events - 1 ].param = ( unsigned char* ) malloc(
        strlen( command ) + 1
    );

    if ( !irc->events[ irc->num_events - 1 ].param ) {
        IRC_PRINT( ( "irc_hook_command(): malloc error\n" ) );
        exit( EXIT_FAILURE );
    }

    strcpy( irc->events[ irc->num_events - 1 ].param, command );
    irc->events[ irc->num_events - 1 ].param[ strlen( command ) ] = '\0';

    irc->events[ irc->num_events - 1 ].callback = callback;
    irc->events[ irc->num_events - 1 ].type = IRC_TYPE_COMMAND;

    hash_table_insert( irc->events_hash,
                       command,
                       &irc->events[ irc->num_events - 1 ],
                       HASH_DJB2 );
}

void irc_unhook_command( IrcContext *irc,
                         IrcCallback* callback,
                         const char *command ) {

}
