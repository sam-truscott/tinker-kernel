/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef MALLOC_CONFIG_H_
#define MALLOC_CONFIG_H_

#if defined(IS_HOST_UNIT_TEST)

#define MSPACES 1
#define ONLY_MSPACES 0

#define HAVE_MORECORE 0
#define MORECORE_CONTIGUOUS 0
#define MORECORE_CANNOT_TRIM 1

#define USE_LOCKS 0

#define NO_MALLINFO 0
#define NO_MALLOC_STATS 1

#if ! defined (_X86_) && defined (__i386__)
# define _X86_ 1
#endif

#else /* IS_HOST_UNIT_TEST */

#undef WIN32
#undef _WIN32
#define DLMALLOC_EXPORT extern

#define size_t uint32_t
#define ptrdiff_t int32_t

#undef DEBUG
#define ABORT_ON_ASSERT_FAILURE 1
#define MALLOC_FAILURE_ACTION malloc_failure()
#define CORRUPTION_ERROR_ACTION malloc_corruption

#define LACKS_UNISTD_H 1
#define LACKS_FCNTL_H 1
#define LACKS_SYS_PARAM_H 1
#define LACKS_SYS_MMAN_H 1
#define LACKS_STRINGS_H 1
#define LACKS_STRING_H 1
#define LACKS_SYS_TYPES_H 1
#define LACKS_ERRNO_H 1
#define LACKS_STDLIB_H 1
#define LACKS_SCHED_H 1
#define LACKS_TIME_H 1

#define ABORT malloc_abort()
#define EINVAL PARAMETERS_INVALID
#define ENOMEM OUT_OF_MEMORY
#define malloc_getpagesize MMU_PAGE_SIZE

#define MSPACES 1
#define ONLY_MSPACES 0

#define HAVE_MMAP 0
#define HAVE_MORECORE 0
#define MORECORE_CONTIGUOUS 0
#define MORECORE_CANNOT_TRIM 1

#define USE_LOCKS 0

#define NO_MALLINFO 0
#define NO_MALLOC_STATS 1

#define USE_BUILTIN_FFS 0

#endif /* IS_HOST_UNIT_TEST */

#endif /* MALLOC_CONFIG_H_ */
