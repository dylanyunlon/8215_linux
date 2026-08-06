#ifndef _TEST_LIB_H_
#define _TEST_LIB_H_

#define AP_WRITE_PT11032(dvd_addr,value) *((volatile UINT32*)(0x70380000+dvd_addr)) = value   //DVD:0x2c04)
#define AP_READ_PT11032(dvd_addr)   *((volatile UINT32*)(0x70380000+dvd_addr))

void AP_access_pt110_reg_init(void);

#endif

