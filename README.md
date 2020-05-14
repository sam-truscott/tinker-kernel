Tinker Hybrid Kernel
===================

Hybrid Kernel written in C and assembly.

## Supported targets
### ARM
- Raspberry Pi

## In development:
### ARM
- Raspberry Pi 2

## Deprecated
### PowerPC
- gdb simulator (aka PSIM)

# License

MIT - see LICENSE

# Overview

I've worked on & off on this hybrid kernel for the last few years to help my understanding in operating systems.

Tinker is:
* Limited - it's an RTOS Kernel so that's the idea
* In development
* Single-core at present
* Written from a clean slate (excluding bits of boot-code from sel4 and osdev) from trail and error. Mostly error.

He is my hobby. The aim is to keep it simple enough for anyone to understand and therefore also
be simple enough to port and possibly even verify. I used to try and read the Linux Kernel but got lost. The aim of Tinker is to be easy to read so it's easy to learn what's going on.

# Features

* Processes & Threads with a priority based pre-emptive scheduler
* Semaphores
* Shared Memory
* Pipes
* Timers
* Clock

# Building

Gradle is used as the build system. It can build debug and release versions of either the parts
(which are static libraries) or executables

For example, to build an individual target use:

    # Windows
    gradlew armRaspPiDebugExecutable armRaspPiReleaseExecutable
    
    # Linux
    ./gradlew armRaspPiDebugExecutable armRaspPiReleaseExecutable
	
# Unit Tests

Unit tests can be enabled at compile time to run at kernel initialisation - this ensures they run on the target
and with the target compiler as opposed to the host compiler. However, for debugging purposes a host variant is provided.

	# Windows
	gradlew hostTestDriverDebugExecutable
	
	# Linux
	./gradlew hostTestDriverDebugExecutable
	
You'll need a gcc available on the PATH for this to build.
	
# Toolchains

GCC is the compiler used by default.

For ARM I'm using 7.1.0 for PowerPC I'm using 4.9.0.

## Building the toolchain

I've added gradle tasks to build a stage-3 gcc compiler with support for C and C++.

You'll need the following (and possibly more) dependencies

* gcc
* g++ (aka gcc-g++)
* make
* automake
* diffutils
* texinfo
* gmp-devel
* mpc-devel (aka libmpc-devel)
* mpfr-devel
* isl-devel
* tar
* libiconv
* libiconv-devel
* flex
* m4
* bison
* expat
* zlib-devel

Then execute:

    # Linux (or MSYS2 under Windows)
    PATH=$PATH:/home/sam/git/tinker-kernel/arm-eabi/bin
    ./gradlew makeInstallBintutilsArm makeInstallGccStage1Arm makeInstallNewlibArm makeInstallGccStage3Arm weaveApiArm makeInstallGdbArm

You should then have a toolchain in 'arm-eabi'.

# Loading / Packaging

The packager will create a binary image with the kernel followed by user-land processes or servers:

 bootstrap
  - kernel >\
  - (setup) |
  - service<| (i.e. filesystem)
  - service<| (i.e. block access for sata)
  - service<| (i.e. tcp/ip stack)
  - service</ (i.e. hardware device driver)
  
The intention is that the kernel is started by firmware or bootloader like u-boot.

## Packaging Builds

The utilBuilder program can be used to package the kernel and user-land services and programs.

We need to tell it the output file 'kernel.img', the format 'arm-eabi' and endianness 'small' and then a list of all the user-land processes.

Firstly, utilBuilder must be built

	cd utilBuilder

    # Windows
    gradlew packageJar

    # Linux
    ./gradlew packageJar
    
Then we can use it to create our final binary:

    # Execute from inside the utilBuilder directory
    gradlew packageJar 
    
    # Windows
    java -jar build\libs\utilBuilder-bin-1.0.0.jar kernel.img arm-eabi small ..\bspRaspberryPi\build\exe\armRaspPi\debug\armRaspPi.exe
    
    # Linux
    java -jar build/libs/utilBuilder-bin-1.0.0.jar kernel.img arm-eabi small ../bspRaspberryPi/build/exe/armRaspPi/debug/armRaspPi.exe
    
We can additionally use the test 'hello world' program

    # Execute from inside the tinker directory
    gradlew armRaspPiDebugExecutable elfLoaderTestTinkerArm4SoftDebugExecutable
    
    # Now lets build the image
    cd utilBuilder
    gradlew packageJar
    java -jar build\libs\utilBuilder-bin-1.0.0.jar kernel.img arm-eabi small ..\bspRaspberryPi\build\exe\armRaspPi\debug\armRaspPi.exe ..\elfLoaderTest\build\exe\elfLoaderTest\tinkerArm4Soft\debug\elfLoaderTest.exe 
   
This will generate a 'kernel.img' file in the current directory with the Raspberry Pi kernel and one userland ELF - the hello world.

# Hello World (no C library)

The following is an example process that can be loaded into the kernel.

    gradlew elfLoaderTestTinkerArm4SoftReleaseExecutable
    
We can check the layout of the process with objdump.

    arm-eabi-objdump -x elfLoaderTest\build\exe\elfLoaderTest\tinkerArm4Soft\release\elfLoaderTest.exe
    
