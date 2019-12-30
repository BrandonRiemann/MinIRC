#ifndef BOT_H__
#define BOT_H__

#include "curses-window.h"

#define BOT_NAME    "MinIRC"
#define BOT_VERSION "1.0"
#define BOT_AUTHOR  "Brandon Sachtleben"
#define BOT_DESC    "Minimalistic IRC client using ncurses"

#define TWITCH_IRC_HOST  "irc.chat.twitch.tv"
#define TWITCH_IRC_PORT  "6667"

#define IRC_NICK         "brandonriemann"
#define IRC_PASS         "oauth:651q0zzssi9zl6kmk0iwwh9n6reyny"
#define IRC_CHANNEL      "vinesauce"

int print_bot_strings( WindowContext* );

#endif
