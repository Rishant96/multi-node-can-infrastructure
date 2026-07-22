ENTRY(Reset_Handler)

/* STM32F103C8T6 (BluePill) Memory Map */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Set the stack pointer to the very end of SRAM */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    /* The vector table and application code go into FLASH */
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Keep vector table at the very start of FLASH */
        *(.text)           /* General code */
        *(.text*)
        *(.rodata)         /* Read-only data (constants, strings) */
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;        /* Mark the end of FLASH code (source for .data copy) */
    } > FLASH

    /* Initialized global/static variables go into SRAM, but their initial values are stored in FLASH */
    .data : AT(_etext)
    {
        . = ALIGN(4);
        _sdata = .;        /* Start of SRAM .data */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;        /* End of SRAM .data */
    } > SRAM

    /* Uninitialized global/static variables (zeroed on boot) go into SRAM */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* Start of SRAM .bss */
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;         /* End of SRAM .bss */
    } > SRAM
}
