sos3
====

microkernel written in c and asm.

* currently broken as i've just pulled in dlmalloc and integrating it

supported targets:
- powerpc32 (gdb simulator)

licence
=======

gpl v3

overview
========

i've worked on & off on this microkernel for the last few years to help my understanding in operating systems.

* sos1 was essentially a framebuffer - and a very broken scheduler
* sos2 was a proper multitasking scheduler with dynamic memory support
* sos3 was where I refactored it to be more OO and do some initial mmu work 

sos3 is:
* not complete
* (very) limited
* probably wrong

he is my hobby. the aim is to keep it simple enough for anyone to understand and therefore also
be simple enough to port and possibly even verify.

loading
=======

you can place user-code in sos.c for now but eventually there'll be a bootstrap that contains
the microkernel and the required user-services to startup. the bootstrap will place the kernel
and services into memory, load the kernel and then start the user-services (much like init).

 bootstrap
  - kernel >\
  - (setup) |
  - service<| (i.e. filesystem)
  - service<| (i.e. block access for sata)
  - service<| (i.e. tcp/ip stack)
  - service</ (i.e. hardware device driver)
  
the intention is that the kernel is started by firmware or bootloader like u-boot.

drivers
=======

drivers and written as userland services with mmio through the mmu.

the kernel bsp should only have drivers for timers and a debugging port such as a uart.

issues / todo
=============

these are the things I need to address in a rough order:

* fix the problem with mem pools not freeing memory they've allocated
* re-write the memory pools, it's from the original sos2 code and poor

1. [done  ] import dlmalloc and configure it
2. [broken] change mempool to use dlmalloc (mspace)

* option to make pipes zero-copy via mmu (memory is already aligned)
* shms and timers aren't implemented yet - use the mmu for performance
* break up the syscalls into seperate static/inline functions
* makefile
* replace the mempool implementation with dlmalloc (?)
* add mechanism to load elfs
* create static library for api/syscalls 
* create newlib port
* create bootstrap for kernel + services
* ARM support
* x86 support
* need to review it all head to toe since refactor
* doc it with doxygen

toolchain
=========

a standard stage 1 gcc compiler will work (i.e. C compiler without libc support)
