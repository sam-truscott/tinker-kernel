/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
 
/* http://www.redhat.com/docs/manuals/enterprise/RHEL-4-Manual/gnu-linker/memory.html */
ENTRY (__kernel_start)

MEMORY 
{
	RAM : ORIGIN = 0x00000000, LENGTH = 0x8000000 /* 128MB */
}

SECTIONS{

	. = 1M;
	
	/* code */
    .text BLOCK(4K) : ALIGN (0x1000) {
        __text = .;
    	*(.boot)
    	. = ALIGN(4);
        *(.text)
        __text_end = .;
    }>RAM
    
    /* code */
    .api ALIGN (0x1000) : {
    	__api = .;
    	*(.api)
    	 . = ALIGN(0x1000);
    	 __api_end = .;
    }>RAM
    
    .utext ALIGN (0x1000) :
    {
        __utext = .;
        *(.utext)
        . = ALIGN(0x1000);
        __euser_text = .;
        __udata = .;
        *(.udata)
        __euser = ALIGN(0x1000);
    }>RAM
    
    /* Small Block Started by Symbol */
    .sbss ALIGN (0x1000): {
        *(.sbss)
    }>RAM
    
        /* Block Started by Symbol */
    .bss ALIGN (0x1000) : {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }>RAM
    
    /* read only data */
    .rodata ALIGN (0x1000) : {
        *(.rodata)
        *(.rodata*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }>RAM
    
    /* static data */
    .sdata ALIGN (0x4) : {
        *(.sdata)
    }>RAM
    
    /* data */
    .data ALIGN (0x4) : {
        *(.data.*)
        *(.got2)
        *(.fixup)
        . = ALIGN(0x1000);
      	__data_end = .;
    }>RAM
    
    .dynsym ALIGN (0x1000) : { *(.dynsym) }>RAM
    .dynstr ALIGN (0x1000) : { *(.dynstr) }>RAM
    .hash   ALIGN (0x1000) : { *(.hash)   }>RAM
    
    .memory_pool ALIGN (0x1000) (NOLOAD) : {
    _end = .;
    }>RAM
}
