

#include "syscalls.h"

#include "lib.h"
#include "types.h"
#include "filesystem.h"
#include "paging.h"
#include "rtc.h"
#include "pit.h"
#include "keyboard.h"
#include "isr_wrapper.h"
#include "x86_desc.h"

/* file-scope variables used as buffers mostly, to pass info between the functions/steps of execute */
const uint8_t* command_buf;
int i, size_of_args;
uint32_t command_len;
uint8_t available_pid;
/* array of pid status */
static uint8_t pid_bits[MAX_PROCESS_NUM];
/* predefined function operations table */
fops_table file_ftable = {(open_t)file_open, (close_t)file_close, (read_t)file_read, (write_t)file_write};
fops_table dir_ftable = {(open_t)dir_open, (close_t)dir_close, (read_t)dir_read, (write_t)dir_write};
fops_table rtc_ftable = {(open_t)rtc_open, (close_t)rtc_close, (read_t)rtc_read, (write_t)rtc_write};
fops_table stdin_ftable = {NULL, NULL, (read_t)terminal_read, NULL};
fops_table stdout_ftable = {NULL, NULL, NULL, (write_t)terminal_write};


/* get_curr_pcb
 *   DESCRIPTION: get the current pcb of process
 *   INPUT: none
 *	 OUTPUT: current pcb address in the memory
 *	 SIDE EFFECTS: changes the data of esp
 */
void* get_curr_pcb(void){

	// fetch current value of esp
	uint32_t esp;
  asm (
    "movl %%esp, %%eax"
    : "=a" (esp)
  );

	// current pcb resides within current process block, but above
	return (void*)(esp & PCB_CALC_OFFSET);
}

/* halt
 *   DESCRIPTION: halt gets rid of the process from the memory except for the first shell
 *   INPUT: status : tells if the halt has executed successfully
 *	 OUTPUT: -1 if unsuccessful
 *           0, or other passed "status" if successful
 *	 SIDE EFFECTS: erases paging for the process that is halted, goes back to the parent's page
 */
int32_t halt(uint8_t status){

	/* step 1: restore parent data */
	pcb_t * current_pcb = get_curr_pcb();
	pcb_t * parent_pcb = (pcb_t*)(current_pcb->parent);
	tss.esp0 = current_pcb->esp0;

	// if closing from root shell, clear everything and start a new shell
	if(parent_pcb == NULL) {
		tss.esp0 = KERNEL_STACK_START - KERNEL_STACK_SIZE * (current_pcb->pid) - sizeof(void * );

		// retrieve eflags and set IF = 1
		uint32_t eflags = 0;
		asm volatile(
				"pushfl;"
				"popl %0;"
				:"=r"(eflags)
				);

		eflags = eflags | EFLAGS_IF_MASK; //set IF = 1

		asm volatile(
        "movl %0, %%ds;"
        "pushl %1;"
        "pushl %2;"
        "pushl %3;"
        "pushl %4;"
        "pushl %5;"
        :
        :
        "r" (USER_DS),
        "r" (USER_DS),
        "r" (current_pcb->esp),
        "r" (eflags),
        "r" (USER_CS),
        "r" (current_pcb->eip)
    );
		asm volatile("iret;");

	}


	/* step 2: restore parent paging */
	pid_bits[current_pcb->pid] = 0; // set pid to available
	free_process_memory(current_pcb->pid); // free current process page
	set_process_memory(parent_pcb->pid);	 // set current page to parent's

	terminal_arr[terminal_num].most_recent_pcb = parent_pcb;

	/* step 3: close any relevant FD's */
	int i;
  for(i=0; i<FD_ARRAY_SIZE; ++i){
    current_pcb->fd_array[i].fxn_tbl_ptr =  NULL;
    current_pcb->fd_array[i].inode = 0;
    current_pcb->fd_array[i].file_pos = 0;
    current_pcb->fd_array[i].flags = 0;
  }

	/* step 4: Jump to Execute Return */
	uint32_t new_status = status;
	asm volatile(
			"movl %0, %%ebp;"
			"movl %1, %%eax;"
			"leave;"
			"ret;"
			:
			:
			"r"(current_pcb->ebp),
			"r"(new_status)
			);


	// never runs
  return -1;
}

