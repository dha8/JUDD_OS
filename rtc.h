#ifndef RTC_H_
#define RTC_H_

#include "types.h"

#define IRQ8 8
//Index of register A,B,and C in RTC port
//Index of register A and B hold 8 in the left four bits
// since 8 disable the NMI
#define R_A 0x8A
#define R_B 0x8B
#define R_C 0x0C

#define RTC_PORT 0x70
#define RTC_DATA_PORT 0x71

#define INIT_BASERATE 15 //default base rate 15
#define INIT_BASERATE_MASK 0x0F //default base rate mask is 0x0F
#define INIT_PREV_OR 0x40 //value to or with prev (old cmos port val) to make new
#define INIT_PREV_AND 0xF0 //value to and with prev (initial value of reg a) to make new

#define MAX_RATE     10
#define MIN_RATE      1

/* rtc intializer */
void rtc_init(void);
/* signals the interrupt has occured, and lets interrupts to happen continuosly */
void rtc_handler(void);
/* initializes the rtc */
int32_t rtc_open(uint8_t const * filename);
/* close the rtc */
int32_t rtc_close(int32_t fd);
/* checks if interrupt has occured */
int32_t rtc_read(int32_t fd, void * buf, int32_t nbytes);
/* sets new frequency for the rtc */
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes);

#endif
