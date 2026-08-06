#include"top_layer.h"
#include"msdc_io.h"
#include"dbg.h"

#define  SOURCE_FREQ_DOWN 0
#if MSDC_DEBUG_ENABLE
extern int msdc_sw_reset_debug;
#endif

void msdc_clock_choose(struct msdc_host *host,unsigned int sclk,unsigned int hclk)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;
	unsigned int test[6];
	if (port == 0) {
		switch (sclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 0); //27M 
				break;
			case 1:
				/* 400M source freq down */

				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PWD	=	%d\r\n", test[5]);
				
#if SOURCE_FREQ_DOWN
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, 27);
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, 2);
#endif				
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PWD	=	%d\r\n", test[5]);
				
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 1);//400M
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 2);//202.5M
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 3);//162
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 4);//120M
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 5);//108M
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 6);//54M
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 7);//48M
				break;
			case 8:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 8);//200M
				break;
			case 9:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 9);//147M
				break;
			case 10:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 10);//324M
				break;
			case 11:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 11);//135M
				break;
			case 12:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 12);//294M
				break;
			case 13:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 13);//100M
				break;
			case 14:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 14);// 27MHZ/2=13.5MHZ (CLK27M_D2)
				break;
			case 15:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD01_AP_SEL, 15);// 27MHZ/4=6.65MHZ (CLK27M_D4)
				break;				
			default:
				break;
				
		}
		
		switch (hclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 0);//CLK27M=27M
				break;
			case 1:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 1);//APLL2_D3=98M
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 2);//USBPLL_D6=120M		
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 3);//SYSPLL_D9=135M
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 4);//USBPLL_D8=162M
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 5);//SYSPLL_D12=270M
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 6);//USBPLL_D10=200M
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD00_AP_SEL, 7);//SYSPLL_D18=147M
				break;
			default:
				break;
				
		}

	} else if (port == 1) {
		switch (sclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,0);//27M
				break;
			case 1:
				/* 400M source freq down */

				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PWD	=	%d\r\n", test[5]);

#if SOURCE_FREQ_DOWN
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, 27);
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, 2);
#endif

				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PWD	=	%d\r\n", test[5]);
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,1);//200
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,2);//202.5
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,3);//162
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,4);//120
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,5);//108
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,6);//54
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,7);//48
				break;
			case 8:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,8);//200
				break;
			case 9:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,9);//147
				break;
			case 10:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,10);//74
				break;
			case 11:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,11);//135
				break;
			case 12:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,12);//81
				break;
			case 13:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0,SD11_AP_SEL,13);//100
				break;
			case 14:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD11_AP_SEL, 14);// 27MHZ/2=13.5MHZ (CLK27M_D2)
				break;
			case 15:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD11_AP_SEL, 15);// 27MHZ/4=6.65MHZ (CLK27M_D4)
				break;

			default:
				break;
				
		}
		
		switch (hclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 0);//27
				break;
			case 1:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 1);//98
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 2);//80
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 3);//72
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 4);//60
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 5);//54
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 6);//48
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD10_AP_SEL, 7);//36
				break;
			default:
				break;
				
		}

	} else if (port == 2) {
		switch (sclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 0);//27
				break;
			case 1:
				/* 400M source freq down */

				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PWD	=	%d\r\n", test[5]);
				
#if SOURCE_FREQ_DOWN
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, 27);
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, 2);
#endif				
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PWD	=	%d\r\n", test[5]);
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 1);//200
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 2);//202.5
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 3);//162
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 4);//120
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 5);//108
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 6);//54
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 7);//48
				break;
			case 8:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL,8);//200
				break;
			case 9:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 9);//147
				break;
			case 10:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 10);//74
				break;
			case 11:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 11);//135
				break;
			case 12:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 12);//81
				break;
			case 13:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1,SD21_AP_SEL,13);//100
				break;
			case 14:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 14);// 27MHZ/2=13.5MHZ (CLK27M_D2)
				break;
			case 15:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD21_AP_SEL, 15);// 27MHZ/4=6.65MHZ (CLK27M_D4)
				break;

			default:
				break;
				
		}
		
		switch(hclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 0);
				break;
			case 1:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 1);	
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 2);				
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 3);				
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 4);				
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 5);				
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 6);				
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE0, SD20_AP_SEL, 7);				
				break;

			default:
				break;	
		}
	}else if (port == 3) {
		switch(sclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 0);// 27MHZ (CLK27MHZ)
				break;
			case 1:
				/* 400M source freq down */

				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	pre RG_MSDCPLL_PWD	=	%d\r\n", test[5]);

#if SOURCE_FREQ_DOWN
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, 27);
				MSDC_SET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, 2);
