#include "targetConfig.h"
#include "preloader_common.h"
#include "chip_test.h"
#include "AP_Show_Test.h"
#include "test_lib.h"



#if(AP_SHOW_TEST_ENABLE==1)

void Panel_Show_PinSetting(void)
{
  UINT32 regs;

  vSimTraceStr("FPD Pin Setting Start");
  
  //regs = CKGEN_READ32(0x68); marked by LHF
  regs &= ~(0x7 << 22);    //pwm2 -> GPIO57 for PWM_LED
  regs |= (1 << 22);

  regs &= ~(0x7 << 28);    //pwm4 -> GPIO29 for VGH_VGL
  regs |= (2 << 28);

  //CKGEN_WRITE32(0x68,regs);marked by LHF

  

  //TTL pin config,multi-fun
  //misc_ctrl[6],enable pad vb0-vb3
  IO_MASK(IO_BASE,0x94,(0x1 << 6),(0x1 << 6));
  //TTL_6_8b_sel, enable pad dclk, vr2-vr7,vg2-vg7,vb2-vb7
  //TTL_8b_sel, enable vr0-vr1,vg0-vg1,vb0-vb1
  //TTL_de_sel, enable pad de
  //TTL_SYNC_SEL, enable pad hsync and vsync
  IO_MASK(IO_BASE,0x5c,(0xF << 6),(0xF << 6));
  //TTL pin config, pre-place
  //fpd_6bit_sel, enable pad vr2-vr7,vg2-vg7,vb2-vb7
  //fpd_8bit_sel, enable pad vr0-vr1,vg0-vg1,vb0-vb1
  //fpd_de_sel, enable pad de
  //fpd_sync_sel, enable pad hsync and vsync
  IO_MASK(IO_BASE,0x298,(0xF << 28),(0xF << 28));
  vSimTraceStr("FPD Pin Setting Finish");
}

void Panel_Show_SourceClockSetting(void)
{
  UINT32 regs;
  vSimTraceStr("FPD Source Clock Setting Start");
  
  IO_MASK(IO_BASE,0xA8,(0x3F << 24),(0x3F << 24)); // pwm0-6 clock gating enable
  IO_MASK(IO_BASE,0xC4,(0x3F << 24),(0x3F << 24)); // pwm0-6 power down reset

 // regs = CKGEN_READ32(0x34);   marked by LHF

  regs &= ~(0x7 << 13);   // pwm2 dclk=27MHz
  regs &= ~(0x7 << 19);   //pwm4 dclk=27MHz

 // CKGEN_WRITE32(0x34,regs);   marked by LHF

  // DDDS -> PLLGP clk 140.16MHz
  IO_WRITE32(IO_BASE,0x52C0C,0x8000061f);   
  IO_WRITE32(IO_BASE,0x52C00,0x449F8FC7);
  IO_WRITE32(IO_BASE,0x52C04,0x020d00c3);
  IO_WRITE32(IO_BASE,0x52C14,0x713e013e);


  // marked by LHF, SYSPLL set at int_3360.s
  //PLLGP -> LVDS clk 70.08MHz
  //regs = CKGEN_READ32(0x284);
  //regs |= (1 << 0);
  //CKGEN_WRITE32(0x284,regs);
  //regs &= ~(1 << 0);
  //CKGEN_WRITE32(0x284,regs);

  // LVDS -> VOPLL clk 175.2MHz
  IO_WRITE32(IO_BASE,0xb0,0x01);
  IO_WRITE32(IO_BASE,0xCC,0x01);
  IO_WRITE32(IO_BASE,0x0a6000,0x01f107a0);
  IO_WRITE32(IO_BASE,0x0a600c,0x03000000);
  IO_WRITE32(IO_BASE,0x0a6020,0x00097800);
  IO_WRITE32(IO_BASE,0x0a6024,0x00039003);

  vSimTraceStr("FPD Source Clock Setting Finish");
}



