/*
 *  Interrupt Service Routine; Assembly wrapper that calls handlers in c
 */
#ifndef ISR_WRAPPER_H
#define ISR_WRAPPER_H
#ifndef ASM
#define ASM  1

#include "interrupt_handler.h"
/* Assembly wrappers for c handlers */

/* Exceptions */
void PAGE_FAULT();
void DIV_BY_ZERO();
void SINGLE_STEP_INT();
void NMI();
void BREAKPOINT();
void OVERFLOW();
void BOUNDS();
void INVALID_OPCODE();
void COPROCESSOR_NA();
void DOUBLE_FAULT();
void COPROCESSOR_SO();
void INVALID_TSS();
void SEG_NOT_PRESENT();
void STACK_FAULT();
void GENERAL_PFAULT();
void RESERVED();
void MATH_FAULT();
void ALIGNMENT_CHECK();
void MACHINE_CHECK();
void SIMD_FP_EXCEP();
void VIRTUAL_EXCEP();
void CTRL_PROT_EXCEP();
/* Interrupts */
void IRQ_KEYBOARD();
void IRQ_RTC();
void IRQ_PIT();

/* System calls */
void SYSTEM_CALL();

#endif
#endif