#endif				
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBSEL, test[0]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_CKTROL, test[1]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_POSDIV, test[2]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PROEDIV, test[3]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_FBDIV, test[4]);
				MSDC_GET_FIELD(REG_PLLGP_CFG28, RG_MSDCPLL_PWD, test[5]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBSEL	=	%d\r\n", test[0]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_CKTROL	=	%d\r\n", test[1]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_POSDIV	=	%d\r\n", test[2]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PROEDIV	=	%d\r\n", test[3]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_FBDIV	=	%d\r\n", test[4]);
				MSDC_LOG(INIT, "[TST]	cur RG_MSDCPLL_PWD	=	%d\r\n", test[5]);	
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 1);// 405MHZ/2=202.5MHZ  (MSDCPLL_D2)
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 2);//(202.5)     (ARMPLL2_D2)
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 3);// 648MHZ/4=162MHZ  (SYSPLL_D4)
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 4);// 480MHZ/4=120MHZ  (SUBPLL_D4)
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 5);// 648MHZ/6=108MHZ  (SYSPLL_D6)
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 6);// 648MHZ/12=54MHZ  (SYSPLL_D12)
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 7);// 480MHZ/10=48MHZ  (USBPLL_D10)
				break;
			case 8:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL,8);//(189) 400MHZ/2=200MHZ  (DMPLL_D2)
				break;
			case 9:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 9);// 294MHZ/2=147MHZ (APLL2_D2)
				break;
			case 10:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 10);//(73.5) 
				break;
			case 11:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 11);// 270MHZ/2=135MHZ  (APLL1_D2)
				break;
			case 12:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 12);//(81) 
				break;
			case 13:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 13);// 405MHZ/4=101MHZ (MSDCPLL_D4)
				break;
			case 14:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 14);// 27MHZ/2=13.5MHZ (CLK27M_D2)
				break;
			case 15:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE1, SD31_AP_SEL, 15);// 27MHZ/4=6.65MHZ (CLK27M_D4)
				break;

			default:
				break;
				
		}
		
		switch(hclk) {
			case 0:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 0);
				break;
			case 1:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 1);	
				break;	
			case 2:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 2);				
				break;
			case 3:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 3);				
				break;
			case 4:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 4);				
				break;
			case 5:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 5);				
				break;
			case 6:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 6);				
				break;
			case 7:
				MSDC_SET_FIELD(MSDC_CLK_CHOOSE3, SD30_AP_SEL, 7);				
				break;

			default:
				break;	
		}
	}
	else {
		MSDC_LOG(INIT,  "port set wrong\n");
	}
}

