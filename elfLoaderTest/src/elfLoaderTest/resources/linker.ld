/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2016] Samuel Steven Truscott
 *  All Rights Reserved.
 */
ENTRY (main)

SECTIONS{

    . = 0x8000;

    /* code */
    .text ALIGN (0x1000) : {
        *(.text)
        *(.api)
    }

    /* Block Started by Symbol */
    .bss : {
        *(COMMON)
        *(.bss)
    }
    
    /* read only data */
    .rodata : {
        *(.rodata)
        *(.rodata*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
    
    /* Small Block Started by Symbol */
    .sbss : {
        *(.sbss)
    }
    
    /* static data */
    .sdata : {
        *(.sdata)
    }
    
    /* data */
    .data : {
        *(.data.*)
        *(.got2)
        *(.fixup)
    }
}