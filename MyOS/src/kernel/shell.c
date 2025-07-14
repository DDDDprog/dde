#include "shell.h"
#include "kernel.h"
#include "vga.h"
#include "keyboard.h"
#include "process.h"
#include "memory.h"

static char current_directory[MAX_PATH_LENGTH] = "/";
static char command_buffer[MAX_COMMAND_LENGTH];
static char* argv_buffer[MAX_ARGS];
static uint32_t system_start_time = 0;

/* Built-in commands table */
static shell_command_t builtin_commands[] = {
    {"help", "Show available commands", cmd_help},
    {"clear", "Clear the screen", cmd_clear},
    {"echo", "Display text", cmd_echo},
    {"ps", "Show running processes", cmd_ps},
    {"kill", "Terminate a process", cmd_kill},
    {"ls", "List directory contents", cmd_ls},
    {"cat", "Display file contents", cmd_cat},
    {"mkdir", "Create directory", cmd_mkdir},
    {"rmdir", "Remove directory", cmd_rmdir},
    {"cd", "Change directory", cmd_cd},
    {"pwd", "Print working directory", cmd_pwd},
    {"free", "Show memory usage", cmd_free},
    {"uptime", "Show system uptime", cmd_uptime},
    {"uname", "Show system information", cmd_uname},
    {"whoami", "Show current user", cmd_whoami},
    {"date", "Show current date and time", cmd_date},
    {"reboot", "Restart the system", cmd_reboot},
    {"shutdown", "Shutdown the system", cmd_shutdown},
    {NULL, NULL, NULL}
};

void shell_init(void) {
    system_start_time = 0; /* Initialize with timer when available */
    vga_puts("MyOS Shell initialized\n");
}

void shell_run(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("Welcome to MyOS Shell!\n");
    vga_puts("Type 'help' for available commands.\n\n");
    
    while (1) {
        /* Display prompt */
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_printf("root@myos:%s$ ", current_directory);
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        /* Read command */
        char* command_line = shell_readline();
        if (command_line && strlen(command_line) > 0) {
            shell_execute(command_line);
        }
        vga_putchar('\n');
    }
}

char* shell_readline(void) {
    int pos = 0;
    char c;
    
    while (1) {
        c = keyboard_getchar();
        
        if (c == '\n') {
            command_buffer[pos] = '\0';
            vga_putchar('\n');
            return command_buffer;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                vga_putchar('\b');
            }
        } else if (c >= 32 && c <= 126 && pos < MAX_COMMAND_LENGTH - 1) {
            command_buffer[pos++] = c;
            vga_putchar(c);
        }
    }
}

int shell_parse_command(char* command_line, char* argv[]) {
    int argc = 0;
    char* token = command_line;
    
    /* Simple tokenization by spaces */
    while (*token && argc < MAX_ARGS - 1) {
        /* Skip leading spaces */
        while (*token == ' ') token++;
        if (!*token) break;
        
        argv[argc++] = token;
        
        /* Find end of token */
        while (*token && *token != ' ') token++;
        if (*token) {
            *token = '\0';
            token++;
        }
    }
    
    argv[argc] = NULL;
    return argc;
}

int shell_execute(char* command_line) {
    if (!command_line || strlen(command_line) == 0) {
        return 0;
    }
    
    int argc = shell_parse_command(command_line, argv_buffer);
    if (argc == 0) return 0;
    
    /* Look for built-in command */
    for (int i = 0; builtin_commands[i].name; i++) {
        if (strcmp(argv_buffer[0], builtin_commands[i].name) == 0) {
            return builtin_commands[i].function(argc, argv_buffer);
        }
    }
    
    /* Command not found */
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_printf("%s: command not found\n", argv_buffer[0]);
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    return -1;
}

/* Built-in command implementations */

int cmd_help(int argc, char* argv[]) {
    (void)argc; (void)argv; /* Suppress unused warnings */
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("MyOS Shell - Available Commands:\n");
    vga_puts("================================\n\n");
    
    for (int i = 0; builtin_commands[i].name; i++) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_printf("  %-12s", builtin_commands[i].name);
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_printf(" - %s\n", builtin_commands[i].description);
    }
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    return 0;
}

int cmd_clear(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_clear();
    return 0;
}

int cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_puts(argv[i]);
        if (i < argc - 1) vga_putchar(' ');
    }
    vga_putchar('\n');
    return 0;
}

int cmd_ps(int argc, char* argv[]) {
    (void)argc; (void)argv;
    process_list();
    return 0;
}

