9350 #include "asm.h"
9351 #include "memlayout.h"
9352 #include "mmu.h"
9353 
9354 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9355 # The BIOS loads this code from the first sector of the hard disk into
9356 # memory at physical address 0x7c00 and starts executing in real mode
9357 # with %cs=0 %ip=7c00.
9358 
9359 .code16                       # Assemble for 16-bit mode
9360 .globl start
9361 start:
9362   cli                         # BIOS enabled interrupts; disable
9363 
9364   # Zero data segment registers DS, ES, and SS.
9365   xorw    %ax,%ax             # Set %ax to zero
9366   movw    %ax,%ds             # -> Data Segment
9367   movw    %ax,%es             # -> Extra Segment
9368   movw    %ax,%ss             # -> Stack Segment
9369 
9370   # Physical address line A20 is tied to zero so that the first PCs
9371   # with 2 MB would run software that assumed 1 MB.  Undo that.
9372 seta20.1:
9373   inb     $0x64,%al               # Wait for not busy
9374   testb   $0x2,%al
9375   jnz     seta20.1
9376 
9377   movb    $0xd1,%al               # 0xd1 -> port 0x64
9378   outb    %al,$0x64
9379 
9380 seta20.2:
9381   inb     $0x64,%al               # Wait for not busy
9382   testb   $0x2,%al
9383   jnz     seta20.2
9384 
9385   movb    $0xdf,%al               # 0xdf -> port 0x60
9386   outb    %al,$0x60
9387 
9388   # Switch from real to protected mode.  Use a bootstrap GDT that makes
9389   # virtual addresses map directly to physical addresses so that the
9390   # effective memory map doesn't change during the transition.
9391   lgdt    gdtdesc
9392   movl    %cr0, %eax
9393   orl     $CR0_PE, %eax
9394   movl    %eax, %cr0
9395 
9396 
9397 
9398 
9399 
9400   # Complete the transition to 32-bit protected mode by using a long jmp
9401   # to reload %cs and %eip.  The segment descriptors are set up with no
9402   # translation, so that the mapping is still the identity mapping.
9403   ljmp    $(SEG_KCODE<<3), $start32
9404 
9405 .code32  # Tell assembler to generate 32-bit code now.
9406 start32:
9407   # Set up the protected-mode data segment registers
9408   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
9409   movw    %ax, %ds                # -> DS: Data Segment
9410   movw    %ax, %es                # -> ES: Extra Segment
9411   movw    %ax, %ss                # -> SS: Stack Segment
9412   movw    $0, %ax                 # Zero segments not ready for use
9413   movw    %ax, %fs                # -> FS
9414   movw    %ax, %gs                # -> GS
9415 
9416   # Set up the stack pointer and call into C.
9417   movl    $start, %esp
9418   call    bootmain
9419 
9420   # If bootmain returns (it shouldn't), trigger a Bochs
9421   # breakpoint if running under Bochs, then loop.
9422   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
9423   movw    %ax, %dx
9424   outw    %ax, %dx
9425   movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
9426   outw    %ax, %dx
9427 spin:
9428   jmp     spin
9429 
9430 # Bootstrap GDT
9431 .p2align 2                                # force 4 byte alignment
9432 gdt:
9433   SEG_NULLASM                             # null seg
9434   SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
9435   SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
9436 
9437 gdtdesc:
9438   .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
9439   .long   gdt                             # address gdt
9440 
9441 
9442 
9443 
9444 
9445 
9446 
9447 
9448 
9449 