void Panel_Show_SetClock(void)
{
  UINT32 regs;
  vSimTraceStr("FPD Clock Setting Start");
  //panel clock config, 800x480 use 29.2MHz
  //VOPLL need provide a 175.2MHz clock
  //select clk_fpd div 6 from vopll, 175.2/6=29.2MHz
  IO_MASK(IO_BASE,0x14,(0x7 << 21),(0x3 << 21));

  //scl+tcon+vdoF+fmtF: Clock Enale bit
  IO_MASK(IO_BASE, 0x000b4, (1<<0)|(1<<5)|(1<<6)|(1<<7), (1<<0)|(1<<5)|(1<<6)|(1<<7));
	//scl+tcon+vdo+fmt: Reset Enale bit
  IO_MASK(IO_BASE, 0x000d0, (1<<0)|(1<<5)|(1<<6)|(1<<7), (1<<0)|(1<<5)|(1<<6)|(1<<7));
  //select TTL clock
  IO_MASK(IO_BASE,0x34,(0x3 << 3),(0x0 << 3));
  IO_MASK(IO_BASE,0xA48E0,(0x3 << 8),(0x0 << 8));

  //fpd_ck icg enable
  IO_MASK(IO_BASE,0xD8,(0x1 << 19),(0x1 << 19));
  IO_MASK(IO_BASE,0xA48E0,(0x1 << 0),(0x1 << 0));

  //select 27 XTAL as VDOut source clock
  IO_MASK(IO_BASE,0xD8,(1 << 31),(1 << 31));
  
  //adjust fpd_ck pahse
  //select fpd_hck not delayed
  //select fpd_hck not inversed
  IO_MASK(IO_BASE,0x0DC,(0x7FFFFF << 0),(0x1 << 0));

  //config PWM4 & PWM2 for panel VGHL & LED
  //PWM4 config,132.35KHz
  IO_WRITE32(IO_BASE,0x50210,0x06503205);
  //PWM4 sync trigger mode,  output pwm directly
  IO_WRITE32(IO_BASE,0x50218,0x00000100);
  //PWM2 config,66.502KHz
  IO_WRITE32(IO_BASE,0x50208,0x0cb02205);
  //PWM2 sync trigger mode output pwm directly
  IO_WRITE32(IO_BASE,0x50218,0x00000010);
  vSimTraceStr("FPD Clock Setting Finish");
}

void TVE_Show_SetClock(void)
{
  //select 27 XTAL as VDOut source clock
  IO_MASK(IO_BASE,0xD8,(1 << 31),(1 << 31));
  //vdoR+fmtR+vdoF+fmtF: Reset Enale bit
  IO_MASK(IO_BASE, 0x000d0, (1<<7), (1<<7));
  //AP VDOR+FMTR clock enbale
  IO_MASK(IO_BASE, 0x000b4, (1<<7), (1<<7));  
  //TVE clock + DVD bim enable
  IO_MASK(IO_BASE, 0x000ac, (1<<1)|(1<<6), (1<<1)|(1<<6));
  //DVD TVE + DVD bim reset enable
  IO_MASK(IO_BASE, 0x000c8, (1<<1)|(1<<6), (1<<1)|(1<<6));
}

//default resultion: 480Px800, bypass scaler
typedef struct
{
  UINT32    *pu4RegSetting;
  UINT32     size;
} REG_SET_T;

UINT32 u4VdoF480p800[] =
{
  //main vdo
  0x42100, 0x02040000,
  0x42104, 0x00080000,
  0x42108, 0x00040000,
  0x4210C, 0x00080000,
  0x42110, 0x01E0C864,
  0x42114, 0x00000080,
  0x42118, 0x7F000000,
  0x4211C, 0x0000C001,
  0x42134, 0x14481414,
  0x42164, 0x00008000,
  0x42178, 0x00000000,
  0x4217C, 0x0000C000,
  0x42180, 0x00040000,
  0x42184, 0x00040000,
  0x42188, 0x4A0186A0,
  0x421E0, 0x010000C8,
  0x421FC, 0x00084000,
 
  //reset vdo
  0x420Ac, 0x00000403,
  0x420Ac, 0x00000003,
  0x4213C, 0x000000FF,
  0x4213C, 0x00000000,
};

UINT32 u4FmtF480p800_60Hz[] =
{
  0x42094, 0x1414EC20,
  0x4209c, 0x00000320,
  0x420A0, 0x00660385,
  0x420A4, 0x002b020B,
  0x420A8, 0x002b020B,
  0x420Ac, 0x00000003,
  0x420B0, 0x01000001,  
  0x420B4, 0x00808010,
  0x420B8, 0x00808010,
  0x420D0, 0x806F038E,
  0x420D4, 0x13B9020D,
  0x420E0, 0x002C020D,
  0x420E4, 0x00000000,
  0x420E8, 0x0c051006,
  0x420F0, 0x002C020D,
  0x420F4, 0x00000000,

  //reset
  0x420Ac, 0x00000403,
  0x420Ac, 0x00000003,
  0x4213C, 0x000000FF,
  0x4213C, 0x00000000,  
};

