#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

extern void keyboard_handler(void);
extern void load_idt(unsigned long *idt_ptr);
extern void kb_init(void);

void kmain(void)
{
	const char *str = "my first kernel with keyboard support";
	clear_screen();
	kprint(str);
	kprint_newline();
	kprint_newline();

	idt_init();
	kb_init();

	while(1);
}

struct IDT_entry{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler; 
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);  
	write_port(0xA1 , 0x00);  

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void itoa(int input, char *output, int base) {
	char buf[50];
	int idx = 0, i;
	int remainder;
	int is_negative = 0;
	unsigned int num;

	if(input < 0 && base == 10) {
		is_negative = 1;
		num = -input;
	} else {
		num = (unsigned int)input;
	}

	while(num > 0) {
		remainder = num % base;
		buf[idx++] = remainder > 9 ? (remainder - 10 + 'a') : (remainder + '0');
		num /= base;
	}
	// Pad out a binary number to 8 bytes
	if(base == 2) {
		for(i = (idx - 2) % 8; i > 0; i--) {
			buf[idx++] = '0';
		}
	}
	// Add '0' prefix to octal number
	if(base == 8) {
		buf[idx++] = '0';
	}
	// Add '0x' prefix to hex number
	if(base == 16) {
		if(idx % 2 != 0) {
			buf[idx++] = '0';
		}
		buf[idx++] = 'x';
		buf[idx++] = '0';
	}
	if(is_negative) {
		buf[idx++] = '-';
	}

	for(i = 0; i < idx; i++) {
		output[i] = buf[idx - i - 1];
	}
	output[i] = 0;
}