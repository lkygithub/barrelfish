#ifndef TT_CUSTOMER_DEBUG_H
#define TT_CUSTOMER_DEBUG_H

/* xterm color control */
#define XTERM_BLACK "\e[1;30m"
#define XTERM_RED "\e[1;31m"
#define XTERM_GREEN "\e[1;32m"
#define XTERM_YELLOW "\e[1;33m"
#define XTERM_BLUE "\e[1;34m"
#define XTERM_PURPLE "\e[1;35m"
#define XTERM_WHITE "\e[1;37m"

#define XTERM_END "\e[0m"

#ifdef CUSTOMER_DEBUG
#define PRINT_DEBUG(format, ...) printf(XTERM_YELLOW "CUSTOMER[DEBUG]: " XTERM_END format, ##__VA_ARGS__)
#else
#define PRINT_DEBUG(format, ...) do{} while (0)
#endif

#define PRINT_ERR(format, ...) printf(XTERM_RED "CUSTOMER[ERR]: " XTERM_END format, ##__VA_ARGS__)

#endif