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
                strlen( BOT_NAME ) + strlen( BOT_VERSION ) + 5,
                "\n%s %s\n\n",
                BOT_NAME,
                BOT_VERSION );

    rows += print_wrap( window,
                wrap - indent,
                indent,
                1,
                strlen( BOT_DESC ) + 3,
                "%s\n\n",
                BOT_DESC );

    rows += print_wrap( window,
                wrap - indent,
                indent,
                1,
                strlen( BOT_AUTHOR ) + 19,
                "%s\n\n",
                BOT_AUTHOR );

    return rows;
}
