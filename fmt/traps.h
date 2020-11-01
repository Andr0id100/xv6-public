3400 // x86 trap and interrupt constants.
3401 
3402 // Processor-defined:
3403 #define T_DIVIDE         0      // divide error
3404 #define T_DEBUG          1      // debug exception
3405 #define T_NMI            2      // non-maskable interrupt
3406 #define T_BRKPT          3      // breakpoint
3407 #define T_OFLOW          4      // overflow
3408 #define T_BOUND          5      // bounds check
3409 #define T_ILLOP          6      // illegal opcode
3410 #define T_DEVICE         7      // device not available
3411 #define T_DBLFLT         8      // double fault
3412 // #define T_COPROC      9      // reserved (not used since 486)
3413 #define T_TSS           10      // invalid task switch segment
3414 #define T_SEGNP         11      // segment not present
3415 #define T_STACK         12      // stack exception
3416 #define T_GPFLT         13      // general protection fault
3417 #define T_PGFLT         14      // page fault
3418 // #define T_RES        15      // reserved
3419 #define T_FPERR         16      // floating point error
3420 #define T_ALIGN         17      // aligment check
3421 #define T_MCHK          18      // machine check
3422 #define T_SIMDERR       19      // SIMD floating point error
3423 
3424 // These are arbitrarily chosen, but with care not to overlap
3425 // processor defined exceptions or interrupt vectors.
3426 #define T_SYSCALL       64      // system call
3427 #define T_DEFAULT      500      // catchall
3428 
3429 #define T_IRQ0          32      // IRQ 0 corresponds to int T_IRQ
3430 
3431 #define IRQ_TIMER        0
3432 #define IRQ_KBD          1
3433 #define IRQ_COM1         4
3434 #define IRQ_IDE         14
3435 #define IRQ_ERROR       19
3436 #define IRQ_SPURIOUS    31
3437 
3438 
3439 
3440 
3441 
3442 
3443 
3444 
3445 
3446 
3447 
3448 
3449 
