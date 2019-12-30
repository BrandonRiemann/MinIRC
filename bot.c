#include "bot.h"
#include "curses-window.h"
#include <string.h>
#include <stdio.h>

int print_bot_strings( WindowContext *window ) {
    int rows = 0;
    int wrap = window->cols;
    int indent = DEFAULT_INDENT;

    if ( window->cols - DEFAULT_WRAP > indent )
        indent = ( window->cols - DEFAULT_WRAP ) / 2;

    if ( window->cols < wrap )
        wrap = window->cols;

    rows += print_wrap( window,
                wrap - indent,
                indent,
                1,
                strlen( BOT_NAME ) + strlen( BOT_VERSION ) + strlen(BOT_AUTHOR) + 13,
                "\n%s %s\t\t\t\t\t\t\t\t%s\n\n",
                BOT_NAME,
                BOT_VERSION,
                BOT_AUTHOR );

    /* rows += print_wrap( window,
                wrap - indent,
                indent,
                1,
                strlen( BOT_DESC ) + 3,
                "%s\t\t",
                BOT_DESC );

    rows += print_wrap( window,
                wrap - indent,
                indent,
                1,
                strlen( BOT_AUTHOR ) + 19,
                "%s",
                BOT_AUTHOR ); */

    return rows;
}
