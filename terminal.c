#include "terminal.h"
#include "lib.h"
#include "types.h"
#include "paging.h"
#include "keyboard.h"
#include "pit.h"


/* terminal_open
 *   DESCRIPTION: initialize variables for terminal
 *   INPUT: fd - file number for consistency with other terminal functions (not used)
 *          buf - screen buffer for consistency with other terminal functions (not used)
 *          nbytes - number of bytes to write
 *	 OUTPUT: 0 - shows the functions is executed
 *	 SIDE EFFECTS: set all variables to 0
 */
int32_t terminal_open(int32_t fd, void* buf, int32_t nbytes){
  int32_t i;

  /* initialize variables */
  for (i = 0; i < SCREEN_BUF_SIZE; i++){
    terminal_arr[terminal_num].screen_buf[i] = '\0';
  }
  terminal_arr[terminal_num].cursor_x = 0;
  terminal_arr[terminal_num].cursor_y = 0;

  clear();      //clear the termianl screen
  return 0;
}

/* terminal_close
 *   DESCRIPTION: erase variables for terminal
 *   INPUT: fd - file number for consistency with other terminal functions (not used)
 *          buf - screen buffer for consistency with other terminal functions (not used)
 *          nbytes - number of bytes to write
 *	 OUTPUT: 0 - shows the functions is executed
 *	 SIDE EFFECTS: set all variables to 0
 */
int32_t terminal_close(int32_t fd, void* buf, int32_t nbytes){
  int32_t i;
  /* fill screen buffer with null terminating character */
  for (i = 0; i < SCREEN_BUF_SIZE; i++){
    terminal_arr[terminal_num].screen_buf[i] = '\0';
  }
  clear();      //clear the termianl screen
  return 0;
}

/* terminal_read
 *   DESCRIPTION: puts keyboard data from keyboard buffer, where inputs are in,
 *                into the screen buffer
 *   INPUT: fd - file number
 *          buf - screen buffer
 *          nbytes - number of bytes to write
 *	 OUTPUT: cnt - number of letters in screen buffer
 *	 SIDE EFFECTS: screen buffer is written with data from keyboard buffer
 */
int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes){
  int i = 0;
  terminal_arr[terminal_num_display].screen_buf[SCREEN_BUF_SIZE - 1] = '\r'; // the last character will always be new line
  if((buf == NULL) | (nbytes < 0)){
    return -1;
  }
  while(enter_flag != 1 || (terminal_num != terminal_num_display)){} // accept keyboard data until enter is pressed
  cli();
  enter_flag = 0;    // reset enter flag
  int cnt = 0;      //count of the letter in buffer (including enter)
  int32_t limit = nbytes > SCREEN_BUF_SIZE ? SCREEN_BUF_SIZE : nbytes; // limit upperbounded at 128
  for (i = 0; i < limit; i++){ // put keyboard data into buffer
    ((char *)buf)[i] = terminal_arr[terminal_num_display].screen_buf[i];
    cnt = cnt + 1;
    if((terminal_arr[terminal_num_display].screen_buf[i] == '\n') | (terminal_arr[terminal_num_display].screen_buf[i] == '\r')){
      break;
    }
  }
  clearbuf();
  sti();
  return cnt;
}


/* terminal_write
 *   DESCRIPTION: writes data from screen buffer to video memory
 *                if the next letter to print is at the end of the screen,
 *                scroll the screen up to print the letter
 *   INPUT: fd - file number
 *          buf - screen buffer
 *          nbytes - number of bytes to write
 *	 OUTPUT: 0 - shows the functions is executed
 *	 SIDE EFFECTS: video memory is written with data from screen buffer
 */
int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes){
  if((buf == NULL) | (nbytes <0)){
    return -1;
  }
  int32_t i; //iterator
  unsigned char c;

  /* if nbytes and size of buf are different or string length of buf < num_bytes,
   * return zero */
  cli();
  for (i = 0; i < nbytes; i++){
    c = ((char *)buf)[i];

    /* homage to putc function */
    if (terminal_arr[terminal_num].cursor_x == NUM_COLS){ // if end of the row, move to the next row
      terminal_arr[terminal_num].cursor_x = 0;
      terminal_arr[terminal_num].cursor_y ++;
      if (terminal_arr[terminal_num].cursor_y == NUM_ROWS){ // if end of screen, scroll up
            scroll_up();
      }
    }
    if(c == '\n' || c == '\r') { // if enter pressed
        terminal_arr[terminal_num].cursor_y++; // move to the next row
        terminal_arr[terminal_num].cursor_x = 0;

        /* scroll up if exceeded video display */
        if (terminal_arr[terminal_num].cursor_y == NUM_ROWS){ // if next row is end of screen, scroll up
              scroll_up();
        }
    }
    else {
        *(uint8_t *)(terminal_arr[terminal_num].vidmem_addr + ((NUM_COLS * terminal_arr[terminal_num].cursor_y + terminal_arr[terminal_num].cursor_x) << 1)) = c; // write a character to terminal
        *(uint8_t *)(terminal_arr[terminal_num].vidmem_addr + ((NUM_COLS * terminal_arr[terminal_num].cursor_y + terminal_arr[terminal_num].cursor_x) << 1) + 1) = ATTRIB; // in grey color
        terminal_arr[terminal_num].cursor_x++;
    }
    if(terminal_arr[terminal_num].cursor_x >= NUM_COLS){ // update scroll position
      terminal_arr[terminal_num].cursor_x = 0;           // if column number exceeds screen limiit
      terminal_arr[terminal_num].cursor_y++;
    }
    if (terminal_arr[terminal_num].cursor_y >= NUM_ROWS){ // update scroll position
      // scroll_up();      // and scroll up
      terminal_arr[terminal_num].cursor_y = NUM_ROWS - 1; // if column number exceeds screen limiit
    }
  }
  sti();
  //display cursor to the next line
  if(terminal_num == terminal_num_display){
    update_cursor(terminal_arr[terminal_num].cursor_x,terminal_arr[terminal_num].cursor_y);
  }
  return 0;
}

