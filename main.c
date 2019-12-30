#define IRC_SHOWOUTPUT

#include "irc-client.h"
#include "bot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static WindowContext window;
static IrcContext irc;

typedef struct {
    char message[ IRC_MAX_MESSAGE_LEN ];
    char buffer[ IRC_MAX_MESSAGE_LEN ];
    int length;
} InputContext;

static InputContext input;

static void init_input( void ) {
    input.length = strlen( IRC_CHANNEL ) + 4;
    window.cur_msg.x = input.length;
    memset( input.message, '\0', IRC_MAX_MESSAGE_LEN );
    memset( input.buffer, '\0', IRC_MAX_MESSAGE_LEN );
    sprintf( input.message, "[#%s] ", IRC_CHANNEL );
    strcpy( input.buffer, input.message );
}

static int join_callback( IrcContext *irc, char **argv, char *line ) {
    if ( argv[ 0 ] && !strcmp( argv[0], irc->username ) ) {
        window_set_color_on( &window, IRC_COLOR_INCOMING );
        window_printw( &window, ">> " );
        window_set_color_off( &window, IRC_COLOR_INCOMING );
        window_printw( &window, "Joined channel.\n\n" );
    }

    return IRC_OK;
}

static int privmsg_callback( IrcContext *irc, char **argv, char *line ) {
    window_bold_on( &window );
    window_set_color_on( &window, IRC_COLOR_NAMES );
    window_printw( &window, "<%s>", argv[ 0 ] );
    window_set_color_off( &window, IRC_COLOR_NAMES );
    window_bold_off( &window );

    window_printw( &window, ": %s\n", argv[ 1 ] );

    return IRC_OK;
}

static void colorize_input( WindowContext *window ) {
    int i;

    if ( input.length > window->cur_msg.x &&
         window->cur_msg.x < window->cols ) {
        window->cur_msg.x = ( input.length > window->cols ) ?
                              window->cols - 1 : input.length - 1;
    } else if ( window->cur_msg.x > window->cols ) {
        window->cur_msg.x = window->cols - 1;
    }

    scrollok( window->window, 0 );

    window_move_cursor( window, 0, window->rows - 1 );
    clrtoeol();

    window_bold_on( window );
    window_set_color_on( window, IRC_COLOR_INPUT );
    wprintw( window->window,
             input.buffer + ( input.length - window->cur_msg.x ) );

    for ( i = 0; i < window->cols - window->cur_msg.x; ++i )
        waddch( window->window, ' ' );

    window_move_cursor( window, window->cur_msg.x, window->rows - 1 );
    window_set_color_off( window, IRC_COLOR_INPUT );
    window_bold_off( window );

    scrollok( window->window, 1 );
    window_refresh();
}

static void input_callback( char ch ) {
    if ( ( ch < 32 && ch != 8 && ch != 13 ) || ch > 255 )
        return;

    switch( ch ) {
    case 13: /* Return */
        if ( input.length == strlen( IRC_CHANNEL ) + 4 )
            return;
        input.message[ input.length ] = '\0';
        irc_say( &irc, input.message + strlen( IRC_CHANNEL ) + 4, 0 );
        input.length = strlen( IRC_CHANNEL ) + 4;
        window_move_cursor( &window, 0, window.rows - 1 );
        clrtoeol();
        int x, y;
        window_get_cursor( &window, &x, &y );
        window_move_cursor( &window, x, y );

        window_bold_on( &window );
        window_set_color_on( &window, IRC_COLOR_NAMES );
        window_printw( &window, "<%s>", IRC_NICK );
        window_set_color_off( &window, IRC_COLOR_NAMES );
        window_bold_off( &window );

        window_printw( &window, ": %s\n", input.message +
                       strlen( IRC_CHANNEL ) + 4 );

        memset( input.message, '\0', IRC_MAX_MESSAGE_LEN );
        memset( input.buffer, '\0', IRC_MAX_MESSAGE_LEN );
        sprintf( input.message, "[#%s] ", IRC_CHANNEL );
        strcpy( input.buffer, input.message );
        window.cur_msg.x = strlen( input.message );
        window_refresh();
        break;
    case 27: /* Esc */
        break;
    case 0x08:
    case 0x7f:  /* Backspace */
        if ( input.length == strlen( IRC_CHANNEL ) + 4 ) {
            window.cur_msg.x = strlen( IRC_CHANNEL ) + 4;
            break;
        }
        input.buffer[ --input.length ] = '\0';
        input.message[ input.length ] = '\0';
        int overflow = input.length - window.cur_msg.x;
        scrollok( window.window, 0 );
        if ( overflow >= 0 ) {
            window_move_cursor( &window, 0, window.rows - 1 );
            clrtoeol();
        } else {
            window_move_cursor( &window, 0, window.rows - 1 );
            clrtoeol();
            --window.cur_msg.x;
        }
        scrollok( window.window, 1 );
        window_refresh();
        break;
    default:
        if ( input.length + 1 > IRC_MAX_MESSAGE_LEN )
            break;
        input.message[ input.length++ ] = ch;
        scrollok( window.window, 0 );
        if ( window.cur_msg.x + 1 < window.cols ) {
            window.cur_msg.x++;
            input.buffer[ input.length - 1 ] = ch;
        } else {
            window_move_cursor( &window, 0, window.rows - 1 );
            clrtoeol();
            input.buffer[ input.length - 1 ] = ch;
        }

        scrollok( window.window, 1 );
        window_refresh();
    }

    colorize_input( &window );
}