UINT32 u4Tcon800x480p[] =
{
  0xA4700, 0x03180700,
  0xA4704, 0x31FFFFFF,
  0xA4724, 0x2B000000,
  
  0xA4804, 0x0104017E,
  0xA4820, 0x28000001,
  0xA4824, 0x001F71F5,
  0xA4830, 0x0006800E,
  0xA4834, 0x00000001,
  0xA48E0, 0x0000247D,
  0xA48E4, 0x0005F51A,
  0xA48F0, 0x00000002,
};

UINT32 u4Tcon480p[] =
{
  0xA4700, 0x03180700,
  0xA4704, 0x31FFFFFF,
  0xA4724, 0x2B000000,
  
  0xA4804, 0x0104017E,
  0xA4820, 0x28000001,
  0xA4824, 0x001F71F5,
  0xA4830, 0x0006800E,
  0xA4834, 0x00000001,
  0xA48E0, 0x00002475,
  0xA48E4, 0x0005F51A,
  0xA48F0, 0x00000002,
};

UINT32 u4Scl480pi800x480po[] =
{
  0xA4514, 0x00100000,
  0xA451C, 0x00518000,
  0xA4530, 0x00002700,  
  0xA453C, 0x000B16D1,
  
  0xA468C, 0x03A0020D,
  0xA4690, 0x00600022,
  0xA46A0, 0x00210347,
  0xA46A4, 0x000A01EA,
  0xA46A8, 0x000A01EA,
  0xA46B0, 0x00E50047,
  0xA46DC, 0x000A000A,
  0xA46E8, 0x035A035A,
};

UINT32 u4VdoR480p[] =
{
  //main vdo
  0x43100, 0x02040000,
  0x43104, 0x00080000,
  0x43108, 0x00040000,
  0x4310C, 0x00080000,
  0x43110, 0x01E0B45A,
  0x43114, 0x00000080,
  0x43118, 0x7F000000,
  0x4311C, 0x0000C001,
  0x43134, 0x14481414,
  0x43164, 0x00008000,
  0x43178, 0x00000000,
  0x4317C, 0x0000C000,
  0x43180, 0x00040000,
  0x43184, 0x00080000,
  0x43188, 0x4A0186A0,
  0x431E0, 0x010000B4,
  0x431E4, 0x00000000,
  0x431FC, 0x00084100,
 
  //reset vdo
  0x430Ac, 0x00000403,
  0x430Ac, 0x00000003,
  0x4313C, 0x000000FF,
  0x4313C, 0x00000000,
};

//480P
UINT32 u4FmtR480p[] =
{
  0x43094, 0x14148C20,
  0x4309C, 0x000002D0,
  0x430A0, 0x006D033C,
  0x430A4, 0x002C020B,
  0x430A8, 0x002C020B,
  0x430AC, 0x00000043,
  0x430B0, 0x01000001,  
  0x430B4, 0x00808010,
  0x430B8, 0x00808010,
  0x430D0, 0x80740343,
  0x430D4, 0x035A020D,
  0x430E0, 0x002B020C,
  0x430E4, 0x00000000,
  0x430E8, 0x0c051006,
  0x430F0, 0x002B020C,
  0x430F4, 0x00000000,

  //reset
  0x430Ac, 0x00000443,
  0x430Ac, 0x00000043,
  0x4313C, 0x000000FF,
  0x4313C, 0x00000000,  
};
//480I
UINT32 u4VdoR480i[] =
{
  //main vdo
  0x43100, 0x02040000,
  0x43104, 0x00080000,
  0x43108, 0x00040000,
  0x4310C, 0x00080000,
  0x43110, 0x01E0B45A,
  0x43114, 0x00000100,
  0x43118, 0x7F000000,
  0x4311C, 0x0000C001,
  0x43134, 0x14481414,
  0x43164, 0x00008000,
  0x43178, 0x00000000,
  0x4317C, 0x0000C000,
  0x43180, 0x00040000,
  0x43184, 0x00080000,
  0x43188, 0x4A0186A0,
  0x431E0, 0x010000B4,
  0x431E4, 0x00000000,
  0x431FC, 0x00084100,
 
  //reset vdo
  0x430Ac, 0x00000403,
  0x430Ac, 0x00000003,
  0x4313C, 0x000000FF,
  0x4313C, 0x00000000,
};

