sos3
====

microkernel targeted at gdb (ppc-sim) written in c

toolchain
=========

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