void msdc_dma_channel_sel(struct msdc_host *host,unsigned int channel_sel)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;

	unsigned int cur_dma_cha_ctrl;
	unsigned int cur_misc_ctrl0, cur_misc_ctrl1, cur_misc_ctrl2, cur_misc_ctrl3;
	MSDC_GET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, cur_dma_cha_ctrl);
	MSDC_GET_FIELD(misc_ctrl, MSDC0_MISC_CTRL, cur_misc_ctrl0);
	MSDC_GET_FIELD(misc_ctrl, MSDC1_MISC_CTRL, cur_misc_ctrl1);
	MSDC_GET_FIELD(misc_ctrl, MSDC2_MISC_CTRL, cur_misc_ctrl2);
	MSDC_GET_FIELD(misc_ctrl, MSDC3_MISC_CTRL, cur_misc_ctrl3);
	MSDC_LOG(INIT,  "[SD%d] pre channel ctrl bit = %d\r\n", port, cur_dma_cha_ctrl);
	MSDC_LOG(INIT,  "[SD%d] MSDC0 misc = %d MSDC1 misc = %d MSDC2 misc = %d MSDC3 misc = %d\r\n",
		port, cur_misc_ctrl0, cur_misc_ctrl1, cur_misc_ctrl2, cur_misc_ctrl3);
	switch (port) {
		case 0:
			if (channel_sel == MSDC_DMA_AGENT5) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC0_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT6) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 1);
				MSDC_SET_FIELD(misc_ctrl, MSDC0_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT16) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC0_MISC_CTRL, 1);
			}
			break;
		case 1:
			if (channel_sel == MSDC_DMA_AGENT5) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC1_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT6) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 1);
				MSDC_SET_FIELD(misc_ctrl, MSDC1_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT16) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC1_MISC_CTRL, 1);
			}
			break;
		case 2:
			if (channel_sel == MSDC_DMA_AGENT5) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC2_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT6) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 1);
				MSDC_SET_FIELD(misc_ctrl, MSDC2_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT16) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC2_MISC_CTRL, 1);
			}
			break;
		case 3:
			if (channel_sel == MSDC_DMA_AGENT5) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC3_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT6) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 1);
				MSDC_SET_FIELD(misc_ctrl, MSDC3_MISC_CTRL, 0);
			} else if (channel_sel == MSDC_DMA_AGENT16) {
				MSDC_SET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, 0);
				MSDC_SET_FIELD(misc_ctrl, MSDC3_MISC_CTRL, 1);
			}
			break;
		default:
			break;
	}
	MSDC_GET_FIELD(dma_channel_ctrl, MSDC_CHA_SEL_CTRL, cur_dma_cha_ctrl);
	MSDC_GET_FIELD(misc_ctrl, MSDC0_MISC_CTRL, cur_misc_ctrl0);
	MSDC_GET_FIELD(misc_ctrl, MSDC1_MISC_CTRL, cur_misc_ctrl1);
	MSDC_GET_FIELD(misc_ctrl, MSDC2_MISC_CTRL, cur_misc_ctrl2);
	MSDC_GET_FIELD(misc_ctrl, MSDC3_MISC_CTRL, cur_misc_ctrl3);
	MSDC_LOG(INIT,  "[SD%d] cur channel ctrl bit = %d\r\n", port, cur_dma_cha_ctrl);
	MSDC_LOG(INIT,  "[SD%d] MSDC0 misc = %d MSDC1 misc = %d MSDC2 misc = %d MSDC3 misc = %d\r\n",
		port, cur_misc_ctrl0, cur_misc_ctrl1, cur_misc_ctrl2, cur_misc_ctrl3);
}

void msdc_clock_enable(struct msdc_host *host,unsigned int enable)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;

	if (port == 0) {
		if (enable == 1)
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_0_CKEN, 1);
		else if (enable == 0)
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_0_CKEN, 0);
		else
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_0_CKEN, 1);

	} else if (port == 1) {
		if (enable == 1)
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_1_CKEN, 1);
		else if (enable==0)
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_1_CKEN, 0);
		else
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_1_CKEN, 1);
	} else if (port == 2) {
		if (enable == 1)
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_2_CKEN, 1);
		else if (enable == 0)
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_2_CKEN, 0);
		else
			MSDC_SET_FIELD(clkgate_cfg3, MSDC_2_CKEN, 1);

	} else if (port == 3) {
		if (enable == 1){
			MSDC_SET_FIELD(clkgate_cfg4, MSDC_3_CKEN, 1);
			MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST_EN, 1);
			MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST, 0);
			MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST, 1);
			}
		else if (enable == 0)
			MSDC_SET_FIELD(clkgate_cfg4, MSDC_3_CKEN, 0);
		else{
			MSDC_SET_FIELD(clkgate_cfg4, MSDC_3_CKEN, 1);
			/*MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST_EN, 1);
			MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST, 0);
			MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST, 1);*/
			}

	} else {
		MSDC_LOG(INIT,  "port set wrong\n");
	}
}

void msdc_sw_reset(struct msdc_host *host)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;
#if MSDC_DEBUG_ENABLE
	if(msdc_sw_reset_debug)
		return ;
