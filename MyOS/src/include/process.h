#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES 64
#define STACK_SIZE 4096
#define PROCESS_NAME_LEN 32

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef struct process {
    uint32_t pid;
    char name[PROCESS_NAME_LEN];
    process_state_t state;
    uint32_t esp;           /* Stack pointer */
    uint32_t ebp;           /* Base pointer */
    uint32_t eip;           /* Instruction pointer */
    uint32_t page_directory; /* Page directory physical address */
    uint32_t stack_base;    /* Stack base address */
    uint32_t heap_start;    /* Heap start address */
    uint32_t heap_end;      /* Heap end address */
    uint32_t priority;      /* Process priority */
    uint32_t time_slice;    /* Time slice remaining */
    uint32_t total_time;    /* Total CPU time used */
    struct process* next;   /* Next process in queue */
} process_t;

/* Process management functions */
void process_init(void);
uint32_t process_create(const char* name, void (*entry_point)(void), uint32_t priority);
void process_exit(uint32_t exit_code);
void process_yield(void);
void process_sleep(uint32_t ms);
process_t* process_get_current(void);
process_t* process_get_by_pid(uint32_t pid);
void process_list(void);

/* Scheduler functions */
void scheduler_init(void);
void scheduler_tick(void);
void schedule(void);
void context_switch(process_t* next);

/* System call interface */
#define SYS_EXIT    1
#define SYS_FORK    2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETPID  20
#define SYS_SLEEP   35

uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

#endif