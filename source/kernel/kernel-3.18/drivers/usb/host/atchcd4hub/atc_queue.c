/*! @file Musb_qmu.c
*
* $Workfile: atc_queue.c$
*
* @par Project:
*    atc usb swip
*
* @par Description:
*    Mtk DMAQ lib interface implement
*
* @par Author_Name:
*    tianhao.fei
*
* @par Last_Changed: add comment for Doxygen
* $Author: songlin.chen $
* $Modtime: 2010-5-7$
* $Revision: #1 $
*
* @todo Need to add support for function X
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>

#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/stat.h>

#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/usb/ch9.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>

//#include "usbhostconf.h"
#include "atc_hcd.h"

#include "atc_queue.h"

#ifdef CONFIG_USB_QUEUE

unsigned mtk_usb_debug=10;

//point to queue lib

bool mtk_is_qmu_enabled(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx)
{
  return _uqft->is_qmu_enabled(pThis->bPhyIndex, EP_Num, isRx);
}

int mtk_qmu_enable(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx, u8 isZLP, u8 isCSCheck, u8 isEmptyCheck)
{
  return _uqft->qmu_enable(pThis->bPhyIndex, EP_Num,isRx,isZLP,isCSCheck,isEmptyCheck);
}

void mtk_qmu_disable(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx)
{
  _uqft->qmu_disable(pThis->bPhyIndex, EP_Num,isRx);
}

int mtk_qmu_insert_task(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx, u8* buf, u32 length, u8 isIOC)
{
  return _uqft->qmu_insert_task(pThis->bPhyIndex, EP_Num,isRx,buf,length,isIOC);
}

int mtk_qmu_remove_task(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx, u8* buf, u16 length, u8* irq_cb, u8* do_stop)
{
  return _uqft->qmu_remove_task(pThis->bPhyIndex, EP_Num,isRx, buf, length, irq_cb, do_stop);
}

int mtk_qmu_cleanup(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx)
{
  return _uqft->qmu_cleanup(pThis->bPhyIndex, EP_Num,isRx);
}

void mtk_qmu_stop(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx)
{
  _uqft->qmu_stop(pThis->bPhyIndex, EP_Num,isRx);
}

int mtk_qmu_restart(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx)
{
  return _uqft->qmu_restart(pThis->bPhyIndex, EP_Num,isRx);
}


int mtk_qmu_irq(MGC_LinuxCd *pThis, u32 qisar, Mtk_USB_Result pResult[][8])
{
  return _uqft->qmu_irq(pThis->bPhyIndex, qisar,(void*)pResult);
}

//end

void MGC_Q_CompleteEndUrb(MGC_LinuxCd * pThis,
                               MGC_LinuxLocalEnd * pEnd, struct urb *urb)
{
    if(urb->status == -EINPROGRESS)
		urb->status = 0;

	// do we need to save toggle for queue ???

    if (MGC_DequeueEndurb(pEnd, urb) == 0)
    {
           if(!MGC_IsEndIdle(pEnd)){
                MGC_GetNextUrb(pEnd);
		   }
            // Unlock pEnd to prevent dead lock with user's submit urb in callback.
           spin_unlock(&pEnd->Lock);
	     #ifdef USB_IRQ_LOCK		
           spin_unlock(&pThis->Lock);
		 #endif
           MGC_CallbackUrb(pThis, urb);
		 #ifdef USB_IRQ_LOCK 
		   spin_lock(&pThis->Lock);
		 #endif
           // Lock pEnd to prevent race condition with other thread.
           spin_lock(&pEnd->Lock);
    }
	else
    {
        ERR("*** pUrb=%p is not queued to bEnd=%d, this is BAD!\n", urb,
            pEnd->bEnd);
    }
}

void mtk_q_dma_select (MGC_LinuxCd *pThis, u8 channel, u8 burstmode)
{
    void *mbase = pThis->pRegs;
    u32 ctrl;

	ctrl = MGC_Read16(mbase,MUSB_HSDMA_CFG);

	MGC_Write16(mbase,MUSB_HSDMA_CFG, ctrl | channel<<4);

	//burst mode
	ctrl = MGC_Read16(mbase,
		MGC_HSDMA_CHANNEL_OFFSET(channel, MGC_O_HSDMA_CONTROL));
		
	MGC_Write16(mbase,
		MGC_HSDMA_CHANNEL_OFFSET(channel, MGC_O_HSDMA_CONTROL), ctrl | burstmode<<9);

}

int mtk_enable_q(MGC_LinuxCd *pThis,u8 address, u8 EP_Num, u8 isRx, u8 type, u16 MaxP, u8 interval, u8 target_ep, u8 isZLP, u8 isCSCheck, u8 isEmptyCheck, u8 hb_mult)
{
    void   *mbase = pThis->pRegs;
    void   *epio = pThis->aLocalEnd[!isRx][EP_Num].regs;
    //u16    csr = 0;
    u16 intr_e = 0;
	int status = 0;

    DBG(LOG_QMU, "select target_ep: %d\n", target_ep);
    DBG(LOG_QMU, "select hw_ep: %d\n", EP_Num);

    //musb_ep_select(mbase, EP_Num);

    flush_ep_csr(pThis, EP_Num,  isRx);

    if (isRx)
    {
    	
	   if (type == USB_ENDPOINT_XFER_ISOC)
       {
 		if(hb_mult == 3){
            MGC_Write16(epio, MGC_O_HDRC_RXMAXP, MaxP|0x1000);
 		}	
		else if(hb_mult == 2){
			MGC_Write16(epio, MGC_O_HDRC_RXMAXP, MaxP|0x800);
	    }	
		else{
			MGC_Write16(epio, MGC_O_HDRC_RXMAXP, MaxP);
		}	
       }
	   else{
       	MGC_Write16(epio, MGC_O_HDRC_RXMAXP, MaxP);
	   }	

		
       MGC_Write16(epio, MGC_O_HDRC_RXCSR, MGC_M_RXCSR_DMAENAB);
       //CC: speed?
       MGC_Write8(epio, MGC_O_HDRC_RXTYPE, type<<4 | target_ep);
       MGC_Write8(epio, MGC_O_HDRC_RXINTERVAL, interval);

        MGC_Write8(mbase, MGC_BUSCTL_OFFSET(EP_Num, MGC_O_MHDRC_RXFUNCADDR),
                  address);
        
        //turn off intrRx
        intr_e = MGC_Read16(mbase, MGC_O_HDRC_INTRRXE);
        intr_e = intr_e & (~(1<<EP_Num));
        MGC_Write16(mbase, MGC_O_HDRC_INTRRXE, intr_e);
    }
    else
    {
        MGC_Write16(epio, MGC_O_HDRC_TXMAXP, MaxP);
 
        MGC_Write16(epio, MGC_O_HDRC_TXCSR, MGC_M_TXCSR_DMAENAB);
        //CC: speed?
        MGC_Write8(epio, MGC_O_HDRC_TXTYPE, type<<4 | target_ep);
        MGC_Write8(epio, MGC_O_HDRC_TXINTERVAL, interval);
     
       
	    MGC_Write8(mbase, MGC_BUSCTL_OFFSET(EP_Num, MGC_O_MHDRC_TXFUNCADDR),
				  address);

		
        //turn off intrTx
        intr_e = MGC_Read16(mbase, MGC_O_HDRC_INTRTXE);
        intr_e = intr_e & (~(1<<EP_Num));
        MGC_Write16(mbase, MGC_O_HDRC_INTRTXE, intr_e);
    }

    status =  mtk_qmu_enable(pThis,EP_Num, isRx, isZLP, isCSCheck, isEmptyCheck);
	//q_disabled = false;
	return status;
}

void mtk_disable_q_all(MGC_LinuxCd *pThis){
	
    u32 EP_Num = 0;
	
    for(EP_Num = 1; EP_Num <= RXQ_NUM; EP_Num++){
        if(mtk_is_qmu_enabled(pThis,EP_Num, 1))
            mtk_disable_q(pThis, EP_Num, 1);
    }
    for(EP_Num = 1; EP_Num <= TXQ_NUM; EP_Num++){
        if(mtk_is_qmu_enabled(pThis,EP_Num, 0))
            mtk_disable_q(pThis, EP_Num, 0);
    }
}

void mtk_disable_q(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx){
    void   *epio = pThis->aLocalEnd[!isRx][EP_Num].regs;

    u16    csr;

    //spin_lock_irqsave(&musb->lock, flags);
    mtk_qmu_disable(pThis,EP_Num, isRx);
    //musb_ep_select(mbase, EP_Num);
    if(isRx){
        csr = MGC_Read16(epio, MGC_O_HDRC_RXCSR);
        csr &= ~MGC_M_RXCSR_DMAENAB;
        MGC_Write16(epio, MGC_O_HDRC_RXCSR, csr);
        flush_ep_csr(pThis, EP_Num,  isRx);
    }else{
        csr = MGC_Read16(epio, MGC_O_HDRC_TXCSR);
        csr &= ~MGC_M_TXCSR_DMAENAB;
        MGC_Write16(epio, MGC_O_HDRC_TXCSR, csr);
        flush_ep_csr(pThis, EP_Num,  isRx);
    }
    //spin_unlock_irqrestore(&musb->lock, flags);
}


//int Mtk_Cleanup_Q(struct musb *musb, u8 EP_Num, u8 isRx);

void mtk_stop_q(MGC_LinuxCd *pThis,u8 EP_Num, u8 isRx){
    mtk_qmu_stop(pThis,EP_Num, isRx);
}

int mtk_restart_q(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx){
    void   *mbase = pThis->pRegs;
    //musb_ep_select(mbase, EP_Num);
    flush_ep_csr(mbase, EP_Num,  isRx);
    mtk_qmu_cleanup(pThis,EP_Num, isRx);
    return mtk_qmu_restart(pThis,EP_Num, isRx);
}

#if 0
int mtk_q_remove_urb(struct musb *musb,struct urb* urb,u8 EP_Num,bool isRx,u8* irq_cb){

    u8 do_stop = false;
	u8* buf = NULL;

    MUSB_ASSERT(urb);
	buf = (u8*)urb->transfer_dma;
	if(buf == NULL)
		buf = virt_to_phys(urb->transfer_buffer);

	MUSB_ASSERT(buf);
	if(musb_qmu_remove_task(musb,EP_Num,isRx,buf,urb->transfer_buffer_length,irq_cb,&do_stop))
	{
	  return 1;
	}

	if(do_stop)
		mtk_restart_q(musb,EP_Num,isRx);
	
	return 0;
}
#endif

irqreturn_t mtk_q_interrupt(MGC_LinuxCd *pThis){
    irqreturn_t    retval = IRQ_NONE;
    u8 RxIndex;
    u8 TxIndex;

    //CC: some way to reduce uncessary function calls?
    if (mtk_qmu_irq(pThis,pThis->int_queue, pThis->pMtk_usb_result))
    {
        for (RxIndex=1; RxIndex<=RXQ_NUM; RxIndex++)
        {
            //CC: add this to save efforts
            if (pThis->pMtk_usb_result[RXQ][RxIndex].status || pThis->pMtk_usb_result[RXQ][RxIndex].number_of_sdu)
            {
                    mtk_q_host_rx(pThis, RxIndex);
            }
        }

        for (TxIndex=1; TxIndex<=TXQ_NUM; TxIndex++)
        {
            //CC: add this to save efforts
            if (pThis->pMtk_usb_result[TXQ][TxIndex].status || pThis->pMtk_usb_result[TXQ][TxIndex].number_of_sdu)
            {
                    mtk_q_host_tx(pThis, TxIndex);
            }
        }
    }

    return retval;
}


//#ifdef CONFIG_USB_MUSB_HDRC_HCD
void mtk_q_host_rx(MGC_LinuxCd *pThis, u8 epnum){
    struct urb        *urb=NULL;
    size_t            xfer_len;
    int            pipe;
    u16            rx_csr;
    u16            ep_type;
    bool            done = false;
    bool            isRx=true;
    int            status=MTK_NO_ERROR;
    Mtk_USB_Result    pResult = pThis->pMtk_usb_result[isRx][epnum];
    PMtk_SDU_Result    sdu_result = pResult.link_header;
	
    //void   *mbase = pThis->pRegs;
	MGC_LinuxLocalEnd *pEnd = &(pThis->aLocalEnd[!isRx][epnum]);
    void   *epio = pEnd->regs;	

    spin_lock(&pEnd->Lock);
	urb = MGC_GetCurrentUrb(pEnd);
    if (unlikely(!urb)) {
            DBG(3, "BOGUS RX%d ready\n", epnum);
            mtk_stop_q(pThis, epnum, isRx);
			spin_unlock(&pEnd->Lock);
            return;
    }	
    //musb_ep_select(mbase, epnum);
    rx_csr = MGC_Read16(epio, MGC_O_HDRC_RXCSR);
    ep_type = MGC_Read8(epio, MGC_O_HDRC_RXTYPE);
    if((ep_type>>4)==USB_ENDPOINT_XFER_ISOC){
        if (rx_csr & MGC_M_RXCSR_DATAERR) {
            printk(KERN_ALERT  "RX end %d ISO data error\n", epnum);
            /* packet error reported later */
            urb->error_count = true;
            //status=-EPROTO;
            rx_csr |= MUSB_RXCSR_H_WZC_BITS;
            rx_csr &= ~MGC_M_RXCSR_DATAERR;
            MGC_Write16(epio, MGC_O_HDRC_RXCSR, rx_csr);
        }else if (rx_csr & MGC_M_RXCSR_INCOMPRX && pResult.status!=MTK_BUS_ERROR) {
            printk(KERN_ALERT "end %d high bandwidth incomplete ISO packet RX\n",
                    epnum);
            //status = -EPROTO;
            rx_csr |= MUSB_RXCSR_H_WZC_BITS;
            rx_csr &= ~MGC_M_RXCSR_INCOMPRX;
            MGC_Write16(epio, MGC_O_HDRC_RXCSR, rx_csr);
        }else if (rx_csr & MGC_M_RXCSR_PID_ERR) {
            printk(KERN_ALERT "end %d PID Error ISO packet RX\n",
                    epnum);
            status = -EILSEQ;
            rx_csr |= MUSB_RXCSR_H_WZC_BITS;
            rx_csr &= ~MGC_M_RXCSR_PID_ERR;
            MGC_Write16(epio, MGC_O_HDRC_RXCSR, rx_csr);
        }else{
            //status= -EINPROGRESS;
        }
    }

    while(pResult.number_of_sdu--){
        
        done = false;
        urb = MGC_GetCurrentUrb(pEnd);
        xfer_len = 0;
        if (unlikely(!urb)) {
            DBG(3, "BOGUS RX%d ready\n", epnum);
            mtk_stop_q(pThis, epnum, isRx);
			spin_unlock(&pEnd->Lock);
            return;
        }
        pipe = urb->pipe;
		
        if (usb_pipeisoc(pipe)) {
            struct usb_iso_packet_descriptor    *d;
            d = urb->iso_frame_desc + pEnd->dwIsoPacket;
            d->actual_length = sdu_result->actual_length;
            urb->actual_length += sdu_result->actual_length;
			//no need?
            //pEnd->dwOffset += sdu_result->actual_length;
            if(status){
                urb->error_count++;
                //Chiachun: MTK status could mapping to descriptor status?
                d->status = status;
            }
            else{
                d->status = 0;
            }
            if (++pEnd->dwIsoPacket >= urb->number_of_packets) {
                done = true;
//                urb->status = 0;
            }
        }
		else{
            done=true;

            if(sdu_result->actual_length == 65024 &&  urb->transfer_buffer_length == 65536){
                    urb->actual_length = 65536;
                    pEnd->dwOffset = 65536;
            }
            else if(urb->transfer_buffer_length == 65536 && sdu_result->actual_length == 512){
                done = false;
            }
            else if(urb->transfer_buffer_length == 65536){
                urb->actual_length = sdu_result->actual_length;
                pEnd->dwOffset = sdu_result->actual_length;
            }
            else{
                urb->actual_length = sdu_result->actual_length;
                pEnd->dwOffset = sdu_result->actual_length;
            }
        }
	
        if(done){
            MGC_Q_CompleteEndUrb(pThis, pEnd, urb);
            pEnd->dwIsoPacket = 0;
        }
        sdu_result = sdu_result->next;
    }
	
	if(pResult.status){ 	   /// issue: if isochronous Module error or length error, maybe need cleanup gpd.		
		mtk_stop_q(pThis, epnum, isRx);	
        urb = MGC_GetCurrentUrb(pEnd);
        if (unlikely(!urb)) {
            DBG(3, "BOGUS RX%d ready\n", epnum);
			spin_unlock(&pEnd->Lock);
            return;
        }	
		switch(pResult.status){
			case MTK_MODULE_ERROR:
				urb->status = -EINPROGRESS;
				break;
			case MTK_LENGTH_ERROR:
				urb->status = -EOVERFLOW;
				break;
			case MTK_BUS_ERROR:
				if (rx_csr & MGC_M_RXCSR_H_RXSTALL) {
					printk(KERN_ALERT "RX end %d STALL\n", epnum);
					/* stall; record URB status */
					urb->status = -EPIPE;
					rx_csr |= MUSB_RXCSR_H_WZC_BITS;
					rx_csr &= ~MGC_M_RXCSR_H_RXSTALL;
				} else if (rx_csr & MGC_M_RXCSR_H_ERROR) {
		   //		  printk(KERN_ALERT "end %d RX proto error\n", epnum);
					urb->status = -EPROTO;
					rx_csr |= MUSB_RXCSR_H_WZC_BITS;
					rx_csr &= ~MGC_M_RXCSR_H_ERROR;
				} else if (rx_csr & MGC_M_RXCSR_DATAERR) {
					printk(KERN_ALERT "RX end %d NAK timeout\n", epnum);
					urb->status = -ETIMEDOUT;
					rx_csr |= MUSB_RXCSR_H_WZC_BITS;
					rx_csr &= ~MGC_M_RXCSR_DATAERR;
				}  else if (rx_csr & MGC_M_RXCSR_INCOMPRX) {
					printk(KERN_ALERT "RX end %d No Response\n", epnum);
					urb->status = -EPROTO;
					rx_csr |= MUSB_RXCSR_H_WZC_BITS;
					rx_csr &= ~MGC_M_RXCSR_INCOMPRX;
				}
				MGC_Write16(epio, MGC_O_HDRC_RXCSR, rx_csr);
				break;
			default:
				urb->status = -EINPROGRESS;
				break;
		}

        if(usb_pipebulk(urb->pipe)){
			mtk_qmu_remove_task(pThis, epnum, isRx, (u8*)urb->transfer_dma, urb->transfer_buffer_length, NULL, NULL);
        }
		
		if(urb->status){
			MGC_Q_CompleteEndUrb(pThis, pEnd, urb);
		}
		
	}

    pResult.status=MTK_NO_ERROR;
	spin_unlock(&pEnd->Lock);
}

