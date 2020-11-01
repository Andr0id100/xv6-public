8150 // Console input and output.
8151 // Input is from the keyboard or serial port.
8152 // Output is written to the screen and serial port.
8153 
8154 #include "types.h"
8155 #include "defs.h"
8156 #include "param.h"
8157 #include "traps.h"
8158 #include "spinlock.h"
8159 #include "sleeplock.h"
8160 #include "fs.h"
8161 #include "file.h"
8162 #include "memlayout.h"
8163 #include "mmu.h"
8164 #include "proc.h"
8165 #include "x86.h"
8166 
8167 static void consputc(int);
8168 
8169 static int panicked = 0;
8170 
8171 static struct {
8172   struct spinlock lock;
8173   int locking;
8174 } cons;
8175 
8176 static void
8177 printint(int xx, int base, int sign)
8178 {
8179   static char digits[] = "0123456789abcdef";
8180   char buf[16];
8181   int i;
8182   uint x;
8183 
8184   if(sign && (sign = xx < 0))
8185     x = -xx;
8186   else
8187     x = xx;
8188 
8189   i = 0;
8190   do{
8191     buf[i++] = digits[x % base];
8192   }while((x /= base) != 0);
8193 
8194   if(sign)
8195     buf[i++] = '-';
8196 
8197   while(--i >= 0)
8198     consputc(buf[i]);
8199 }
8200 
8201 
8202 
8203 
8204 
8205 
8206 
8207 
8208 
8209 
8210 
8211 
8212 
8213 
8214 
8215 
8216 
8217 
8218 
8219 
8220 
8221 
8222 
8223 
8224 
8225 
8226 
8227 
8228 
8229 
8230 
8231 
8232 
8233 
8234 
8235 
8236 
8237 
8238 
8239 
8240 
8241 
8242 
8243 
8244 
8245 
8246 
8247 
8248 
8249 
8250 // Print to the console. only understands %d, %x, %p, %s.
8251 void
8252 cprintf(char *fmt, ...)
8253 {
8254   int i, c, locking;
8255   uint *argp;
8256   char *s;
8257 
8258   locking = cons.locking;
8259   if(locking)
8260     acquire(&cons.lock);
8261 
8262   if (fmt == 0)
8263     panic("null fmt");
8264 
8265   argp = (uint*)(void*)(&fmt + 1);
8266   for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
8267     if(c != '%'){
8268       consputc(c);
8269       continue;
8270     }
8271     c = fmt[++i] & 0xff;
8272     if(c == 0)
8273       break;
8274     switch(c){
8275     case 'd':
8276       printint(*argp++, 10, 1);
8277       break;
8278     case 'x':
8279     case 'p':
8280       printint(*argp++, 16, 0);
8281       break;
8282     case 's':
8283       if((s = (char*)*argp++) == 0)
8284         s = "(null)";
8285       for(; *s; s++)
8286         consputc(*s);
8287       break;
8288     case '%':
8289       consputc('%');
8290       break;
8291     default:
8292       // Print unknown % sequence to draw attention.
8293       consputc('%');
8294       consputc(c);
8295       break;
8296     }
8297   }
8298 
8299 
8300   if(locking)
8301     release(&cons.lock);
8302 }
8303 
8304 void
8305 panic(char *s)
8306 {
8307   int i;
8308   uint pcs[10];
8309 
8310   cli();
8311   cons.locking = 0;
8312   // use lapiccpunum so that we can call panic from mycpu()
8313   cprintf("lapicid %d: panic: ", lapicid());
8314   cprintf(s);
8315   cprintf("\n");
8316   getcallerpcs(&s, pcs);
8317   for(i=0; i<10; i++)
8318     cprintf(" %p", pcs[i]);
8319   panicked = 1; // freeze other CPU
8320   for(;;)
8321     ;
8322 }
8323 
8324 
8325 
8326 
8327 
8328 
8329 
8330 
8331 
8332 
8333 
8334 
8335 
8336 
8337 
8338 
8339 
8340 
8341 
8342 
8343 
8344 
8345 
8346 
8347 
8348 
8349 
8350 #define BACKSPACE 0x100
8351 #define CRTPORT 0x3d4
8352 static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
8353 
8354 static void
8355 cgaputc(int c)
8356 {
8357   int pos;
8358 
8359   // Cursor position: col + 80*row.
8360   outb(CRTPORT, 14);
8361   pos = inb(CRTPORT+1) << 8;
8362   outb(CRTPORT, 15);
8363   pos |= inb(CRTPORT+1);
8364 
8365   if(c == '\n')
8366     pos += 80 - pos%80;
8367   else if(c == BACKSPACE){
8368     if(pos > 0) --pos;
8369   } else
8370     crt[pos++] = (c&0xff) | 0x0700;  // black on white
8371 
8372   if(pos < 0 || pos > 25*80)
8373     panic("pos under/overflow");
8374 
8375   if((pos/80) >= 24){  // Scroll up.
8376     memmove(crt, crt+80, sizeof(crt[0])*23*80);
8377     pos -= 80;
8378     memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
8379   }
8380 
8381   outb(CRTPORT, 14);
8382   outb(CRTPORT+1, pos>>8);
8383   outb(CRTPORT, 15);
8384   outb(CRTPORT+1, pos);
8385   crt[pos] = ' ' | 0x0700;
8386 }
8387 
8388 
8389 
8390 
8391 
8392 
8393 
8394 
8395 
8396 
8397 
8398 
8399 
8400 void
8401 consputc(int c)
8402 {
8403   if(panicked){
8404     cli();
8405     for(;;)
8406       ;
8407   }
8408 
8409   if(c == BACKSPACE){
8410     uartputc('\b'); uartputc(' '); uartputc('\b');
8411   } else
8412     uartputc(c);
8413   cgaputc(c);
8414 }
8415 
8416 #define INPUT_BUF 128
8417 struct {
8418   char buf[INPUT_BUF];
8419   uint r;  // Read index
8420   uint w;  // Write index
8421   uint e;  // Edit index
8422 } input;
8423 
8424 #define C(x)  ((x)-'@')  // Control-x
8425 
8426 void
8427 consoleintr(int (*getc)(void))
8428 {
8429   int c, doprocdump = 0;
8430 
8431   acquire(&cons.lock);
8432   while((c = getc()) >= 0){
8433     switch(c){
8434     case C('P'):  // Process listing.
8435       // procdump() locks cons.lock indirectly; invoke later
8436       doprocdump = 1;
8437       break;
8438     case C('U'):  // Kill line.
8439       while(input.e != input.w &&
8440             input.buf[(input.e-1) % INPUT_BUF] != '\n'){
8441         input.e--;
8442         consputc(BACKSPACE);
8443       }
8444       break;
8445     case C('H'): case '\x7f':  // Backspace
8446       if(input.e != input.w){
8447         input.e--;
8448         consputc(BACKSPACE);
8449       }
8450       break;
8451     default:
8452       if(c != 0 && input.e-input.r < INPUT_BUF){
8453         c = (c == '\r') ? '\n' : c;
8454         input.buf[input.e++ % INPUT_BUF] = c;
8455         consputc(c);
8456         if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
8457           input.w = input.e;
8458           wakeup(&input.r);
8459         }
8460       }
8461       break;
8462     }
8463   }
8464   release(&cons.lock);
8465   if(doprocdump) {
8466     procdump();  // now call procdump() wo. cons.lock held
8467   }
8468 }
8469 
8470 int
8471 consoleread(struct inode *ip, char *dst, int n)
8472 {
8473   uint target;
8474   int c;
8475 
8476   iunlock(ip);
8477   target = n;
8478   acquire(&cons.lock);
8479   while(n > 0){
8480     while(input.r == input.w){
8481       if(myproc()->killed){
8482         release(&cons.lock);
8483         ilock(ip);
8484         return -1;
8485       }
8486       sleep(&input.r, &cons.lock);
8487     }
8488     c = input.buf[input.r++ % INPUT_BUF];
8489     if(c == C('D')){  // EOF
8490       if(n < target){
8491         // Save ^D for next time, to make sure
8492         // caller gets a 0-byte result.
8493         input.r--;
8494       }
8495       break;
8496     }
8497     *dst++ = c;
8498     --n;
8499     if(c == '\n')
8500       break;
8501   }
8502   release(&cons.lock);
8503   ilock(ip);
8504 
8505   return target - n;
8506 }
8507 
8508 int
8509 consolewrite(struct inode *ip, char *buf, int n)
8510 {
8511   int i;
8512 
8513   iunlock(ip);
8514   acquire(&cons.lock);
8515   for(i = 0; i < n; i++)
8516     consputc(buf[i] & 0xff);
8517   release(&cons.lock);
8518   ilock(ip);
8519 
8520   return n;
8521 }
8522 
8523 void
8524 consoleinit(void)
8525 {
8526   initlock(&cons.lock, "console");
8527 
8528   devsw[CONSOLE].write = consolewrite;
8529   devsw[CONSOLE].read = consoleread;
8530   cons.locking = 1;
8531 
8532   ioapicenable(IRQ_KBD, 0);
8533 }
8534 
8535 
8536 
8537 
8538 
8539 
8540 
8541 
8542 
8543 
8544 
8545 
8546 
8547 
8548 
8549 