#endif
	if (port == 0) {
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_0_PDRST, 1);
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_0_SWRST, 0);
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_0_SWRST, 1);
	} else if (port == 1) {
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_1_PDRST, 1);
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_1_SWRST, 0);
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_1_SWRST, 1);
	} else if (port == 2) {
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_2_PDRST, 1);
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_2_SWRST, 0);
		MSDC_SET_FIELD(sync_reset_cfg3, MSDC_2_SWRST, 1);
	} else if (port == 3) {
		MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST_EN, 1);
		MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST, 0);
		MSDC_SET_FIELD(pd_reset_en_cfg3, MSDC_3_SWRST, 1);
	} else
		MSDC_LOG(INIT,  "port set wrong\n");
}

void msdc_multip_function(struct msdc_host *host, unsigned int share_nand_pin)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;

/*MT3365 not use*/
#ifndef CONFIG_CHIP_VER_CURR_MT3365
	if (port == 0) {
		MSDC_SET_FIELD(misc_control, msdc0_gpio_mode_sel, 0);
		if (share_nand_pin == 0) {
			//port0 use for SD card
			MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd0_gpio_ctl_5_0, 0);
			MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd0_reset_gpio_ctl, 1);
			MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd2_gpio_ctl_9_0, 0x3FF);
		} else {
			//port0 use for emmc card
			MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd_8bit_reset_gpio_ctl, 1);
			MSDC_SET_FIELD(GPIOEN3, pad_emmc_reset_output_en, 1);
			MSDC_SET_FIELD(GPIOOUT3, pad_emmc_reset_output_value, 1);		
			MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd2_gpio_ctl_9_0, 0);
		}
	} else if (port == 1) {
		MSDC_SET_FIELD(misc_control, msdc1_gpio_mode_sel, 0);
		MSDC_SET_FIELD(pad_msdc_cfg36, pad_msdc_cfg36_8_4, 0);//Port1's pin was used for port1
		MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd1_reset_gpio_ctl, 0);
		MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd1_gpio_ctl_5_0, 0);
	} else if (port == 2) {
		MSDC_SET_FIELD(misc_control, msdc2_gpio_mode_sel, 0);
		MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd2_reset_gpio_ctl, 0);
		MSDC_SET_FIELD(pad_msdc_cfg18, pad_sd2_gpio_ctl_5_0, 0);
	} else {
		//MSDC_LOG(INIT,  "port set wrong\n");
	}
#endif
}