/* execute
 *   DESCRIPTION: executes the executable file given the filename
 *   INPUT: command: filename of the executable and the arguments of the command
 *	 OUTPUT: -1 if unsuccessful
 *           none if successful
 *	 SIDE EFFECTS: sets up paging for the executable file of the command
 *                 loads the file into memory (!28 MB for virtual)
 *                 Creates PCB in the kernel stack,
 *                 Opens File directory if there are space in the file directory
 *                 push stack pointers for context switch
 */
int32_t execute(const uint8_t* command){
	cli();

	command_buf = command;
	command_len = strlen((const int8_t*)command_buf);
	uint8_t fname[command_len+1]; // name of the executable
	uint8_t args[command_len+1];
	memset(fname,'\0',command_len+1); // clear buf
	memset(args,'\0',command_len+1); // clear buf

  /* step 1. parse arguments */
	/* step 2. check to see if valid executable */
  /* step 3. set pid bit and allocate page */
	if(execute_setup(args, fname) == -1) return -1;

  /* step 4. load the filedata into the allocated page */
	// read file content into corresponding phys addr
	if(read_data(opened_file.inode_num, 0, (uint8_t*)FILE_LOCATION, PAGE_SIZE)==-1)
		return -1;

  /* step 5. create pcb and populate it */
	pcb_t* pcb_new = (pcb_t*)(KERNEL_STACK_START - KERNEL_STACK_SIZE * (available_pid + 1));
	execute_fillpcb(pcb_new);
	if(terminal_arr[terminal_num].active == OFF){
		terminal_arr[terminal_num].active = ON;
		pcb_new->parent = NULL;
		pcb_new->tid = terminal_num;
	}
	else{
		pcb_new->parent = (pcb_t*)get_curr_pcb();
		pcb_new->tid = pcb_new->parent->tid;
	}
	terminal_arr[terminal_num].most_recent_pcb = pcb_new;

	// set args
	strcpy((int8_t*)pcb_new->args, (int8_t*)&(args));
	pcb_new->args_size = size_of_args;

  /* set bookkeeping info: parent, esp0, ss0, esp, ebp, eip and args */
  //save ebp
  asm volatile(
    "movl %%ebp, %0"
    : "=r"(pcb_new->ebp)
  );

	// set tss values; note: tss is always current
	pcb_new->esp0 = tss.esp0; // saving old esp0 into our new pcb
	tss.esp0 = KERNEL_STACK_START - KERNEL_STACK_SIZE * (pcb_new->pid) - sizeof(void * ); //
  pcb_new->esp = VIRTUAL_ADDR_START + USER_SPACE_SIZE - sizeof(void *);
  // set executable code's eip to pcb's eip
  read_data(opened_file.inode_num, EIP_ADDR_OFFSET, (uint8_t*)(&(pcb_new->eip)), EIP_ADDR_SIZE); // 24~27B holds eip

  /* step 6. context switch */
	execute_cswitch(pcb_new);

	return 0; // does nothing, always passes
}

/* read
 *   DESCRIPTION: general system call read that maps to different read functions
 *								depending on the filetype
 *   INPUT: fd - file descriptor entry number
 *          buf - screen buffer that is read as a result of function
 *          nbytes - number of bytes to read
 *	 OUTPUT: ret: -1 if unsuceesful
 *                number of bytes read if succesful
 *	 SIDE EFFECTS: change the buffer data with what is read
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){

  pcb_t* current_pcb = get_curr_pcb(); // replace function by getting current_pcb function

  int32_t ret;

	/* if fd is invalid number, or stdout, or if the file doesn't exist
	 * read is unsucceesful */
  if (fd < 0 || fd >= FD_ARRAY_SIZE || fd == 1 || current_pcb->fd_array[fd].flags == 0){
    return -1;
  }
  /* read == number of bytes if successful, -1 is unsucessful */

  ret = current_pcb->fd_array[fd].fxn_tbl_ptr->read(fd, buf, nbytes);
  return ret;
}

