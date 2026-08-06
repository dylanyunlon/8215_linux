SECTIONS
{
	.main_text :
	{
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
	}
}
