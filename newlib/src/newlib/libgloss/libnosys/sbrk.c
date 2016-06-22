/* Version of sbrk for no operating system.  */

#include "config.h"
#include <_syslist.h>
#include "tinker_api.h"

void *
_sbrk (incr)
     int incr;
{
   static char * heap_end = 0;
   return (caddr_t) tinker_sbrk(heap_end, incr);
}