int cmd_kill(int argc, char* argv[]) {
    if (argc < 2) {
        vga_puts("Usage: kill <pid>\n");
        return -1;
    }
    
    uint32_t pid = 0;
    /* Simple atoi implementation */
    for (int i = 0; argv[1][i]; i++) {
        if (argv[1][i] >= '0' && argv[1][i] <= '9') {
            pid = pid * 10 + (argv[1][i] - '0');
        } else {
            vga_puts("Invalid PID\n");
            return -1;
        }
    }
    
    process_t* proc = process_get_by_pid(pid);
    if (proc) {
        vga_printf("Terminating process %d (%s)\n", pid, proc->name);
        /* In a real OS, this would send a signal to the process */
        vga_puts("Signal sent (not implemented)\n");
    } else {
        vga_printf("Process %d not found\n", pid);
    }
    
    return 0;
}

int cmd_ls(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_printf("Directory listing for %s:\n", current_directory);
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    /* Simulate directory contents */
    vga_puts("drwxr-xr-x  2 root root  4096 Jan  1 12:00 .\n");
    vga_puts("drwxr-xr-x  3 root root  4096 Jan  1 12:00 ..\n");
    vga_puts("-rw-r--r--  1 root root   256 Jan  1 12:00 readme.txt\n");
    vga_puts("-rwxr-xr-x  1 root root  1024 Jan  1 12:00 hello\n");
    vga_puts("drwxr-xr-x  2 root root  4096 Jan  1 12:00 bin\n");
    vga_puts("drwxr-xr-x  2 root root  4096 Jan  1 12:00 etc\n");
    
    return 0;
}

int cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        vga_puts("Usage: cat <filename>\n");
        return -1;
    }
    
    vga_printf("Contents of %s:\n", argv[1]);
    vga_puts("================\n");
    
    /* Simulate file contents */
    if (strcmp(argv[1], "readme.txt") == 0) {
        vga_puts("Welcome to MyOS!\n");
        vga_puts("This is a simple operating system built from scratch.\n");
        vga_puts("Features include process management, memory management,\n");
        vga_puts("and a basic shell interface.\n");
    } else {
        vga_printf("cat: %s: No such file or directory\n", argv[1]);
        return -1;
    }
    
    return 0;
}

int cmd_pwd(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_printf("%s\n", current_directory);
    return 0;
}

int cmd_cd(int argc, char* argv[]) {
    if (argc < 2) {
        strcpy(current_directory, "/");
    } else {
        /* Simple path handling */
        if (argv[1][0] == '/') {
            strcpy(current_directory, argv[1]);
        } else {
            /* Relative path - simplified */
            if (strcmp(current_directory, "/") != 0) {
                strcat(current_directory, "/");
            }
            strcat(current_directory, argv[1]);
        }
    }
    return 0;
}

int cmd_free(int argc, char* argv[]) {
    (void)argc; (void)argv;
    memory_info();
    return 0;
}

int cmd_uptime(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    /* Simulate uptime */
    vga_puts("System uptime: 0 days, 0 hours, 5 minutes\n");
    vga_puts("Load average: 0.00, 0.01, 0.05\n");
    return 0;
}

int cmd_uname(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    vga_puts("MyOS 1.0.0 i386\n");
    vga_puts("Kernel: MyOS version 1.0.0\n");
    vga_puts("Architecture: i386\n");
    vga_puts("Built: " __DATE__ " " __TIME__ "\n");
    return 0;
}

int cmd_whoami(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_puts("root\n");
    return 0;
}

int cmd_date(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    /* Simulate current date */
    vga_puts("Mon Jan  1 12:00:00 UTC 2024\n");
    return 0;
}

int cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        vga_puts("Usage: mkdir <directory>\n");
        return -1;
    }
    
    vga_printf("mkdir: created directory '%s'\n", argv[1]);
    return 0;
}

int cmd_rmdir(int argc, char* argv[]) {
    if (argc < 2) {
        vga_puts("Usage: rmdir <directory>\n");
        return -1;
    }
    
    vga_printf("rmdir: removed directory '%s'\n", argv[1]);
    return 0;
}

int cmd_reboot(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("System is rebooting...\n");
    
    /* Reboot using keyboard controller */
    uint8_t temp;
    __asm__ volatile ("cli");
    do {
        temp = inb(0x64);
        if (temp & 0x01) {
            inb(0x60);
        }
    } while (temp & 0x02);
    
    outb(0x64, 0xFE);
    
    /* Fallback: triple fault */
    __asm__ volatile ("int $0x00");
    return 0;
}

int cmd_shutdown(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("System is shutting down...\n");
    vga_puts("It is now safe to turn off your computer.\n");
    
    while (1) {
        __asm__ volatile ("hlt");
    }
    return 0;
}