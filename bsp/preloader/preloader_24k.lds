ENTRY(_start)

MEMORY {
    hl : ORIGIN = 0xF4000000, LENGTH = 0x7600
}

SECTIONS
{
	. = 0xF4000000;
	_start = .;
	. = ALIGN (32);
	__resident_start = .;
	.resident_text _start : AT(_start)
	{

		*(.resident.text)

	} > hl
	
	. = ALIGN (32);
	__resident_end = .;
	.main :
	{
		*(.main_text)
	} > hl
	
	. = ALIGN (32);
	.lib :
	{
		*(.lib_text)
	} > hl
  . = ALIGN (4);

	.ddr_special :
	{
		*(.ddr_special_text)
	} > hl

	_loader_start = .;
	.loader :
	{
		*(.msdc_text)
		*(.nfi_text)
		*(.nor_text)
		. = ALIGN (32);
	} > hl
	_loader_end = .;



	
	. = ALIGN (32);
	_ro_rw_start = .;

	.ro_rw_data : 
	{
		*(.rodata .rodata.*)
		*(.data .data.*)
		. = ALIGN (32);
	} > hl
	. = ALIGN (32);
	_ro_rw_end = .;
	_bss_start = .;
	.bss _bss_start : 
	{ 
		*(.bss) 
		*(COMMON)
	} > hl
	. = ALIGN (32);
	_bss_end = .;

	ASSERT(_ro_rw_end <= 0xF4007000,"text > 0x7000")

	_end = .;
	. = ALIGN (512);
	__dramk_flash_address = 0xF4007E00;

	
	. = 0xF4008000;
	__dramk_start = .;                
	.dramk : AT(__dramk_flash_address)
	{
			*(power_down)
			*(.ddr_init_text)
			*(.ddr_cal_text)
			*(.sys_manager)
			. = ALIGN (1024);
	}
	. = ALIGN (1024);
	__dramk_end = .;   
	
	/DISCARD/ : {
	  *(.eh_frame)
	}	

	PROVIDE(_dramk_loader_start = LOADADDR(.dramk));
	_dramk_loader_end = __dramk_end;

}

