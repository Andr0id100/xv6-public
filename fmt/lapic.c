7600 // The local APIC manages internal (non-I/O) interrupts.
7601 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7602 
7603 #include "param.h"
7604 #include "types.h"
7605 #include "defs.h"
7606 #include "date.h"
7607 #include "memlayout.h"
7608 #include "traps.h"
7609 #include "mmu.h"
7610 #include "x86.h"
7611 
7612 // Local APIC registers, divided by 4 for use as uint[] indices.
7613 #define ID      (0x0020/4)   // ID
7614 #define VER     (0x0030/4)   // Version
7615 #define TPR     (0x0080/4)   // Task Priority
7616 #define EOI     (0x00B0/4)   // EOI
7617 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7618   #define ENABLE     0x00000100   // Unit Enable
7619 #define ESR     (0x0280/4)   // Error Status
7620 #define ICRLO   (0x0300/4)   // Interrupt Command
7621   #define INIT       0x00000500   // INIT/RESET
7622   #define STARTUP    0x00000600   // Startup IPI
7623   #define DELIVS     0x00001000   // Delivery status
7624   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7625   #define DEASSERT   0x00000000
7626   #define LEVEL      0x00008000   // Level triggered
7627   #define BCAST      0x00080000   // Send to all APICs, including self.
7628   #define BUSY       0x00001000
7629   #define FIXED      0x00000000
7630 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7631 #define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7632   #define X1         0x0000000B   // divide counts by 1
7633   #define PERIODIC   0x00020000   // Periodic
7634 #define PCINT   (0x0340/4)   // Performance Counter LVT
7635 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7636 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7637 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7638   #define MASKED     0x00010000   // Interrupt masked
7639 #define TICR    (0x0380/4)   // Timer Initial Count
7640 #define TCCR    (0x0390/4)   // Timer Current Count
7641 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7642 
7643 volatile uint *lapic;  // Initialized in mp.c
7644 
7645 
7646 
7647 
7648 
7649 
7650 static void
7651 lapicw(int index, int value)
7652 {
7653   lapic[index] = value;
7654   lapic[ID];  // wait for write to finish, by reading
7655 }
7656 
7657 void
7658 lapicinit(void)
7659 {
7660   if(!lapic)
7661     return;
7662 
7663   // Enable local APIC; set spurious interrupt vector.
7664   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7665 
7666   // The timer repeatedly counts down at bus frequency
7667   // from lapic[TICR] and then issues an interrupt.
7668   // If xv6 cared more about precise timekeeping,
7669   // TICR would be calibrated using an external time source.
7670   lapicw(TDCR, X1);
7671   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7672   lapicw(TICR, 10000000);
7673 
7674   // Disable logical interrupt lines.
7675   lapicw(LINT0, MASKED);
7676   lapicw(LINT1, MASKED);
7677 
7678   // Disable performance counter overflow interrupts
7679   // on machines that provide that interrupt entry.
7680   if(((lapic[VER]>>16) & 0xFF) >= 4)
7681     lapicw(PCINT, MASKED);
7682 
7683   // Map error interrupt to IRQ_ERROR.
7684   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7685 
7686   // Clear error status register (requires back-to-back writes).
7687   lapicw(ESR, 0);
7688   lapicw(ESR, 0);
7689 
7690   // Ack any outstanding interrupts.
7691   lapicw(EOI, 0);
7692 
7693   // Send an Init Level De-Assert to synchronise arbitration ID's.
7694   lapicw(ICRHI, 0);
7695   lapicw(ICRLO, BCAST | INIT | LEVEL);
7696   while(lapic[ICRLO] & DELIVS)
7697     ;
7698 
7699 
7700   // Enable interrupts on the APIC (but not on the processor).
7701   lapicw(TPR, 0);
7702 }
7703 
7704 int
7705 lapicid(void)
7706 {
7707   if (!lapic)
7708     return 0;
7709   return lapic[ID] >> 24;
7710 }
7711 
7712 // Acknowledge interrupt.
7713 void
7714 lapiceoi(void)
7715 {
7716   if(lapic)
7717     lapicw(EOI, 0);
7718 }
7719 
7720 // Spin for a given number of microseconds.
7721 // On real hardware would want to tune this dynamically.
7722 void
7723 microdelay(int us)
7724 {
7725 }
7726 
7727 #define CMOS_PORT    0x70
7728 #define CMOS_RETURN  0x71
7729 
7730 // Start additional processor running entry code at addr.
7731 // See Appendix B of MultiProcessor Specification.
7732 void
7733 lapicstartap(uchar apicid, uint addr)
7734 {
7735   int i;
7736   ushort *wrv;
7737 
7738   // "The BSP must initialize CMOS shutdown code to 0AH
7739   // and the warm reset vector (DWORD based at 40:67) to point at
7740   // the AP startup code prior to the [universal startup algorithm]."
7741   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7742   outb(CMOS_PORT+1, 0x0A);
7743   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7744   wrv[0] = 0;
7745   wrv[1] = addr >> 4;
7746 
7747 
7748 
7749 
7750   // "Universal startup algorithm."
7751   // Send INIT (level-triggered) interrupt to reset other CPU.
7752   lapicw(ICRHI, apicid<<24);
7753   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7754   microdelay(200);
7755   lapicw(ICRLO, INIT | LEVEL);
7756   microdelay(100);    // should be 10ms, but too slow in Bochs!
7757 
7758   // Send startup IPI (twice!) to enter code.
7759   // Regular hardware is supposed to only accept a STARTUP
7760   // when it is in the halted state due to an INIT.  So the second
7761   // should be ignored, but it is part of the official Intel algorithm.
7762   // Bochs complains about the second one.  Too bad for Bochs.
7763   for(i = 0; i < 2; i++){
7764     lapicw(ICRHI, apicid<<24);
7765     lapicw(ICRLO, STARTUP | (addr>>12));
7766     microdelay(200);
7767   }
7768 }
7769 
7770 #define CMOS_STATA   0x0a
7771 #define CMOS_STATB   0x0b
7772 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7773 
7774 #define SECS    0x00
7775 #define MINS    0x02
7776 #define HOURS   0x04
7777 #define DAY     0x07
7778 #define MONTH   0x08
7779 #define YEAR    0x09
7780 
7781 static uint
7782 cmos_read(uint reg)
7783 {
7784   outb(CMOS_PORT,  reg);
7785   microdelay(200);
7786 
7787   return inb(CMOS_RETURN);
7788 }
7789 
7790 static void
7791 fill_rtcdate(struct rtcdate *r)
7792 {
7793   r->second = cmos_read(SECS);
7794   r->minute = cmos_read(MINS);
7795   r->hour   = cmos_read(HOURS);
7796   r->day    = cmos_read(DAY);
7797   r->month  = cmos_read(MONTH);
7798   r->year   = cmos_read(YEAR);
7799 }
7800 // qemu seems to use 24-hour GWT and the values are BCD encoded
7801 void
7802 cmostime(struct rtcdate *r)
7803 {
7804   struct rtcdate t1, t2;
7805   int sb, bcd;
7806 
7807   sb = cmos_read(CMOS_STATB);
7808 
7809   bcd = (sb & (1 << 2)) == 0;
7810 
7811   // make sure CMOS doesn't modify time while we read it
7812   for(;;) {
7813     fill_rtcdate(&t1);
7814     if(cmos_read(CMOS_STATA) & CMOS_UIP)
7815         continue;
7816     fill_rtcdate(&t2);
7817     if(memcmp(&t1, &t2, sizeof(t1)) == 0)
7818       break;
7819   }
7820 
7821   // convert
7822   if(bcd) {
7823 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7824     CONV(second);
7825     CONV(minute);
7826     CONV(hour  );
7827     CONV(day   );
7828     CONV(month );
7829     CONV(year  );
7830 #undef     CONV
7831   }
7832 
7833   *r = t1;
7834   r->year += 2000;
7835 }
7836 
7837 
7838 
7839 
7840 
7841 
7842 
7843 
7844 
7845 
7846 
7847 
7848 
7849 