#if 0
extern unsigned int clk_pad_driv;
extern unsigned int cmd_pad_driv;
extern unsigned int dat_pad_driv;
#else
unsigned int clk_pad_driv = 0x1d;
unsigned int cmd_pad_driv = 0x15;
unsigned int dat_pad_driv = 0x1d;
#endif
/*********clk-pull down ;	cmd-pull up; 	data-pull up***********/
void msdc_init_pad(struct msdc_host *host)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;

	if (port == 0) {
		//clock pad init
		MSDC_SET_FIELD(pad_msdc_cfg19, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg19, pad_r, host->hw->clk_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg19, pad_pupd, 1);	//pull-down  
		MSDC_SET_FIELD(pad_msdc_cfg19, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg19, pad_drv, host->hw->clk_drv);
		MSDC_SET_FIELD(pad_msdc_cfg19, pad_sr, host->hw->clk_sr);
		
		//cmd pad init
		MSDC_SET_FIELD(pad_msdc_cfg20, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg20, pad_r, host->hw->cmd_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg20, pad_pupd, 0);	//pull-up  
		MSDC_SET_FIELD(pad_msdc_cfg20, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg20, pad_drv, host->hw->cmd_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg20, pad_sr, host->hw->cmd_sr);		
		
		//data pad init
		//data0
		MSDC_SET_FIELD(pad_msdc_cfg21, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg21, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg21, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg21, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg21, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg21, pad_sr, host->hw->dat_sr);
		//data1
		MSDC_SET_FIELD(pad_msdc_cfg22, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg22, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg22, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg22, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg22, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg22, pad_sr, host->hw->dat_sr);
		//data2
		MSDC_SET_FIELD(pad_msdc_cfg23, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg23, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg23, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg23, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg23, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg23, pad_sr, host->hw->dat_sr);
		//data3
		MSDC_SET_FIELD(pad_msdc_cfg24, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg24, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg24, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg24, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg24, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg24, pad_sr, host->hw->dat_sr);

		//data4
		MSDC_SET_FIELD(pad_msdc_cfg25, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg25, pad_r, host->hw->dat_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg25, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg25, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg25, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg25, pad_sr, host->hw->dat_sr);

		//data5
		MSDC_SET_FIELD(pad_msdc_cfg26, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg26, pad_r, host->hw->dat_res);	
		MSDC_SET_FIELD(pad_msdc_cfg26, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg26, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg26, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg26, pad_sr, host->hw->dat_sr);

		//data6
		MSDC_SET_FIELD(pad_msdc_cfg27, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg27, pad_r, host->hw->dat_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg27, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg27, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg27, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg27, pad_sr, host->hw->dat_sr);

		//data7
		MSDC_SET_FIELD(pad_msdc_cfg28, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg28, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg28, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg28, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg28, pad_drv,host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg28, pad_sr, host->hw->dat_sr);
		
		//rst pad init 
		MSDC_SET_FIELD(pad_msdc_cfg30, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg30, pad_r, 1);   
		MSDC_SET_FIELD(pad_msdc_cfg30, pad_pupd, 1);
		MSDC_SET_FIELD(pad_msdc_cfg30, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg30, pad_drv, 0x1d);
		MSDC_SET_FIELD(pad_msdc_cfg30, pad_sr, 0x0);

		//ds pad init 
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_r, host->hw->ds_res);   
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_pupd, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_drv, host->hw->ds_drv);
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_sr, host->hw->ds_sr);
	} else if (port == 1) {
		//clock pad init
		MSDC_SET_FIELD(pad_msdc_cfg6, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg6, pad_r, host->hw->clk_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg6, pad_pupd, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg6, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg6, pad_drv, host->hw->clk_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg6, pad_sr, host->hw->clk_sr);	  
		
		//cmd pad init
		MSDC_SET_FIELD(pad_msdc_cfg7, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg7, pad_r, host->hw->cmd_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg7, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg7, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg7, pad_drv, host->hw->cmd_drv);
		MSDC_SET_FIELD(pad_msdc_cfg7, pad_sr, host->hw->cmd_sr);
		
		//data pad init
		//data0
		MSDC_SET_FIELD(pad_msdc_cfg8, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg8, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg8, pad_pupd, 0);
		MSDC_SET_FIELD(pad_msdc_cfg8, pad_ies, 1);
		MSDC_SET_FIELD(pad_msdc_cfg8, pad_drv, host->hw->dat_drv);
		MSDC_SET_FIELD(pad_msdc_cfg8, pad_sr, host->hw->dat_sr);//0x5
		//data1
		MSDC_SET_FIELD(pad_msdc_cfg9, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg9, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg9, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg9, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg9, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg9, pad_sr, host->hw->dat_sr);
		//data2
		MSDC_SET_FIELD(pad_msdc_cfg10, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg10, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg10, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg10, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg10, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg10, pad_sr, host->hw->dat_sr);	
		//data3
		MSDC_SET_FIELD(pad_msdc_cfg11, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg11, pad_r, host->hw->dat_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg11, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg11, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg11, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg11, pad_sr, host->hw->dat_sr);	
#if 0
		//rst pad init 
		MSDC_SET_FIELD(pad_msdc_cfg31,pad_smit,1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg31,pad_r,1);   //50K 
		MSDC_SET_FIELD(pad_msdc_cfg31,pad_pupd,1);	  
		MSDC_SET_FIELD(pad_msdc_cfg31,pad_ies,1);	  
		MSDC_SET_FIELD(pad_msdc_cfg31,pad_drv,0x4);	
		MSDC_SET_FIELD(pad_msdc_cfg31,pad_sr,0xf);	
#endif

	} else if (port == 2) {
		//clock pad init
		MSDC_SET_FIELD(pad_msdc_cfg12, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg12, pad_r, host->hw->clk_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg12, pad_pupd, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg12, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg12, pad_drv, host->hw->clk_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg12, pad_sr, host->hw->clk_sr);//0x0	  
		
		//cmd pad init
		MSDC_SET_FIELD(pad_msdc_cfg13, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg13, pad_r, host->hw->cmd_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg13, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg13, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg13, pad_drv, host->hw->cmd_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg13, pad_sr, host->hw->cmd_sr);//0x5		
		
		//data pad init
		//data0
		MSDC_SET_FIELD(pad_msdc_cfg14, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg14, pad_r, host->hw->dat_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg14, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg14, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg14, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg14, pad_sr, host->hw->dat_sr);	   
		//data1
		MSDC_SET_FIELD(pad_msdc_cfg15, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg15, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg15, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg15, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg15, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg15, pad_sr, host->hw->dat_sr);
		//data2
		MSDC_SET_FIELD(pad_msdc_cfg16, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg16, pad_r, host->hw->dat_res);	   
		MSDC_SET_FIELD(pad_msdc_cfg16, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg16, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg16, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg16, pad_sr, host->hw->dat_sr);	
		//data3
		MSDC_SET_FIELD(pad_msdc_cfg17, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg17, pad_r, host->hw->dat_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg17, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg17, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg17, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg17, pad_sr, host->hw->dat_sr);	
#if 0
		//rst pad init 
		MSDC_SET_FIELD(pad_msdc_cfg32,pad_smit,1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg32,pad_r,1);   //50K 
		MSDC_SET_FIELD(pad_msdc_cfg32,pad_pupd,1);	  
		MSDC_SET_FIELD(pad_msdc_cfg32,pad_ies,1);	  
		MSDC_SET_FIELD(pad_msdc_cfg32,pad_drv,0x1f);	
		MSDC_SET_FIELD(pad_msdc_cfg32,pad_sr,0xf);	
#endif

	} else if (port == 3) {
		//clock pad init
		MSDC_SET_FIELD(pad_msdc_cfg0, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg0, pad_r, host->hw->clk_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg0, pad_pupd, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg0, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg0, pad_drv, host->hw->clk_drv);
		MSDC_SET_FIELD(pad_msdc_cfg0, pad_sr, host->hw->clk_sr);	  
		
		//cmd pad init
		MSDC_SET_FIELD(pad_msdc_cfg1, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg1, pad_r, host->hw->cmd_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg1, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg1, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg1, pad_drv, host->hw->cmd_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg1, pad_sr, host->hw->cmd_sr);	//0x5	
		
		//data pad init
		//data0
		MSDC_SET_FIELD(pad_msdc_cfg2, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg2, pad_r, host->hw->dat_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg2, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg2, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg2, pad_drv, host->hw->dat_drv);	//0x1a timing measurement
		MSDC_SET_FIELD(pad_msdc_cfg2, pad_sr, host->hw->dat_sr);	   
		//data1
		MSDC_SET_FIELD(pad_msdc_cfg3, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg3, pad_r, host->hw->dat_res);	 
		MSDC_SET_FIELD(pad_msdc_cfg3, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg3, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg3, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg3, pad_sr, host->hw->dat_sr);
		//data2
		MSDC_SET_FIELD(pad_msdc_cfg4, pad_smit,1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg4, pad_r,host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg4, pad_pupd,0);	  
		MSDC_SET_FIELD(pad_msdc_cfg4, pad_ies,1);	  
		MSDC_SET_FIELD(pad_msdc_cfg4, pad_drv,host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg4, pad_sr,host->hw->dat_sr);	
		//data3
		MSDC_SET_FIELD(pad_msdc_cfg5, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg5, pad_r, host->hw->dat_res);	  
		MSDC_SET_FIELD(pad_msdc_cfg5, pad_pupd, 0);	  
		MSDC_SET_FIELD(pad_msdc_cfg5, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg5, pad_drv, host->hw->dat_drv);	
		MSDC_SET_FIELD(pad_msdc_cfg5, pad_sr, host->hw->dat_sr);	
#if 0	
		//rst pad init 
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_smit, 1); //enable smit
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_r, 1);   //50K 
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_pupd, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_ies, 1);	  
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_drv, 0x1f);	
		MSDC_SET_FIELD(pad_msdc_cfg32, pad_sr, 0xf);	
