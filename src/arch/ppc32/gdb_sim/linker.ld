/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */
 
/* http://www.redhat.com/docs/manuals/enterprise/RHEL-4-Manual/gnu-linker/memory.html */
ENTRY (__kernel_start)

__VECTOR_SIZE = 0xF00;
__VECTOR_STACK_SIZE = 0x1000;

MEMORY 
{
	RAM : ORIGIN = 0x00000000, LENGTH = 0x8000000 /* 128MB */
	/* FLASH : ORIGIN = 0xFE000000, LENGTH = 0xetc */
}

SECTIONS{
    
    /* interrupt vector table */
    .ivt 0x100 : {
        __ivt = .;
        *(.vectors)
        . += __VECTOR_SIZE;
    } > RAM
    
    .ivt_stack 0x1000 : {
        __ivt_stack = .;
        . += __VECTOR_STACK_SIZE;
        __ivt_stack_end = .;
    } > RAM
    
    /* Small Block Started by Symbol */
    .sbss ALIGN (0x1000): {
        *(.sbss)
    }>RAM
    
    /* read only data */
    .rodata ALIGN (0x1000) : {
        *(.rodata)
        *(.rodata*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }>RAM

    /* Block Started by Symbol */
    .bss ALIGN (0x1000) : {
        *(COMMON)
        *(.bss)
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
    }>RAM
   
	/* code */
    .text ALIGN (0x1000) : { /* 0x110000 : { */
        __text = .;
    	*(.boot)
    	*(.vector_code)
    	. = ALIGN(4);
        *(.text)
        __text_end = .;
    }>RAM
    
    .user_text ALIGN (0x1000) :
    {
        __user_text = .;
        *(.user_text)
        _euser_text = ALIGN(0x1000);
    }
        
    .user_data ALIGN (0x1000) :
    {
        __user_data = .;
        *(.user_data)
        __euser = ALIGN(0x1000);
    }>RAM
    
    .dynsym ALIGN (0x1000) : { *(.dynsym) }>RAM
    .dynstr ALIGN (0x1000) : { *(.dynstr) }>RAM
    .hash   ALIGN (0x1000) : { *(.hash)   }>RAM
    
    .memory_pool ALIGN (0x1000) : {
    	end = .;
    }>RAM
    
    /* .page_table ALIGN(0x1000) : { */
    .page_table 0x7F00000 : {
        *(.page_table)
    }>RAM
}