void mtk_q_host_tx(MGC_LinuxCd *pThis, u8 epnum){
    int            pipe;
    bool        done = false;
    u16            tx_csr;
    u8            isRx=false;
    struct urb    *urb=NULL;

    int status = MTK_NO_ERROR;
    Mtk_USB_Result pResult = pThis->pMtk_usb_result[isRx][epnum];
    PMtk_SDU_Result sdu_result = pResult.link_header;

    //void   *mbase = pThis->pRegs;
	MGC_LinuxLocalEnd *pEnd = &(pThis->aLocalEnd[!isRx][epnum]);
    void   *epio = pEnd->regs;		

    spin_lock(&pEnd->Lock);
    //musb_ep_select(mbase, epnum);
    tx_csr = MGC_Read16(epio, MGC_O_HDRC_TXCSR);

    while (pResult.number_of_sdu--)
    {
        done = false;
        //CC: what is this???
        urb = MGC_GetCurrentUrb(pEnd);
        status = 0;
        if (!urb) {
            DBG(4, "extra TX%d ready\n", epnum);
			spin_unlock(&pEnd->Lock);
            return;
        }
        pipe=urb->pipe;

        //CC: supports for only isochronous and bulk
        //CC: disable isochronous support temporarily

#if 0
        if (usb_pipeisoc(pipe))
        {
            struct usb_iso_packet_descriptor    *d;
            d = urb->iso_frame_desc + qh->iso_idx;

            if (++qh->iso_idx >= urb->number_of_packets) {
                done = true;
                qh->iso_idx = 0;
            }
        }
        else
#endif
        {
            done = true;
            urb->status = status;
            urb->actual_length = sdu_result->actual_length;

        }

        if (done)
        {
            MGC_Q_CompleteEndUrb(pThis, pEnd, urb);
        }
        sdu_result = sdu_result->next;
    }

    if(pResult.status){  /// issue: if isochronous Module error or length error, maybe need cleanup gpd.
        mtk_stop_q(pThis, epnum, isRx);
//        printk(KERN_ALERT "HOST TX ERROR\n");
        urb = MGC_GetCurrentUrb(pEnd);
        if (!urb) {
            DBG(4, "extra TX%d ready\n", epnum);
			spin_unlock(&pEnd->Lock);
            return;
        }
        pipe=urb->pipe;
        switch(pResult.status){
            case MTK_MODULE_ERROR:
            case MTK_LENGTH_ERROR:
                urb->status = -EINPROGRESS;
                break;
            case MTK_BUS_ERROR:
                if (tx_csr & MGC_M_TXCSR_H_RXSTALL) {
                    /* dma was disabled, fifo flushed */
                    printk(KERN_ALERT "TX end %d stall\n", epnum);

                    /* stall; record URB status */
                    urb->status = -EPIPE;

                } else if (tx_csr & MGC_M_TXCSR_H_ERROR) {
                    /* (NON-ISO) dma was disabled, fifo flushed */
                    printk(KERN_ALERT "TX 3strikes on ep=%d\n", epnum);

                    urb->status = -ETIMEDOUT;

                } else if (tx_csr & MGC_M_TXCSR_H_NAKTIMEOUT) {
                    printk(KERN_ALERT "TX end=%d device not responding\n", epnum);

                    urb->status = -ETIMEDOUT;
                    spin_unlock(&pEnd->Lock);
                    return;
                }
                break;
            default:
                urb->status = -EINPROGRESS;
                break;
        }

        if(usb_pipebulk(urb->pipe)){
			mtk_qmu_remove_task(pThis, epnum, isRx, (u8*)urb->transfer_dma, urb->transfer_buffer_length, NULL, NULL);
        }
		
        if(urb->status){
            MGC_Q_CompleteEndUrb(pThis, pEnd, urb);
        }
    }

    pThis->pMtk_usb_result[isRx][epnum].status=MTK_NO_ERROR;
	spin_unlock(&pEnd->Lock);
}
//#endif
#endif

