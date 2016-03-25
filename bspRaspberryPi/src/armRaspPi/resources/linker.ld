/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
 
/* http://www.redhat.com/docs/manuals/enterprise/RHEL-4-Manual/gnu-linker/memory.html */
ENTRY (kernel_start)

__VECTOR_SIZE = 0x40;
__VECTOR_STACK_SIZE = 0x1000;

MEMORY 
{
	RAM : ORIGIN = 0x00000000, LENGTH = 0x10000000 /* 256MB */
}

SECTIONS{

	/* interrupt vector table */
    .ivt (NOLOAD) : {
        __ivt = .;
        *(.vectors)
        . += __VECTOR_SIZE;
    } > RAM
    
    . = 0x8000;
    .entry 0x8000 : {
    	*(.entry)
    } > RAM
    
    .boots (NOLOAD) : {
        __boots = .;
        . += __VECTOR_STACK_SIZE;
        __bootse = .;
    } > RAM
    
    .ivts (NOLOAD) : {
        __ivts = .;
        . += __VECTOR_STACK_SIZE;
        __ivtse = .;
    } > RAM
    
     /* Block Started by Symbol */
    .bss ALIGN (0x1000) : {
        *(COMMON)
        _bss_start = .;
        *(.bss)
		_bss_end = .;
    }>RAM
    
    /* read only data */
    .rodata ALIGN (0x1000) : {
        *(.rodata)
        *(.rodata*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }>RAM
    
    /* Small Block Started by Symbol */
    .sbss ALIGN (0x1000): {
        *(.sbss)
    }>RAM
    
    /* static data */
    .sdata ALIGN (0x1000) : {
        *(.sdata)
    }>RAM
    
    /* data */
    .data ALIGN (0x1000) : {
        *(.data.*)
        *(.got2)
        *(.fixup)
        . = ALIGN(0x1000);
      	__data_end = .;
    }>RAM
    
    /* code */
    .api ALIGN (0x1000) : {
    	__api = .;
    	*(.api)
    	 . = ALIGN(0x1000);
    	 __api_end = .;
    }>RAM
   
	/* code */
    .text ALIGN (0x1000) : {
        __text = .;
    	*(.boot)
    	*(.vector_code)
    	. = ALIGN(4);
        *(.text)
        __text_end = .;
    }>RAM
    
    .utext ALIGN (0x1000) :
    {
        __utext = .;
        *(.utext)
        . = ALIGN(0x1000);
        _euser_text = .;
        __udata = .;
        *(.udata)
        __euser = ALIGN(0x1000);
    }>RAM
    
    .memory_pool ALIGN (0x1000) (NOLOAD) : {
    	end = .;
    }>RAM
}
