/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT      0x20
#define MASTER_8259_DATA_PORT 0x21
#define SLAVE_8259_PORT       0xA0
#define SLAVE_8259_DATA_PORT  0xA1

#define ENABLE_IRQ_MASK		  0xFE
#define DISABLE_IRQ_MASK      0x01

#define SLAVE_BOUNDARY			 8
#define EOI_NUM					 2

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01
#define MASK_ALL            0xFF

//port boundaries
#define MASTER_IR0 0x00
#define MASTER_IR7 0x07
#define SLAVE_IR0  0x08
#define SLAVE_IR7  0x0F

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