/* scroll_up
 *   DESCRIPTION: scrolls the screen up moving each row up,
 *                and erasing the last row
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: video memory is updated, data of the first row is gone,
 *                 and the last row of video memory is erased
 */
void scroll_up(){
  int32_t i;
    for (i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++) {
        /* replace the entire row with next row */
        *(uint8_t *)(terminal_arr[terminal_num].vidmem_addr + (i << 1)) = *(uint8_t *)(terminal_arr[terminal_num].vidmem_addr + ((i + NUM_COLS) << 1));
        *(uint8_t *)(terminal_arr[terminal_num].vidmem_addr + (i << 1) + 1) = ATTRIB;
    }
    /* clear the last row */
    for (i = (NUM_ROWS - 1) * NUM_COLS; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(terminal_arr[terminal_num].vidmem_addr + (i << 1)) = ' ';
        *(uint8_t *)(terminal_arr[terminal_num].vidmem_addr + (i << 1) + 1) = ATTRIB;
     }
    terminal_arr[terminal_num].cursor_y = NUM_ROWS - 1;
    terminal_arr[terminal_num].cursor_x = 0;
}

/* update_cursor
 *   DESCRIPTION: updates cursor position using the inputs,
 *                and draws it using cursor port
 *   INPUT: x - x position of the cursor
 *          y - y position of the cursor
 *	 OUTPUT: none
 *	 SIDE EFFECTS: video memory is updated with cursor image
 */
void update_cursor(unsigned int x,unsigned int y){
  /* boundary checking */
  if(x >= NUM_COLS){ // update scroll position
    x = 0;           // if column number exceeds screen limiit
    y++;
  }
  if (y >= NUM_ROWS){ // update scroll position
    scroll_up();      // and scroll up
    y = NUM_ROWS - 1; // if column number exceeds screen limiit
  }
  unsigned short position=(y*NUM_COLS) + x;

  /* update cursor position in terminal */
    outb(MAX_SCANLINE, GREY_COLOR);
    outb((position & CURSOR_SHIFT), GREY_COLOR2);
    outb(MIN_SCANLINE, GREY_COLOR);
    outb(((position >> BLINK) & CURSOR_SHIFT), GREY_COLOR2);
}

/* putc_addr
 *   DESCRIPTION: updates the video memory with an input,
 *                on a position defined with cursor_x and cursor_y
 *   INPUT: c - character to draw
 *          cursor_x - x position of the cursor
 *          cursor_y - y position of the cursor
 *	 OUTPUT: none
 *	 SIDE EFFECTS: video memory is updated with the character image
 */
void putc_addr(char c, unsigned int cursor_x, unsigned int cursor_y)
{
	/* This should take care of line wrapping and scrolling */
  if(c == '\n' || c == '\r'){
    return;
  }
	if(terminal_arr[terminal_num_display].cursor_x == NUM_COLS && (c != '\n' && c != '\r'))
	{
	  terminal_arr[terminal_num_display].cursor_x = 0;
		terminal_arr[terminal_num_display].cursor_y++;
	}
	if(terminal_arr[terminal_num_display].cursor_y >= NUM_ROWS)
		scroll_up();
  *(uint8_t *)(terminal_arr[terminal_num_display].vidmem_addr + ((NUM_COLS * terminal_arr[terminal_num_display].cursor_y + terminal_arr[terminal_num_display].cursor_x) << 1)) = c; // write a character to terminal
  *(uint8_t *)(terminal_arr[terminal_num_display].vidmem_addr + ((NUM_COLS * terminal_arr[terminal_num_display].cursor_y + terminal_arr[terminal_num_display].cursor_x) << 1) + 1) = ATTRIB; // in grey color
}
