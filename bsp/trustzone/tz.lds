ENTRY(_start)
SECTIONS
{
	. = 0x100000;
	. = ALIGN (32);
	.resident_text :
	{
		*(.resident.text)
	}
	
	. = ALIGN (32);
	.init :
	{
		*(.init_text)
	}
        . = ALIGN (32);
	
	_ro_rw_start = .;

	.ro_rw_data _ro_rw_start : AT(_ro_rw_start)
	{
		*(.rodata .rodata.*)
		*(.data .data.*)
	}

        .got : { *(.got) }
	. = ALIGN (32);
	_bss_start = .;
	.bss : 
	{ 
		*(.bss) 
		*(COMMON)
	}
	. = ALIGN (32);
	_end = .;
	

}

