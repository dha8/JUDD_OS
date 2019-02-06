/* keyboard.h - includes functions and directives for keyboard init and use.
 * vim:ts=4 noexpandtab
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "types.h"
#include "lib.h"
#include "terminal.h"

#define SCREEN_BUF_SIZE 128
/* default keyboard irq number */
#define KEYBOARD_DATA_PORT 		0x60
#define KEYBOARD_STATUS_PORT 	0x64
#define KEYBOARD_IRQ 			0x01

/* define values for handler conversion_scheme zero: scantable_small */
#define KEY_RETURN        '\n'
#define KEY_ESCAPE        0x1001
#define KEY_BACKSPACE     '\b'
//#define KEY_F1            0x1201
//#define KEY_F2            0x1202
//#define KEY_F3            0x1203
#define KEY_F4            0x1204
#define KEY_F5            0x1205
#define KEY_F6            0x1206
#define KEY_F7            0x1207
#define KEY_F8            0x1208
#define KEY_F9            0x1209
#define KEY_F10           0x120a
#define KEY_TAB               0x4000
#define KEY_CAPSLOCK          0x4001
#define KEY_LSHIFT            0x4002
#define KEY_LCTRL             0x4003
#define KEY_RSHIFT            0x4006
#define KEY_RALT              0x4008
#define KEY_UNKNOWN           0x0000

//special define symbols for functions
#define KEY_F1 117
#define KEY_F2 116
#define KEY_F3 115
#define DBH 118 //debug buffer hack
#define CRL 119 //ctrl+l symbol

//define symbols from scancodes to ignore
#define ESC 1
#define TAB 15
#define CAP 58

#define TF1KEY 0x4000
#define TF2KEY 0x4010
#define TF3KEY 0x4020

#define KF3 61
#define KF2 60
#define KF1 59

#define KF4 62
#define KF0 68
#define PGU 87
#define PGD 88
#define NUM 90
#define DOT 108
#define INS 69
#define DEL 83
#define PRS 55

//define symbols from scancodes to care about (mods mostly)
#define CTRL_P 29
#define CTRL_R 157
#define ALT_P 56
#define ALT_R 184
#define SHIL_P 42
#define SHIR_P 54
#define SHIL_R 170
#define SHIR_R 182
#define CAPS_R 186
#define GENL_R 120

//video memory definitions for screen swapping; 80 wide 25 high; 2 bc colors
#define SIZE_OF_VIDMEM		80*25*2


//for all to see
//extern unsigned char screen_buf[SCREEN_BUF_SIZE];
//extern unsigned char screen_buf_top;

//for terminal to see
extern unsigned char buf_flag;
//extern unsigned int cursor_x;
//extern unsigned int cursor_y;
extern unsigned char enter_flag;

extern uint8_t terminal_num_display;
//keyboard functions
extern uint8_t passed_buf[SCREEN_BUF_SIZE];
void clearbuf(void);
void enter_press(unsigned char ascii);
void add2buf(unsigned char);
void delete_char_from_buf();
void delete_char_from_screen();
void keyboard_init(void);
void init_cursor();
unsigned char translate(unsigned int scan);
void keyboard_handler_main(void);

//video memory swapping for cp5
void swap_terminal(uint8_t index);

#endif /* KEYBOARD_H_ */
