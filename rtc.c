#include "idt.h"
#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "types.h"

/* Static variable int rtc_signal becomes 1 when interrupt has occured*/
static int rtc_signal;

/* rtc_init
 *   DESCRIPTION: initializer for rtc
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: initialize the rtc and does the following:
 *   				-changes the frequency of periodic interrupts
 *   				-enable interrupts
 */
void rtc_init()
{

  unsigned char rate;
  rate = INIT_BASERATE;
  rate &= INIT_BASERATE_MASK;
  outb(R_A, RTC_PORT); //rtc reg a, rtc port
  unsigned char prev = inb(RTC_DATA_PORT); //cmos port
  outb(R_B, RTC_PORT); //rtc reg b, rtc port
  outb(prev | INIT_PREV_OR, RTC_DATA_PORT); //a old, cmos port
  enable_irq(RTC_PORT); //rtc irq

  disable_irq(RTC_PORT); // rate must be above 2 and not over 15
  outb(R_A, RTC_PORT); // set index to register A, disable NMI
  prev = inb(RTC_DATA_PORT); // get initial value of register A
  outb(R_A, RTC_PORT); //set index to A
  outb((prev & INIT_PREV_AND) | rate, RTC_DATA_PORT);
  enable_irq(RTC_PORT); //rtc irq
  rtc_signal = 0;
  return;
}

/* rtc_handler
 *   DESCRIPTION: handler for rtc interrupts, enables system to accept
 *                periodic interrupts
 *   INPUT: none
 *   OUTPUT: none
 *   SIDE EFFECTS: send end-of-interrupts signal for rtc
 */
void rtc_handler(){
  send_eoi(IRQ8); //rtc irq
  cli();
  rtc_signal = 1; //interrupt has occured
  /* enable interrupt again by reading register C */
  outb(R_C, RTC_PORT); //x0c, rtc port
  inb(RTC_DATA_PORT);  // just throw away contents
  sti();
  return;
}

/* rtc_open
 *   DESCRIPTION: initialize the rtc with a certain frequency
 *                periodic interrupts
 *   INPUT: filename - name of the file
 *   OUTPUT: 0 upon success
 *   SIDE EFFECTS: changes the rtc frequency
 */
int32_t rtc_open(uint8_t const * filename){
  unsigned char rate;
  rate = INIT_BASERATE;
  rate &= INIT_BASERATE_MASK;
  outb(R_A, RTC_PORT); //rtc reg a, rtc port
  unsigned char prev = inb(RTC_DATA_PORT); //cmos port
  outb(R_B, RTC_PORT); //rtc reg b, rtc port
  outb(prev | INIT_PREV_OR, RTC_DATA_PORT); //a old, cmos port
  enable_irq(RTC_PORT); //rtc irq

  disable_irq(RTC_PORT); // rate must be above 2 and not over 15
  outb(R_A, RTC_PORT); // set index to register A, disable NMI
  prev = inb(RTC_DATA_PORT); // get initial value of register A
  outb(R_A, RTC_PORT); //set index to A
  outb((prev & INIT_PREV_AND) | rate, RTC_DATA_PORT);
  enable_irq(RTC_PORT); //rtc irq
  rtc_signal = 0;
  return 0;
}

/* rtc_close
 *   DESCRIPTION: placeholder
 *   INPUT: fd - file number
 *   OUTPUT: 0
 *   SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd){
  return 0;
}

/* rtc_read
 *   DESCRIPTION: checks if interrupt has occured
 *   INPUT: fd - file number
 *          buf - rtc buffer (holds frequency)
 *          nbytes - number of bytes
 *   OUTPUT: 0 - if interrupt has occured
 *   SIDE EFFECTS: none
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
  while(!rtc_signal); // stuck when interrupt has not occured yet
  rtc_signal = 0; // clears flag immediately after the interrupt happened
  return 0; // tells the interrupt has happened
}

/* rtc_write
 *   DESCRIPTION: updates the frequency of the rtc,
 *   INPUT: fd - file number
 *          buf - rtc buffer (holds frequency)
 *          nbytes - number of bytes
 *   OUTPUT: 0 - if rtc_write was successful
 *          -1 - if rtc_write was unsuceessful
 *   SIDE EFFECTS: change the freqency of rtc
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
  int freq = *(uint32_t *)buf;
  int rate = 0;   //if 2hz is buf then 2 can be divided into two while we want value 1
  int rest = 0;
  if ((nbytes != sizeof(uint32_t)) | (freq <= 1)){ // if nbytes and freq is not proper, return -1
    return -1;
  }
  while(freq > 1){ // calculate the log base 2 of frequency
    if(freq % 2 != 0){
      rest = 1; // if freq is not divisible by 2, it's not apporpriate frequency
    }
    freq = freq / 2; // update frequency
    rate += 1; // increment rate in interms of log(freq, 2)
  }
  if((rate < MIN_RATE) | (rate > MAX_RATE) | (rest != 0)){       //6 represent 32768 >> 5 which makes 1024 hz
    return -1;
  }
  rate = INIT_BASERATE + 1 - rate; //adding baserate rate by one because it should be 16
  rate = rate & INIT_BASERATE_MASK; //then subtracting by old rate itself, then & masking up back up
  disable_irq(RTC_PORT);
  outb(R_A, RTC_PORT); // set index to register A, disable NMI
  unsigned char prev = inb(RTC_DATA_PORT); // get initial value of register A
  outb(R_A, RTC_PORT); //set index to A
  outb((prev & INIT_PREV_AND) | rate, RTC_DATA_PORT);
  enable_irq(RTC_PORT); //rtc irq
  return 0;
}
