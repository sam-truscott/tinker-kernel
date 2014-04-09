/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef MALLOC_CONFIG_H_
#define MALLOC_CONFIG_H_

#undef WIN32
#define DLMALLOC_EXPORT extern

#undef DEBUG
#define ABORT_ON_ASSERT_FAILURE 1
#define MALLOC_FAILURE_ACTION __kernel_panic()

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

#define ABORT __kernel_panic()
#define EINVAL PARAMETERS_INVALID
#define ENOMEM OUT_OF_MEMORY
#define malloc_getpagesize MMU_PAGE_SIZE

#define MSPACES 1
#define ONLY_MSPACES 1

#define HAVE_MMAP 0
#define HAVE_MORECORE 0
#define MORECORE_CONTIGUOUS 0
#define MORECORE_CANNOT_TRIM 1

#define USE_LOCKS 0

#define NO_MALLINFO 1
#define NO_MALLOC_STATS 1

#define USE_BUILTIN_FFS 0

#endif /* MALLOC_CONFIG_H_ */
