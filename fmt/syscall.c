3750 #include "types.h"
3751 #include "defs.h"
3752 #include "param.h"
3753 #include "memlayout.h"
3754 #include "mmu.h"
3755 #include "proc.h"
3756 #include "x86.h"
3757 #include "syscall.h"
3758 
3759 // User code makes a system call with INT T_SYSCALL.
3760 // System call number in %eax.
3761 // Arguments on the stack, from the user call to the C
3762 // library system call function. The saved user %esp points
3763 // to a saved program counter, and then the first argument.
3764 
3765 // Fetch the int at addr from the current process.
3766 int
3767 fetchint(uint addr, int *ip)
3768 {
3769   struct proc *curproc = myproc();
3770 
3771   if(addr >= curproc->sz || addr+4 > curproc->sz)
3772     return -1;
3773   *ip = *(int*)(addr);
3774   return 0;
3775 }
3776 
3777 // Fetch the nul-terminated string at addr from the current process.
3778 // Doesn't actually copy the string - just sets *pp to point at it.
3779 // Returns length of string, not including nul.
3780 int
3781 fetchstr(uint addr, char **pp)
3782 {
3783   char *s, *ep;
3784   struct proc *curproc = myproc();
3785 
3786   if(addr >= curproc->sz)
3787     return -1;
3788   *pp = (char*)addr;
3789   ep = (char*)curproc->sz;
3790   for(s = *pp; s < ep; s++){
3791     if(*s == 0)
3792       return s - *pp;
3793   }
3794   return -1;
3795 }
3796 
3797 
3798 
3799 
3800 // Fetch the nth 32-bit system call argument.
3801 int
3802 argint(int n, int *ip)
3803 {
3804   return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
3805 }
3806 
3807 // Fetch the nth word-sized system call argument as a pointer
3808 // to a block of memory of size bytes.  Check that the pointer
3809 // lies within the process address space.
3810 int
3811 argptr(int n, char **pp, int size)
3812 {
3813   int i;
3814   struct proc *curproc = myproc();
3815 
3816   if(argint(n, &i) < 0)
3817     return -1;
3818   if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
3819     return -1;
3820   *pp = (char*)i;
3821   return 0;
3822 }
3823 
3824 // Fetch the nth word-sized system call argument as a string pointer.
3825 // Check that the pointer is valid and the string is nul-terminated.
3826 // (There is no shared writable memory, so the string can't change
3827 // between this check and being used by the kernel.)
3828 int
3829 argstr(int n, char **pp)
3830 {
3831   int addr;
3832   if(argint(n, &addr) < 0)
3833     return -1;
3834   return fetchstr(addr, pp);
3835 }
3836 
3837 
3838 
3839 
3840 
3841 
3842 
3843 
3844 
3845 
3846 
3847 
3848 
3849 
3850 extern int sys_chdir(void);
3851 extern int sys_close(void);
3852 extern int sys_dup(void);
3853 extern int sys_exec(void);
3854 extern int sys_exit(void);
3855 extern int sys_fork(void);
3856 extern int sys_fstat(void);
3857 extern int sys_getpid(void);
3858 extern int sys_kill(void);
3859 extern int sys_link(void);
3860 extern int sys_mkdir(void);
3861 extern int sys_mknod(void);
3862 extern int sys_open(void);
3863 extern int sys_pipe(void);
3864 extern int sys_read(void);
3865 extern int sys_sbrk(void);
3866 extern int sys_sleep(void);
3867 extern int sys_unlink(void);
3868 extern int sys_wait(void);
3869 extern int sys_write(void);
3870 extern int sys_uptime(void);
3871 
3872 extern int sys_ps(void);
3873 extern int sys_waitx(void);
3874 
3875 static int (*syscalls[])(void) = {
3876 [SYS_fork]    sys_fork,
3877 [SYS_exit]    sys_exit,
3878 [SYS_wait]    sys_wait,
3879 [SYS_pipe]    sys_pipe,
3880 [SYS_read]    sys_read,
3881 [SYS_kill]    sys_kill,
3882 [SYS_exec]    sys_exec,
3883 [SYS_fstat]   sys_fstat,
3884 [SYS_chdir]   sys_chdir,
3885 [SYS_dup]     sys_dup,
3886 [SYS_getpid]  sys_getpid,
3887 [SYS_sbrk]    sys_sbrk,
3888 [SYS_sleep]   sys_sleep,
3889 [SYS_uptime]  sys_uptime,
3890 [SYS_open]    sys_open,
3891 [SYS_write]   sys_write,
3892 [SYS_mknod]   sys_mknod,
3893 [SYS_unlink]  sys_unlink,
3894 [SYS_link]    sys_link,
3895 [SYS_mkdir]   sys_mkdir,
3896 [SYS_close]   sys_close,
3897 
3898 
3899 
3900 [SYS_ps]      sys_ps,
3901 [SYS_waitx]   sys_waitx,
3902 };
3903 
3904 void
3905 syscall(void)
3906 {
3907   int num;
3908   struct proc *curproc = myproc();
3909 
3910   num = curproc->tf->eax;
3911   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
3912     curproc->tf->eax = syscalls[num]();
3913   } else {
3914     cprintf("%d %s: unknown sys call %d\n",
3915             curproc->pid, curproc->name, num);
3916     curproc->tf->eax = -1;
3917   }
3918 }
3919 
3920 
3921 
3922 
3923 
3924 
3925 
3926 
3927 
3928 
3929 
3930 
3931 
3932 
3933 
3934 
3935 
3936 
3937 
3938 
3939 
3940 
3941 
3942 
3943 
3944 
3945 
3946 
3947 
3948 
3949 
