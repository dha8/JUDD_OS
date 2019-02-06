#include "paging.h"
#include "lib.h"
#include "pit.h"
#include "syscalls.h"

#define VID_MEM_OFFSET 0xb8

/* init_paging
 *   DESCRIPTION: initialize paging for the initial boot
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: initializes paging and enables pages needed for initial set up
 */
void init_paging(){

  int i; //iterator

  for (i = 0; i < NUM_PTE; i++){
    /* initialize page directory */
    Page_Directory_Entry[i].val = READ_WRITE_BIT; // only read_write is 1

    /* initialize page table */
    if (i == VID_MEM_OFFSET){ // video memory
      Page_Table_Entry[i].val = (i * ADDR_START_OFFSET) | READ_WRITE_BIT | PRESENT_BIT; //the address starts at 13th bit (0x1000), read/write = 1, present = 1, supervisor = 0;
    }
    else { // everything else
      Page_Table_Entry[i].val = (i * ADDR_START_OFFSET) | READ_WRITE_BIT; //the address starts at 13th bit (0x1000), read/write = 1, present = 0, supervisor = 0;
    }
  }
  Page_Table_Entry_For_Video[i].val = READ_WRITE_BIT;
  /* connecting page directory with page table */
  // the first pde is present
  Page_Directory_Entry[0].present = 1; //attributes : supervisor, read/write, present
  Page_Directory_Entry[0].read_write = 1; //attributes : supervisor, read/write, present
  Page_Directory_Entry[0].page_table_addr = ((unsigned int)Page_Table_Entry >> ALIGN); // 4KB aligned

  // the second pde at 4MB, attributes: supervisor, read/write, present
  Page_Directory_Entry[1].present = 1; //attributes : supervisor, read/write, present
  Page_Directory_Entry[1].read_write = 1; //attributes : supervisor, read/write, present
  Page_Directory_Entry[1].page_size = 1; // 4MB size page
  Page_Directory_Entry[1].page_table_addr = (KERNEL_SPACE_OFFSET >> ALIGN); // 4KB aligned

  // set up paging (connect cr3 with PDE[0])
  asm volatile(
                 "movl %0, %%eax;"
                 "movl %%eax, %%cr3;"
                 "movl %%cr4, %%eax;"
                 "orl $0x00000010, %%eax;"
                 "movl %%eax, %%cr4;"
                 "movl %%cr0, %%eax;"
                 "orl $0x80000000, %%eax;"
                 "movl %%eax, %%cr0;"
                 :                      /* no outputs */
                 :"r"(Page_Directory_Entry)    /* input */
                 :"%eax"                /* clobbered register */
               );

}

/* set_process_memory
 *   DESCRIPTION: sets up extended paging (w/o Page Table) for the process
 *   INPUT: PID - process ID number to open
 *	 OUTPUT: none
 *	 SIDE EFFECTS: enables paging in the memory space in user space depending on the PID
 */
void set_process_memory(uint32_t PID){

  /* 32 reprsents 128MB in the RAM */
  Page_Directory_Entry[USER_VIRTUAL_ADDR].present = 1; // make a page
  Page_Directory_Entry[USER_VIRTUAL_ADDR].read_write = 1; // the page is read/write
  Page_Directory_Entry[USER_VIRTUAL_ADDR].user_supervisor = 1; //can be accessed by the user program
  Page_Directory_Entry[USER_VIRTUAL_ADDR].page_size = 1; // 4MB size page without page table

  Page_Directory_Entry[USER_VIRTUAL_ADDR].page_table_addr = (USER_SPACE_OFFSET >> ALIGN) + PID*(USER_SPACE_SIZE >> ALIGN); // 4KB aligned (right shift by 4 kB)

  /* flush TLB */
  asm volatile(
                "movl %0, %%eax;"
                "movl %%eax, %%cr3;"
                :                      /* no outputs */
                :"r"(Page_Directory_Entry)    /* input */
                :"%eax"                /* clobbered register */
  );
}

/* free_process_memory
 *   DESCRIPTION: gets rid of the extended page of selected process
 *   INPUT: PID - process ID number to close
 *	 OUTPUT: none
 *	 SIDE EFFECTS: disables paging in the memory space in user space depending on the PID
 */
