
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"
#include "lib.h"
#include "types.h"
#include "x86_desc.h"

#include "terminal.h"
#include "filesystem.h"
#include "paging.h"
#include "rtc.h"

/* First four bytes to check if executable */
#define exe_B0   0x7f
#define exe_B1   0x45
#define exe_B2   0x4c
#define exe_B3   0x46

#define KERNEL_STACK_START 0x00800000

#define KERNEL_STACK_SIZE      0x2000

#define VIRTUAL_ADDR_START 0x08000000

#define PAGE_SIZE          0x00400000   //4 MB

#define EIP_ADDR_OFFSET            24
#define EIP_ADDR_SIZE               4

#define MAX_PROCESS_NUM             6
#define EXE_CHECK_BYTENUM           4

#define FILE_LOCATION      0x08048000 // 128MB in virtual mmr; but
                                     // actual physical addr set by pid
#define VIRTUAL_VIDEO_ADDRESS 0x08500000

#define PCB_CALC_OFFSET 	0xFFFFE000 // calculate addr of current pcb

#define EFLAGS_IF_MASK		0x00000020 // mask to set EFLAG's IF to 1 for STI
#define BUF_SIZE								 128
#define FD_ARRAY_SIZE							 8
#define FIRST_AVAILABLE_FD         2

/* Structures regarding pcb below */

/* generic function type casting for system calls to be in jumptable */
typedef uint32_t (*open_t)(const uint8_t*);
typedef uint32_t (*close_t)(int32_t);
typedef uint32_t (*read_t)(int32_t, void*, int32_t);
typedef uint32_t (*write_t)(int32_t, const void*, int32_t);

/* file operations table pointer; jump table to diff syscalls */
typedef struct{
    open_t open;
    close_t close;
    read_t read;
    write_t write;
}fops_table;


/* file descriptor table */
typedef struct{
    fops_table * fxn_tbl_ptr;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
}fd_t;



/* Process Control Block, keeps track of processes/files opened */
typedef struct pcb_t{

  struct pcb_t* parent;

	uint8_t pid; // holds pid of this process

	uint8_t tid; // holds terminal id (0,1,2)

	uint32_t esp0; // esp0 of old process(return point), needed for halt;
								 // after halt, the esp needs to point to the original
								 // esp(base of the new kernel stack) bc that's where
								 // new process will begin.
								 // On the other hand, tss.esp0 holds the new esp0

	/* info on current process. used for context switching */
	uint32_t ebp;
	uint32_t eip;
	uint32_t esp;
	fd_t fd_array[FD_ARRAY_SIZE];   // holds file descriptor tables
	uint8_t args[BUF_SIZE];  // holds the args
	int args_size;

}pcb_t;

/* helper functions */

void* get_curr_pcb(void); /* gets the addr of current pcb based on current esp */
/*execute's helper subroutines/steps*/
int32_t execute_setup(uint8_t* args, uint8_t* fname); //completes steps 1 2 and 3, of null checking, parsing the command, and allocating the new page
void execute_fillpcb(pcb_t* pcb_new); //fills the input pointer pcb with the values, mostly gathered from get_curr_pcb helper
void execute_cswitch(pcb_t* pcb_new); /* executes context switching based off of the new pcb sent to it */


/* system call declarations */

/*The halt system call terminates a process, returning the specified value to its parent process.*/
int32_t halt(uint8_t status);

/*The execute system call attempts to load and execute a new program, handing off the processor to the new program
until it terminates. The command is a space-separated sequence of words. The first word is the file name of the
program to be executed, and the rest of the command—stripped of leading spaces—should be provided to the new
program on request via the getargs system call. The execute call returns -1 if the command cannot be executed*/
int32_t execute(const uint8_t* command); //helpers listed below

/*The read system call reads data from the keyboard, a file, device (RTC), or directory. This call returns the number
of bytes read. If the initial file position is at or beyond the end of file, 0 shall be returned (for normal files and the
directory). Uses a jump table referenced by the task’s file array to call from a generic handler for this call into a
file-type-specific function.*/
int32_t read(int32_t fd, void* buf, int32_t nbytes);

/*The write system call writes data to the terminal or to a device (RTC). In the case of the terminal, all data should
be displayed to the screen immediately.  RTC: the system call should always accept only a 4-byte
integer specifying the interrupt rate in Hz, and should set the rate of periodic interrupts accordingly. FILES:
return -1 to indicate failure since the file system is read-only. The call returns the number of bytes written, or -1 on failure.*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes);

/*The open system call provides access to the file system. The call should find the directory entry corresponding to the
named file, allocate an unused file descriptor, and set up any data necessary to handle the given type of file (directory,
RTC device, or regular file). If the named file does not exist or no descriptors are free, the call returns -1.*/
int32_t open(const uint8_t* filename);

/*The close system call closes the specified file descriptor and makes it available for return from later calls to open.
You should not allow the user to close the default descriptors (0 for input and 1 for output). Trying to close an invalid
descriptor should result in a return value of -1; successful closes should return 0.*/
int32_t close(int32_t fd);

/*The getargs call reads the program’s command line arguments into a user-level buffer.  Obviously, these arguments
must be stored as part of the task data when a new program is loaded. Here they are merely copied into user space. If
there are no arguments, or if the arguments and a terminal NULL (0-byte) do not fit in the buffer, simply return -1. The
shell does not request arguments, but TSS still initialized */
int32_t getargs(uint8_t* buf, int32_t nbytes);

/*The vidmap call maps the text-mode video memory into user space at a pre-set virtual address. Although the address
returned is always the same, it should be written into the memory location provided by the caller. If the location is invalid, the call should
return -1. To avoid adding kernel-side exception handling for this sort of check, we simply check whether the address falls
within the address range covered by the single user-level page.*/
int32_t vidmap(uint8_t** screen_start);

/*Even if our operating system does not support signals, we support these system calls; in such a case, however,
we immediately return failure from these calls.*/
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);



#endif /* SYSCALLS_H */