//480I
UINT32 u4FmtR480i[] =
{
  0x43094, 0x14140820,
  0x4309C, 0x000002D0,
  0x430A0, 0x00D80667,
  0x430A4, 0x00140103,
  0x430A8, 0x011b020A,
  0x430AC, 0x00000003,
  0x430B0, 0x01000001,  
  0x430B4, 0x00808010,
  0x430B8, 0x00808010,
  0x430D0, 0x80740343,
  0x430D4, 0x08980465,
  0x430E0, 0x00130102,
  0x430E4, 0x00000000,
  0x430E8, 0x0c051006,
  0x430F0, 0x001A0209,
  0x430F4, 0x00000000,

  //reset
  0x430Ac, 0x00000403,
  0x430Ac, 0x00000003,
  0x4313C, 0x000000FF,
  0x4313C, 0x00000000,  
};
UINT32 u4TveSetting[] =
{
  0x2C04, 0x00090E00,
  0x2C34, 0x083F407F,
  0x2C78, 0x00000000,
  0x2C60, 0x00000001,
};

UINT32 u4DvdFmt480p[] =
{
  0x1094, 0x14140305,
  0x109c, 0xC0230280,
  0x10A0, 0x00E30683,
  0x10A4, 0x10160106,
  0x10A8, 0x111D020D,
  0x10Ac, 0x00000002,
  0x10B0, 0x00EC0001,  
  0x10B4, 0x00404040,
  0x10B8, 0x00404040,
  0x10E0, 0x00130102,
  0x10E4, 0x00000000,
  0x10E8, 0x0C051006,
  0x10F0, 0x011A0209,
  0x10F4, 0x00DF0682,

  //reset
  0x10Ac, 0x00000402,
  0x0Ac,  0x00000002,//0x10AC ???
  0xC3C,  0x000000FF,
  0xC3C,  0x00000000,  
};

REG_SET_T _ArVdoPlaySetting[] =
{
  {u4VdoF480p800, sizeof(u4VdoF480p800)/4},                  // 0
  {u4FmtF480p800_60Hz, sizeof(u4FmtF480p800_60Hz)/4},        // 1
  {u4Tcon800x480p, sizeof(u4Tcon800x480p)/4},                // 2
  {u4VdoR480p, sizeof(u4VdoR480p)/4},                        // 3
  {u4FmtR480p, sizeof(u4FmtR480p)/4},                        // 4
  {u4TveSetting, sizeof(u4TveSetting)/4},                    // 5
  {u4Tcon480p, sizeof(u4Tcon480p)/4},                        // 6
  {u4Scl480pi800x480po, sizeof(u4Scl480pi800x480po)/4},      // 7
  {u4DvdFmt480p, sizeof(u4DvdFmt480p)/4},                    // 8
  {u4FmtR480i, sizeof(u4FmtR480i)/4},                        // 9
  {u4VdoR480i, sizeof(u4VdoR480i)/4},                        // 10
};

void vPmxRegSet(UINT32 pu4Array[], UINT32 u4Size)
{
  UINT32 u4Idx;
  UINT32 u4RegAddr, u4RegVal;
  
  u4Size = u4Size/2;
  
  for(u4Idx =0; u4Idx<u4Size; u4Idx++)
  {
    u4RegAddr = pu4Array[u4Idx*2];
    u4RegVal = pu4Array[u4Idx*2+1];
    IO_WRITE32(IO_BASE, u4RegAddr, u4RegVal);
  }
} 

void vPmxDvdRegSet(UINT32 pu4Array[], UINT32 u4Size)
{
  UINT32 u4Idx;
  UINT32 u4RegAddr, u4RegVal;
  
  u4Size = u4Size/2;
  
  for(u4Idx =0; u4Idx<u4Size; u4Idx++)
  {
    u4RegAddr = pu4Array[u4Idx*2];
    u4RegVal = pu4Array[u4Idx*2+1];
    AP_WRITE_PT11032(u4RegAddr, u4RegVal);
  }
} 


