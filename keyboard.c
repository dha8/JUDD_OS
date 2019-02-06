#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "idt.h"
#include "terminal.h"
#include "pit.h"
//mod flags
//unsigned int cursor_x,cursor_y;
unsigned char ctrl_flag, alt_flag, shift_flag, caps_flag; //flags for handling mods
unsigned char buf_flag; //0 = nothing, 1 = clear, 2 = add, 3 = enter registered
unsigned char back_flag;
unsigned char ctrl_flag_L;
//unsigned char screen_buf[SCREEN_BUF_SIZE]; //current screen buffer to display
//unsigned char screen_buf_top; //pointer to cap of array for adding more elements
unsigned char enter_flag;

/* flags for switching between terminals. NOTE: NEED CODE TO SET FLAGS TO 1*/
unsigned char terminal_zero_flag;
unsigned char terminal_one_flag;
unsigned char terminal_two_flag;
uint8_t terminal_num_display;
char* video_mem = (char *)VIDEO;

uint8_t passed_buf[SCREEN_BUF_SIZE];
//unsigned char scan;
//unsigned char ascii;

/* handler conversion_scheme zero: scantable_small */
unsigned int scantable_small[5][69] = {
  { //SHIFT'd
    KEY_UNKNOWN, KEY_ESCAPE, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',	KEY_BACKSPACE,
    KEY_TAB, 'Q',	'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', KEY_RETURN,
    KEY_LCTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
    KEY_LSHIFT, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', KEY_RSHIFT,
    '*', KEY_RALT, ' ',	KEY_CAPSLOCK,	KEY_F1, KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,	KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10
  },
  { //CAPS'd
    KEY_UNKNOWN, KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',	KEY_BACKSPACE,
    KEY_TAB, 'Q',	'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', KEY_RETURN,
    KEY_LCTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
    KEY_LSHIFT, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', KEY_RSHIFT,
    '*', KEY_RALT, ' ',	KEY_CAPSLOCK,	KEY_F1, KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,	KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10
  },
  { //ALT'd (same as NORMAL atm)
    KEY_UNKNOWN, KEY_ESCAPE, DBH, '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',	KEY_BACKSPACE,
    KEY_TAB, 'q',	'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_RETURN,
    KEY_LCTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KEY_LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RSHIFT,
    '*', KEY_RALT, ' ',	KEY_CAPSLOCK,	TF1KEY, TF2KEY,	TF3KEY,	KEY_F4,	KEY_F5,	KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10
  },
  { //CTRL'd (same as NORMAL atm)
    KEY_UNKNOWN, KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',	KEY_BACKSPACE,
    KEY_TAB, 'q',	'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_RETURN,
    KEY_LCTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', CRL, ';', '\'', '`',
    KEY_LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RSHIFT,
    '*', KEY_RALT, ' ',	KEY_CAPSLOCK,	KEY_F1, KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,	KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10
  },
  { //NORMAL
    KEY_UNKNOWN, KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',	KEY_BACKSPACE,
    KEY_TAB, 'q',	'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_RETURN,
    KEY_LCTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KEY_LSHIFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RSHIFT,
    '*', KEY_RALT, ' ',	KEY_CAPSLOCK,	KEY_F1, KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,	KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10
  }
};

/* keyboard_init
 *   DESCRIPTION: initializes the keyboard unit
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: none
 */
void keyboard_init()
{
  enable_irq(KEYBOARD_IRQ);
  ctrl_flag = 0;
  alt_flag = 0;
  shift_flag = 0;
  caps_flag = 0;
  buf_flag = 0;
  back_flag = 0;
  enter_flag = 0;
  init_cursor(); //initialize the cursor
  return;
}

/* init_cursor
 *   DESCRIPTION: initialize cursor variables
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: cursor drawn at the top of the terminal
 */
void init_cursor(){
	int i;
	for(i=0;i<NUM_TERMINALS;++i){
		terminal_arr[i].cursor_x = 0;
		terminal_arr[i].cursor_y = 0;
		update_cursor(terminal_arr[i].cursor_x, terminal_arr[i].cursor_y);
	}
  return;
}

/* translate
 *   DESCRIPTION: translate the keyboard scancode to an ascii value
 *                to print those on the terminal
 *   INPUT: scan - scan code from keyboard
 *   OUTPUT: ascii - ascii value to print out
 *   SIDE EFFECTS: manipulate ctrl_flag, alt_flag, shift_flag, caps_flag
 */
