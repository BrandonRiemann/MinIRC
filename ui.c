#include "ui.h"
#include "curses-window.h"
#include <string.h>
#include <stdio.h>

int print_ui_strings( WindowContext *window ) {
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
                strlen( UI_NAME ) + strlen( UI_VERSION ) + strlen(UI_AUTHOR) + 13,
                "\n%s %s\t\t\t\t\t\t\t\t%s\n\n",
                UI_NAME,
                UI_VERSION,
                UI_AUTHOR );

    return rows;
}
