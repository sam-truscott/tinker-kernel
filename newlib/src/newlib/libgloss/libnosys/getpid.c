/*
 * Stub version of getpid.
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
_DEFUN (_getpid, (),
        _NOARGS)
{
  return tinker_get_pid();
}

stub_warning(_getpid)
