Tinker Microkernel
===================

Microkernel written in C and assembly.

Supported targets:
- PowerPC (gdb simulator)

In development:
- x86, arm6 (Raspberry Pi)

Licence
=======

GPL v3

Overview
========

I've worked on & off on this microkernel for the last few years to help my understanding in operating systems.

* SOS1 was essentially a framebuffer - and a very broken scheduler
* SOS2 was a proper multitasking scheduler with dynamic memory support
* SOS3 was where I refactored it to be more OO and do some initial MMU work
* Tinker was renamed from SOS3!

Tinker is:
* Limited - it's an RTOS Microkernel so that's the idea
* In development
* Single-core at present

He is my hobby. The aim is to keep it simple enough for anyone to understand and therefore also
be simple enough to port and possibly even verify.

Features
========

* Processes & Threads with a priority based pre-emptive scheduler
* Semaphores
* Shared Memory
* Pipes
* Timers
* Clock

Building
========

Gradle is used as the build system. It can build debug and release versions of either the parts
(which are static libraries) or executables

For example, for PowerPC use:

    # Windows
    gradlew releasePpc32GdbExecutable
    gradlew releasex86Executable
    
    # Linux
    ./gradlew releasePpc32GdbExecutable
    ./gradlew releasex86Executable

Loading
=======

You can place user-code in tinker.c for now but eventually there'll be a bootstrap that contains
the microkernel and the required user-services to startup. The bootstrap will place the kernel
and services into memory, load the kernel and then start the user-services (much like init).

 bootstrap
  - kernel >\
  - (setup) |
  - service<| (i.e. filesystem)
  - service<| (i.e. block access for sata)
  - service<| (i.e. tcp/ip stack)
  - service</ (i.e. hardware device driver)
  
The intention is that the kernel is started by firmware or bootloader like u-boot.

Drivers
=======

Drivers and written as userland services with MMIO through the MMU.

The kernel Board Support Package (BSP) should only have drivers for timers and a debugging port such as a UART.

The root Interrupt Handler should be part of the kernel, other ones should really be installed as
user processes/servers that listen to the pipe.

Issues / TODO
=============

These are the things I need to address in a rough order:

* PowerPC: Check page table is in range
* Target: x86 support
* Target: ARM support
* Kernel: Timeouts on pipe (open/read/write)
* Kernel: Allow user-mode to map to real addresses (user-mode drivers)
    * (interrupts can be handled by pipes in the bsp)
* Kernel: Break up the syscalls into seperate static/inline functions
* Kernel: Add mechanism to load elfs
* Support: Create newlib port
* Support: Create bootstrap for kernel + services
* Kernel: Option to make pipes zero-copy via MMU (memory is already aligned)
* Kernel: Review the powerpc page table code
* All: Need to review it all head to toe since refactor
* Doc: Doc it with doxygen

Toolchain
=========

A standard stage 1 gcc compiler will work (i.e. C compiler without libc support).
