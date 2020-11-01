3500 #include "mmu.h"
3501 
3502   # vectors.S sends all traps here.
3503 .globl alltraps
3504 alltraps:
3505   # Build trap frame.
3506   pushl %ds
3507   pushl %es
3508   pushl %fs
3509   pushl %gs
3510   pushal
3511 
3512   # Set up data segments.
3513   movw $(SEG_KDATA<<3), %ax
3514   movw %ax, %ds
3515   movw %ax, %es
3516 
3517   # Call trap(tf), where tf=%esp
3518   pushl %esp
3519   call trap
3520   addl $4, %esp
3521 
3522   # Return falls through to trapret...
3523 .globl trapret
3524 trapret:
3525   popal
3526   popl %gs
3527   popl %fs
3528   popl %es
3529   popl %ds
3530   addl $0x8, %esp  # trapno and errcode
3531   iret
3532 
3533 
3534 
3535 
3536 
3537 
3538 
3539 
3540 
3541 
3542 
3543 
3544 
3545 
3546 
3547 
3548 
3549 
