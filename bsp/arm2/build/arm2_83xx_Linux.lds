ENTRY(_start)

MEMORY {
    arm2_dram : ORIGIN = 0x40000, LENGTH = 0x500000
}

SECTIONS
{
    . = 0x40000;
    _start = .;
    .resident_text _start : AT(_start)
    {
        _resident_start = .;
        *(.resident.text)
        _resident_end = .;
    } > arm2_dram

    .dram_section :
    {
        . = _resident_end;
        . = ALIGN (1024);
        __dram_section_start = .;
        *(.dram_text)
        *(i.__ARM_common_memclr4_8)
        *(i.__ARM_common_memcpy4_12)
        *(.eh_frame)
        __dram_section_end = .;
    } > arm2_dram

    .ctext :
    {
        . = ALIGN(8);
        KEEP(*(.init))
        KEEP(*(.fini))

        /* Add by Qt - Begin */
        /* .ctors */
        . = ALIGN(8);
        KEEP(*crtbegin.o(.ctors))
        KEEP(*crtbegin?.o(.ctors))
        KEEP(*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))

        /* .dtors */
        KEEP(*crtbegin.o(.dtors))
        KEEP(*crtbegin?.o(.dtors))
        KEEP(*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))

        /* Add by Qt - Begin */
        . = ALIGN(8);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(8);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(8);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

    } > arm2_dram

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > arm2_dram

    .got : { *(.got) } > arm2_dram
    .glue_7 : {*(.glue_7)} >arm2_dram
    .glue_7t : {*(.glue_7t)} > arm2_dram
    .vfp11_veneer : {*(.vfp11_veneer)} > arm2_dram
    .v4_bx : {*(.v4_bx)} > arm2_dram
    .ksym : {*(___ksymtab*)} > arm2_dram

    .dram_rodata ALIGN(4) : {
        __dram_rodata_start = .;
        *(.dram_rodata)
        __dram_rodata_end = .;
    } > arm2_dram

    .dram_data ALIGN(0x1000) : {
        __dram_data_start = .;
        *(.dram_data)
        __dram_data_end = .;
    } > arm2_dram

    . = ALIGN(1024);
    __func_table_start = .;

    .bss ALIGN(1024) :AT(_bss_start)
    {
        _bss_start = .;
        __dram_bss_start = .;
        . = ALIGN (32);
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(.sbss)
        *(.sbss.*)
        *(COMMON)
        *(.dram_bss)

        _end = .;
    } > arm2_dram

    .pagetbl ALIGN(0x4000) (NOLOAD):
    {
        *(.mmu.l1)
        *(.mmu.l2)
    }

    .heap ALIGN (32) (NOLOAD):
    {
        __heap_start = .;
        *(.global_heap)
        __heap_end = ALIGN (0x1000);
    } > arm2_dram
}

