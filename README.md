Tinker Hybrid Kernel
===================

Hybrid Kernel written in C and assembly.

Supported targets:
- PowerPC (gdb simulator)

In development:
- arm6 (Raspberry Pi)

Licence
=======

GPL v3

Overview
========

I've worked on & off on this hybrid kernel for the last few years to help my understanding in operating systems.

* SOS1 was essentially a framebuffer - and a very broken scheduler
* SOS2 was a proper multitasking scheduler with dynamic memory support
* SOS3 was where I refactored it to be more OO and do some initial MMU work
* Tinker was renamed from SOS3!

Tinker is:
* Limited - it's an RTOS Kernel so that's the idea
* In development
* Single-core at present
* Written from a clean slate (excluding bits of boot-code from sel4 and osdev)

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

For example, to build an individual target use:

    # Windows
    gradlew releasePpc32GdbExecutable
    gradlew clean debugArmRaspPiExecutable
    
    # Linux
    ./gradlew releasePpc32GdbExecutable
    ./gradlew clean debugArmRaspPiExecutable
    
To build everything (all targets, debug and release) use:

	# Windows
	gradlew clean assemble
	
	# Linux
	./gradlew clean assemble

Loading
=======

You can place user-code in tinker.c for now but eventually there'll be a bootstrap that contains
the hybridkernel and the required user-services to startup. The bootstrap will place the kernel
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

* Target: ARM support (Raspberry Pi)
	* Fix the reboot issue - caused by threads exiting
	* Fix the system timer - test app waits but the interrupt doesn't map/callback
	* Add ARM MMU support
* Kernel: Shell should use pipes for reading I/O
* Kernel: Timeouts on pipe (open/read/write)
* Kernel: Allow user-mode to map to real addresses (user-mode drivers)
    * (interrupts can be handled by pipes in the bsp)
* Kernel: Add DMA support for pipes    
* Kernel: Break up the syscalls into seperate static/inline functions
* Kernel: Add mechanism to load elfs
* Support: Create newlib port
* Support: Create bootstrap for kernel + services
* Kernel: Option to make pipes zero-copy via MMU (memory is already aligned)
* Kernel: Review the powerpc page table code
* Target: x86 support
* All: Need to review it all head to toe since refactor
* Doc: Doc it with doxygen

Toolchain
=========

A standard stage 1 gcc compiler will work (i.e. C compiler without libc support).

Helpful Commands
================

Starting QEMU for the Raspberry Pi build

	# Disassemble the Raspberry Pi build so we can look at addresses
	arm-eabi-objdump -dS build\binaries\armRaspPiExecutable\debug\armRaspPi.exe > dis.txt
	
	# Start a listening netcat port to listen to the UART
	nc64 -L -p 5555 -vv
	# Start the emulator
	qemu-system-arm -m 512M -kernel build\binaries\armRaspPiExecutable\debug\armRaspPi.exe -gdb tcp::1234,ipv4 -no-reboot -no-shutdown -machine raspi -serial tcp:127.0.0.1:5555 -S

	# Start a debugger
	arm-eabi-gdb build\binaries\armRaspPiExecutable\debug\armRaspPi.exe
	# Connect to the debugger
	target remote localhost:1234
	# Set a breakpoint at the start
	b kernel_main
	# Start the kernel
	continue
