SECTIONS
{

    .nor_text :
    {
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.data .data.*)
    }

}
