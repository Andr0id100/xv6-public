3950 #include "types.h"
3951 #include "x86.h"
3952 #include "defs.h"
3953 #include "date.h"
3954 #include "param.h"
3955 #include "memlayout.h"
3956 #include "mmu.h"
3957 #include "proc.h"
3958 
3959 int
3960 sys_fork(void)
3961 {
3962   return fork();
3963 }
3964 
3965 int
3966 sys_exit(void)
3967 {
3968   exit();
3969   return 0;  // not reached
3970 }
3971 
3972 int
3973 sys_wait(void)
3974 {
3975   return wait();
3976 }
3977 
3978 int
3979 sys_kill(void)
3980 {
3981   int pid;
3982 
3983   if(argint(0, &pid) < 0)
3984     return -1;
3985   return kill(pid);
3986 }
3987 
3988 int
3989 sys_getpid(void)
3990 {
3991   return myproc()->pid;
3992 }
3993 
3994 
3995 
3996 
3997 
3998 
3999 
4000 int
4001 sys_sbrk(void)
4002 {
4003   int addr;
4004   int n;
4005 
4006   if(argint(0, &n) < 0)
4007     return -1;
4008   addr = myproc()->sz;
4009   if(growproc(n) < 0)
4010     return -1;
4011   return addr;
4012 }
4013 
4014 int
4015 sys_sleep(void)
4016 {
4017   int n;
4018   uint ticks0;
4019 
4020   if(argint(0, &n) < 0)
4021     return -1;
4022   acquire(&tickslock);
4023   ticks0 = ticks;
4024   while(ticks - ticks0 < n){
4025     if(myproc()->killed){
4026       release(&tickslock);
4027       return -1;
4028     }
4029     sleep(&ticks, &tickslock);
4030   }
4031   release(&tickslock);
4032   return 0;
4033 }
4034 
4035 // return how many clock tick interrupts have occurred
4036 // since start.
4037 int
4038 sys_uptime(void)
4039 {
4040   uint xticks;
4041 
4042   acquire(&tickslock);
4043   xticks = ticks;
4044   release(&tickslock);
4045   return xticks;
4046 }
4047 
4048 
4049 
4050 int sys_ps(void) {
4051   ps();
4052   return 0;
4053 }
4054 
4055 int sys_waitx(void) {
4056   int  *wtime, *rtime;
4057 
4058 
4059   if (argptr(0, (void*)&wtime, sizeof(int*)) || (argptr(1, (void*)&rtime, sizeof(int*))))
4060     return -1;
4061 
4062   return waitx(wtime, rtime);
4063 }
4064 
4065 
4066 
4067 
4068 
4069 
4070 
4071 
4072 
4073 
4074 
4075 
4076 
4077 
4078 
4079 
4080 
4081 
4082 
4083 
4084 
4085 
4086 
4087 
4088 
4089 
4090 
4091 
4092 
4093 
4094 
4095 
4096 
4097 
4098 
4099 
