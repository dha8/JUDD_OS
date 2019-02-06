#include "lib.h"
#include "pit.h"
#include "paging.h"
#include "keyboard.h"
#include "i8259.h"
/* num of current terminal(0,1,2) */
uint8_t terminal_num = 0;
terminal_t terminal_arr[NUM_TERMINALS];

/* pit_init
 *   DESCRIPTION: initialize & enable PIT to 50Hz default
 *   INPUT: none
 *   OUTPUT: none
 */
void pit_init(void){

	terminal_num = 0;
  enable_irq(PIT_IRQ);

	int16_t value = INPUT_CLOCK_HZ * FREQ / MILLI_INV;
	int8_t highbyte = (int8_t)(value >> MASK_HIGH);
	int8_t lowbyte = (int8_t)(value & 0x00FF);
  outb(RATE_GERNERATOR,COMMAND_REG);  /* Set our command word for rate generator */
  outb(lowbyte,CHANNEL_0);   /* Set low byte of divisor */
  outb(highbyte,CHANNEL_0);     /* Set high byte of divisor */
}


/* pit_handler
 *   DESCRIPTION: handler for when interrupt comes in; switch the process to that of 
 *                next terminal; in a roundrobin manner
 *   INPUT: none
 *   OUTPUT: none
 */
void pit_handler(void){
	send_eoi(PIT_IRQ);
	/* set ptr to curr and next terminals */
  terminal_t * curr_terminal = &(terminal_arr[terminal_num]);
	terminal_num = (terminal_num + 1) % NUM_TERMINALS;
  terminal_t * next_terminal = &(terminal_arr[terminal_num]);

	/* save current ebp and esp0 */
  asm volatile(
    "movl %%ebp, %0"
    :"=r"(curr_terminal->ebp)
      );
	curr_terminal->esp0 = tss.esp0;

	/* next terminal should always be on; if not, turn it on */
  if(next_terminal->active == OFF){
		execute((const uint8_t*)"shell\n");  // execute next terminal
  }

	/* set paging for upcoming process(move onto processes in next terminal) */
	set_process_memory(next_terminal->most_recent_pcb->pid);

	/* store current terminal's esp0 into tss */
  tss.esp0 = next_terminal->esp0;

	/* set actual ebp to next terminal's ebp, to move to next terminal*/
  asm volatile(
			"movl %0, %%ebp;"
			"leave;"
			"ret;"
			:
			:
			"r"(next_terminal->ebp)
			);

}

/* terminal_init
 *   DESCRIPTION: initializes global terminal structs to swap between them
 *	 INPUT: none
 *   OUTPUT: 0 for success, -1 for fail
 */
void terminal_init(void){
	int i;
	terminal_num_display = 0;
	for(i=0;i<NUM_TERMINALS;++i){
		terminal_arr[i].active = OFF;
		terminal_arr[i].vidmem_addr = (char *)VIRTUAL_VIDEO_ADDRESS + i * SIZE_OF_ENTRY;
	}
	set_up_virtual_to_video(terminal_num_display);
	fill_terminal_attribute();
}


/* fill_terminal_attribute
 *   DESCRIPTION: empty out(clear) newly created terminals video memory(buffers)
 *   INPUT: none
 *   OUTPUT: none
 */
void fill_terminal_attribute(void){
	int i;
	int j;
	for(i=1; i < NUM_TERMINALS;i++){
		for(j = 0;j < NUM_ROWS * NUM_COLS; j ++){
			*(uint8_t *)(terminal_arr[i].vidmem_addr + (j << 1)) = ' ';
			*(uint8_t *)(terminal_arr[i].vidmem_addr + (j << 1) + 1) = ATTRIB;
		}
	}
}
