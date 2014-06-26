Tinker Micro Kernel
===================

microkernel written in c and asm.

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
* tinker was renamed from sos3!

tinker is:
* limited
* probably wrong

he is my hobby. the aim is to keep it simple enough for anyone to understand and therefore also
be simple enough to port and possibly even verify.

features
========

* semaphores
* shared memory
* pipes
* timers
* clock

building
========

Gradle is used as the build system. It can build debug and release versions of either the parts
(which are static libraries) or executables

For example, for PowerPC use:

    # Windows
    gradlew.bat releasePpc32GdbExecutable
    
    # Linux
    ./gradlew.sh releasePpc32GdbExecutable

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

* powerpc: check page table is in rage
* interrupts: parameters to intc_enable (priority, edge/level, active high/low etc)
* target: ARM support
* target: x86 support
* kernel: timeouts on pipe (open/read/write)
* kernel: allow user-mode to map to real addresses (user-mode drivers)
    * (interrupts can be handled by pipes in the bsp)
* kernel: break up the syscalls into seperate static/inline functions
* kernel: add mechanism to load elfs
* support: create newlib port
* support: create bootstrap for kernel + services
* kernel: option to make pipes zero-copy via mmu (memory is already aligned)
* kernel: review the powerpc page table code
* all: need to review it all head to toe since refactor
* doc: doc it with doxygen

toolchain
=========

a standard stage 1 gcc compiler will work (i.e. C compiler without libc support)
