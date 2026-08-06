SECTIONS
{
	.ddr_special_text :
	{
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
	}
}