/* write
 *   DESCRIPTION: general system call write that maps to different read functions
 *								depending on the filetype
 *   INPUT: fd - file descriptor entry number
 *          buf - screen buffer that is to be written as a result of function
 *          nbytes - number of bytes to write
 *	 OUTPUT: ret: -1 if unsucceesful
 *                0  if successful
 *	 SIDE EFFECTS: change the buffer data with what is read
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){

  pcb_t* current_pcb = get_curr_pcb(); // replace function by getting current_pcb function

  int32_t ret;

	/* if fd is invalid number, or stdin, or if the file doesn't exist
	 * write is unsucceesful */
  if (fd < 0 || fd >= FD_ARRAY_SIZE || fd == 0 || current_pcb->fd_array[fd].flags == 0){
    return -1;
  }

  /* check if something is other than terminal write or rtc write */
  // insert code for the check

  /* ret == 0 if successful, -1 if unsuceesful */
  ret = current_pcb->fd_array[fd].fxn_tbl_ptr->write(fd, buf, nbytes);

  return ret;
}

/* open
 *   DESCRIPTION: opens the file and puts info about it into file descriptor using given the filename
 *   INPUT: filename - name of the file to open
 *	 OUTPUT: ret: -1 if unsucceesful
 *                fd (file descriptor entry number) if successful
 *	 SIDE EFFECTS: write to the file descriptor entry
 */
int32_t open(const uint8_t* filename){
  /* same thing as one line below */
  // int32_t pid = function(); // get current pid, replace function() with get current pid function
  // pcb_t* current_pcb = EIGHT_MEGA - FOUR_KILO*pid - 4;

	/* if file name is invalid, open is unsuceesful */
	if(filename == NULL || filename[0] == '\0' || filename[0] == ' ' ||
		 filename == ((uint8_t*)""))
		return -1;

  pcb_t* current_pcb = get_curr_pcb(); // replace function by getting current_pcb function

	/* dentry to load */
  dentry_t current_dentry;

	/* read the entry by name */
  if (read_dentry_by_name(filename, &current_dentry) == -1){
    return -1;
  }

  int i; // iterator

  /* find empty file */
  for (i = FIRST_AVAILABLE_FD; i < FD_ARRAY_SIZE; i ++){
    if (current_pcb->fd_array[i].flags == 0){ // empty file
      break;
    }
  }

  if (i == FD_ARRAY_SIZE) return -1; // if the files are full, return -1

	/* different fops pointer depending on the filetype
	 * filetype 0: rtc
	 *          1: directory
	 *          2: file */
  if (current_dentry.filetype == 0){
    current_pcb->fd_array[i].fxn_tbl_ptr = &rtc_ftable;
    current_pcb->fd_array[i].inode = 0; // rtc doesn't have inode
    current_pcb->fd_array[i].file_pos = 0; // file position not specified yet
    current_pcb->fd_array[i].flags = 1; // the file descriptor entry is occupied
		current_pcb->fd_array[i].fxn_tbl_ptr->open(filename);
  }
  else if (current_dentry.filetype == 1){
    current_pcb->fd_array[i].fxn_tbl_ptr = &dir_ftable;
    current_pcb->fd_array[i].inode = 0; // directory doesn't have inode
    current_pcb->fd_array[i].file_pos = 0; // file position not specified yet
    current_pcb->fd_array[i].flags = 1; // the file descriptor entry is occupied
  }
  else if (current_dentry.filetype == 2){
		int size_of_dentry_filename = 0;
		while(size_of_dentry_filename < FILENAME_SIZE){
			if(current_dentry.filename[size_of_dentry_filename] == '\0'){
				break;
			}
			size_of_dentry_filename = size_of_dentry_filename + 1;
		}

		int size_of_filename = 0;
		while(size_of_filename < FILENAME_SIZE+1){
			if(filename[size_of_filename] == '\0'){
				break;
			}
			size_of_filename = size_of_filename + 1;
		}

		if(size_of_filename != size_of_dentry_filename) return -1;

		if(strncmp((int8_t*)current_dentry.filename,(int8_t*)filename,size_of_filename) !=0) return -1; // null check

    current_pcb->fd_array[i].fxn_tbl_ptr = &file_ftable; // file doesn't have inode
    current_pcb->fd_array[i].inode = current_dentry.inode_num; // file has inode number
    current_pcb->fd_array[i].file_pos = 0; // file position not specified yet
    current_pcb->fd_array[i].flags = 1; // the file descriptor entry is occupied
  }
  else{
    return -1; // if filetype is invalid, read is unsucessful.
  }

  return i; // return fd number
}

