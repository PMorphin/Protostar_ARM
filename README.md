# Protostar_ARM
Writeups for Protostar (exploit-exercises) on ARM Architecture
-

### Introduction

In this repository, we will see some writeups from [Protostar](https://www.vulnhub.com/entry/exploit-exercises-protostar-v2,32/). The original challenges were based on the `X86` architecture and I decided to solve them on a `ARM` architecture.

### Environment  

I will use a `RaspberryPi 3 Model B`

```code
pi@raspberrypi:~ $ uname -a
Linux raspberrypi 4.19.97-v7+ #1294 SMP Thu Jan 30 13:15:58 GMT 2020 armv7l GNU/Linux
pi@raspberrypi:~ $ gcc --version
gcc (Raspbian 8.3.0-6+rpi1) 8.3.0
Copyright (C) 2018 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```
__*ASLR disabled*__
```code
pi@raspberrypi:~ $ echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
```

### Raspberry Emulation

If you don't have a RaspberryPi, you can emulate it using [QEMU](https://www.qemu.org/) and follow this tutorial [Raspbian "stretch" for Raspberry Pi 3 on QEMU](https://github.com/wimvanderbauwhede/limited-systems/wiki/Raspbian-%22stretch%22-for-Raspberry-Pi-3-on-QEMU)



### Memory protections

###### ASLR - Address Space Layout Randomization  

`0. No randomization is applied`  
`1. Partial randomization is applied`  
`2. Full randomization`

###### PIE - Position Independent Executable - The executable could be placed at a random address in memory  
###### Stack canaries - Values placed on the stack and checked before a function returns
###### NX - Not executable stack and heap
###### RELRO (Relocation Read Only) - Makes some binary sections read-only

###### __*Resources about*__ [Memory protections](https://ctf101.org/binary-exploitation/overview/)

### Resources  

[C](https://www.learn-c.org/)  
[ELF File Format](https://linuxhint.com/understanding_elf_file_format/)  
[GDB](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf)  
[ARM Assembly#1](https://developer.arm.com/docs/dui0473/k/writing-arm-assembly-language)  
[ARM Assembly#2](https://simplemachines.it/doc/arm_inst.pdf)  

[Heap&Stack](https://gribblelab.org/CBootCamp/7_Memory_Stack_vs_Heap.html)  
[Stack Overflow](http://phrack.org/issues/49/14.html)  
[Malloc Internals](https://sourceware.org/glibc/wiki/MallocInternals)  
[Format String Vulnerability](https://crypto.stanford.edu/cs155old/cs155-spring08/papers/formatstring-1.2.pdf)  

[ARM Explotation for IoT](https://www.exploit-db.com/docs/english/43906-arm-exploitation-for-iot.pdf)  
[Shellcode on ARM Architecture](http://shell-storm.org/blog/Shellcode-On-ARM-Architecture/)  
[Modern Binary Exploitation](http://security.cs.rpi.edu/courses/binexp-spring2015/)
