#ifndef TT_SHAPER_DEBUG_H
#define TT_SHAPER_DEBUG_H

/* xterm color control */
#define XTERM_BLACK "\e[1;30m"
#define XTERM_RED "\e[1;31m"
#define XTERM_GREEN "\e[1;32m"
#define XTERM_YELLOW "\e[1;33m"
#define XTERM_BLUE "\e[1;34m"
#define XTERM_PURPLE "\e[1;35m"
#define XTERM_WHITE "\e[1;37m"

#define XTERM_END "\e[0m"

#ifdef SHAPER_DEBUG
#define PRINT_DEBUG(format, ...) printf(XTERM_BLUE "SHAPER  [DEBUG]: " XTERM_END format, ##__VA_ARGS__)
#else
#define PRINT_DEBUG(format, ...) do{} while (0)
#endif

#define PRINT_ERR(format, ...) printf(XTERM_RED "SHAPER  [ERR]: " XTERM_END format, ##__VA_ARGS__)

static inline uint64_t debug_get_syscounter(void);
static inline uint64_t us_to_ticks(uint64_t us);
static inline uint64_t ticks_to_us(uint64_t ticks);

static inline uint64_t debug_get_syscounter(void){
    uint64_t cntpct;
    __asm volatile(
        "mrs %[cntpct], cntpct_el0 \n\t"
        : [cntpct] "=r"(cntpct)
        );
    return cntpct;
}

static inline uint64_t us_to_ticks(uint64_t us) {
    return us * 100;
}

static inline uint64_t ticks_to_us(uint64_t ticks){
    return ticks / 100;
}

#endif  //TT_SHAPER_DEBUG_H