/* interrupt_handler.h - Handler functions for each exception and handler in c.
 * 						 Firt called through assembly wrapper in real use.
 * vim:ts=4 noexpandtab
 */

#ifndef INTERRUPT_HANDLER_H
#define INTERRUPT_HANDLER_H

/* Handler functions for exceptions below */
extern void PAGE_FAULT_handler();

extern void DIV_BY_ZERO_handler();

extern void SINGLE_STEP_INT_handler();

extern void NMI_handler();

extern void BREAKPOINT_handler();

extern void OVERFLOW_handler();

extern void BOUNDS_handler();

extern void INVALID_OPCODE_handler();

extern void COPROCESSOR_NA_handler();

extern void DOUBLE_FAULT_handler();

extern void COPROCESSOR_SO_handler();

extern void INVALID_TSS_handler();

extern void SEG_NOT_PRESENT_handler();

extern void STACK_FAULT_handler();

extern void GENERAL_PFAULT_handler();

extern void RESERVED_handler();

extern void MATH_FAULT_handler();

extern void ALIGNMENT_CHECK_handler();

extern void MACHINE_CHECK_handler();

extern void SIMD_FP_EXCEP_handler();

extern void VIRTUAL_EXCEP_handler();

extern void CTRL_PROT_EXCEP_handler();

/* IRQ line handlers */
extern void IRQ_KEYBOARD_handler();
extern void IRQ_RTC_handler();
extern void IRQ_PIT_handler();

#endif /* INTERRUPT_HANDLER_H */
