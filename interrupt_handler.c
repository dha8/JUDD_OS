/* interrupt_handler.h - Handler functions for each exception and handler in c.
 *                       Firt called through assembly wrapper in real use.
 * vim:ts=4 noexpandtab
 */

#include "types.h"
#include "lib.h"
#include "interrupt_handler.h"
#include "keyboard.h"
#include "rtc.h"
#include "i8259.h"
#include "syscalls.h"
#include "pit.h"

/* PAGE_FAULT_handler
 *   DESCRIPTION: called upon receiving page fault exception, first from
 *   			  a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle page fault error
 */
extern void PAGE_FAULT_handler(){
    cli();
	uint8_t message[] = "PFAULT ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* DIV_BY_ZERO_handler
 *   DESCRIPTION: called upon receiving page DIV_BY_ZERO exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle div by zero error
 */
extern void DIV_BY_ZERO_handler(){
    cli();
	uint8_t message[] = "DIV_BY_ZERO_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* SINGLE_STEP_INT_handler
 *   DESCRIPTION: called upon receiving single step int exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle single step int error
 */
extern void SINGLE_STEP_INT_handler(){
    cli();
	uint8_t message[] = "SINGLE_STEP_INT_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* NMI_handler
 *   DESCRIPTION: called upon receiving NMI exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle NMI interrupt error
 */
extern void NMI_handler(){
    cli();
	uint8_t message[] = "NMI_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* BREAKPOINT_handler
 *   DESCRIPTION: called upon receiving breakpoint exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle breakpoint(int3) error
 */
extern void BREAKPOINT_handler(){
    cli();
	uint8_t message[] = "BREAKPOINT(INT3)_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* OVERFLOW_handler
 *   DESCRIPTION: called upon receiving overflow exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle overflow(into) error
 */
extern void OVERFLOW_handler(){
    cli();
	uint8_t message[] = "OVERFLOW(INTO)_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* BOUNDS_handler
 *   DESCRIPTION: called upon receiving Bounds range exceeded exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle bounds range exceeded error
 */
extern void BOUNDS_handler(){
    cli();
	uint8_t message[] = "BOUNDS_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* INVALID_OPCODE_handler
 *   DESCRIPTION: called upon receiving Invalid Opcode(UD2) exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle Invalid Opcode(UD2) error
 */
extern void INVALID_OPCODE_handler(){
    cli();
	uint8_t message[] = "INVALID_OPCODE(UD2)_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* COPROCESSOR_NA_handler
 *   DESCRIPTION: called upon receiving Coprocessor_NA exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle coprocssor_N/A error
 */
extern void COPROCESSOR_NA_handler(){
    cli();
	uint8_t message[] = "COPROCESSOR_NA_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* DOUBLE_FAULT_handler
 *   DESCRIPTION: called upon receiving Double Fault exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle Double Fault error
 */
extern void DOUBLE_FAULT_handler(){
    cli();
	uint8_t message[] = "DOUBLE_FAULT_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* COPROCESSOR_SO_handler
 *   DESCRIPTION: called upon receiving Coprocessor_SO exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle coprocssor_SO error
 */
extern void COPROCESSOR_SO_handler(){
    cli();
	uint8_t message[] = "COPROCESSOR_SO_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* INVALID_TSS_handler
 *   DESCRIPTION: called upon receiving INVALID_TSS exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle INVALID_TSS error
 */
extern void INVALID_TSS_handler(){
    cli();
	uint8_t message[] = "INVALID_TSS_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* SEG_NOT_PRESENT_handler
 *   DESCRIPTION: called upon receiving Segment not present exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle SEG_NOT_PRESENT error
 */
extern void SEG_NOT_PRESENT_handler(){
    cli();
	uint8_t message[] = "SEG_NOT_PRESENT_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* STACK_FAULT_handler
 *   DESCRIPTION: called upon receiving STACK_FAULT exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle STACK_FAULT error
 */
extern void STACK_FAULT_handler(){
    cli();
	uint8_t message[] = "STACK_FAULT_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* GENERAL_PFAULT_handler
 *   DESCRIPTION: called upon receiving General Pfault exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle General Pfault error
 */
extern void GENERAL_PFAULT_handler(){
    cli();
	uint8_t message[] = "GENERAL_PFAULT_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* RESERVED_handler
 *   DESCRIPTION: called upon receiving an exception signal on the reserved
 *   			  spot, first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: executes code when receiving reserved signal
 */
extern void RESERVED_handler(){
    cli();
	uint8_t message[] = "RESERVED_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* MATH_FAULT_handler
 *   DESCRIPTION: called upon receiving MATH_FAULT exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle math fault error
 */
extern void MATH_FAULT_handler(){
    cli();
	uint8_t message[] = "MATH_FAULT\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* ALIGNMENT_CHECK_handler
 *   DESCRIPTION: called upon receiving ALIGNMENT_CHECK exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle alignment check error
 */
extern void ALIGNMENT_CHECK_handler(){
    cli();
	uint8_t message[] = "ALIGNMENT_CHECK_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* MACHINE_CHECK_handler
 *   DESCRIPTION: called upon receiving MACHINE_CHECK exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle machine check error
 */
extern void MACHINE_CHECK_handler(){
    cli();
	uint8_t message[] = "MACHINE_CHECK_ERROR\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* SIMD_FP_EXCEP_handler
 *   DESCRIPTION: called upon receiving SIMD floating-point exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle SIMD_FP_EXCEP error
 */
extern void SIMD_FP_EXCEP_handler(){
    cli();
	uint8_t message[] = "SIMD_FP_EXCEPTION\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* VIRTUAL_EXCEP_handler
 *   DESCRIPTION: called upon receiving virtualization exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle virtualization error
 */
extern void VIRTUAL_EXCEP_handler(){
    cli();
	uint8_t message[] = "VIRTUALIZATION_EXCEPTION\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}

/* CTRL_PROT_EXCEP_handler
 *   DESCRIPTION: called upon receiving control protection exception,
 *   			  first from a wrapper assembly function in isr_wrapper.S
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle control protection exception  error
 */
extern void CTRL_PROT_EXCEP_handler(){
    cli();
	uint8_t message[] = "CONTROL_PROTECTION_EXCEPTION\n";
	terminal_write(0,message,strlen((const int8_t*)message));
	halt(-1);
  	sti();
}


/************ IRQ line handlers *****************/

/* IRQ_KEYBOARD_handler
 *   DESCRIPTION: called upon receiving keyboard IRQ's as specified in idt.
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle the keyboard interrupt requests. currently prints
 *	 			   the keypress onto the screen
 */
extern void IRQ_KEYBOARD_handler(){
    cli();
    send_eoi(KEYBOARD_IRQ);
    keyboard_handler_main(); //coming from keyboard.c
    sti();
}

/* IRQ_RTC_handler
 *   DESCRIPTION: called upon receiving RTC IRQ's as specified in idt.
 *   INPUT: none
 *	 OUTPUT: none
 *	 SIDE EFFECTS: handle the RTC interrupt requests.
 */
extern void IRQ_RTC_handler(){
    cli();
    send_eoi(IRQ8);
    rtc_handler();
    sti();
}

extern void IRQ_PIT_handler(){
    cli();
    send_eoi(PIT_IRQ);
    pit_handler();
    sti();
}
