#ifndef TERMINAL_H_
#define TERMINAL_H_


#include "types.h"
#include "keyboard.h"
#define NUM_COLS 80
#define NUM_ROWS 25
#define ATTRIB 0x7
#define VIDEO       0xB8000
#define SCREEN_BUF_SIZE 128
#define MIN_SCANLINE 0x0E
#define MAX_SCANLINE 0x0F
#define GREY_COLOR 0x3D4
#define GREY_COLOR2 0x3D5
#define CURSOR_SHIFT 0x00FF
#define BLINK 8


typedef struct terminal
{
  unsigned int cursor_x; //x position of cursor
  unsigned int cursor_y; //y position of cursor
  unsigned char* vidmem_loc; //pointer of video mem allocated for this terminal
} term_t;

/* opens the terminal */
int32_t terminal_open(int32_t fd, void* buf, int32_t nbytes);
/* close the terminal */
int32_t terminal_close(int32_t fd, void* buf, int32_t nbytes);
/* read keyboard buffer into screen buffer */
int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes);
/* write data stored in screen buffer */
int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes);
/* scroll up the terminal by one row */
void scroll_up(void);
/* update the cursor position */
void update_cursor(unsigned int x, unsigned int y);
/* draws a character on the terminal */
void putc_addr(char c, unsigned int cursor_x, unsigned int cursor_y);

extern unsigned char screen_buf[SCREEN_BUF_SIZE]; // screen buffer to contain data
extern unsigned char screen_buf_top;  // position of the next empty element of screen buffer

#endif
