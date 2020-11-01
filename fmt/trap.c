3550 #include "types.h"
3551 #include "defs.h"
3552 #include "param.h"
3553 #include "memlayout.h"
3554 #include "mmu.h"
3555 #include "proc.h"
3556 #include "x86.h"
3557 #include "traps.h"
3558 #include "spinlock.h"
3559 
3560 // Interrupt descriptor table (shared by all CPUs).
3561 struct gatedesc idt[256];
3562 extern uint vectors[];  // in vectors.S: array of 256 entry pointers
3563 struct spinlock tickslock;
3564 uint ticks;
3565 
3566 void
3567 tvinit(void)
3568 {
3569   int i;
3570 
3571   for(i = 0; i < 256; i++)
3572     SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
3573   SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
3574 
3575   initlock(&tickslock, "time");
3576 }
3577 
3578 void
3579 idtinit(void)
3580 {
3581   lidt(idt, sizeof(idt));
3582 }
3583 
3584 
3585 
3586 
3587 
3588 
3589 
3590 
3591 
3592 
3593 
3594 
3595 
3596 
3597 
3598 
3599 
3600 void
3601 trap(struct trapframe *tf)
3602 {
3603   if(tf->trapno == T_SYSCALL){
3604     if(myproc()->killed)
3605       exit();
3606     myproc()->tf = tf;
3607     syscall();
3608     if(myproc()->killed)
3609       exit();
3610     return;
3611   }
3612 
3613   switch(tf->trapno){
3614   case T_IRQ0 + IRQ_TIMER:
3615     if(cpuid() == 0){
3616       acquire(&tickslock);
3617       ticks++;
3618       wakeup(&ticks);
3619       release(&tickslock);
3620 
3621       // If we are here it means that we should update process if it ran
3622       struct proc* p = myproc();
3623 
3624       if (p) {
3625         if (p->state == RUNNING) {
3626           p->rtime++;
3627         }
3628       }
3629 
3630       update_times();
3631 
3632     }
3633     lapiceoi();
3634 
3635     break;
3636   case T_IRQ0 + IRQ_IDE:
3637     ideintr();
3638     lapiceoi();
3639     break;
3640   case T_IRQ0 + IRQ_IDE+1:
3641     // Bochs generates spurious IDE1 interrupts.
3642     break;
3643   case T_IRQ0 + IRQ_KBD:
3644     kbdintr();
3645     lapiceoi();
3646     break;
3647   case T_IRQ0 + IRQ_COM1:
3648     uartintr();
3649     lapiceoi();
3650     break;
3651   case T_IRQ0 + 7:
3652   case T_IRQ0 + IRQ_SPURIOUS:
3653     cprintf("cpu%d: spurious interrupt at %x:%x\n",
3654             cpuid(), tf->cs, tf->eip);
3655     lapiceoi();
3656     break;
3657 
3658 
3659   default:
3660     if(myproc() == 0 || (tf->cs&3) == 0){
3661       // In kernel, it must be our mistake.
3662       cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
3663               tf->trapno, cpuid(), tf->eip, rcr2());
3664       panic("trap");
3665     }
3666     // In user space, assume process misbehaved.
3667     cprintf("pid %d %s: trap %d err %d on cpu %d "
3668             "eip 0x%x addr 0x%x--kill proc\n",
3669             myproc()->pid, myproc()->name, tf->trapno,
3670             tf->err, cpuid(), tf->eip, rcr2());
3671     myproc()->killed = 1;
3672   }
3673 
3674   // Force process exit if it has been killed and is in user space.
3675   // (If it is still executing in the kernel, let it keep running
3676   // until it gets to the regular system call return.)
3677   if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
3678     exit();
3679 
3680   // Force process to give up CPU on clock tick.
3681   // If interrupts were on while locks held, would need to check nlock.
3682   if(myproc() && myproc()->state == RUNNING &&
3683      tf->trapno == T_IRQ0+IRQ_TIMER)
3684     yield();
3685 
3686   // Check if the process has been killed since we yielded
3687   if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
3688     exit();
3689 }
3690 
3691 
3692 
3693 
3694 
3695 
3696 
3697 
3698 
3699 
