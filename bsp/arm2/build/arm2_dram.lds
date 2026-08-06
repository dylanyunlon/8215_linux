SECTIONS
{
	.dram_text ALIGN(8) :
	{
		*(.resident.text)
		*(.text .text.*)
	}
	.dram_data ALIGN(8) :
	{
		*(.data .data.*)
	}
	.dram_rodata ALIGN(8) :
	{
		*(.rodata .rodata.*)
	}
	.dram_bss ALIGN(8) :
	{
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
	}
}
