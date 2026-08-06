#include "targetConfig.h"
#include "preloader_common.h"
#include "AP_Show_Test.h"

#if(AP_BOOTUP_DVD_ENABLE)
void ApBootupDvd(void)
{

  IO_WRITE32(IO_BASE,0xac,0xFFFFFFFF);  //open clk gate for DVD all module
  IO_WRITE32(IO_BASE,0xc8,0xFFFFFFFF);  //open rst for DVD all module
  
  IO_WRITE32(IO_BASE,0x38308,0xF);      //  DVD use 240-256M mem space 0xF770000  
  IO_WRITE32(IO_BASE,0x94,0x01);        //open DVD top clk gate

 //YU need confirm DVD and AP uart 
 // IO_WRITE32(IO_BASE,0x58,0x198000);
 // IO_WRITE32(IO_BASE,0x6c,0x142800);

 // IO_WRITE32(IO_BASE,0x38088,0x10000); //enable pt100 access ap (TVE)
  IO_WRITE32(IO_BASE,0x3A0008,0x7C0);  //0xF800000,8032 code
  IO_WRITE32(IO_BASE,0x3A0010,0x3);    //tell 8032 code in dram

 
  IO_WRITE32(IO_BASE,0x94,0x3);//open 8032 clock gate,8032 can run

	

}
#endif 