/* close
 *   DESCRIPTION: get rid of the file in the file descriptor
 *   INPUT: fd - index of filedescriptor to close
 *	 OUTPUT: ret: -1 if unsucceesful
 *                0 if successful
 *	 SIDE EFFECTS: changes the flags variable in the file descriptor entry
 */
int32_t close(int32_t fd){

  if (fd < 2 || fd >= FD_ARRAY_SIZE){
    return -1; // if trying to close default descriptors or invalid descriptiors, fail
  }

  pcb_t* current_pcb = get_curr_pcb(); // replace function by getting current_pcb function

	if(current_pcb->fd_array[fd].flags == 0){	//means it is unopened
		return -1;
	}
  current_pcb->fd_array[fd].flags = 0; // file decriptor is now free to be occupied

  return 0; // close was succesful
}

/* getargs
 *   DESCRIPTION: copy contents of args stored in each process to specified buf
 *   INPUT:  buf - buffer to copy the args to
 *   				 nbytes - number of bytes in args to copy
 *	 OUTPUT: ret - 0 indicates success, -1 indicates fail
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
	/* pcb saves the args when new process is created; thus, fetch it */
	pcb_t* current_pcb = get_curr_pcb();

	/* if no args or args is longer than required, fail it */
	if(current_pcb->args_size == 0 || current_pcb->args_size > nbytes)
  	return -1;
	else{
		/* copy the args into buf */
		strncpy((int8_t*)buf,(int8_t*)(current_pcb->args),nbytes);
		return 0;
	}
}

/* vidmap
 *   DESCRIPTION: copies the addr of video mmr to screen_start
 *   INPUT:  screen_start - ptr to addr of video mmr to be copied into
 *	 OUTPUT: -1 indicates fail, otherwise holds the virtual video address
 */
int32_t vidmap(uint8_t** screen_start){
	int32_t start_address = (int32_t)(screen_start);

	/* ensure that the start addr is in 128~132 MB, aka correct pg*/
	if (start_address < FILE_LOCATION || start_address >= (FILE_LOCATION + PAGE_SIZE)){
		return -1;	//check that given address of screen_start is located in correct page
	}


	/* VIRTUAL_VIDEO_ADDRESS already holds needed virtual addr */
	//int32_t virtual_address = VIRTUAL_VIDEO_ADDRESS;
	uint8_t * virtual_address = (uint8_t *)(terminal_arr[((pcb_t *)get_curr_pcb())->tid].vidmem_addr);
	/* copy the addr into screen_start */
	*screen_start = virtual_address;
  return (int32_t)virtual_address;
}


/* Function implemented for signaling for extra credit, but not implemented */
int32_t set_handler(int32_t signum, void* handler_address){
  return -1;
}

/* Function implemented for signaling for extra credit, but not implemented */
int32_t sigreturn(void){
  return -1;
}





/* ///EXECUTE HELPER FUNCTIONS/// */

/* execute_setup
 *   DESCRIPTION: execute helper function, does steps 1 2 and 3 for error checking, command parsing, and exe verification, and page setup
 *   INPUT: the arguments and function name to initialize
 *	 OUTPUT: -1 if unsuccessful
 *           0, if successful
 *	 SIDE EFFECTS: edits the args and fname, populates the arrays
 */
