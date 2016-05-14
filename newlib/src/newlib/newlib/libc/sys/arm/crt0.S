#include "newlib.h"
#include "arm.h"

/* ANSI concatenation macros.  */
#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

#ifdef __USER_LABEL_PREFIX__
#define FUNCTION( name ) CONCAT (__USER_LABEL_PREFIX__, name)
#else
#error __USER_LABEL_PREFIX is not defined
#endif

#ifdef HAVE_INITFINI_ARRAY
#define _init	__libc_init_array
#define _fini	__libc_fini_array
#endif

#if defined(__ARM_EABI__) && defined(__thumb__) && !defined(__thumb2__)
/* For Thumb1 we need to force the architecture to be sure that we get the
   correct attributes on the object file; otherwise the assembler will get
   confused and mark the object as being v6T2.  */
#if defined(__ARM_ARCH_4T__)
	.arch armv4t
#elif defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__)
	/* Nothing in this object requires higher than v5.  */
	.arch armv5t
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
	|| defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) \
	|| defined(__ARM_ARCH_6ZK__)
	/* Nothing in this object requires higher than v6.  */
	.arch armv6
#elif defined(__ARM_ARCH_6M__)
#ifdef ARM_RDP_MONITOR
	/* Object file uses SVC, so mark as v6s-m.  */
	.arch armv6s-m
#else
	.arch armv6-m
#endif
#endif
#endif

/* .text is used instead of .section .text so it works with arm-aout too.  */
	.text
	.syntax unified
#ifdef PREFER_THUMB
	.thumb
.macro FUNC_START name
	.global \name
	.thumb_func
\name:
.endm	
#else
	.code 32
.macro FUNC_START name
	.global	\name
\name:
.endm	
#endif

.macro indirect_call reg
#ifdef HAVE_CALL_INDIRECT
	blx \reg
#else
	mov	lr, pc
	mov	pc, \reg
#endif
.endm

	.align 	0

	FUNC_START	_mainCRTStartup
	FUNC_START	_start
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Annotation for EABI unwinding tables.  */
	.fnstart
#endif

#ifdef __USES_INITFINI__
	/* Some arm/elf targets use the .init and .fini sections
	   to create constructors and destructors, and for these
	   targets we need to call the _init function and arrange
	   for _fini to be called at program exit.  */
	movs	r4, r0
	movs	r5, r1
	ldr	r0, .Lfini
	bl	FUNCTION (atexit)
	bl	FUNCTION (_init)
	movs	r0, r4
	movs	r1, r5
#endif	
	bl	FUNCTION (main)

	bl	FUNCTION (exit)		/* Should not return.  */

	/* For Thumb, constants must be after the code since only
	   positive offsets are supported for PC relative addresses.  */
	.align 0
.LC0:
	/* Changes by toralf: Provide alternative "stack" variable whose value
	   may be defined externally; .Lstack will be used instead of .LC0 if
	   it points to a non-0 value. Also set up references to "hooks" that
           may be used by the application to provide additional init code.  */
#ifdef __pe__
	.word	0x800000
#else
	.word	0x80000			/* Top of RAM on the PIE board.  */
#endif

.Lstack:
	.word	__stack
.Lhwinit:
	.word	FUNCTION (hardware_init_hook)
.Lswinit:
	.word	FUNCTION (software_init_hook)

	/* Set up defaults for the above variables in the form of weak symbols
	   - so that application will link correctly, and get value 0 in
	   runtime (meaning "ignore setting") for the variables, when the user
	   does not provide the symbols. (The linker uses a weak symbol if,
	   and only if, a normal version of the same symbol isn't provided
	   e.g. by a linker script or another object file).  */

	.weak __stack
	.weak FUNCTION (hardware_init_hook)
	.weak FUNCTION (software_init_hook)
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Protect against unhandled exceptions.  */
	.cantunwind
	.fnend
#endif
.LC1:
	.word	__bss_start__
.LC2:
	.word	__bss_end__
#ifdef __USES_INITFINI__
.Lfini:
	.word	FUNCTION(_fini)
#endif

#ifdef __pe__
	.section .idata$3
	.long	0,0,0,0,0,0,0,0
#endif