unsigned char translate(unsigned int scan)
{
  uint32_t ascii = 0;
  if(scan == ESC || scan == TAB || scan == CAP || (scan >= KF4 && scan <= KF0) || scan == PGU || scan == PGD ||
	(scan >= NUM && scan <= DOT) || (scan >= INS && scan <= DEL) || scan == PRS ||
  ((scan == KF1 || scan == KF2 || scan == KF3) && alt_flag == 0) )
	{return NULL;}
  else if(scan == CTRL_P) //ctrl press
    ctrl_flag = 1;
  else if(scan == CTRL_R) //ctrl release
    ctrl_flag = 0;
  else if(scan == ALT_P) //alt press
    alt_flag = 1;
  else if(scan == ALT_R) //alt release
    alt_flag = 0;
  else if(scan == SHIL_P || scan == SHIR_P) //shift press
    shift_flag = 1;
  else if(scan == SHIL_R || scan == SHIR_R) //shift release
    shift_flag = 0;
  //toggle keys
  else if(scan == CAPS_R) { //caps release
    if(caps_flag == 1)
      caps_flag = 0;
    else if(caps_flag == 0)
      caps_flag = 1;
  }

  //ignoring all other releases
  else if(scan < GENL_R) {
    if(shift_flag == 1 && caps_flag == 0)
      ascii = scantable_small[0][scan];
    else if(caps_flag == 1 && shift_flag == 0 && ctrl_flag == 0 && alt_flag == 0)
      ascii = scantable_small[1][scan];
    else if(alt_flag == 1)
      ascii = scantable_small[2][scan];
    else if(ctrl_flag == 1)
      ascii = scantable_small[3][scan];
    else if((shift_flag == 0 && caps_flag == 0 && alt_flag == 0 && ctrl_flag == 0) || (caps_flag == 1 && shift_flag == 1))
      ascii = scantable_small[4][scan];

    //backspace erases a character from buffer and screen
    if(ascii == KEY_BACKSPACE){
      back_flag = 1;
      return ascii;
    }
    if(alt_flag == 1 && ascii == TF1KEY){
      // printf("T1");
      terminal_zero_flag = 1;
      return ascii;
    }
    if(alt_flag == 1 && ascii == TF2KEY){
      // printf("T2");
      terminal_one_flag = 1;
      return ascii;
    }
    if(alt_flag == 1 && ascii == TF3KEY){
      // printf("T3");
      terminal_two_flag = 1;
      return ascii;
    }

    //special Ctrl+L command
    if(ascii == CRL && ctrl_flag == 1){
      ctrl_flag_L = 1;
      return ascii;
    }

    //hack to paste buffer
    else if(ascii == DBH && alt_flag == 1){
      uint8_t i = 0;
      //printf("|");
      while(i < terminal_arr[terminal_num_display].screen_buf_top){
        printf("%c", terminal_arr[terminal_num_display].screen_buf[i]);
        i++;
      }
      printf("|");
    }
    //enter code
    else if(ascii == KEY_RETURN)
      enter_flag = 1;
    //add normal key to buffer
    else{
      buf_flag = 3;
    }
    return (unsigned char)ascii;
  }
  return NULL;
}

/* clearbuf
 *   DESCRIPTION: clears the screen buffer
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: erase all the data in the screen buffer
 */
void clearbuf()
{
  uint8_t i;
  for(i = 0; i < SCREEN_BUF_SIZE; i++) // buffer size 128
    terminal_arr[terminal_num_display].screen_buf[i] = '\0';
  terminal_arr[terminal_num_display].screen_buf_top = 0;
}

/* enter_press
 *   DESCRIPTION: update the cursor when enter is pressed
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: writes on the video memory
 */
void enter_press(unsigned char ascii)
{
  terminal_t * check_terminal =  &terminal_arr[terminal_num_display];
  int buf_val = check_terminal->screen_buf_top;
  if(buf_val == 0 || (check_terminal->screen_buf[buf_val - 1] != '\n' && check_terminal->screen_buf[buf_val - 1] != '\r')){
    add2buf(ascii);
    terminal_arr[terminal_num_display].cursor_x = 0;
    ++terminal_arr[terminal_num_display].cursor_y;
    if (terminal_arr[terminal_num_display].cursor_y >= NUM_ROWS){ // update scroll position
      int i;
      for (i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++) {
          /* replace the entire row with next row */
          *(uint8_t *)(video_mem + (i << 1)) = *(uint8_t *)(video_mem + ((i + NUM_COLS) << 1));
          *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
      }
          /* clear the last row */
      for (i = (NUM_ROWS - 1) * NUM_COLS; i < NUM_ROWS * NUM_COLS; i++) {
          *(uint8_t *)(video_mem + (i << 1)) = ' ';
          *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
      }
        terminal_arr[terminal_num_display].cursor_y = NUM_ROWS - 1; // if column number exceeds screen limiit
      }

    //update_cursor(terminal_arr[terminal_num_display].cursor_x, ++terminal_arr[terminal_num_display].cursor_y);

  }
  //clearbuf();
}

/* add2buf
 *   DESCRIPTION: update the cursor when enter is pressed
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: writes on the video memory
 */
