#ifndef UI_H__
#define UI_H__

#include "curses-window.h"

#define UI_NAME    "MinIRC"
#define UI_VERSION "1.0"
#define UI_AUTHOR  "Brandon Sachtleben"
#define UI_DESC    "Minimalistic IRC client using ncurses"

#define TWITCH_IRC_HOST  "irc.chat.twitch.tv"
#define TWITCH_IRC_PORT  "6667"

#define IRC_NICK         ""
#define IRC_PASS         ""
#define IRC_CHANNEL      ""

int print_ui_strings( WindowContext* );

#endif