#if (AP_SHOW_TEST_ENABLE == PANEL_COLORBAR)
void vPmxFPlayColorBar(void)
{
  //Scaler: Enable DCLK, DE, H/Vsync
  //IO_MASK(IO_BASE, 0x0005c, (0xf<<6), (0xf<<6));

  //Set TCON
  vPmxRegSet(_ArVdoPlaySetting[2].pu4RegSetting, _ArVdoPlaySetting[2].size);  
  //Set Fmt F
  vPmxRegSet(_ArVdoPlaySetting[1].pu4RegSetting, _ArVdoPlaySetting[1].size);
  //Enable Fmt ColorBar
  IO_MASK(IO_BASE, 0x420f4, (1<<0), (1<<0));
  //Mix select Fmt 800x480 Timming
  IO_MASK(IO_BASE, 0x1f000, (1<<1)|(1<<0), (1<<1)|(1<<0));
}
#elif(AP_SHOW_TEST_ENABLE == PANEL_PICTURE)
void vPmxFPlayPicture(void)
{
  //y.bin-0x2100000, c.bin-0x2200000.
  //Scaler: Enable DCLK, DE, H/Vsync
  //IO_MASK(IO_BASE, 0x0005c, (0xf<<6), (0xf<<6));

  //Set TCON
  vPmxRegSet(_ArVdoPlaySetting[2].pu4RegSetting, _ArVdoPlaySetting[2].size);
  //Set vdo F
  vPmxRegSet(_ArVdoPlaySetting[0].pu4RegSetting, _ArVdoPlaySetting[0].size);
  //Set Fmt F
  vPmxRegSet(_ArVdoPlaySetting[1].pu4RegSetting, _ArVdoPlaySetting[1].size);
  //Mix select Fmt 800x480 Timming
  IO_MASK(IO_BASE, 0x1f000, (1<<1)|(1<<0), (1<<1)|(1<<0));
}
#elif(AP_SHOW_TEST_ENABLE == TVE_COLORBAR)
void vPmxRPlayColorBar(void)
{
  IO_WRITE32(IO_BASE, 0x0094, 0x00000001);
  IO_WRITE32(IO_BASE, 0x0094, 0x00000003);

  IO_WRITE32(IO_BASE, 0x8300, 0x83600000);
  IO_MASK(IO_BASE, 0x8304, (0x1<<20), (0x1<<20));
  IO_MASK(IO_BASE, 0x8304, (0xffff<<0), (0x2751<<0));
#ifdef config_TARGET_FPGA
  //Emulation use
  IO_WRITE32(IO_BASE, 0x3fe194, 0);
#endif
  //Set TVE
  vPmxRegSet(_ArVdoPlaySetting[5].pu4RegSetting, _ArVdoPlaySetting[5].size);
  //Set vdo R
  //vPmxRegSet(_ArVdoPlaySetting[10].pu4RegSetting, _ArVdoPlaySetting[10].size);
  //Set Fmt R
  vPmxRegSet(_ArVdoPlaySetting[9].pu4RegSetting, _ArVdoPlaySetting[9].size);
  //TVE color bar. (fmtR not color bar)
  IO_MASK(IO_BASE, 0x2c04, (1<<1), (1<<1));
}
#elif(AP_SHOW_TEST_ENABLE == TVE_PICTURE)
void vPmxRPlayPicture(void)
{ 
  //y.bin-0x2100000, c.bin-0x2200000.
  IO_WRITE32(IO_BASE, 0x0094, 0x00000001);
  IO_WRITE32(IO_BASE, 0x0094, 0x00000003);

  IO_WRITE32(IO_BASE, 0x8300, 0x83600000);
  IO_MASK(IO_BASE, 0x8304, (0x1<<20), (0x1<<20));
  IO_MASK(IO_BASE, 0x8304, (0xffff<<0), (0x2751<<0));
#ifdef config_TARGET_FPGA
    //Emulation use
    IO_WRITE32(IO_BASE, 0x3fe194, 0);
#endif

  //Set TVE
  vPmxRegSet(_ArVdoPlaySetting[5].pu4RegSetting, _ArVdoPlaySetting[5].size);
  //Set vdo R
  vPmxRegSet(_ArVdoPlaySetting[10].pu4RegSetting, _ArVdoPlaySetting[10].size);
  //Set Fmt R
  vPmxRegSet(_ArVdoPlaySetting[9].pu4RegSetting, _ArVdoPlaySetting[9].size);
}
#elif(AP_SHOW_TEST_ENABLE == TVE_FMT_BIC)
void vPmxRPlayBuildinColor(void)
{ 
  IO_WRITE32(IO_BASE, 0x0094, 0x00000001);
  IO_WRITE32(IO_BASE, 0x0094, 0x00000003);

  IO_WRITE32(IO_BASE, 0x8300, 0x83600000);
  IO_MASK(IO_BASE, 0x8304, (0x1<<20), (0x1<<20));
  IO_MASK(IO_BASE, 0x8304, (0xffff<<0), (0x2751<<0));
#ifdef config_TARGET_FPGA
    //Emulation use
    IO_WRITE32(IO_BASE, 0x3fe194, 0);
#endif

  //Set TVE
  vPmxRegSet(_ArVdoPlaySetting[5].pu4RegSetting, _ArVdoPlaySetting[5].size);
  //Set Fmt R
  vPmxRegSet(_ArVdoPlaySetting[9].pu4RegSetting, _ArVdoPlaySetting[9].size);

  //ap R FMT build-in color
  IO_WRITE32(IO_BASE, 0x430AC, 2);
  IO_WRITE32(IO_BASE, 0x430B4, 0x00f0f0f0);
  IO_WRITE32(IO_BASE, 0x430B8, 0x00f0f0f0);
}
#elif(AP_SHOW_TEST_ENABLE == DVD2PANNEL_BIC)
void vPmxDvd2PanelPlayCb(void)
{ 
  IO_WRITE32(IO_BASE, 0x0094, 0x00000001);
  IO_WRITE32(IO_BASE, 0x0094, 0x00000003);

  IO_WRITE32(IO_BASE, 0x8300, 0x83600000);
  IO_MASK(IO_BASE, 0x8304, (0x1<<20), (0x1<<20));
  IO_MASK(IO_BASE, 0x8304, (0xffff<<0), (0x2751<<0));
  //Emulation use
  //IO_WRITE32(IO_BASE, 0x3fe194, 0);

  //Set TCON
  vPmxRegSet(_ArVdoPlaySetting[6].pu4RegSetting, _ArVdoPlaySetting[6].size);
  //Set scl
  vPmxRegSet(_ArVdoPlaySetting[7].pu4RegSetting, _ArVdoPlaySetting[7].size);
  //Mix select Fmt 800x480 Timming
  IO_WRITE32(IO_BASE, 0x1f000, 0x00432182);

  //reset DVD
  AP_WRITE_PT11032(0x00AC, 0x00000043);
  AP_WRITE_PT11032(0x00C8, 0x00000043);
  AP_WRITE_PT11032(0x0094, 0x00000001);
  AP_WRITE_PT11032(0x0094, 0x00000003);
  
  //AP access DVD register init
  AP_access_pt110_reg_init();

  //Set DVD Fmt
  vPmxDvdRegSet(_ArVdoPlaySetting[8].pu4RegSetting, _ArVdoPlaySetting[8].size);
}
#endif

