SECTIONS
{

	
	.test_text :
	{
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
	}
	

}
