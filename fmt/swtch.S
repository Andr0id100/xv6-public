3250 # Context switch
3251 #
3252 #   void swtch(struct context **old, struct context *new);
3253 #
3254 # Save the current registers on the stack, creating
3255 # a struct context, and save its address in *old.
3256 # Switch stacks to new and pop previously-saved registers.
3257 
3258 .globl swtch
3259 swtch:
3260   movl 4(%esp), %eax
3261   movl 8(%esp), %edx
3262 
3263   # Save old callee-saved registers
3264   pushl %ebp
3265   pushl %ebx
3266   pushl %esi
3267   pushl %edi
3268 
3269   # Switch stacks
3270   movl %esp, (%eax)
3271   movl %edx, %esp
3272 
3273   # Load new callee-saved registers
3274   popl %edi
3275   popl %esi
3276   popl %ebx
3277   popl %ebp
3278   ret
3279 
3280 
3281 
3282 
3283 
3284 
3285 
3286 
3287 
3288 
3289 
3290 
3291 
3292 
3293 
3294 
3295 
3296 
3297 
3298 
3299 
