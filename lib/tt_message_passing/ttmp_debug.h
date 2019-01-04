#ifndef TT_MESSAGE_PASSING_DEBUG_H
#define TT_MESSAGE_PASSING_DEBUG_H

/* xterm color control */
#define XTERM_BLACK                 "\e[1;30m"
#define XTERM_RED                   "\e[1;31m"
#define XTERM_GREEN                 "\e[1;32m"
#define XTERM_YELLOW                "\e[1;33m"
#define XTERM_BLUE                  "\e[1;34m"
#define XTERM_PURPLE                "\e[1;35m"
#define XTERM_WHITE                 "\e[1;37m"

#define XTERM_END                   "\e[0m"

#ifdef TTMP_DEBUG
#define PRINT_DEBUG(format, ...) printf(XTERM_YELLOW "TT_MSG_PASSING[DEBUG]: " XTERM_END format, ## __VA_ARGS__)
#define PRINT_ERR(format, ...) printf(XTERM_RED "TT_MSG_PASSING[DEBUG]: " XTERM_END format, ## __VA_ARGS__)
#else
#define PRINT_DEBUG(format, ...) do{}while(0)
#define PRINT_ERR(format, ...) do{}while(0);
#endif

#endif