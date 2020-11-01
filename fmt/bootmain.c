9450 // Boot loader.
9451 //
9452 // Part of the boot block, along with bootasm.S, which calls bootmain().
9453 // bootasm.S has put the processor into protected 32-bit mode.
9454 // bootmain() loads an ELF kernel image from the disk starting at
9455 // sector 1 and then jumps to the kernel entry routine.
9456 
9457 #include "types.h"
9458 #include "elf.h"
9459 #include "x86.h"
9460 #include "memlayout.h"
9461 
9462 #define SECTSIZE  512
9463 
9464 void readseg(uchar*, uint, uint);
9465 
9466 void
9467 bootmain(void)
9468 {
9469   struct elfhdr *elf;
9470   struct proghdr *ph, *eph;
9471   void (*entry)(void);
9472   uchar* pa;
9473 
9474   elf = (struct elfhdr*)0x10000;  // scratch space
9475 
9476   // Read 1st page off disk
9477   readseg((uchar*)elf, 4096, 0);
9478 
9479   // Is this an ELF executable?
9480   if(elf->magic != ELF_MAGIC)
9481     return;  // let bootasm.S handle error
9482 
9483   // Load each program segment (ignores ph flags).
9484   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9485   eph = ph + elf->phnum;
9486   for(; ph < eph; ph++){
9487     pa = (uchar*)ph->paddr;
9488     readseg(pa, ph->filesz, ph->off);
9489     if(ph->memsz > ph->filesz)
9490       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9491   }
9492 
9493   // Call the entry point from the ELF header.
9494   // Does not return!
9495   entry = (void(*)(void))(elf->entry);
9496   entry();
9497 }
9498 
9499 
9500 void
9501 waitdisk(void)
9502 {
9503   // Wait for disk ready.
9504   while((inb(0x1F7) & 0xC0) != 0x40)
9505     ;
9506 }
9507 
9508 // Read a single sector at offset into dst.
9509 void
9510 readsect(void *dst, uint offset)
9511 {
9512   // Issue command.
9513   waitdisk();
9514   outb(0x1F2, 1);   // count = 1
9515   outb(0x1F3, offset);
9516   outb(0x1F4, offset >> 8);
9517   outb(0x1F5, offset >> 16);
9518   outb(0x1F6, (offset >> 24) | 0xE0);
9519   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9520 
9521   // Read data.
9522   waitdisk();
9523   insl(0x1F0, dst, SECTSIZE/4);
9524 }
9525 
9526 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9527 // Might copy more than asked.
9528 void
9529 readseg(uchar* pa, uint count, uint offset)
9530 {
9531   uchar* epa;
9532 
9533   epa = pa + count;
9534 
9535   // Round down to sector boundary.
9536   pa -= offset % SECTSIZE;
9537 
9538   // Translate from bytes to sectors; kernel starts at sector 1.
9539   offset = (offset / SECTSIZE) + 1;
9540 
9541   // If this is too slow, we could read lots of sectors at a time.
9542   // We'd write more to memory than asked, but it doesn't matter --
9543   // we load in increasing order.
9544   for(; pa < epa; pa += SECTSIZE, offset++)
9545     readsect(pa, offset);
9546 }
9547 
9548 
9549 
