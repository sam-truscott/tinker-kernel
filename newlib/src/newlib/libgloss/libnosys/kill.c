/*
 * Stub version of kill.
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
_DEFUN (_kill, (pid, sig),
        int pid  _AND
        int sig)
{
  tinker_exit_thread();
  return 0;
}

stub_warning(_kill)
