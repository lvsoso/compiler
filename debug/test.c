#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

typedef unsigned long ulong;
 
ulong get_frame_pointer(void)
{
    ulong a;
    __asm__ __volatile__(
        "str x29, %0"
        : "=m" (a)
        :
    );
    return a;
}
 
void print_pc(void *main_begin_addr, void *main_end_addr)
{    
    ulong frame = get_frame_pointer();
    ulong *addr = (ulong *)frame;
    int i;
    char addr_line[1024*2];
    int addr_p = 0;
    extern void _start(void);

    addr_line[0] = '\0';
    //printf("main addr:%p\n", main_begin_addr);
    i = 1;
    while(1)
    {
    ulong next_frame = *addr;
    ulong pc = *(addr + 1);
    addr = (ulong *)next_frame;
    //printf("pc%d:%p addr:%p\n", i, pc, addr);
    addr_p += sprintf(addr_line + addr_p," %p", (void *)((ulong)pc-(ulong)_start));
    i++;
    if (pc >= (ulong)main_begin_addr && pc <= (ulong)main_end_addr)
    {
        break;
    }
    }
    printf("addr: %s\n", addr_line);
}
 
int sys_backtrace(void)
{
    void *buffer[10];
    int rev = backtrace(buffer, 10);
    int i;
    if (rev <= 0)
    {
    return -1;
    }

    char **stacks = backtrace_symbols(buffer, rev);
    if (stacks == NULL) {
    perror("backtrace_symbols");
    return 0;
    }
    printf("backtrace_symbols:%p", stacks);
    for (i = 0; i < rev; i ++)
    {
    printf("%d  %s\n", i, stacks[i]);
    }
    return 0;
}
 
static void _main_end(void);
int main(int argc, char *argv[]);
 
void parse_config(void)
{
    print_pc(main, _main_end);
}
   
void signal_interrupt(int signo)
{
    print_pc(main, _main_end);
}
 
int main(int argc, char *argv[])
{
    signal(SIGSEGV, signal_interrupt);
   
    parse_config();
    if (argc > 1)
    {
       int *a = 0;
       *a = 0;
    }
    return 0;
}

static void _main_end(void)
{
}