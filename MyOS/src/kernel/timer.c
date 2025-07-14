#include "timer.h"
#include "kernel.h"
#include "vga.h"
#include "interrupts.h"
#include "process.h"

static volatile uint32_t timer_ticks = 0;
static system_time_t system_time = {0, 0, 12, 1, 1, 2024}; /* Default: Jan 1, 2024 12:00:00 */

void timer_init(void) {
    /* Calculate divisor for desired frequency */
    uint32_t divisor = PIT_FREQUENCY / TIMER_FREQUENCY;
    
    /* Send command byte */
    outb(0x43, 0x36); /* Channel 0, lobyte/hibyte, rate generator */
    
    /* Send divisor */
    outb(0x40, divisor & 0xFF);        /* Low byte */
    outb(0x40, (divisor >> 8) & 0xFF); /* High byte */
    
    vga_printf("Timer initialized at %d Hz\n", TIMER_FREQUENCY);
}

void timer_handler(void) {
    timer_ticks++;
    
    /* Update system time every second (1000 ticks) */
    if (timer_ticks % TIMER_FREQUENCY == 0) {
        system_time.seconds++;
        
        if (system_time.seconds >= 60) {
            system_time.seconds = 0;
            system_time.minutes++;
            
            if (system_time.minutes >= 60) {
                system_time.minutes = 0;
                system_time.hours++;
                
                if (system_time.hours >= 24) {
                    system_time.hours = 0;
                    system_time.day++;
                    
                    /* Simple month handling (30 days per month) */
                    if (system_time.day > 30) {
                        system_time.day = 1;
                        system_time.month++;
                        
                        if (system_time.month > 12) {
                            system_time.month = 1;
                            system_time.year++;
                        }
                    }
                }
            }
        }
    }
    
    /* Call scheduler */
    scheduler_tick();
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

uint32_t timer_get_seconds(void) {
    return timer_ticks / TIMER_FREQUENCY;
}

void timer_sleep(uint32_t ms) {
    uint32_t target_ticks = timer_ticks + ms;
    while (timer_ticks < target_ticks) {
        __asm__ volatile ("hlt"); /* Wait for next interrupt */
    }
}

void time_init(void) {
    /* Initialize with default time */
    vga_puts("System time initialized\n");
}

system_time_t time_get(void) {
    return system_time;
}

void time_set(system_time_t* time) {
    if (time) {
        system_time = *time;
    }
}

char* time_to_string(system_time_t* time) {
    static char time_str[32];
    
    /* Format: "Mon Jan 01 12:00:00 2024" */
    const char* months[] = {
        "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    /* Simple sprintf implementation */
    char* ptr = time_str;
    
    /* Day of week (simplified - always Mon) */
    *ptr++ = 'M'; *ptr++ = 'o'; *ptr++ = 'n'; *ptr++ = ' ';
    
    /* Month */
    const char* month = months[time->month];
    while (*month) *ptr++ = *month++;
    *ptr++ = ' ';
    
    /* Day */
    if (time->day < 10) *ptr++ = '0';
    if (time->day >= 10) *ptr++ = '0' + (time->day / 10);
    *ptr++ = '0' + (time->day % 10);
    *ptr++ = ' ';
    
    /* Hour */
    if (time->hours < 10) *ptr++ = '0';
    if (time->hours >= 10) *ptr++ = '0' + (time->hours / 10);
    *ptr++ = '0' + (time->hours % 10);
    *ptr++ = ':';
    
    /* Minute */
    if (time->minutes < 10) *ptr++ = '0';
    if (time->minutes >= 10) *ptr++ = '0' + (time->minutes / 10);
    *ptr++ = '0' + (time->minutes % 10);
    *ptr++ = ':';
    
    /* Second */
    if (time->seconds < 10) *ptr++ = '0';
    if (time->seconds >= 10) *ptr++ = '0' + (time->seconds / 10);
    *ptr++ = '0' + (time->seconds % 10);
    *ptr++ = ' ';
    
    /* Year */
    uint32_t year = time->year;
    *ptr++ = '0' + (year / 1000); year %= 1000;
    *ptr++ = '0' + (year / 100);  year %= 100;
    *ptr++ = '0' + (year / 10);   year %= 10;
    *ptr++ = '0' + year;
    
    *ptr = '\0';
    return time_str;
}