To load the elf it needs to be loaded either in kernel_main or via a syscall from another process (such as an init process).

# Hello World (newlib C library)

As I write newlib support I've also written hello world which uses printf

	gradlew elfNewlibTestDebugExecutable
	
This is still work in progress.

# Issues / TODO

These are the things I need to address in a rough order:

## Immediate

* Caching on at startup causes BSP init failures - Device mappings shouldn't be cached
* Timer not firing - cache disabled? Or WORD alignment issue due to caching again - can't be if caching disabled
  * Target time is too small, even when increased, didn't fire

## Longer term

* Multicore not supported; hence why works on Pi1 but not Pi2 
* Stdio for Pi is reading one char a time, should read all available data
* Kernel: Review TODO/FIXMEs and add them here
* Kernel: Add DMA support for pipes
* Kernel: Timeouts on pipe (open/read/write)
* Doc: Doc it with doxygen

# Debugging

Starting QEMU for the Raspberry Pi build

	# Disassemble the Raspberry Pi build so we can look at addresses
	arm-eabi-objdump -dS bspRaspberryPi\build\exe\armRaspPi\debug\armRaspPi.exe > dis.txt

	# Start the emulator (Windows)
	qemu-system-arm -m 512M -kernel bspRaspberryPi/build/exe/armRaspPi/debug/armRaspPi.exe -gdb tcp::1234,ipv4 -no-reboot -no-shutdown -machine raspi -serial tcp:127.0.0.1:12345 -S

	# Start the emulator (Linux)
	qemu-system-arm -m 512M -kernel bspRaspberryPi/build/exe/armRaspPi/debug/armRaspPi -gdb tcp::1234 -no-reboot -no-shutdown -machine raspi -serial tcp:127.0.0.1:12345 -S

	# Start a debugger
	arm-eabi-gdb build\binaries\armRaspPiExecutable\debug\armRaspPi.exe
	# Connect to the debugger
	target remote localhost:1234
	# Set a breakpoint at the start
	b kernel_main
	# Start the kernel
	continue
	
# Building QEMU

	git clone https://github.com/Torlus/qemu.git
	cd qemu

	# Cut down QEMU for just ARM
	CFLAGS="-Ofast -g0 -march=native -pipe -fomit-frame-pointer" LDFLAGS="-g0 -Ofast" ./configure --target-list=arm-softmmu --disable-werror --disable-stack-protector --audio-drv-list= --enable-trace-backends=nop --disable-slirp --enable-tcg-interpreter --disable-user --disable-linux-user --enable-pie --disable-vte --disable-sdl --disable-vnc --disable-curses --disable-virtfs --disable-xen --disable-brlapi --disable-curl --enable-fdt --disable-bluez --enable-kvm --disable-rdma --enable-uuid --disable-vde --disable-netmap --enable-linux-aio --disable-cap-ng --enable-attr --disable-vhost-net --disable-spice --disable-rbd --disable-libiscsi --disable-libnfs --disable-smartcard-nss --disable-libusb --disable-usb-redir --disable-lzo --disable-snappy --disable-bzip2 --disable-seccomp --disable-tpm --disable-numa --disable-gtk --disable-docs  --disable-vhdx --disable-vhost-scsi
	
	make
	make install

# Concepts
Below is a description of the core concepts of the kernel and the rational of any design decisions.
## Locking
Locking is absent as it's a single-core kernel and pre-emption in syscalls/interrupts is disabled. In the future this will change but for now it keeps it simple and small.
## Process & Threads
Proceses are used as the container that stores everything related to the process - threads, objects and memory etc. As we're aiming for real-time, when a process creates a new process, any memory for the new process is allocated from the parent process. The exclusion for this is the processes initialised from the kmain() code where the process's memory is taken from the main pool. This avoids problems with possible starvation. Later on, with an ELF loader, it's possible that any new process could also be loaded from the main pool.
## Pipes (messaging inc. interrupts)
Pipes are used to send messages between processes and can register as interrupt handlers. They can be created as senders, receivers or both. Their width (message size) and depth (message count) are predefined. When multipled together, and aligned, it equals the amount of memory requred.

Once a message has been read by the application, it must be acknowledged. This is because the read gets a pointer to the buffer and the acknowledment frees that memory so another can write into it.

There's a built in Pipe in the kernel called 'in' which can be used to read characters from the character input device (like a UART or keyboard).
## Semaphores
Nothing special here. Support priority inheritance.
## Shared Memory
Nothing special here.
## Timers
Nothing special here.
## Objects (the registry)
The register stores objects by name so they can be looked up by different processes.
## Drivers
Drivers are written as userland services with MMIO through the MMU.

The kernel Board Support Package (BSP) should only have drivers for timers and a debugging port such as a UART.

The root Interrupt Handler should be part of the kernel, other ones should really be installed as
user processes/servers that listen to the pipe.

# Third Party Software

A list of third party software under their associated licenses.

## ISC License

* libelf, https://github.com/gdboot/elfload

Used to parse ELF files when loading them as new processes

## Public Domain

* dlmalloc, http://g.oswego.edu/dl/html/malloc.html

Use of mspaces as the underlying memory pool implementations
