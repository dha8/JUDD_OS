/* idt.h - includes functions initializaing and handling idt
 * vim:ts=4 noexpandtab
 */

#ifndef IDT_H
#define IDT_H


/* INT # for each exception in the Interrupt Vector Table */
#define NUM_DIV_BY_ZERO     0x00
#define NUM_SINGLE_STEP_INT 0x01
#define NUM_NMI             0x02
#define NUM_BREAKPOINT      0x03
#define NUM_OVERFLOW        0X04
#define NUM_BOUNDS          0x05
#define NUM_INVALID_OPCODE  0x06
#define NUM_COPROCESSOR_NA  0x07
#define NUM_DOUBLE_FAULT    0x08
#define NUM_COPROCESSOR_SO  0x09
#define NUM_INVALID_TSS     0x0A
#define NUM_SEG_NOT_PRESENT 0x0B
#define NUM_STACK_FAULT     0x0C
#define NUM_GENERAL_PFAULT  0x0D
#define NUM_PAGE_FAULT      0x0E
#define NUM_RESERVED        0x0F
#define NUM_MATH_FAULT      0x10
#define NUM_ALIGNMENT_CHECK 0x11
#define NUM_MACHINE_CHECK   0x12
#define NUM_SIMD_FP_EXCEP   0x13
#define NUM_VIRTUAL_EXCEP   0x14
#define NUM_CTRL_PROT_EXCEP 0x15

/* Syscall Exception Number */
#define NUM_SYSTEM_CALL 	0x80

/* Opcodes for hardware IRQ lines */
#define PIT_IRQ_OPCODE  0x20
#define KEYBOARD_IRQ_OPCODE 0x21
#define IRQ_RTC_NUM			0x28

/* DPL values for each level */
#define DPL_KERNEL      0
#define DPL_USER        3

/* Umbrella function for initializing IDT */
void init_idt();

/* Functions to define each interrupt in the table */
void define_exception(unsigned char entry,void const * const f,int dpl);
void define_interrupt(unsigned char entry,void const * const f,int dpl);

#endif /* IDT_H */
