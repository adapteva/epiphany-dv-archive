.section IVT ,"a",@progbits;        ;
 b _my_func

.section reserved_crt0,"a",@progbits;        ;
.global _my_func
.type   _my_func , %function                ;
_my_func:
    add r1,r3,r5
	mov r0, %low(0x072000000)

	movt r0, %high(0x072000000)

	str r7,[r0,0]	
	nop
     nop
    nop
      nop
    nop
      nop
    nop
      nop
    nop

    nop
      nop
    nop
     nop
    nop
     nop
    nop
    idle
    wand
.size _my_func, .-_my_func
.set my_func_size, .-_my_func