#endif

	} else {
		MSDC_LOG(INIT, "port set wrong\n");
	}
}


void MSDC_Init_cmd_dat_pull_up_down(struct msdc_host *host, unsigned int mode)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;

	switch (port) {
	case 0:
		if (mode == MSDC_PIN_PULL_NONE) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg20, pad_r, 0);	  //Hi-Z 
			MSDC_SET_FIELD(pad_msdc_cfg20, pad_pupd, 0);	  
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg21, pad_r, 0);	  //Hi-Z  
			MSDC_SET_FIELD(pad_msdc_cfg21, pad_pupd, 0);
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg22, pad_r, 0);	  //Hi-Z  
			MSDC_SET_FIELD(pad_msdc_cfg22, pad_pupd, 0);
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg23, pad_r, 0);	  //Hi-Z  
			MSDC_SET_FIELD(pad_msdc_cfg23, pad_pupd, 0);
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg24, pad_r, 0);	  //Hi-Z 
			MSDC_SET_FIELD(pad_msdc_cfg24, pad_pupd, 0);
			//data4
			MSDC_SET_FIELD(pad_msdc_cfg25, pad_r, 0);	  //Hi-Z  
			MSDC_SET_FIELD(pad_msdc_cfg25, pad_pupd, 0);
			//data5
			MSDC_SET_FIELD(pad_msdc_cfg26, pad_r, 0);	  //Hi-Z  
			MSDC_SET_FIELD(pad_msdc_cfg26, pad_pupd, 0);
			//data6
			MSDC_SET_FIELD(pad_msdc_cfg27, pad_r, 0);	  //Hi-Z   
			MSDC_SET_FIELD(pad_msdc_cfg27, pad_pupd, 0);
			//data7
			MSDC_SET_FIELD(pad_msdc_cfg28, pad_r, 0);	  //Hi-Z   
			MSDC_SET_FIELD(pad_msdc_cfg28, pad_pupd, 0);	  
			
		} else if (mode == MSDC_PIN_PULL_DOWN) {

		} else if (mode == MSDC_PIN_PULL_UP) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg20, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg20, pad_pupd, 0);	  
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg21, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg21, pad_pupd, 0);
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg22, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg22, pad_pupd, 0);
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg23, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg23, pad_pupd, 0);
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg24, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg24, pad_pupd, 0);
			//data4
			MSDC_SET_FIELD(pad_msdc_cfg25, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg25, pad_pupd, 0);
			//data5
			MSDC_SET_FIELD(pad_msdc_cfg26, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg26, pad_pupd, 0);
			//data6
			MSDC_SET_FIELD(pad_msdc_cfg27, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg27, pad_pupd, 0);
			//data7
			MSDC_SET_FIELD(pad_msdc_cfg28, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg28, pad_pupd, 0);	
		}
		break;
	case 1:
		if (mode == MSDC_PIN_PULL_NONE) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg7, pad_r, 0);	  //Hi-Z    
			MSDC_SET_FIELD(pad_msdc_cfg7, pad_pupd, 0);
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg8, pad_r, 0);	  //Hi-Z    
			MSDC_SET_FIELD(pad_msdc_cfg8, pad_pupd, 0);
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg9, pad_r, 0);	  //Hi-Z    
			MSDC_SET_FIELD(pad_msdc_cfg9, pad_pupd, 0);	  
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg10, pad_r, 0);	  //Hi-Z     
			MSDC_SET_FIELD(pad_msdc_cfg10, pad_pupd, 0);	  
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg11, pad_r, 0);	  //Hi-Z     
			MSDC_SET_FIELD(pad_msdc_cfg11, pad_pupd, 0);
		} else if (mode == MSDC_PIN_PULL_DOWN) {

		} else if (mode == MSDC_PIN_PULL_UP) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg7, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg7, pad_pupd, 0);
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg8, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg8, pad_pupd, 0);
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg9, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg9, pad_pupd, 0);	  
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg10, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg10, pad_pupd, 0);	  
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg11, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg11, pad_pupd, 0);	  
		}
		break;
	case 2:
		if (mode == MSDC_PIN_PULL_NONE) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg13, pad_r, 0);	  //Hi-Z      
			MSDC_SET_FIELD(pad_msdc_cfg13, pad_pupd, 0);		
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg14, pad_r, 0);	  //Hi-Z      
			MSDC_SET_FIELD(pad_msdc_cfg14, pad_pupd, 0);	  
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg15, pad_r, 0);	  //Hi-Z      
			MSDC_SET_FIELD(pad_msdc_cfg15, pad_pupd, 0);	  
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg16, pad_r, 0);	  //Hi-Z      
			MSDC_SET_FIELD(pad_msdc_cfg16, pad_pupd, 0);	  
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg17, pad_r, 0);	  //Hi-Z      
			MSDC_SET_FIELD(pad_msdc_cfg17, pad_pupd, 0);	  	
		} else if (mode == MSDC_PIN_PULL_DOWN) {

		} else if (mode == MSDC_PIN_PULL_UP) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg13, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg13, pad_pupd, 0);		
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg14, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg14, pad_pupd, 0);	  
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg15, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg15, pad_pupd, 0);	  
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg16, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg16, pad_pupd, 0);	  
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg17, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg17, pad_pupd, 0);	  	
		}
		break;
	case 3:
		if (mode == MSDC_PIN_PULL_NONE) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg1, pad_r, 0);	  //Hi-Z   
			MSDC_SET_FIELD(pad_msdc_cfg1, pad_pupd, 0);	  
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg2, pad_r, 0);	  //Hi-Z   
			MSDC_SET_FIELD(pad_msdc_cfg2, pad_pupd, 0);	   
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg3, pad_r, 0);	  //Hi-Z   
			MSDC_SET_FIELD(pad_msdc_cfg3, pad_pupd, 0);	  
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg4, pad_r, 0);	  //Hi-Z   
			MSDC_SET_FIELD(pad_msdc_cfg4, pad_pupd,0);	  
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg5, pad_r, 0);	  //Hi-Z   
			MSDC_SET_FIELD(pad_msdc_cfg5, pad_pupd, 0);	  
		} else if (mode == MSDC_PIN_PULL_DOWN) {

		} else if (mode == MSDC_PIN_PULL_UP) {
			//cmd pad init
			MSDC_SET_FIELD(pad_msdc_cfg1, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg1, pad_pupd, 0);	  
			//data0
			MSDC_SET_FIELD(pad_msdc_cfg2, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg2, pad_pupd, 0);	   
			//data1
			MSDC_SET_FIELD(pad_msdc_cfg3, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg3, pad_pupd, 0);	  
			//data2
			MSDC_SET_FIELD(pad_msdc_cfg4, pad_r,1);	  		//50K 
			MSDC_SET_FIELD(pad_msdc_cfg4, pad_pupd,0);	  
			//data3
			MSDC_SET_FIELD(pad_msdc_cfg5, pad_r, 1);	  //50K 
			MSDC_SET_FIELD(pad_msdc_cfg5, pad_pupd, 0);	  
		}
		break;
	}
}

void msdc_dump_pad(struct msdc_host *host)
{
	u64 base = host->base_pin;
	unsigned int port=host->id;
	MSDC_LOG(INIT, "dandan dump the PAD params \n");
	MSDC_LOG(INIT, "register:0x%x = 0x%x\n",pad_msdc_cfg0,MSDC_READ32(pad_msdc_cfg0));
	MSDC_LOG(INIT, "register:0x%x = 0x%x\n",pad_msdc_cfg1,MSDC_READ32(pad_msdc_cfg1));
	MSDC_LOG(INIT, "register:0x%x = 0x%x\n",pad_msdc_cfg2,MSDC_READ32(pad_msdc_cfg2));
	MSDC_LOG(INIT, "register:0x%x = 0x%x\n",pad_msdc_cfg3,MSDC_READ32(pad_msdc_cfg3));
	MSDC_LOG(INIT, "register:0x%x = 0x%x\n",pad_msdc_cfg4,MSDC_READ32(pad_msdc_cfg4));

}

