sos3
====

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

** re-do hashmaps; they're just wrong!
* mmu code on ppc isn't finished, e.g. no flushing out when process dies
* makefile
* no way to load usermode elfs in
* replace the mempool implementation with dlmalloc (?)
* pipes, shms and timers aren't implemented yet
* no crt / newlib code for userland yet
* re-write memcpy to be target specific with weak reference for basic implementation
* need to review it all head to toe since refactor
* doc it more with doxygen
* ARM support!

toolchain
=========

a standard stage 1 gcc compiler will work (i.e. C compiler without libc support)

 arm
 ===
 
  binutils
  ========
  
  ../binutils-2.23.2/configure --target=arm-none-eabi --prefix=/usr/local/xtools/arm-none-eabi
  
  gcc
  ===
  
  ../gcc-4.8.1/configure --target=arm-none-eabi --prefix=/usr/local/xtools/arm-none-eabi --enable-languages=c,lto --disable-multilib --enable-lto --disable-libquadmath --without-headers --with-newlib --disable-libssp
  
  gdb
  ===
  
  ../gdb-7.6/configure --target=arm-none-eabi --prefix=/usr/local/xtools/arm-none-eabi --enable-sim --enable-sim-bswap --enable-sim-stdio --enable-sim-trace

 powerpc
 =======

  binutils
  ========

  ../binutils-2.23.2/configure --target=powerpc-none-eabi --prefix=/usr/local/xtools/powerpc-none-eabi
 
  gcc
  ===

  ../gcc-4.8.1/configure --target=powerpc-none-eabi --prefix=/usr/local/xtools/powerpc-none-eabi --enable-languages=c,lto --disable-multilib --enable-lto --disable-libquadmath --without-headers --with-newlib --disable-libssp

  gdb
  ===

  Need to add a weak reference to option_mpc... to get it to compile with inline turned on
  
  ../gdb-7.6/configure --prefix=/usr/local/xtools/powerpc-none-eabi --target=powerpc-none-eabi --enable-sim --enable-sim-powerpc --enable-sim-bitsize=32 --enable-sim-bswap --enable-sim-duplicate --enable-sim-endian=big --enable-sim-env=operating --enable-sim-float --disable-sim-reserved-bits --disable-sim-smp --enable-sim-timebase --enable-sim-trace --enable-sim-inline --disable-sim-xor-endian --disable-sim-assert --enable-sim-switch --disable-sim-icache --enable-sim-clfags='-g0,-O3,-fno-strength-reduce,-fomit-frame-pointer'

