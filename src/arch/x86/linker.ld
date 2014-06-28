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

__VECTOR_SIZE = 0xF00;
__VECTOR_STACK_SIZE = 0x1000;

MEMORY 
{
	RAM : ORIGIN = 0x00000000, LENGTH = 0x8000000 /* 128MB */
}

SECTIONS{
    
    /* interrupt vector table */
    .ivt 0x100 (NOLOAD) : {
        __ivt = .;
        *(.vectors)
        . += __VECTOR_SIZE;
    } > RAM
    
    .ivts 0x1000 (NOLOAD) : {
        __ivts = .;
        . += __VECTOR_STACK_SIZE;
        __ivtse = .;
    } > RAM
    
    /* Small Block Started by Symbol */
    .sbss ALIGN (0x1000): {
        *(.sbss)
    }>RAM
    
        /* Block Started by Symbol */
    .bss ALIGN (0x1000) : {
        *(COMMON)
        *(.bss)
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
      	___data_end = .;
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
        ___text = .;
    	*(.boot)
    	*(.vector_code)
    	. = ALIGN(4);
        *(.text)
        ___text_end = .;
    }>RAM
    
    .utext ALIGN (0x1000) :
    {
        ___utext = .;
        *(.utext)
        . = ALIGN(0x1000);
        ___euser_text = .;
        ___udata = .;
        *(.udata)
        ___euser = ALIGN(0x1000);
    }>RAM
    
    .dynsym ALIGN (0x1000) : { *(.dynsym) }>RAM
    .dynstr ALIGN (0x1000) : { *(.dynstr) }>RAM
    .hash   ALIGN (0x1000) : { *(.hash)   }>RAM
    
    .memory_pool ALIGN (0x1000) (NOLOAD) : {
    	end = .;
    }>RAM
}
