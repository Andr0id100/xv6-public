2400 #include "types.h"
2401 #include "defs.h"
2402 #include "param.h"
2403 #include "memlayout.h"
2404 #include "mmu.h"
2405 #include "x86.h"
2406 #include "proc.h"
2407 #include "spinlock.h"
2408 
2409 #include "schedulers.h"
2410 
2411 struct {
2412   struct spinlock lock;
2413   struct proc proc[NPROC];
2414 } ptable;
2415 
2416 static struct proc *initproc;
2417 
2418 int nextpid = 1;
2419 extern void forkret(void);
2420 extern void trapret(void);
2421 
2422 static void wakeup1(void *chan);
2423 
2424 void
2425 pinit(void)
2426 {
2427   initlock(&ptable.lock, "ptable");
2428 }
2429 
2430 // Must be called with interrupts disabled
2431 int
2432 cpuid() {
2433   return mycpu()-cpus;
2434 }
2435 
2436 
2437 
2438 
2439 
2440 
2441 
2442 
2443 
2444 
2445 
2446 
2447 
2448 
2449 
2450 // Must be called with interrupts disabled to avoid the caller being
2451 // rescheduled between reading lapicid and running through the loop.
2452 struct cpu*
2453 mycpu(void)
2454 {
2455   int apicid, i;
2456 
2457   if(readeflags()&FL_IF)
2458     panic("mycpu called with interrupts enabled\n");
2459 
2460   apicid = lapicid();
2461   // APIC IDs are not guaranteed to be contiguous. Maybe we should have
2462   // a reverse map, or reserve a register to store &cpus[i].
2463   for (i = 0; i < ncpu; ++i) {
2464     if (cpus[i].apicid == apicid)
2465       return &cpus[i];
2466   }
2467   panic("unknown apicid\n");
2468 }
2469 
2470 // Disable interrupts so that we are not rescheduled
2471 // while reading proc from the cpu structure
2472 struct proc*
2473 myproc(void) {
2474   struct cpu *c;
2475   struct proc *p;
2476   pushcli();
2477   c = mycpu();
2478   p = c->proc;
2479   popcli();
2480   return p;
2481 }
2482 
2483 
2484 
2485 
2486 
2487 
2488 
2489 
2490 
2491 
2492 
2493 
2494 
2495 
2496 
2497 
2498 
2499 
2500 // Look in the process table for an UNUSED proc.
2501 // If found, change state to EMBRYO and initialize
2502 // state required to run in the kernel.
2503 // Otherwise return 0.
2504 static struct proc*
2505 allocproc(void)
2506 {
2507   struct proc *p;
2508   char *sp;
2509 
2510   acquire(&ptable.lock);
2511 
2512   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2513     if(p->state == UNUSED)
2514       goto found;
2515 
2516   release(&ptable.lock);
2517   return 0;
2518 
2519 found:
2520   p->state = EMBRYO;
2521   p->pid = nextpid++;
2522 
2523   acquire(&tickslock);
2524   p->ctime = ticks;
2525   release(&tickslock);
2526 
2527   p->rtime = 0;
2528   p->stime = 0;
2529   p->wtime = 0;
2530 
2531   p->priority = 60;
2532   p->n_run = 0;
2533   p->cur_q = -1;
2534 
2535   for (int i=0;i<5;i++) {
2536     p->queue_tick[i] = 0;
2537   }
2538 
2539   release(&ptable.lock);
2540 
2541   // Allocate kernel stack.
2542   if((p->kstack = kalloc()) == 0){
2543     p->state = UNUSED;
2544     return 0;
2545   }
2546   sp = p->kstack + KSTACKSIZE;
2547 
2548 
2549 
2550   // Leave room for trap frame.
2551   sp -= sizeof *p->tf;
2552   p->tf = (struct trapframe*)sp;
2553 
2554   // Set up new context to start executing at forkret,
2555   // which returns to trapret.
2556   sp -= 4;
2557   *(uint*)sp = (uint)trapret;
2558 
2559   sp -= sizeof *p->context;
2560   p->context = (struct context*)sp;
2561   memset(p->context, 0, sizeof *p->context);
2562   p->context->eip = (uint)forkret;
2563 
2564   return p;
2565 }
2566 
2567 
2568 // Set up first user process.
2569 void
2570 userinit(void)
2571 {
2572   struct proc *p;
2573   extern char _binary_initcode_start[], _binary_initcode_size[];
2574 
2575   p = allocproc();
2576 
2577   initproc = p;
2578   if((p->pgdir = setupkvm()) == 0)
2579     panic("userinit: out of memory?");
2580   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2581   p->sz = PGSIZE;
2582   memset(p->tf, 0, sizeof(*p->tf));
2583   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2584   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2585   p->tf->es = p->tf->ds;
2586   p->tf->ss = p->tf->ds;
2587   p->tf->eflags = FL_IF;
2588   p->tf->esp = PGSIZE;
2589   p->tf->eip = 0;  // beginning of initcode.S
2590 
2591   safestrcpy(p->name, "initcode", sizeof(p->name));
2592   p->cwd = namei("/");
2593 
2594   // this assignment to p->state lets other cores
2595   // run this process. the acquire forces the above
2596   // writes to be visible, and the lock is also needed
2597   // because the assignment might not be atomic.
2598   acquire(&ptable.lock);
2599 
2600   p->state = RUNNABLE;
2601 
2602   release(&ptable.lock);
2603 }
2604 
2605 // Grow current process's memory by n bytes.
2606 // Return 0 on success, -1 on failure.
2607 int
2608 growproc(int n)
2609 {
2610   uint sz;
2611   struct proc *curproc = myproc();
2612 
2613   sz = curproc->sz;
2614   if(n > 0){
2615     if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
2616       return -1;
2617   } else if(n < 0){
2618     if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
2619       return -1;
2620   }
2621   curproc->sz = sz;
2622   switchuvm(curproc);
2623   return 0;
2624 }
2625 
2626 // Create a new process copying p as the parent.
2627 // Sets up stack to return as if from system call.
2628 // Caller must set state of returned proc to RUNNABLE.
2629 int
2630 fork(void)
2631 {
2632   int i, pid;
2633   struct proc *np;
2634   struct proc *curproc = myproc();
2635 
2636   // Allocate process.
2637   if((np = allocproc()) == 0){
2638     return -1;
2639   }
2640 
2641   // Copy process state from proc.
2642   if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
2643     kfree(np->kstack);
2644     np->kstack = 0;
2645     np->state = UNUSED;
2646     return -1;
2647   }
2648   np->sz = curproc->sz;
2649   np->parent = curproc;
2650   *np->tf = *curproc->tf;
2651 
2652   // Clear %eax so that fork returns 0 in the child.
2653   np->tf->eax = 0;
2654 
2655   for(i = 0; i < NOFILE; i++)
2656     if(curproc->ofile[i])
2657       np->ofile[i] = filedup(curproc->ofile[i]);
2658   np->cwd = idup(curproc->cwd);
2659 
2660   safestrcpy(np->name, curproc->name, sizeof(curproc->name));
2661 
2662   pid = np->pid;
2663 
2664   acquire(&ptable.lock);
2665 
2666   np->state = RUNNABLE;
2667 
2668   release(&ptable.lock);
2669 
2670   return pid;
2671 }
2672 
2673 // Exit the current process.  Does not return.
2674 // An exited process remains in the zombie state
2675 // until its parent calls wait() to find out it exited.
2676 void
2677 exit(void)
2678 {
2679   struct proc *curproc = myproc();
2680   struct proc *p;
2681   int fd;
2682 
2683   if(curproc == initproc)
2684     panic("init exiting");
2685 
2686   // Close all open files.
2687   for(fd = 0; fd < NOFILE; fd++){
2688     if(curproc->ofile[fd]){
2689       fileclose(curproc->ofile[fd]);
2690       curproc->ofile[fd] = 0;
2691     }
2692   }
2693 
2694   begin_op();
2695   iput(curproc->cwd);
2696   end_op();
2697   curproc->cwd = 0;
2698 
2699   acquire(&ptable.lock);
2700   // Parent might be sleeping in wait().
2701   wakeup1(curproc->parent);
2702 
2703   // Pass abandoned children to init.
2704   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2705     if(p->parent == curproc){
2706       p->parent = initproc;
2707       if(p->state == ZOMBIE)
2708         wakeup1(initproc);
2709     }
2710   }
2711 
2712   // Jump into the scheduler, never to return.
2713   curproc->state = ZOMBIE;
2714   acquire(&tickslock);
2715   curproc->etime = ticks;
2716   release(&tickslock);
2717 
2718   sched();
2719   panic("zombie exit");
2720 }
2721 
2722 // Wait for a child process to exit and return its pid.
2723 // Return -1 if this process has no children.
2724 int
2725 wait(void)
2726 {
2727   struct proc *p;
2728   int havekids, pid;
2729   struct proc *curproc = myproc();
2730 
2731   acquire(&ptable.lock);
2732   for(;;){
2733     // Scan through table looking for exited children.
2734     havekids = 0;
2735     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2736       if(p->parent != curproc)
2737         continue;
2738       havekids = 1;
2739       if(p->state == ZOMBIE){
2740         // Found one.
2741         pid = p->pid;
2742         kfree(p->kstack);
2743         p->kstack = 0;
2744         freevm(p->pgdir);
2745         p->pid = 0;
2746         p->parent = 0;
2747         p->name[0] = 0;
2748         p->killed = 0;
2749         p->state = UNUSED;
2750         release(&ptable.lock);
2751         return pid;
2752       }
2753     }
2754 
2755     // No point waiting if we don't have any children.
2756     if(!havekids || curproc->killed){
2757       release(&ptable.lock);
2758       return -1;
2759     }
2760 
2761     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2762     sleep(curproc, &ptable.lock);  //DOC: wait-sleep
2763   }
2764 }
2765 
2766 int waitx(int* wtime, int* rtime) {
2767   *wtime = 0;
2768   *rtime = 0;
2769 
2770   // int start_time = ticks;
2771 
2772   struct proc *p;
2773   int havekids, pid;
2774   struct proc *curproc = myproc();
2775 
2776   acquire(&ptable.lock);
2777   for(;;){
2778     // Scan through table looking for exited children.
2779     havekids = 0;
2780     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2781       if(p->parent != curproc)
2782         continue;
2783       havekids = 1;
2784       if(p->state == ZOMBIE){
2785         // Found one.
2786         pid = p->pid;
2787         kfree(p->kstack);
2788         p->kstack = 0;
2789         freevm(p->pgdir);
2790         p->pid = 0;
2791         p->parent = 0;
2792         p->name[0] = 0;
2793         p->killed = 0;
2794         p->state = UNUSED;
2795 
2796         // *rtime = p->wtime;
2797         *rtime = p->rtime;
2798         *wtime = p->etime - p->ctime - p->rtime - p->stime;
2799 
2800         release(&ptable.lock);
2801 
2802 
2803         return pid;
2804       }
2805     }
2806 
2807     // No point waiting if we don't have any children.
2808     if(!havekids || curproc->killed){
2809       release(&ptable.lock);
2810       *wtime = 0;
2811       *rtime = 0;
2812       return -1;
2813     }
2814 
2815     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2816     sleep(curproc, &ptable.lock);  //DOC: wait-sleep
2817   }
2818 }
2819 
2820 
2821 
2822 
2823 
2824 
2825 
2826 
2827 
2828 
2829 
2830 
2831 
2832 
2833 
2834 
2835 
2836 
2837 
2838 
2839 
2840 
2841 
2842 
2843 
2844 
2845 
2846 
2847 
2848 
2849 
2850 // Per-CPU process scheduler.
2851 // Each CPU calls scheduler() after setting itself up.
2852 // Scheduler never returns.  It loops, doing:
2853 //  - choose a process to run
2854 //  - swtch to start running that process
2855 //  - eventually that process transfers control
2856 //      via swtch back to the scheduler.
2857 void
2858 scheduler(void)
2859 {
2860   struct proc *p ;
2861   struct cpu *c = mycpu();
2862   c->proc = 0;
2863 
2864   for (;;) {
2865     sti();
2866     acquire(&ptable.lock);
2867 
2868     p = get_process(ptable.proc);
2869     if (p == 0) {
2870       continue;
2871     }
2872 
2873     c->proc = p;
2874     switchuvm(p);
2875 
2876     p->state = RUNNING;
2877     p->wtime = 0;
2878 
2879     swtch(&(c->scheduler), p->context);
2880     switchkvm();
2881 
2882     c->proc = 0;
2883     release(&ptable.lock);
2884 
2885   }
2886 
2887 
2888 
2889   // struct proc *p;
2890   // struct cpu *c = mycpu();
2891   // c->proc = 0;
2892 
2893   // for(;;){
2894   //   // Enable interrupts on this processor.
2895   //   sti();
2896 
2897 
2898 
2899 
2900   //   // Loop over process table looking for process to run.
2901   //   acquire(&ptable.lock);
2902   //   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2903   //     if(p->state != RUNNABLE)
2904   //       continue;
2905 
2906   //     // Switch to chosen process.  It is the process's job
2907   //     // to release ptable.lock and then reacquire it
2908   //     // before jumping back to us.
2909   //     c->proc = p;
2910 
2911   //     switchuvm(p);
2912       // p->state = RUNNING;
2913   //     p->wtime = 0;
2914 
2915   //     swtch(&(c->scheduler), p->context);
2916   //     switchkvm();
2917 
2918   //     // Process is done running for now.
2919   //     // It should have changed its p->state before coming back.
2920   //     c->proc = 0;
2921   //   }
2922   //   release(&ptable.lock);
2923 
2924   // }
2925 }
2926 
2927 // Enter scheduler.  Must hold only ptable.lock
2928 // and have changed proc->state. Saves and restores
2929 // intena because intena is a property of this
2930 // kernel thread, not this CPU. It should
2931 // be proc->intena and proc->ncli, but that would
2932 // break in the few places where a lock is held but
2933 // there's no process.
2934 void
2935 sched(void)
2936 {
2937   int intena;
2938   struct proc *p = myproc();
2939 
2940   if(!holding(&ptable.lock))
2941     panic("sched ptable.lock");
2942   if(mycpu()->ncli != 1)
2943     panic("sched locks");
2944   if(p->state == RUNNING)
2945     panic("sched running");
2946   if(readeflags()&FL_IF)
2947     panic("sched interruptible");
2948   intena = mycpu()->intena;
2949   swtch(&p->context, mycpu()->scheduler);
2950   mycpu()->intena = intena;
2951 }
2952 
2953 // Give up the CPU for one scheduling round.
2954 void
2955 yield(void)
2956 {
2957   acquire(&ptable.lock);  //DOC: yieldlock
2958   myproc()->state = RUNNABLE;
2959   sched();
2960   release(&ptable.lock);
2961 }
2962 
2963 // A fork child's very first scheduling by scheduler()
2964 // will swtch here.  "Return" to user space.
2965 void
2966 forkret(void)
2967 {
2968   static int first = 1;
2969   // Still holding ptable.lock from scheduler.
2970   release(&ptable.lock);
2971 
2972   if (first) {
2973     // Some initialization functions must be run in the context
2974     // of a regular process (e.g., they call sleep), and thus cannot
2975     // be run from main().
2976     first = 0;
2977     iinit(ROOTDEV);
2978     initlog(ROOTDEV);
2979   }
2980 
2981   // Return to "caller", actually trapret (see allocproc).
2982 }
2983 
2984 // Atomically release lock and sleep on chan.
2985 // Reacquires lock when awakened.
2986 void
2987 sleep(void *chan, struct spinlock *lk)
2988 {
2989   struct proc *p = myproc();
2990 
2991   if(p == 0)
2992     panic("sleep");
2993 
2994   if(lk == 0)
2995     panic("sleep without lk");
2996 
2997 
2998 
2999 
3000   // Must acquire ptable.lock in order to
3001   // change p->state and then call sched.
3002   // Once we hold ptable.lock, we can be
3003   // guaranteed that we won't miss any wakeup
3004   // (wakeup runs with ptable.lock locked),
3005   // so it's okay to release lk.
3006   if(lk != &ptable.lock){  //DOC: sleeplock0
3007     acquire(&ptable.lock);  //DOC: sleeplock1
3008     release(lk);
3009   }
3010   // Go to sleep.
3011   p->chan = chan;
3012   p->state = SLEEPING;
3013 
3014   sched();
3015 
3016   // Tidy up.
3017   p->chan = 0;
3018 
3019   // Reacquire original lock.
3020   if(lk != &ptable.lock){  //DOC: sleeplock2
3021     release(&ptable.lock);
3022     acquire(lk);
3023   }
3024 }
3025 
3026 
3027 
3028 
3029 
3030 
3031 
3032 
3033 
3034 
3035 
3036 
3037 
3038 
3039 
3040 
3041 
3042 
3043 
3044 
3045 
3046 
3047 
3048 
3049 
3050 // Wake up all processes sleeping on chan.
3051 // The ptable lock must be held.
3052 static void
3053 wakeup1(void *chan)
3054 {
3055   struct proc *p;
3056 
3057   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
3058     if(p->state == SLEEPING && p->chan == chan)
3059       p->state = RUNNABLE;
3060 }
3061 
3062 // Wake up all processes sleeping on chan.
3063 void
3064 wakeup(void *chan)
3065 {
3066   acquire(&ptable.lock);
3067   wakeup1(chan);
3068   release(&ptable.lock);
3069 }
3070 
3071 // Kill the process with the given pid.
3072 // Process won't exit until it returns
3073 // to user space (see trap in trap.c).
3074 int
3075 kill(int pid)
3076 {
3077   struct proc *p;
3078 
3079   acquire(&ptable.lock);
3080   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3081     if(p->pid == pid){
3082       p->killed = 1;
3083       // Wake process from sleep if necessary.
3084       if(p->state == SLEEPING)
3085         p->state = RUNNABLE;
3086       release(&ptable.lock);
3087       return 0;
3088     }
3089   }
3090   release(&ptable.lock);
3091   return -1;
3092 }
3093 
3094 
3095 
3096 
3097 
3098 
3099 
3100 // Print a process listing to console.  For debugging.
3101 // Runs when user types ^P on console.
3102 // No lock to avoid wedging a stuck machine further.
3103 void
3104 procdump(void)
3105 {
3106   static char *states[] = {
3107   [UNUSED]    "unused",
3108   [EMBRYO]    "embryo",
3109   [SLEEPING]  "sleep ",
3110   [RUNNABLE]  "runble",
3111   [RUNNING]   "run   ",
3112   [ZOMBIE]    "zombie"
3113   };
3114   int i;
3115   struct proc *p;
3116   char *state;
3117   uint pc[10];
3118 
3119   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3120     if(p->state == UNUSED)
3121       continue;
3122     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
3123       state = states[p->state];
3124     else
3125       state = "???";
3126     cprintf("%d %s %s", p->pid, state, p->name);
3127     if(p->state == SLEEPING){
3128       getcallerpcs((uint*)p->context->ebp+2, pc);
3129       for(i=0; i<10 && pc[i] != 0; i++)
3130         cprintf(" %p", pc[i]);
3131     }
3132     cprintf("\n");
3133   }
3134 }
3135 
3136 
3137 
3138 
3139 
3140 
3141 
3142 
3143 
3144 
3145 
3146 
3147 
3148 
3149 
3150 void ps(void) {
3151   static char *states[] = {
3152   [UNUSED]    "unused",
3153   [EMBRYO]    "embryo",
3154   [SLEEPING]  "sleeping ",
3155   [RUNNABLE]  "runable",
3156   [RUNNING]   "running",
3157   [ZOMBIE]    "zombie"
3158   };
3159 
3160   acquire(&ptable.lock);
3161   cprintf("PID\tPriority State   \tr_time\tw_time\tn_run\tcur_q");
3162   for (int i=0;i<5;i++) {
3163     cprintf("\tq%d", i);
3164   }
3165   cprintf("\n");
3166 
3167   struct proc *p;
3168   for (p=ptable.proc; p < &ptable.proc[NPROC]; p++) {
3169     if (p->pid == 0) {
3170       continue;
3171     }
3172     acquire(&tickslock);
3173     cprintf("%d\t%d\t %s\t%d\t%d\t%d\t%d",
3174       p->pid,
3175       p->priority,
3176       states[p->state],
3177       p->rtime,
3178       p->wtime,
3179       p->n_run,
3180       p->cur_q
3181     );
3182     release(&tickslock);
3183     for (int j=0;j<5;j++) {
3184       cprintf("\t%d", p->queue_tick[j]);
3185     }
3186     cprintf("\n");
3187 
3188   }
3189 
3190   release(&ptable.lock);
3191 }
3192 
3193 
3194 
3195 
3196 
3197 
3198 
3199 
3200 void update_times() {
3201   acquire(&ptable.lock);
3202   struct proc *p;
3203   for (p=ptable.proc; p<&ptable.proc[NPROC]; p++) {
3204     if (p->state == SLEEPING) {
3205       p->stime++;
3206     }
3207     else if(p->state == RUNNABLE) {
3208       p->wtime++;
3209     }
3210   }
3211 
3212   release(&ptable.lock);
3213 }
3214 
3215 
3216 
3217 
3218 
3219 
3220 
3221 
3222 
3223 
3224 
3225 
3226 
3227 
3228 
3229 
3230 
3231 
3232 
3233 
3234 
3235 
3236 
3237 
3238 
3239 
3240 
3241 
3242 
3243 
3244 
3245 
3246 
3247 
3248 
3249 
