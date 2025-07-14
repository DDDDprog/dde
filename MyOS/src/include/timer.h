#ifndef TIMER_H
#define TIMER_H

#include "types.h"

#define TIMER_FREQUENCY 1000  /* 1000 Hz = 1ms per tick */
#define PIT_FREQUENCY 1193180 /* PIT base frequency */

/* Timer functions */
void timer_init(void);
void timer_handler(void);
uint32_t timer_get_ticks(void);
uint32_t timer_get_seconds(void);
void timer_sleep(uint32_t ms);

/* System time structure */
typedef struct {
    uint32_t seconds;
    uint32_t minutes;
    uint32_t hours;
    uint32_t day;
    uint32_t month;
    uint32_t year;
} system_time_t;

void time_init(void);
system_time_t time_get(void);
void time_set(system_time_t* time);
char* time_to_string(system_time_t* time);

#endif