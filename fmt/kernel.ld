9550 /* Simple linker script for the JOS kernel.
9551    See the GNU ld 'info' manual ("info ld") to learn the syntax. */
9552 
9553 OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
9554 OUTPUT_ARCH(i386)
9555 ENTRY(_start)
9556 
9557 SECTIONS
9558 {
9559 	/* Link the kernel at this address: "." means the current address */
9560         /* Must be equal to KERNLINK */
9561 	. = 0x80100000;
9562 
9563 	.text : AT(0x100000) {
9564 		*(.text .stub .text.* .gnu.linkonce.t.*)
9565 	}
9566 
9567 	PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */
9568 
9569 	.rodata : {
9570 		*(.rodata .rodata.* .gnu.linkonce.r.*)
9571 	}
9572 
9573 	/* Include debugging information in kernel memory */
9574 	.stab : {
9575 		PROVIDE(__STAB_BEGIN__ = .);
9576 		*(.stab);
9577 		PROVIDE(__STAB_END__ = .);
9578 	}
9579 
9580 	.stabstr : {
9581 		PROVIDE(__STABSTR_BEGIN__ = .);
9582 		*(.stabstr);
9583 		PROVIDE(__STABSTR_END__ = .);
9584 	}
9585 
9586 	/* Adjust the address for the data segment to the next page */
9587 	. = ALIGN(0x1000);
9588 
9589 	/* Conventionally, Unix linkers provide pseudo-symbols
9590 	 * etext, edata, and end, at the end of the text, data, and bss.
9591 	 * For the kernel mapping, we need the address at the beginning
9592 	 * of the data section, but that's not one of the conventional
9593 	 * symbols, because the convention started before there was a
9594 	 * read-only rodata section between text and data. */
9595 	PROVIDE(data = .);
9596 
9597 
9598 
9599 
9600 	/* The data segment */
9601 	.data : {
9602 		*(.data)
9603 	}
9604 
9605 	PROVIDE(edata = .);
9606 
9607 	.bss : {
9608 		*(.bss)
9609 	}
9610 
9611 	PROVIDE(end = .);
9612 
9613 	/DISCARD/ : {
9614 		*(.eh_frame .note.GNU-stack)
9615 	}
9616 }
9617 
9618 
9619 
9620 
9621 
9622 
9623 
9624 
9625 
9626 
9627 
9628 
9629 
9630 
9631 
9632 
9633 
9634 
9635 
9636 
9637 
9638 
9639 
9640 
9641 
9642 
9643 
9644 
9645 
9646 
9647 
9648 
9649 
