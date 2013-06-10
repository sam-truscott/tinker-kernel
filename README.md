sos3
====

microkernel targeted at gdb (ppc-sim) written in c

toolchain
=========

 binutils
 ========

 ../binutils-2.23.1/configure --target=powerpc-none-eabi --prefix=/usr/local/xtools/powerpc-none-eabi
 
 gcc
 ===

 ../gcc-4.8.1/configure --target=powerpc-none-eabi --prefix=/usr/local/xtools/powerpc-none-eabi --enable-languages=c,lto --disable-multilib --enable-lto --disable-libquadmath --without-headers --with-newlib --disable-libssp

 gdb
 ===

 ../gdb-7.5.1/configure --prefix=/usr/local/xtools/powerpc-none-eabi --target=powerpc-none-eabi --enable-sim --enable-sim-bitsize=32 --enable-sim-bswap --enable-sim-duplicate --enable-sim-endian=big --enable-sim-env=operating --enable-sim-float --enable-sim-reserved-bits --enable-sim-smp=1 --enable-sim-timebase --enable-sim-trace --enable-sim-stdio --enable-sim-monitor

