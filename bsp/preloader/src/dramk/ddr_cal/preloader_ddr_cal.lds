SECTIONS
{
	.ddr_cal_text :
	{
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
	}
}