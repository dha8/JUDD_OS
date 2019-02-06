#ifndef PIT_H
#define PIT_H

#include "syscalls.h"
#include "keyboard.h"

#define TEN_MILLISEC_HZ 			 100
#define FITFY_MILLISEC_HZ 			20
#define TWENTY_FIVE_MILLISEC_HZ 			60
#define INPUT_CLOCK_HZ 		 1193182
#define FREQ 										50
#define MILLI_INV							1000

#define RATE_GERNERATOR 			0x34
#define COMMAND_REG 					0x43
#define CHANNEL_0 						0x40
#define MASK_HIGH								 8

#define PIT_IRQ 							0x00
#define NUM_TERMINALS						 3
#define OFF											 0
#define ON											 1

extern uint8_t terminal_num;

/* struct to hold necessary info for each terminal */
typedef struct {

 	/* bookkeeping info for jumping between terminals */
	pcb_t * most_recent_pcb;
	uint32_t ebp;
	uint32_t esp0;

	/* saving keyboard cursor pos and buffer */
	uint8_t cursor_x;
	uint8_t cursor_y;
	uint8_t screen_buf[SCREEN_BUF_SIZE]; // 128 sized buf
	uint8_t screen_buf_top; // pos in screen buf

	/* buffer holding video mmr */
	char * vidmem_addr; // ptr to 4KB pg holding vidmem for each terminal

	uint8_t active;

} terminal_t;

extern terminal_t terminal_arr[NUM_TERMINALS];

/* initialize pit */
void pit_init(void);

/* handler for pit; should run every time interval */
void pit_handler(void);

void terminal_init(void);

int16_t milliseconds_to_PIT_val(int32_t ms);

void fill_terminal_attribute(void);


#endif
