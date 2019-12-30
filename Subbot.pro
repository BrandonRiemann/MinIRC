TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.c \
    bot.c \
    string-utils.c \
    curses-window.c \
    memory-utils.c \
    irc-client.c \
    input-utils.c \
    hash-table.c

QMAKE_CFLAGS += -s -ffunction-sections -Wl,--gc-sections

HEADERS += \
    bot.h \
    curses-window.h \
    memory-utils.h \
    string-utils.h \
    irc-client.h \
    input-utils.h \
    irc-types.h \
    hash-table.h


symbian: LIBS += -lpthread
else:unix|win32: LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/ -lpthread

INCLUDEPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/release/ -lncurses
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/debug/ -lncurses
else:symbian: LIBS += -lncurses
else:unix: LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/ -lncurses

INCLUDEPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu

OTHER_FILES += \
    TODO.txt
