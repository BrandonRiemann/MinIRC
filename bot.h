#ifndef BOT_H__
#define BOT_H__

#include "curses-window.h"

#define BOT_NAME    "CVBot"
#define BOT_VERSION "1.0"
#define BOT_AUTHOR  "twitch.tv/subhype"
#define BOT_DESC    "Performs image processing and conversion for Twitch " \
                    "streams into multiple viewing formats (PNG, GIF, JPG, " \
                    "etc)."

#define TWITCH_IRC_HOST  "irc.twitch.tv"
#define TWITCH_IRC_PORT  "6667"

#define IRC_NICK         "resubhype"
#define IRC_PASS         "oauth:gi273qg4asdmyihf613rh7cc9x4pffj"
#define IRC_CHANNEL      "subhype"

int print_bot_strings( WindowContext* );

#endif