static void resize_callback( WindowContext *window ) {
    window_destroy( window );
    window_create_colored( window );

    window_move_cursor( window, 0, 0 );
    window_set_cursor( window, 0, 0 );

    window_set_color_on( window, IRC_COLOR_BANNER );
    int rows = print_bot_strings( window );
    window_set_color_off( window, IRC_COLOR_BANNER );

    window_move_cursor( window, 0, rows + 2 );
    window_save_cursor( window );

    colorize_input( window );

    wsetscrreg( window->window, rows + 2, window->rows - 3 );
    window_refresh();
}

static int print_connect_str( IrcContext *irc, char **argv, char *line ) {
    window_printw( &window, "\n\n" );
    window_set_color_on( &window, IRC_COLOR_OUTGOING );
    window_printw( &window, "<< " );
    window_set_color_off( &window, IRC_COLOR_OUTGOING );
    window_printw( &window, "Connecting to %s/%s...\n", irc->host, irc->port );

    return IRC_OK;
}

static int print_auth_str( IrcContext *irc, char **argv, char *line ) {
    window_set_color_on( &window, IRC_COLOR_OUTGOING );
    window_printw( &window, "<< " );
    window_set_color_off( &window, IRC_COLOR_OUTGOING );
    window_printw( &window, "Sending " );
    window_set_color_on( &window, IRC_COLOR_COMMAND );
    window_printw( &window, "NICK" );
    window_set_color_off( &window, IRC_COLOR_COMMAND );
    window_printw( &window, " and " );
    window_set_color_on( &window, IRC_COLOR_COMMAND );
    window_printw( &window, "PASS" );
    window_set_color_off( &window, IRC_COLOR_COMMAND );
    window_printw( &window, " to server...\n" );

    return IRC_OK;
}

static int print_join_str( IrcContext *irc, char **argv, char *line ) {
    window_set_color_on( &window, IRC_COLOR_OUTGOING );
    window_printw( &window, "<< " );
    window_set_color_off( &window, IRC_COLOR_OUTGOING );
    window_printw( &window, "Joining channel " );
    window_set_color_on( &window, IRC_COLOR_CHANNEL );
    window_printw( &window, "#%s", irc->channels[ irc->num_channels - 1 ] );
    window_set_color_off( &window, IRC_COLOR_CHANNEL );
    window_printw( &window, "...\n" );

    return IRC_OK;
}

static int print_server_msgs( IrcContext *irc, char **argv, char *line ) {
    window_set_color_on( &window, IRC_COLOR_INCOMING );
    window_printw( &window, ">> %s\n", line );
    window_set_color_off( &window, IRC_COLOR_INCOMING );
}

int main( int argc, char **argv ) {
    window_init_context( &window );
    window_create_colored( &window );

    window_set_signal( &window, SIGWINCH, resize_callback );

    /* Normal text */
    window_set_color_pair( &window,
                           IRC_COLOR_NORMAL, COLOR_WHITE, COLOR_BLACK );
    /* Banner */
    window_set_color_pair( &window,
                           IRC_COLOR_BANNER, COLOR_WHITE, COLOR_MAGENTA );
    /* Outgoing messages */
    window_set_color_pair( &window,
                           IRC_COLOR_OUTGOING, COLOR_GREEN, COLOR_BLACK );
    /* Incoming messages */
    window_set_color_pair( &window,
                           IRC_COLOR_INCOMING, COLOR_CYAN, COLOR_BLACK );
    /* Commands */
    window_set_color_pair( &window,
                           IRC_COLOR_COMMAND, COLOR_YELLOW, COLOR_BLACK );
    /* Channels */
    window_set_color_pair( &window,
                           IRC_COLOR_CHANNEL, COLOR_GREEN, COLOR_BLACK );
    /* Input box */
    window_set_color_pair( &window,
                           IRC_COLOR_INPUT, COLOR_WHITE, COLOR_CYAN );
    /* Names */
    window_set_color_pair( &window,
                           IRC_COLOR_NAMES, COLOR_RED, COLOR_BLACK );

    scrollok( window.window, 1 );

    window_set_color_on( &window, IRC_COLOR_BANNER );
    int rows = print_bot_strings( &window );
    window_set_color_off( &window, IRC_COLOR_BANNER );

    wsetscrreg( window.window, rows + 2, window.rows - 3 );

    init_input();
    colorize_input( &window );

    window_refresh();

    irc_init_context( &irc );

    irc_hook_command( &irc, &privmsg_callback, "PRIVMSG" );
    irc_hook_command( &irc, &join_callback, "JOIN" );

    irc_connect( &irc, &print_connect_str, TWITCH_IRC_HOST, TWITCH_IRC_PORT );
    irc_auth( &irc, &print_auth_str, IRC_NICK, IRC_PASS );
    irc_join( &irc, &print_join_str, IRC_CHANNEL );

    window_wait( &window, &input_callback );

    irc_destroy_context( &irc );
    window_destroy_context( &window );

    return 0;
}