int32_t execute_setup(uint8_t* args, uint8_t* fname){
	// check if valid start string
  if(command_buf == NULL || command_buf[0] == '\0' || command_buf[0] == ' ')
    return -1;

  i = 0;
	size_of_args = 0;


  // get first word of typed command
  while(1){
     if(i == command_len || command_buf[i] == '\0' ||
            command_buf[i] == '\n' || command_buf[i] == '\r') break;
		if(command_buf[i] == ' '){
			if(i >= FILENAME_SIZE) return -1;						  // no file name is greater than 32 in length
			i = i + 1;
			while(1){
				if(i == command_len || command_buf[i] == '\0' ||
	             command_buf[i] == '\n' || command_buf[i] == '\r') break;
				args[size_of_args] = command_buf[i];
				++size_of_args;
				++i;
			}
			break;
		}
     fname[i] = command_buf[i];
     ++i;
  }

	uint8_t exe_check[EXE_CHECK_BYTENUM];
	file_open(fname);

	// open first 4B of filereturn -
	if(read_data(opened_file.inode_num,0,exe_check,EXE_CHECK_BYTENUM) == -1) return -1;

	// check if executable magic bytes
	if(exe_check[0] != exe_B0 || exe_check[1] != exe_B1 ||
		 exe_check[2] != exe_B2 || exe_check[3] != exe_B3)
		 return -1;

	i = 0;
	while(i < MAX_PROCESS_NUM && pid_bits[i] == 1) i++;

	// if max process reached, ret -1
	if(i == MAX_PROCESS_NUM) return -1;
	available_pid = i;

	pid_bits[available_pid] = 1;
	// otherwise, allocate page
	set_process_memory(available_pid);

	//assume the step passed
	return 0;
}

/* execute_fillpcb
 *   DESCRIPTION: execute helper function, does part of the pcb step 4 to fill the passed in pcb
 *   INPUT: the pointer to the new pcb to fill
 *	 OUTPUT: -1 if unsuccessful
 *           0, if successful (always)
 *	 SIDE EFFECTS: edits the new passed in pcb by filling it's values using the get_cur_pcb funciton mostly
 */
void execute_fillpcb(pcb_t* pcb_new){
	// set parent
	if(available_pid == 0) pcb_new->parent = NULL;
	else{
		pcb_new -> parent = (pcb_t*)get_curr_pcb();
	}

	// create fd entries
	fd_t fd_stdin = {&stdin_ftable, 0, 0, 1};
	fd_t fd_stdout = {&stdout_ftable, 0, 0, 1};

	// fill pcb
	pcb_new->pid = available_pid;
	pcb_new->fd_array[0] = fd_stdin;
	pcb_new->fd_array[1] = fd_stdout;
	for(i=FIRST_AVAILABLE_FD;i<FD_ARRAY_SIZE;++i){
		pcb_new->fd_array[i].fxn_tbl_ptr =  NULL;
		pcb_new->fd_array[i].inode = 0;
		pcb_new->fd_array[i].file_pos = 0;
		pcb_new->fd_array[i].flags = 0;
	}
}

/* execute_cswitch
 *   DESCRIPTION: execute helper function, does step 6 which manages context switching to the current passed in new pcb, then step 7 iret's
 *   INPUT: pointer to the new pcb to context switch to
 *	 OUTPUT: -1 if unsuccessful
 *           0, if successful (always)
 *	 SIDE EFFECTS: edits eflags and pcb data then sets up stack for an iret to return to userspace, breaking control
 */
void execute_cswitch(pcb_t* pcb_new){
	// retrieve eflags and set IF = 1, for sys_halt
  uint32_t eflags = 0; // arbitrary value for now
	asm volatile(
			"pushfl;"
			"popl %0;"
			:"=r"(eflags)
			);

	eflags = eflags | EFLAGS_IF_MASK; //set IF = 1

  asm volatile(
    "movl %0, %%ds;"
    "pushl %1;"
    "pushl %2;"
    "pushl %3;"
    "pushl %4;"
    "pushl %5;"
		:
    :"r"(USER_DS),
     "r"(USER_DS),
     "r"(pcb_new->esp),
     "r"(eflags),
     "r"(USER_CS),
     "r"(pcb_new->eip)
  		);

  /* step 7. IRET, go to userspace */
  asm volatile("iret;");
}
