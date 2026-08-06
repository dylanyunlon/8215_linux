SECTIONS
{
	.serial_text :
	{
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
	}
}