void free_process_memory(uint32_t PID){

  /* 32 reprsents 128MB in the RAM */
  Page_Directory_Entry[USER_VIRTUAL_ADDR].present = 0; // erase a page
  Page_Directory_Entry[USER_VIRTUAL_ADDR].read_write = 0; // the page is read/write
  Page_Directory_Entry[USER_VIRTUAL_ADDR].user_supervisor = 0; //can be accessed by the user program
  Page_Directory_Entry[USER_VIRTUAL_ADDR].page_size = 0; // 4MB size page without page table

  Page_Directory_Entry[USER_VIRTUAL_ADDR].page_table_addr = (USER_SPACE_OFFSET >> ALIGN) + PID*(USER_SPACE_SIZE >> ALIGN); // 4KB aligned (right shift by 4 kB)

  /* flush TLB */
  asm volatile(
                "movl %0, %%eax;"
                "movl %%eax, %%cr3;"
                :                      /* no outputs */
                :"r"(Page_Directory_Entry)    /* input */
                :"%eax"                /* clobbered register */
  );
}


/* set_up_virtual_to_video 
 *   DESCRIPTION: maps the terminal video virtual addr to correct video buffer / video memory
 *                in detail, maps the specified terminal to phys. vid mmr, and others to vid buf
 *                in memory
 *   INPUT: tid; terminal id to map to physical vid mmr to
 *	 OUTPUT: none
 */
void set_up_virtual_to_video(uint8_t tid){

  // get tid of next two terminals
	uint8_t buf_1_tid = ((tid + 1) % NUM_TERMINALS);
	uint8_t buf_2_tid = ((tid + 2) % NUM_TERMINALS);

  // get the virtual addr of terminal video memory
	int32_t v_addr_to_video = (int32_t)(terminal_arr[tid].vidmem_addr); // 133+tid*4
	int32_t v_addr_to_buf_1 = (int32_t)(terminal_arr[buf_1_tid].vidmem_addr);
	int32_t v_addr_to_buf_2 = (int32_t)(terminal_arr[buf_2_tid].vidmem_addr);

  //Set up the corresponding virtual address's page directory to 4kb page
  int dir_ent = (v_addr_to_video >> DENTRY_SHIFT_OFFSET) & MASK_D_P;

  Page_Directory_Entry[dir_ent].present = 1;          // create a page
  Page_Directory_Entry[dir_ent].read_write = 1;       // the page is read/write
  Page_Directory_Entry[dir_ent].user_supervisor = 1;  // can be accessed by the user program
  Page_Directory_Entry[dir_ent].page_size = 0;        // 4kb size page

  //Set up the corresponding virtual address's page table to video memory 4kb
  int page_ent = ((uint32_t)v_addr_to_video >> ALIGN) & MASK_D_P;
  Page_Table_Entry_For_Video[page_ent].val = (VID_MEM_OFFSET * ADDR_START_OFFSET) | READ_WRITE_BIT | PRESENT_BIT;
  Page_Table_Entry_For_Video[page_ent].user_supervisor = 1;

  //Set up the corresponding virtual address's page table to video buf 1 (4kb)
  int page_ent_1 = ((uint32_t)v_addr_to_buf_1 >> ALIGN) & MASK_D_P;
  Page_Table_Entry_For_Video[page_ent_1].val = (VID_BUF_ADDR_BASE + buf_1_tid * ADDR_START_OFFSET) | READ_WRITE_BIT | PRESENT_BIT;
  Page_Table_Entry_For_Video[page_ent_1].user_supervisor = 1;

  //Set up the corresponding virtual address's page table to video buf 2 (4kb)
  int page_ent_2 = ((uint32_t)v_addr_to_buf_2 >> ALIGN) & MASK_D_P;
  Page_Table_Entry_For_Video[page_ent_2].val =  (VID_BUF_ADDR_BASE + buf_2_tid * ADDR_START_OFFSET) | READ_WRITE_BIT | PRESENT_BIT;
  Page_Table_Entry_For_Video[page_ent_2].user_supervisor = 1;

	// Set up corresponding directory entry map to Page_Table_Entry_For_Video
  Page_Directory_Entry[dir_ent].page_table_addr = ((unsigned int)Page_Table_Entry_For_Video >> ALIGN);

  asm volatile(
                "movl %0, %%eax;"
                "movl %%eax, %%cr3;"
                :                      /* no outputs */
                :"r"(Page_Directory_Entry)    /* input */
                :"%eax"                /* clobbered register */
  );
}
