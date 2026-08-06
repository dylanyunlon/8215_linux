SECTIONS
{
	.module_text :
	{
	  *(.resident.text)	
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
	}
}
