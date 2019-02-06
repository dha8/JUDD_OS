/* idt.h - includes functions initializaing and handling idt
 * vim:ts=4 noexpandtab
 */

#include "idt.h"
#include "x86_desc.h"
#include "types.h"
#include "keyboard.h"
#include "rtc.h"
#include "isr_wrapper.h"
#include "pit.h"
/* init_idt
 *   DESCRIPTION: sets the entries in idt descriptor table located in
 *                x86_desc.h (line 166)
 *   INPUT: None
 *   OUTPUT: None
 *   SIDE_EFFECT: initialize IDT
 */
void init_idt(){

    /* Initialize exceptions in the table */
    define_exception(NUM_DIV_BY_ZERO,DIV_BY_ZERO,DPL_KERNEL);
    define_exception(NUM_SINGLE_STEP_INT,SINGLE_STEP_INT,DPL_KERNEL);
    define_exception(NUM_NMI,NMI,DPL_KERNEL);
    define_exception(NUM_BREAKPOINT,BREAKPOINT,DPL_KERNEL);
    define_exception(NUM_OVERFLOW,OVERFLOW,DPL_KERNEL);
    define_exception(NUM_BOUNDS,BOUNDS,DPL_KERNEL);
    define_exception(NUM_INVALID_OPCODE,INVALID_OPCODE,DPL_KERNEL);
    define_exception(NUM_COPROCESSOR_NA,COPROCESSOR_NA,DPL_KERNEL);
    define_exception(NUM_DOUBLE_FAULT,DOUBLE_FAULT,DPL_KERNEL);
    define_exception(NUM_COPROCESSOR_SO,COPROCESSOR_SO,DPL_KERNEL);
    define_exception(NUM_INVALID_TSS,INVALID_TSS,DPL_KERNEL);
    define_exception(NUM_SEG_NOT_PRESENT,SEG_NOT_PRESENT,DPL_KERNEL);
    define_exception(NUM_STACK_FAULT,STACK_FAULT,DPL_KERNEL);
    define_exception(NUM_GENERAL_PFAULT,GENERAL_PFAULT,DPL_KERNEL);
    define_exception(NUM_PAGE_FAULT,PAGE_FAULT,DPL_KERNEL);
    define_exception(NUM_RESERVED,RESERVED,DPL_KERNEL);

    define_exception(NUM_MATH_FAULT,MATH_FAULT,DPL_KERNEL);
    define_exception(NUM_ALIGNMENT_CHECK,ALIGNMENT_CHECK,DPL_KERNEL);
    define_exception(NUM_MACHINE_CHECK,MACHINE_CHECK,DPL_KERNEL);
    define_exception(NUM_SIMD_FP_EXCEP,SIMD_FP_EXCEP,DPL_KERNEL);
    define_exception(NUM_VIRTUAL_EXCEP,VIRTUAL_EXCEP,DPL_KERNEL);
    define_exception(NUM_CTRL_PROT_EXCEP,CTRL_PROT_EXCEP,DPL_KERNEL);

    /* initialize system call in the table */
    define_interrupt(NUM_SYSTEM_CALL,SYSTEM_CALL,DPL_USER);

    /* initialize hardware IRQ lines in the table */
    define_exception(PIT_IRQ_OPCODE,IRQ_PIT,DPL_KERNEL); // goes to c handler
    define_exception(KEYBOARD_IRQ_OPCODE,IRQ_KEYBOARD,DPL_KERNEL); // goes to assembly
    define_exception(IRQ_RTC_NUM,IRQ_RTC,DPL_KERNEL); // goes to assembly
}


/* define_each_exception
 *   DESCRIPTION: sets the idt values for exception
 *   INPUT: entry - index, of each IDT entry
 *          f - function pointer for each handler
 *          dpl - dpl level to set for each entry
 *   OUTPUT: none
 *   SIDE_EFFECT: initialize each IDT entry into the table
 */
void define_exception(unsigned char entry,void const * const f,int dpl){

	/* Note: offset_15_00 and 31_16 are taken care
	 * through SET_IDT_ENTRY macro */
	SET_IDT_ENTRY(idt[entry],f);
	idt[entry].seg_selector = KERNEL_CS;
	idt[entry].reserved4 = 0;
	idt[entry].reserved3 = 1; // reserved3 = 1 for exceptions
	idt[entry].reserved2 = 1;
	idt[entry].reserved1 = 1;
	idt[entry].reserved0 = 0;
	idt[entry].present = 1;
	idt[entry].size = 1;
	idt[entry].dpl = dpl;
}

/* define_each_interrupt
 *   DESCRIPTION: sets the idt values for interrupt
 *   INPUT: entry - index, opcode of each IDT entry
 *          f - function pointer for each handler
 *          dpl - dpl level to set for each entry
 *   OUTPUT: none
 *   SIDE_EFFECT: initialize each IDT entry into the table
 */
void define_interrupt(unsigned char entry,void const * const f,int dpl){

      /* Note: offset_15_00 and 31_16 are taken care
	   * through SET_IDT_ENTRY macro */
      SET_IDT_ENTRY(idt[entry],f);
      idt[entry].seg_selector = KERNEL_CS;
      idt[entry].reserved4 = 0;
      idt[entry].reserved3 = 0; // reserved3 = 0 for interrupts
      idt[entry].reserved2 = 1;
      idt[entry].reserved1 = 1;
      idt[entry].reserved0 = 0;
      idt[entry].present = 1;
      idt[entry].size = 1;
      idt[entry].dpl = dpl;
}
