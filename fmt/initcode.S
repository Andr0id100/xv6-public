8650 # Initial process execs /init.
8651 # This code runs in user space.
8652 
8653 #include "syscall.h"
8654 #include "traps.h"
8655 
8656 
8657 # exec(init, argv)
8658 .globl start
8659 start:
8660   pushl $argv
8661   pushl $init
8662   pushl $0  // where caller pc would be
8663   movl $SYS_exec, %eax
8664   int $T_SYSCALL
8665 
8666 # for(;;) exit();
8667 exit:
8668   movl $SYS_exit, %eax
8669   int $T_SYSCALL
8670   jmp exit
8671 
8672 # char init[] = "/init\0";
8673 init:
8674   .string "/init\0"
8675 
8676 # char *argv[] = { init, 0 };
8677 .p2align 2
8678 argv:
8679   .long init
8680   .long 0
8681 
8682 
8683 
8684 
8685 
8686 
8687 
8688 
8689 
8690 
8691 
8692 
8693 
8694 
8695 
8696 
8697 
8698 
8699 