void AP_Show(UINT32 cpu)
{
  vSimTraceStr("AP Show Start");

	#if  (AP_SHOW_TEST_ENABLE == PANEL_COLORBAR)
	  Panel_Show_PinSetting();
	  Panel_Show_SourceClockSetting();
	  Panel_Show_SetClock();
	  vSimTraceStr("vPmxFPlayColorBar Entry");\
	  vPmxFPlayColorBar();
	  vSimTraceStr("vPmxFPlayColorBar Exit");
	#elif(AP_SHOW_TEST_ENABLE == PANEL_PICTURE)
	  Panel_Show_PinSetting();
	  Panel_Show_SourceClockSetting();
	  Panel_Show_SetClock();
	  vSimTraceStr("vPmxFPlayPicture Entry");\
	  vPmxFPlayPicture();
	  vSimTraceStr("vPmxFPlayPicture Exit");
	#elif(AP_SHOW_TEST_ENABLE == TVE_COLORBAR)
	  TVE_Show_SetClock();
	  vSimTraceStr("vPmxRPlayColorBar Entry");\
	  vPmxRPlayColorBar();
	  vSimTraceStr("vPmxRPlayColorBar Exit");
	#elif(AP_SHOW_TEST_ENABLE == TVE_PICTURE)
	  TVE_Show_SetClock();
	  vSimTraceStr("vPmxRPlayPicture Entry");\
	  vPmxRPlayPicture();
	  vSimTraceStr("vPmxRPlayPicture Exit");
	#elif(AP_SHOW_TEST_ENABLE == TVE_FMT_BIC)
	  TVE_Show_SetClock();
	  vSimTraceStr("vPmxRPlayBIC Entry");
	  vPmxRPlayBuildinColor();
	  vSimTraceStr("vPmxRPlayBIC Exit");
	#elif(AP_SHOW_TEST_ENABLE == DVD2PANNEL_BIC)
	  vSimTraceStr("vPmxDVDPlayCBviaPanel Entry");
	  Panel_Show_PinSetting();
	  Panel_Show_SourceClockSetting();
	  Panel_Show_SetClock();
	  vPmxDvd2PanelPlayCb();
	  vSimTraceStr("vPmxDVDPlayCBviaPanel Exit");
	#endif

  vSimTraceStr("AP Show End");
}

