/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "swi.h"
#include "tinker_api.h"

/* Forward prototypes.  */
int     _system     _PARAMS ((const char *));
int     _rename     _PARAMS ((const char *, const char *));
int     _isatty		_PARAMS ((int));
clock_t _times		_PARAMS ((struct tms *));
int     _gettimeofday	_PARAMS ((struct timeval *, void *));
void    _raise 		_PARAMS ((void));
int     _unlink		_PARAMS ((const char *));
int     _link 		_PARAMS ((void));
int     _stat 		_PARAMS ((const char *, struct stat *));
int     _fstat 		_PARAMS ((int, struct stat *));
caddr_t _sbrk		_PARAMS ((int));
int     _getpid		_PARAMS ((int));
int     _kill		_PARAMS ((int, int));
void    _exit		_PARAMS ((int));
int     _close		_PARAMS ((int));
int     _open		_PARAMS ((const char *, int, ...));
int     _write 		_PARAMS ((int, char *, int));
int     _lseek		_PARAMS ((int, int, int));
int     _read		_PARAMS ((int, char *, int));
void    initialise_monitor_handles _PARAMS ((void));

static int	error		_PARAMS ((int));

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");

static int
error (int result)
{
  return result;
}

int __attribute__((weak))
_read (int file,
       char * ptr,
       int len)
{
	if (file == 0)
	{
		return 0;
	}
	return error(-1);
}

int
_lseek (int file,
	int ptr,
	int dir)
{
	return error(-1);
}

int __attribute__((weak))
_write (int    file,
	char * ptr,
	int    len)
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

extern int strlen (const char *);

int
_open (const char * path,
       int          flags,
       ...)
{
	return error(-1);
}

int
_close (int file)
{
	return error(-1);
}

int
_kill (int pid, int sig)
{
  (void)pid; (void)sig;
  tinker_exit_thread();
}

void
_exit (int status)
{
  /* There is only one SWI for both _exit and _kill. For _exit, call
     the SWI with the second argument set to -1, an invalid value for
     signum, so that the SWI handler can distinguish the two calls.
     Note: The RDI implementation of _kill throws away both its
     arguments.  */
  _kill(status, -1);
}

int
_getpid (int n)
{
  (void)n;
  return tinker_get_pid();
}

caddr_t __attribute__((weak))
_sbrk (int incr)
{
  static char * heap_end = 0;
  return (caddr_t) tinker_sbrk(heap_end, incr);
}

extern void memset (struct stat *, int, unsigned int);

int
_fstat (int file, struct stat * st)
{
  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFCHR;
  st->st_blksize = 1024;
  return 0;
  file = file;
}

int _stat (const char *fname, struct stat *st)
{
  int file;

  /* The best we can do is try to open the file readonly.  If it exists,
     then we can guess a few things about it.  */
  if ((file = _open (fname, O_RDONLY)) < 0)
    return -1;

  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFREG | S_IREAD;
  st->st_blksize = 1024;
  _close (file); /* Not interested in the error.  */
  return 0;
}

int
_link (void)
{
  return -1;
}

int
_unlink (const char *path)
{
  return -1;
}

void
_raise (void)
{
	// TODO
  return;
}

int
_gettimeofday (struct timeval * tp, void * tzvp)
{
  // TODO
  struct timezone *tzp = tzvp;
  if (tp)
  {
	  tp->tv_sec = 0;
	  tp->tv_usec = 0;
  }

  /* Return fixed data for the timezone.  */
  if (tzp)
  {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
  }

  return 0;
}

/* Return a clock that ticks at 100Hz.  */
clock_t 
_times (struct tms * tp)
{
  clock_t timeval;
  // TODO
  return timeval;
};


int
_isatty (int fd)
{
  return (fd <= 2) ? 1 : 0;  /* one of stdin, stdout, stderr */
}

int
_system (const char *s)
{
  if (s == NULL)
  {
    return 0;
  }
  errno = ENOSYS;
  return -1;
}

int
_rename (const char * oldpath, const char * newpath)
{
  errno = ENOSYS;
  return -1;
}
