/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT2 (MASTER_8259_PORT + 1)
#define SLAVE_8259_PORT2  (SLAVE_8259_PORT + 1)

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask = ~0; /* IRQs 0-7 */
uint8_t slave_mask = ~0;  /* IRQs 8-15 */

/* i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes 2 pics into cascaded master and slave
 */
void i8259_init(void)
{
	/* save backup of initial state of master and slave data ports */
	unsigned char master_backup, slave_backup;
	master_backup = inb(MASTER_8259_DATA_PORT);
	slave_backup = inb(SLAVE_8259_DATA_PORT);

	/* disable (mask) all interrupts */
	outb(MASK_ALL, MASTER_8259_DATA_PORT); //mask all Master Data Ports
	outb(MASK_ALL, SLAVE_8259_DATA_PORT); //mask all Slave Data Ports

	/* init PICs 1 and 2 - begin */
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);

	/* stage 2 init PICS master and slave - set vector offsets */
	outb(ICW2_MASTER, MASTER_8259_DATA_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT);

	/* stage 3 init PICS master and slave - connect two pics */
	outb(ICW3_MASTER, MASTER_8259_DATA_PORT);
	outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);

	/* stage 4 init PICS master adn slave - EOI and x86 protocol */
	outb(ICW4, MASTER_8259_DATA_PORT);
	outb(ICW4, SLAVE_8259_DATA_PORT);

	/* reenable master's line that slave attatches to */
	enable_irq(0x02); //0x02 is the slave line on the master

	/* restore original state of PICs */
	outb(master_backup, MASTER_8259_DATA_PORT);
	outb(slave_backup, SLAVE_8259_DATA_PORT);
}

/* enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: the number irq to enable (0-15 hopefully)
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enabling the passed IRQ number, or printing error
 */
void enable_irq(uint32_t irq_num)
{
	/* initially we mask everything but the first line */
	uint8_t mask = ENABLE_IRQ_MASK;

	/* check if the irq num is invalid first */
 	if ((irq_num < MASTER_IR0) || (irq_num > SLAVE_IR7)) {
 		return;
 	}
	/* check if the irq num is within the master's bounds */
	else if((irq_num >= MASTER_IR0) && (irq_num <= MASTER_IR7)) {
 		int b;
 		for (b = 0; b < irq_num; b++) {
 			mask = (mask << 1) + 1;
 		}
		/* send out to master line */
 		master_mask = master_mask & mask;
 		outb(master_mask, MASTER_8259_PORT2);
 		return;
 	}
	/* otherwise, assume it's a slave irq */
	else
	{
 		int b;
 		for (b = 0; b < irq_num - SLAVE_BOUNDARY; b++) { // < irq# - 8
 			mask = (mask << 1) + 1;
 		}
		/* send out to slave line */
 		slave_mask = slave_mask & mask;
 		outb(slave_mask, SLAVE_8259_PORT2);
 		return;
 	}
}

/* disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: the number irq to enable (0-15 hopefully)
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: diabling the passed IRQ number, or printing error
 */
void disable_irq(uint32_t irq_num)
{
	/* initial mask = 11111110 in flipped nomenclature*/
	uint8_t mask = DISABLE_IRQ_MASK;

	/* check if the irq num is invalid first */
 	if ((irq_num < MASTER_IR0) || (irq_num > SLAVE_IR7)) {
 		return;
 	}
	/* check if the irq num is within the master's bounds */
	else if((irq_num >= MASTER_IR0) && (irq_num <= MASTER_IR7)) {
 		int b;
 		for (b = 0; b < irq_num; b++) {
 			mask = (mask << 1);
 		}
		/* send out to master line */
 		master_mask = master_mask | mask;
 		outb(master_mask, MASTER_8259_PORT2);
 	}
	/* otherwise, assume it's a slave irq */
	else
	{
 		int b;
 		for (b = 0; b < irq_num - SLAVE_BOUNDARY; b++) { // irq# - 8
 			mask = (mask << 1);
 		}
		/* send out to slave line */
 		slave_mask = slave_mask | mask;
 		outb(slave_mask, SLAVE_8259_PORT2);
 	}
}

/* send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: the number irq to send the eoi on
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: tells the PIC to stop assuming commands will come in
 */
void send_eoi(uint32_t irq_num)
{
	/* check if the irq num is invalid first */
    if(irq_num >= SLAVE_BOUNDARY){
 		outb( EOI | (irq_num - SLAVE_BOUNDARY), SLAVE_8259_PORT2-1);
		send_eoi(EOI_NUM);
	}
	outb(EOI | (irq_num), MASTER_8259_PORT);
}
