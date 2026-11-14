ENTRY(_start)

SECTIONS {
	PROVIDE(__ipl_start = NYX_LOAD_ADDR);
	. = __ipl_start;
	.text : {
		/* Only the assembly entry belongs before the fixed Nyx header.
		 * C callbacks named _start also have a .text._start section. */
		KEEP(*(.text.nyx_entry));
		KEEP(*(._ipl_version));
		*(.text._irq_setup);
		*(.text*);
	}
	ASSERT(ipl_ver == __ipl_start + 0x98, "Nyx version header must stay at offset 0x98")
	.data : {
		*(.data*);
		*(.rodata*);
	}
	. = ALIGN(0x10);
	__ipl_end = .;
	.bss : {
		__bss_start = .;
		*(COMMON)
		*(.bss*)
		__bss_end = .;
	}
}
