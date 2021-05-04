/* filst@ida

   pintos -v -k --fs-disk=2 --qemu -p ../examples/wait_test -a wait -p ../examples/dummy -a fast -p ../examples/slow_child -a slow -- -f -q run 'wait'

   Simple tests for the system call wait.

   First, starts a fast process and waits for it. The wait will likely happen
   after the child terminated.

   Second, starts a slow process and waits for it. The wait will likely happen
   before the child terminates.

   If the program freezes when calling "sleep", make sure you
   #include "devices/timer.h" in syscall.h (you likely have a warning about this).
  */

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include "sleep.h"

int main(void)
{
  printf("Starting the fast child...\n");
  int pid = exec("fast 10");

  // Wait a little while to make sure that the child terminated.
  sleep(1000);

  int result = wait(pid);
  if (result != 10)
  {
    printf("ERROR: Expected 10 from wait, but got: %d\n", result);
    return -1;
  }

  printf("Seems OK, starting the slow child...\n");
  pid = exec("slow 20");

  // Don't sleep here, we want to call wait before the child is done.

  result = wait(pid);
  if (result != 20)
  {
    printf("ERROR: Expected 20 from wait, but got: %d\n", result);
    return -1;
  }

  printf("Your implementation looks OK for simple cases, now try 'longrun'!\n");
  // You might want to print the process list here:
  // plist();

  return 0;
}
