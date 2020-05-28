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
  printf ("Jump to : %x%x%x%x\n", buffer[JUMP_GUARD], buffer[72], buffer[73], buffer[74]);
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
