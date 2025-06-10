OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
    RAM(rwx) : org = 0x20000000, len = 4M
    BOOT(rx) : org = 0x00000000, len = 4M
}

__boot = ORIGIN(BOOT);
__eboot = ORIGIN(BOOT) + LENGTH(BOOT);
__ram = ORIGIN(RAM);
__eram = ORIGIN(RAM) + LENGTH(RAM);
__tos = __eram;

SECTIONS {
      .text __boot : AT (__boot) {
        __text = .;
        KEEP(*(.vectors))
        *(.text)
        *(.text.*)
        . = ALIGN(4);
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
        __etext = .;
    }
    
    . = __ram;
    .data ALIGN(4) : AT (__etext) {
        __data = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        __edata = .;
    }
    
    .bss ALIGN(4) : AT (__edata) { 
        __bss = .;      
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __ebss = .;
    }
}