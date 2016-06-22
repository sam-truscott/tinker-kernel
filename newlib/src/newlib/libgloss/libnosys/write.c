/*
 * Stub version of write.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"
#include "tinker_api.h"

int
_DEFUN (_write, (file, ptr, len),
        int   file  _AND
        char *ptr   _AND
        int   len)
{
  if (file == 1 || file == 2)
  {
	tinker_debug(ptr, len);
    return 0;
  }
  else
  {
    return error(-1);
  }
}

stub_warning(_write)