void add2buf(unsigned char ascii)
{
  /* adds characters in to the screen buffer */
  int test;
  if(terminal_arr[terminal_num_display].screen_buf_top == (SCREEN_BUF_SIZE-2)){
    test = 1;
  }
  if(terminal_arr[terminal_num_display].screen_buf_top < SCREEN_BUF_SIZE - 1){ // if the buffer is not full, write on it
    putc_addr(ascii,terminal_arr[terminal_num_display].cursor_x,terminal_arr[terminal_num_display].cursor_y);
    terminal_arr[terminal_num_display].screen_buf[terminal_arr[terminal_num_display].screen_buf_top] = ascii;
    terminal_arr[terminal_num_display].screen_buf_top += 1;
  }
  else{
    return;
  }
  if(terminal_arr[terminal_num_display].cursor_x < NUM_COLS - 1)
    update_cursor(++terminal_arr[terminal_num_display].cursor_x, terminal_arr[terminal_num_display].cursor_y); //updates cursor according to columns and rows
  else{
    terminal_arr[terminal_num_display].cursor_x = 0;
    update_cursor(terminal_arr[terminal_num_display].cursor_x, ++terminal_arr[terminal_num_display].cursor_y);
  }
}

/* delete_char_from_buf
 *   DESCRIPTION: deletes a character when backspace pressed
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: updates the cursor,
 */
void delete_char_from_buf(){
  /* delets a character from the screen buffer */
  if(terminal_arr[terminal_num_display].screen_buf_top > 0){
    terminal_arr[terminal_num_display].screen_buf_top -= 1; //update the position
    terminal_arr[terminal_num_display].screen_buf[terminal_arr[terminal_num_display].screen_buf_top] = '\0'; // erase the character
  }
  else{
    return;
  }
  if(terminal_arr[terminal_num_display].cursor_x > 0)
    update_cursor(--terminal_arr[terminal_num_display].cursor_x, terminal_arr[terminal_num_display].cursor_y); //update cursor
  else{
      terminal_arr[terminal_num_display].cursor_x = NUM_COLS - 1;
      terminal_arr[terminal_num_display].cursor_y--;
    update_cursor(terminal_arr[terminal_num_display].cursor_x, terminal_arr[terminal_num_display].cursor_y); //update cursor
  }
}

/* delete_char_from_screen
 *   DESCRIPTION: erase a character from screen
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: erase a character from screen
 */
void delete_char_from_screen(){
  /* delets a character from the screen */
  putc_addr('\0',terminal_arr[terminal_num_display].cursor_x,terminal_arr[terminal_num_display].cursor_y);
}
/* keyboard_handler_main
 *   DESCRIPTION: handler for keyboard interrupts
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: none
 */
void keyboard_handler_main()
{
  //STEP 1: read data (scancode) from keyboard port
  unsigned int scan = inb(KEYBOARD_DATA_PORT);
  //printf("%d", scan);
  //STEP 3: make flags for ctrl, shift, alt, caps, num lock, scroll lock
  //STEP 4: fill asci_code appropriately
  unsigned char ascii = translate(scan);
  if(ctrl_flag_L == 1){
    clearbuf(); // clears keyboard buffer
    clear(); //clears the entire screen
    init_cursor(); // relocate the cursor at the top of the screen
  }

  /* CP5: if alt + F1,2,3 pressed, run swap_terminal(0,1,2) respectively */
	else if(terminal_zero_flag) swap_terminal(0);
	else if(terminal_one_flag) swap_terminal(1);
	else if(terminal_two_flag) swap_terminal(2);

  else if(enter_flag == 1)
    enter_press(ascii);
  else if(buf_flag == 3 && ascii != NULL)
    add2buf(ascii);
  else if(back_flag == 1){
    delete_char_from_buf();
    delete_char_from_screen();
  }
  buf_flag = 0;
  back_flag = 0;
  ctrl_flag_L = 0;
  terminal_zero_flag = 0;
  terminal_one_flag = 0;
  terminal_two_flag = 0;
//STEP 5: check special cases
//STEP 6: put non-special keypress to terminal
//STEP 7: echo to screen
  return;
}

/* swap_terminal
 *	 DESCRIPTION: swap between terminals, upon hitting alt+F1/2/3
 * 				  to terminal 0,1,2 respectively.
 *				  only swapping vid mem in this function;
 *				  note: paging is done in pit handler,
 *				  cursor and buffer dispatchaed and modified from
 *				  terminal_write and putc
 *	 INPUT: index 0,1,2 for terminals
 *   OUTPUT: none
 */
void swap_terminal(uint8_t index){
  if(index == terminal_num_display){
    return;
  }
  cli();
	/* swap the content of physical video memory */
  char temp[SIZE_OF_VIDMEM];
	/* 1. save physical vidmem to one of three terminals */
  memcpy(temp,terminal_arr[index].vidmem_addr,SIZE_OF_VIDMEM);

  set_up_virtual_to_video(index);


	memcpy(terminal_arr[terminal_num_display].vidmem_addr, video_mem, SIZE_OF_VIDMEM);
	/* 2. put indexed video mmr to physical video mmr */
	memcpy(terminal_arr[index].vidmem_addr, temp,SIZE_OF_VIDMEM);
  terminal_num_display = index;

	/* 3. set paging to reflect new video memory being loaded */

  update_cursor(terminal_arr[terminal_num_display].cursor_x,terminal_arr[terminal_num_display].cursor_y);
  sti();
}
