#include "curses-window.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

void window_save_cursor( WindowContext *window ) {
    int x, y;
    window_retrieve_cursor( window, &x, &y );
    window_set_cursor( window, x, y );
}

void window_retrieve_cursor( WindowContext *window, int *x, int *y ) {
    getyx( window->window, *y, *x );
}

void window_move_cursor( WindowContext *window, int x, int y ) {
    wmove( window->window, y, x );
    wrefresh( window->window );
}

void window_set_cursor( WindowContext *window, int x, int y ) {
    window->cur_curr.x = x;
    window->cur_curr.y = y;
}

void window_get_cursor( WindowContext *window, int *x, int *y ) {
    *x = window->cur_curr.x;
    *y = window->cur_curr.y;
}

void window_get_last_cursor( WindowContext *window, int *x, int *y ) {
    *x = window->cur_last.x;
    *y = window->cur_last.y;
}

void window_printw( WindowContext *window, const char *str, ... ) {

    char *buffer = malloc( sizeof *buffer * 2048 );

    va_list args;
    va_start( args, str );
    vsnprintf( buffer, 2048, str, args );

    size_t ret = strlen( buffer ) - 2;
    if ( buffer[ ret ] == '\r' ) {
        buffer[ ret ] = '\n';
        buffer[ ret + 1 ] = '\0';
    }

    int x, y;
    window_get_cursor( window, &x, &y );
    window_move_cursor( window, x, y );

    printw( buffer );
    window_refresh();

    window_retrieve_cursor( window, &x, &y );
    window_set_cursor( window, x, y );
    window_move_cursor( window, window->cur_msg.x, window->cur_msg.y );
    window_refresh();

    va_end( args );
    free( buffer );
}

void window_set_color_pair( WindowContext *window,
                            short pair, short fg, short bg ) {
    if ( window->colors )
        init_pair( pair, fg, bg );
}

void window_bold_on( WindowContext *window ) {
    attron( A_BOLD );
}

void window_bold_off( WindowContext *window ) {
    attroff( A_BOLD );
}

void window_set_color_on( WindowContext *window, int pair ) {
    if ( window->colors )
        attron( COLOR_PAIR( pair ) );
}

void window_set_color_off( WindowContext *window, int pair ) {
    if ( window->colors )
        attroff( COLOR_PAIR( pair ) );
}

void window_destroy( WindowContext *window ) {
    endwin();
    window->window = NULL;
}

void window_refresh( void ) {
    refresh();
}

void window_clear( void ) {
    clear();
}

void window_destroy_context( WindowContext *window ) {
    if ( window->sigs )
        free( window->sigs );

    window->sigs = NULL;
    window_destroy( window );
}

void window_init_context( WindowContext *window ) {
    window->window = NULL;
    window->sigs = NULL;
    window->cols = window->rows = 0;
    window->colors = 0;
    window->num_sigs = 0;
    window->cur_curr.x = window->cur_last.x = 0;
    window->cur_curr.y = window->cur_last.y = 0;
    window->cur_msg.x = window->cur_msg.y = 0;
}

void window_create( WindowContext *window ) {
    window->window = initscr();
    refresh();
    clear();
    window->cols = COLS;
    window->rows = LINES;
    window->colors = 0;
    window->cur_msg.y = window->rows - 1;

    if ( !window->sigs ) {
        window->sigs = NULL;
        window->num_sigs = 0;
    }
}

void window_create_colored( WindowContext *window ) {
    window_create( window );
    if ( has_colors() ) {
        window->colors = 1;
        start_color();
    }
}

void window_set_signal( WindowContext *window, int sig, void *handler ) {
    window->sigs = realloc( window->sigs,
                            sizeof *window->sigs * ++window->num_sigs );

    if ( !window->sigs ) {
        exit( EXIT_FAILURE );
    }

    window->sigs[ window->num_sigs - 1 ].sig = sig;
    window->sigs[ window->num_sigs - 1 ].handler = handler;
}

void window_unset_signal( WindowContext *window, int sig ) {

}

static void window_read_input( void ( *callback )( char ) ) {
    while ( 1 )
        callback( getc(stdin) );
}

void window_wait( WindowContext *window, void ( *callback )( char ) ) {
    pthread_t input;
    sigset_t set;
    int sig = SIGINT + 1;
    int i;

    pthread_create( &input, NULL, &window_read_input, callback );

    sigemptyset( &set );
    sigaddset( &set, SIGINT );

    for ( i = 0; i < window->num_sigs; ++i ) {
        if ( sigismember( &set, window->sigs[ i ].sig ) )
            continue;

        if ( sigaddset( &set, window->sigs[ i ].sig ) ) {
            exit( EXIT_FAILURE );
        }
    }

    if ( pthread_sigmask( SIG_BLOCK, &set, NULL ) ) {
        exit( EXIT_FAILURE );
    }

    while ( sig != SIGINT ) {
        if ( sigwait( &set, &sig ) ) {
            exit( EXIT_FAILURE );
        }

        for ( i = 0; i < window->num_sigs; ++i ) {
            if ( window->sigs[ i ].sig == sig ) {
                window->sigs[ i ].handler( window );
            }
        }
    }

    pthread_detach( input );
}

/*
  print_wrap

  Wraps the provided null-terminated string at the specified wrap. The resulting
  wrap will respect the indentation value given.
*/

int print_wrap( WindowContext *window,
                 int wrap, int indent, int padding,
                 int size, const char *str, ... ) {
    char *buffer = malloc( sizeof *buffer * ( size + 1 ) );

    if ( !buffer ) {
        wprintw( window->window, "print_wrap(): malloc error\n" );
        exit( EXIT_FAILURE );
    }

    memset( buffer, '\0', size + 1 );

    va_list args;
    va_start( args, str );
    vsnprintf( buffer, size, str, args );
    va_end( args );

    if ( wrap < indent )
        wrap = indent;

    char *ptr = buffer;
    int rem = wrap - ( indent % wrap );
    int i, until_nl = 0;
    int rows = 0;
    int old_rows = 0;
    int old_cols = 0;

    int x, y;
    window_get_cursor( window, &x, &y );
    window_move_cursor( window, x, y );

    getyx( window->window, old_rows, old_cols );

    while ( *ptr != '\0' ) {
        until_nl = 0;

        for ( i = 0; i < indent; ++i ) {
            mvaddch( rows + old_rows, i, ' ' );
        }

        for ( i = 0; i < rem + indent; ++i ) {
            if ( *ptr != '\0' && i < rem ) {
                if ( *ptr == '\n' ) {
                    int j;
                    for ( j = 0; j < wrap - until_nl; ++j ) {
                        mvaddch( rows + old_rows, indent + i + j, ' ' );
                    }
                    until_nl = 0;
                    ++ptr;
                    break;
                } else {
                    mvaddch( rows + old_rows, indent + i, *ptr );
                    ++until_nl;
                    ++ptr;
                }
            } else if ( padding ) {
                mvaddch( rows + old_rows, indent + i, ' ' );
            }
        }
        if ( window->cols > DEFAULT_WRAP )
            mvaddch( rows + old_rows, rem + 2 * indent, '\n' );

        ++rows;
    }

    wrefresh( window->window );

    window_retrieve_cursor( window, &x, &y );
    window_set_cursor( window, x, y );
    window_move_cursor( window, window->cur_msg.x, window->cur_msg.y );
    window_refresh();

    free( buffer );
    return rows;
}
