/*! @file Musb_qmu.h
* $Workfile: Musb_qmu.h$
*
* @par Project: 
*    Mtk usb swip
*
* @par Description: 
*    Mtk DMAQ lib interface header file
*    Define DMAQ interface basic Flow
*
* @par Author_Name: 
*    tianhao.fei 
*
* @par Last_Changed: add comment for Doxygen
* $Author: songlin.chen $
* $Modtime: 2010-5-7$
* $Revision: #1 $
*
*/
#ifndef ATC_QUEUE_H
#define ATC_QUEUE_H

//#include <linux/dmapool.h>

//#include "../../usb_bltin/usb_bltin.h"

/// @brief DMA channel select for DMAQ
/// .
/// @param musb:   struct musb
/// @param channel: DMA channel selected
/// @param burstmode: DMA burst mode selected
extern void mtk_q_dma_select (
	MGC_LinuxCd *pThis, 			
	u8 channel, 					
	u8 burstmode				
);

/// @brief musb Enable Queue
/// .
/// @param musb:   struct musb
/// @param EP_Num: Endpoint number mapping Queue 
/// @param isRx: RxQ or TxQ
/// @param type: Transfer Type for the selected Queue
/// @param MaxP: Transfer Max Packet size for the selected Queue
/// @param interval: Transfer interval for periodical transfer, or NAK limit for buld transfer
/// @param target_ep: If Host mode, target Endpoint of Pipe
/// @param isZLP: if open ZLP feature
/// @param isCSCheck: if open checksum check
/// @param isEmptyCheck: if open empty notification
/// @return status.
extern int mtk_enable_q(
    MGC_LinuxCd *pThis,
    u8 address, 
	u8 EP_Num, 					
	u8 isRx, 						
	u8 type, 						
	u16 MaxP, 					
	u8 interval, 					
	u8 target_ep, 				
	u8 isZLP, 					
	u8 isCSCheck, 				
	u8 isEmptyCheck,
	u8 hb_mult
);

/// @brief musb disable Queue
/// .
/// @param musb:   struct musb
/// @param EP_Num: Endpoint number mapping Queue  
/// @param isRx: RxQ or TxQ
/// @return status.
extern void mtk_disable_q(
	MGC_LinuxCd *pThis, 			
	u8 EP_Num, 					
	u8 isRx						
);
/// @brief musb disable Queue
/// .
/// @param musb:   struct musb
/// @param EP_Num: Endpoint number mapping Queue  
/// @param isRx: RxQ or TxQ
/// @return status.
extern void mtk_disable_q_all(MGC_LinuxCd *pThis);
#if 0
/// @brief Insert a urb task into Queue
/// .
/// @param urb: urb task   
/// @param qh: usb_host_endpoint.hcpriv for scheduled endpoints
/// @return status.
extern int mtk_start_urb(
	struct urb *urb,
	struct musb_qh *qh
);

/// @brief Insert a usb_request task into Queue
/// .
/// @param musb:   struct musb
/// @param request: request task  
/// @return status.
extern int mtk_start_request(
	MGC_LinuxCd *pThis, 			
	struct musb_request *req		 
);


/// @brief remove a urb task into Queue
/// .
/// @param urb: urb task   
/// @return status.
extern int mtk_q_remove_urb(MGC_LinuxCd *pThis,struct urb* urb,u8 EP_Num,bool isRx,u8* irq_cb);

/// @brief Remove a usb_request task into Queue
/// .
/// @param request: request task   
/// @return status.
extern int mtk_remove_request(
	struct usb_request request		
);


/// @brief musb clean up the selected Queue
/// .
/// @param musb:   struct musb
/// @param EP_Num: Endpoint number mapping Queue 
/// @param isRx: RxQ or TxQ
/// @return status.
extern int mtk_cleanup_q(
	MGC_LinuxCd *pThis, 			
	u8 EP_Num, 					
	u8 isRx						
);
#endif

/// @brief musb stop the selected Queue
/// .
/// @param musb:   struct musb
/// @param EP_Num: Endpoint number mapping Queue
/// @param isRx: RxQ or TxQ
/// @return status.
extern void mtk_stop_q(
    MGC_LinuxCd *pThis,
	u8 EP_Num, 					
	u8 isRx						
);

/// @brief musb restart the selected Queue
/// .
/// @param musb:   struct musb
/// @param EP_Num: Endpoint number mapping Queue
/// @param isRx: RxQ or TxQ
/// @return status.
extern int mtk_restart_q(
	MGC_LinuxCd *pThis, 			
	u8 EP_Num, 					  
	u8 isRx						
);

/// @brief musb DMAQ interrupt handling
/// .
/// @param musb:   struct musb
/// @return irq status.   
extern irqreturn_t mtk_q_interrupt(MGC_LinuxCd *pThis);

/// @brief musb host mode DMAQ Rx task done handling
/// .
/// @param musb:   struct musb
/// @epnum: endpoint number
void mtk_q_host_rx(
	MGC_LinuxCd *pThis, 			
	u8 epnum					
);

/// @brief musb host mode DMAQ Tx task done handling
/// .
/// @param musb:   struct musb
/// @param epnum: endpoint number
void mtk_q_host_tx(
	MGC_LinuxCd *pThis, 			
	u8 epnum					

);
extern void flush_ep_csr(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx);
extern bool mtk_is_qmu_enabled(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx);
extern int mtk_qmu_insert_task(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx, u8* buf, u32 length, u8 isIOC);
#endif

