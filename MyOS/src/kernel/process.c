#include "process.h"
#include "kernel.h"
#include "vga.h"

static process_t processes[MAX_PROCESSES];
static process_t* current_process = NULL;
static process_t* ready_queue = NULL;
static uint32_t next_pid = 1;
static uint32_t process_count = 0;

/* Timer for scheduling */
static uint32_t timer_ticks = 0;
static uint32_t time_slice_ticks = 10; /* 10 timer ticks per time slice */

void process_init(void) {
    /* Initialize process table */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = 0;
        processes[i].state = PROCESS_TERMINATED;
        processes[i].next = NULL;
    }
    
    /* Create kernel idle process */
    process_create("idle", idle_process, 0);
    
    vga_puts("Process management initialized\n");
}

uint32_t process_create(const char* name, void (*entry_point)(void), uint32_t priority) {
    if (process_count >= MAX_PROCESSES) {
        return 0; /* No free process slots */
    }
    
    /* Find free process slot */
    process_t* proc = NULL;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_TERMINATED) {
            proc = &processes[i];
            break;
        }
    }
    
    if (!proc) return 0;
    
    /* Initialize process */
    proc->pid = next_pid++;
    strncpy(proc->name, name, PROCESS_NAME_LEN - 1);
    proc->name[PROCESS_NAME_LEN - 1] = '\0';
    proc->state = PROCESS_READY;
    proc->priority = priority;
    proc->time_slice = time_slice_ticks;
    proc->total_time = 0;
    
    /* Allocate stack */
    proc->stack_base = 0x200000 + (proc->pid * STACK_SIZE); /* Simple stack allocation */
    proc->esp = proc->stack_base + STACK_SIZE - 4;
    proc->ebp = proc->esp;
    proc->eip = (uint32_t)entry_point;
    
    /* Initialize heap */
    proc->heap_start = proc->stack_base + STACK_SIZE;
    proc->heap_end = proc->heap_start;
    
    /* Add to ready queue */
    proc->next = ready_queue;
    ready_queue = proc;
    
    process_count++;
    
    vga_printf("Created process '%s' (PID: %d)\n", name, proc->pid);
    return proc->pid;
}

void process_exit(uint32_t exit_code) {
    if (!current_process) return;
    
    vga_printf("Process '%s' (PID: %d) exiting with code %d\n", 
               current_process->name, current_process->pid, exit_code);
    
    current_process->state = PROCESS_TERMINATED;
    process_count--;
    
    /* Remove from ready queue if present */
    if (ready_queue == current_process) {
        ready_queue = current_process->next;
    } else {
        process_t* prev = ready_queue;
        while (prev && prev->next != current_process) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = current_process->next;
        }
    }
    
    current_process = NULL;
    schedule(); /* Switch to next process */
}

void process_yield(void) {
    if (!current_process) return;
    
    current_process->state = PROCESS_READY;
    schedule();
}

void process_sleep(uint32_t ms) {
    if (!current_process) return;
    
    /* Simple sleep implementation - just yield for now */
    /* In a real OS, this would block the process for the specified time */
    uint32_t sleep_ticks = ms / 10; /* Approximate */
    for (uint32_t i = 0; i < sleep_ticks; i++) {
        process_yield();
    }
}

process_t* process_get_current(void) {
    return current_process;
}

process_t* process_get_by_pid(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == pid && processes[i].state != PROCESS_TERMINATED) {
            return &processes[i];
        }
    }
    return NULL;
}

void process_list(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Process List:\n");
    vga_puts("PID  Name           State    Priority  CPU Time\n");
    vga_puts("---  ----           -----    --------  --------\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROCESS_TERMINATED) {
            const char* state_str;
            switch (processes[i].state) {
                case PROCESS_READY: state_str = "READY"; break;
                case PROCESS_RUNNING: state_str = "RUNNING"; break;
                case PROCESS_BLOCKED: state_str = "BLOCKED"; break;
                default: state_str = "UNKNOWN"; break;
            }
            
            vga_printf("%-3d  %-13s  %-7s  %-8d  %d\n",
                       processes[i].pid,
                       processes[i].name,
                       state_str,
                       processes[i].priority,
                       processes[i].total_time);
        }
    }
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void scheduler_init(void) {
    timer_ticks = 0;
    vga_puts("Scheduler initialized\n");
}

void scheduler_tick(void) {
    timer_ticks++;
    
    if (current_process) {
        current_process->total_time++;
        current_process->time_slice--;
        
        /* Time slice expired? */
        if (current_process->time_slice == 0) {
            current_process->time_slice = time_slice_ticks;
            if (current_process->state == PROCESS_RUNNING) {
                current_process->state = PROCESS_READY;
            }
            schedule();
        }
    }
}

void schedule(void) {
    process_t* next = NULL;
    process_t* highest_priority = NULL;
    uint32_t max_priority = 0;
    
    /* Simple priority-based round-robin scheduler */
    process_t* proc = ready_queue;
    while (proc) {
        if (proc->state == PROCESS_READY && proc->priority >= max_priority) {
            max_priority = proc->priority;
            highest_priority = proc;
        }
        proc = proc->next;
    }
    
    next = highest_priority;
    
    if (next && next != current_process) {
        process_t* prev = current_process;
        current_process = next;
        current_process->state = PROCESS_RUNNING;
        current_process->time_slice = time_slice_ticks;
        
        if (prev) {
            context_switch(next);
        }
    }
}

/* Simple context switch - in a real OS this would be in assembly */
void context_switch(process_t* next) {
    /* This is a simplified version - real context switching requires assembly */
    /* For now, just update the current process pointer */
    (void)next; /* Suppress unused parameter warning */
    
    /* In a real implementation, this would:
     * 1. Save current process registers to its PCB
     * 2. Load next process registers from its PCB
     * 3. Switch page directory
     * 4. Jump to next process
     */
}

/* System call handler */
uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    switch (syscall_num) {
        case SYS_EXIT:
            process_exit(arg1);
            return 0;
            
        case SYS_GETPID:
            return current_process ? current_process->pid : 0;
            
        case SYS_SLEEP:
            process_sleep(arg1);
            return 0;
            
        case SYS_WRITE:
            /* Simple write to console */
            if (arg1 == 1) { /* stdout */
                char* str = (char*)arg2;
                for (uint32_t i = 0; i < arg3; i++) {
                    vga_putchar(str[i]);
                }
                return arg3;
            }
            return -1;
            
        default:
            return -1; /* Unknown system call */
    }
}

