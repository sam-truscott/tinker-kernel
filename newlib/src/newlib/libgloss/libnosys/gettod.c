/*
 * Stub version of gettimeofday.
 */

#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#undef errno
extern int errno;
#include "warning.h"
#include "tinker_api.h"

struct timeval;

int
_DEFUN (_gettimeofday, (ptimeval, ptimezone),
        struct timeval  *ptimeval  _AND
        void *ptimezone)
{
  if (ptimeval)
  {
	ptimeval->tv_sec = 0;
	ptimeval->tv_usec = 0;
  }

  /* Return fixed data for the timezone.  */
  struct timezone *tzp = ptimezone;
  if (tzp)
  {
	tzp->tz_minuteswest = 0;
	tzp->tz_dsttime = 0;
  }

  return 0;
}

stub_warning(_gettimeofday)