#endif

#if(MT3363_AP_SHOW_TEST_ENABLE)


void AP_Show_BAR_OR_PIC(UINT32 cpu)
{

  vSimTraceStr("AP Show Start");
	
		IO_WRITE32(IO_BASE, 0x000000b4 ,0xffffffff );// rst enable
		IO_WRITE32(IO_BASE, 0x000000d0 ,0xffffffff );// rst enable
		//IO_WRITE32(IO_BASE, 0x0000005c ,0x00400040 );// [6]: 1 -- select dclk output
		
		IO_MASK(IO_BASE,0x5c,(0x1<<22),(0x1<<22));
		IO_MASK(IO_BASE,0x94,(0x1<<6),(0x1<<6));
		IO_MASK(IO_BASE,0x5c,(0xf<<6),(0xf<<6));
		IO_MASK(IO_BASE,0x298,(0xf<<28),(0xf<<28));
		
		
		IO_MASK(IO_BASE,0xD8,(0x1 << 19),(0x1 << 19));

		                                           
		//------------------------------------------);
		// pwm begin                                );
		//------------------------------------------);
				
		#ifdef config_TARGET_FPGA
        //Emulation usePWM_NEED_SUPPORT
        IO_WRITE32(IO_BASE, 0x00032200 ,0x06503205 );
		IO_WRITE32(IO_BASE, 0x00032218 ,0x00000003 );
		IO_WRITE32(IO_BASE, 0x00032204 ,0x0CB02205 );
		IO_WRITE32(IO_BASE, 0x00032218 ,0x0000000F );
        #endif                                           
		//------------------------------------------);
		//   Display Pin Setting Start              );
		//------------------------------------------);
		//IO_WRITE32(IO_BASE, 0x00000094 ,0x00000040 );   // 0x1<<6
		//IO_WRITE32(IO_BASE, 0x0000005c ,0x004003c0 );   // 0xf<<6
		//IO_WRITE32(IO_BASE, 0x00000298 ,0xf0000000 );   // |0xf<<28
		                                           
        //Clock 27 MHz 		                                           
		IO_WRITE32(IO_BASE, 0x00000014 ,0x00000000 );// fpd_ap_sel[2:0]   //27 Mhz		                                           
		//Clk 30 MHz from DDDS 180MHz with 6 div           
		//IO_WRITE32(IO_BASE, 0x00000014 ,0x00600000); // fpd_ap_sel[2:0]   //180MHz/6=30MHz
		
		
		//------------------------------------------);
		//FPD TCON begin                            );
		//------------------------------------------);
			                                           
		//IO_WRITE32(IO_BASE, 0x000a4700 ,0x07180700);                                           
		IO_WRITE32(IO_BASE, 0x000a4700 ,0x03180700 );
		                                           
		                                           
		IO_WRITE32(IO_BASE, 0x000a4704 ,0x31ffffff );
		IO_WRITE32(IO_BASE, 0x000a4724 ,0x2b000000 );
		                                           
		IO_WRITE32(IO_BASE, 0x000a4804 ,0x0104017e );
		IO_WRITE32(IO_BASE, 0x000a4820 ,0x28000001 );
		IO_WRITE32(IO_BASE, 0x000a4824 ,0x001f715f );
		IO_WRITE32(IO_BASE, 0x000a4830 ,0x0006800e );
		IO_WRITE32(IO_BASE, 0x000a4834 ,0x00000001 );
		IO_WRITE32(IO_BASE, 0x000a48e0 ,0x0000247d );// ck sel & fpd En 
		IO_WRITE32(IO_BASE, 0x000a48e4 ,0x0005f51a );
		IO_WRITE32(IO_BASE, 0x000a48f0 ,0x00000002 );
		                                           
		//VDO                                           
		IO_WRITE32(IO_BASE, 0x00042100 ,0x00080a00 );// Y buf1
		IO_WRITE32(IO_BASE, 0x00042104 ,0x00100a00 );// C buf1
		IO_WRITE32(IO_BASE, 0x00042108 ,0x00080a00 );// Y buf2
		IO_WRITE32(IO_BASE, 0x0004210c ,0x00100a00 );// C buf2
		                                           
		IO_WRITE32(IO_BASE, 0x00042110 ,0x01e0c864 );// height & DW per line SD mode & Block per line
		                                           
		IO_WRITE32(IO_BASE, 0x00042114 ,0x00000080 );// V scl ratio = 128/VSCL(i); 256/VSCL(p);
		IO_WRITE32(IO_BASE, 0x0004211c ,0x0000c001 );// out combine buf1_buf2 & VDO En
		IO_WRITE32(IO_BASE, 0x00042130 ,0x00000003 );// filter using whole frame
		IO_WRITE32(IO_BASE, 0x00042134 ,0x14481414 );// Fetch control
		                                           
		IO_WRITE32(IO_BASE, 0x000421e0 ,0x010000c8 );// DW per line when HD mode(HD Mode);
		                                           
		                                           
		//FMT                                       
		IO_WRITE32(IO_BASE, 0x00042094 ,0x1414ec20 );// HD mode select (HD Mode);
		IO_WRITE32(IO_BASE, 0x0004209c ,0x00000320 );// pixel number per line
		                                           
		IO_WRITE32(IO_BASE, 0x000420a0 ,0x00650385 );// H active area
		                                           
		IO_WRITE32(IO_BASE, 0x000420a4 ,0x002c020b );// V Odd active area
		IO_WRITE32(IO_BASE, 0x000420a8 ,0x002c020b );// V Even active area
		                                           
		IO_WRITE32(IO_BASE, 0x000420b0 ,0x01000001 );// H scaler 1:1
		IO_WRITE32(IO_BASE, 0x000420b4 ,0x0080f010 );// build in color blue
		IO_WRITE32(IO_BASE, 0x000420b8 ,0x00808010 );// back ground color
		IO_WRITE32(IO_BASE, 0x000420d0 ,0x806f038E );// CCIR H active area
		                                           
		IO_WRITE32(IO_BASE, 0x000420d4 ,0x13b9020d );// H Total & VTotal
		                                           
		IO_WRITE32(IO_BASE, 0x000420e0 ,0x002c020d );// CCIR V Odd
		IO_WRITE32(IO_BASE, 0x000420f0 ,0x002c020d );// CCIR V Even
		                                                                                   
		//IO_WRITE32(IO_BASE, 0xf00420f4 ,0x00000001); // fmt colorbar	                                         
		IO_WRITE32(IO_BASE, 0x000420f4 ,0x00000000 );// fmt colorbar
		//fpd colorbar                                           
		#if(MT3363_AP_SHOW_FPD_COLORBAR_TEST_ENABLE)
		IO_WRITE32(IO_BASE, 0x000a4700 ,0x07180700);                                           
		#endif
                //fmt colorbar
		#if(MT3363_AP_SHOW_FMT_COLORBAR_TEST_ENABLE)
				IO_WRITE32(IO_BASE, 0x000420f4 ,0x00000001); // fmt colorbar
		#endif         
		                                     
		//VDOP_FMT Reset                            
		IO_WRITE32(IO_BASE, 0x000420ac ,0x00000403 );// FMT En & Reset
		IO_WRITE32(IO_BASE, 0x000420ac ,0x00000003 );//
		IO_WRITE32(IO_BASE, 0x0004213c ,0x000000ff );// VDO Reset
		IO_WRITE32(IO_BASE, 0x0004213c ,0x00000000 );//
		                                           
		IO_WRITE32(IO_BASE, 0x0001f000 ,0x40432103 );//
                                        
		
		//if &BRING_PICTURE==0x1
		//(
		//D.LOAD.Binary Img\800_480_PANEL_PICTURE_Y.bin 0x80202800
		//D.LOAD.Binary Img\800_480_PANEL_PICTURE_C.bin 0x80402800
		//);
		//else
		//(
		//D.LOAD.Binary Img\800_480_y.bin 0x80202800
		//D.LOAD.Binary Img\800_480_c.bin 0x80402800
		//);

  vSimTraceStr("AP Show End");
}

DECLARE_TEST_ITEM("AP_Show_Test",AP_Show_BAR_OR_PIC)


#endif


