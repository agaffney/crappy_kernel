#include <arch/x86/io.h>
#include <arch/x86/idt.h>

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void) {
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt (IRQ1) */
	keyboard_address = (unsigned long)keyboard_handler; 
	IDT[PIC1_OFFSET_ADDR + 1].offset_lowerbits = keyboard_address & 0xffff;
	IDT[PIC1_OFFSET_ADDR + 1].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[PIC1_OFFSET_ADDR + 1].zero = 0;
	IDT[PIC1_OFFSET_ADDR + 1].type_attr = IDT_TYPE_INTERRUPT_GATE;
	IDT[PIC1_OFFSET_ADDR + 1].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/* ICW1 - begin initialization */
	outb(PIC1_COMMAND_ADDR, 0x11);
	outb(PIC2_COMMAND_ADDR, 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	outb(PIC1_DATA_ADDR, PIC1_OFFSET_ADDR);
	outb(PIC2_DATA_ADDR, PIC2_OFFSET_ADDR);

	/* ICW3 - setup cascading */
	outb(PIC1_DATA_ADDR, 0x00);
	outb(PIC2_DATA_ADDR, 0x00);

	/* ICW4 - environment info */
	outb(PIC1_DATA_ADDR, 0x01);
	outb(PIC2_DATA_ADDR, 0x01);
	/* Initialization finished */

	/* mask interrupts */
	outb(PIC1_DATA_ADDR, 0xff);
	outb(PIC2_DATA_ADDR, 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}
