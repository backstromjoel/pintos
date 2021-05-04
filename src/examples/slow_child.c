/* filst@ida

   A process that is just a bit slow. Used in the "wait_test" program.
 */

#include <stdlib.h>
#include "sleep.h"

int main(int argc, char* argv[])
{
  if (argc != 2)
    return 0;

  sleep(1000);

  return atoi(argv[1]);
}
