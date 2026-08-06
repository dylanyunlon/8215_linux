ENTRY(_start)
SECTIONS
{
	. = 0x40000;
	_start = .;
	. = ALIGN (32);
	.resident_text _start : AT(_start)
	{
		*(.resident.text)
	}
	
	. = ALIGN (32);
	.init :
	{
		*(.init_text)
	}
        . = ALIGN (32);
	.module_text :
	{
		*(.module_text)
	}

        . = ALIGN (32);
	
        _aec_ndclib_start = .;
        .aec_ndclib : 
	{
		*(.text .text.*)
		*(i.__ARM_common_memclr4_8)
                *(i.__ARM_common_memcpy4_12)
	}
	
	. = ALIGN (32);
	_ro_rw_start = .;

	.ro_rw_data _ro_rw_start : AT(_ro_rw_start)
	{
		*(.rodata .rodata.*)
		*(.data .data.*)
        *(.backcar_res)
	}

        .got : { *(.got) }
	.glue_7 : {*(.glue_7)}
	.glue_7t : {*(.glue_7t)}
	.vfp11_veneer : {*(.vfp11_veneer)}
	.v4_bx : {*(.v4_bx)}
        .ksym :
        {
             *(___ksymtab*)
        }

	. = ALIGN (32);
	_bss_start = .;
	.bss : 
	{ 
		*(.bss) 
		*(COMMON)
	}
	. = ALIGN (32);
	_end = .;
	

	/DISCARD/ : {
	  *(.eh_frame)
	}	

}

