/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

/** @file atc_hcd_src.c
 *  This C file implements the atc83xx USB host controller driver.
 */

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/usb.h>
#include <linux/usb/quirks.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include <asm/system.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>

#include <linux/dma-mapping.h>
#include <linux/random.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ctype.h> 
#include <linux/version.h>

#include <mach/ac83xx_gpio_pinmux.h>  
#include <mach/ac83xx_pinmux_table.h>
#include <linux/gpio.h>  
#include <mach/ac83xx_gpio_pinmux_mapping.h>
#include <mach/pinmux.h>

#include <linux/usb/hcd.h>

#include <mach/ac83xx_system.h>

//#include "usbhostconf.h"
//#define CONFIG_SONY_USB_POWER_CTRL 1
//#define CONFIG_USB_QUEUE           1

#include "atc_hcd.h"

#ifdef CONFIG_USB_QUEUE
#include "atc_queue.h"
#endif

#include "atc_dev_ids.h"

#if defined(UNIFIED_USB)
#include <mach/ic_version.h>
#include <mach/hardware.h>
#include <mach/cache_operation.h>
#include <asm/memory.h>
#endif


//#include "../../../../../../../platform/kernel/drivers/inc/x_ver.h"


#define MTK_KERNEL_LINUX_LICENSE     "Proprietary"



//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------
#define DRIVER_VERSION    "12 Nov 2009"

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------
#ifdef USB_SINGLE_PORT
    #define MUC_NUM_PLATFORM_DEV (1)
#else 
    #define MUC_NUM_PLATFORM_DEV (2)
#endif

#define USB_READ_WRITE_TEST   1
#define CONFIG_USB_HIBERNATION 1
#define USB_AUTOK_SLEWRATE  0

#define USB_TEST_QUEUE_DISABLE 0

#ifdef UNIFIED_USB
#define USB_SUSPEND_TEST    0
#else
#define USB_SUSPEND_TEST    0
#endif

#ifdef USB_READ_WRITE_TEST
#define USB_DMA_BUFFER_SIZE     (64*1024)
#endif

#define USB_MAX_URB_NUM  (64)

#define EPRX (0)
#define EPTX (1)
#define EP0 (0)

#ifdef UNIFIED_USB
uint8_t MGC_PortEpNumConfig[] = 
{
#ifdef USB_SINGLE_PORT
#if (USB_SINGLE_PORT == 0)
 5,
#elif (USB_SINGLE_PORT == 1)
7,
#else
 5,7,
#endif 
#else 
 5,7,
#endif 
};


uint8_t MGC_DMAChannelNumConfig[]=
{
#ifdef USB_SINGLE_PORT
#if (USB_SINGLE_PORT == 0)
  2,
#elif (USB_SINGLE_PORT == 1)
 4,
#else
  2,4,
#endif 
#else 
  2,4,
#endif  
};

uint8_t MGC_PortQConfig[] = 
{
#if USB_TEST_QUEUE_DISABLE
  0,0,0,
#else
#ifdef USB_SINGLE_PORT
#if (USB_SINGLE_PORT == 0)
   1,
#elif (USB_SINGLE_PORT == 1)
  0,
#else
   1,0,
#endif 
#else 
   1,0,
#endif 

#endif
};

uint8_t MGC_EpQConfig[2][MUSB_C_NUM_EPS] = 
{
#if USB_TEST_QUEUE_DISABLE
  {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}, //rx
  {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}  //tx
#else
   {0,1,1,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}, //rx
   {0,1,1,1, 1,0,0,0, 0,0,0,0, 0,0,0,0}  //tx
#endif
};

static u32 ep_total_fifo_sz_cfg[]={
#ifdef USB_SINGLE_PORT
#if (USB_SINGLE_PORT == 0)
   8,
#elif (USB_SINGLE_PORT == 1)
  12,
#else
   8,12,
#endif 
#else 
   8,12,
#endif 
};

#endif

#define WIFI_USE_QUEUE    (0x00000001)
#define USB_SETTING_CLEAR (0x80000000)
static uint32_t MGC_usb_setting = 0;
static char *usb_protocol = "usb2.0";
static uint8_t plug_status_pm = FALSE;
static uint32_t reset_done = 0;

static uint8_t trace_tx_cmd_status_flag = 0;//virify the vale in probe and resume function
static uint8_t no_response_count = 0;
static uint8_t reset_port_count = 0;
//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------
#define MUC_ASSERT(expr) \
    if(!(expr)) { \
        printk( "\n" __FILE__ ":%d: Assertion " #expr " failed!\n",__LINE__); \
        panic(#expr); \
    }

#define USB_USB_LIST_POOL 1
//---------------------------------------------------------------------------
// Imported variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Imported functions
//---------------------------------------------------------------------------
extern int MUC_usb_config_suspend(void);
//extern void unmap_urb_for_dma(struct usb_hcd *hcd, struct urb *urb);
static void unmap_urb_for_dma(struct usb_hcd *hcd, struct urb *urb)
{
    if (hcd->driver->unmap_urb_for_dma)
        hcd->driver->unmap_urb_for_dma(hcd, urb);
    else
        usb_hcd_unmap_urb_for_dma(hcd, urb);
}

//---------------------------------------------------------------------------
// Static function forward declarations
//---------------------------------------------------------------------------

static void MGC_ServiceDefaultEnd(MGC_LinuxCd * pThis);
static void MGC_ServiceTxAvail(MGC_LinuxCd * pThis, uint8_t bEnd);
static void MGC_ServiceRxReady(MGC_LinuxCd * pThis, uint8_t bEnd);

static irqreturn_t MUC_Irq(struct usb_hcd *hcd);
static int MUC_start(struct usb_hcd *hcd);
static void MUC_stop(struct usb_hcd *hcd);
static int MUC_urb_enqueue(struct usb_hcd *hcd, struct urb *pUrb, gfp_t mem_flags);
static int MUC_urb_dequeue(struct usb_hcd *hcd, struct urb *pUrb, int status);
static void MUC_endpoint_disable(struct usb_hcd *hcd,
                                 struct usb_host_endpoint *hep);
static int MUC_get_frame(struct usb_hcd *hcd);
static int MUC_hub_status_data(struct usb_hcd *hcd, char *buf);
static int MUC_hub_control(struct usb_hcd *hcd, uint16_t typeReq,
                           uint16_t wValue, uint16_t wIndex, char *buf,
                           uint16_t wLength);
static int MUC_bus_suspend(struct usb_hcd *hcd);
static int MUC_bus_resume(struct usb_hcd *hcd);
static void MUC_hub_descriptor(MGC_LinuxCd * pThis,
                               struct usb_hub_descriptor *desc);
static int MUC_reset_device(struct usb_hcd *hcd, struct usb_device *dev);

static int MUC_hcd_probe(struct platform_device *pdev);
static int MUC_hcd_remove (struct platform_device *pdev);
static int MUC_hcd_suspend (struct platform_device *pdev, pm_message_t state);
static int MUC_hcd_resume (struct platform_device *pdev);
static int MGC_UnlinkUrb(MGC_LinuxCd * pThis, struct urb *pUrb);
static inline uint8_t *MGC_GetUrbBuffer(struct urb *pUrb);
static int MGC_IsUacDevice(struct usb_device_descriptor *pDescriptor, 
                                              struct usb_device *pDev);
static int MGC_IsMFIDevice(struct usb_device_descriptor *pDescriptor, 
                                              struct usb_device *pDev);
static int MGC_IsUvcDevice(struct usb_device_descriptor * pDescriptor,
                                            struct usb_device * pDev);
static void MGC_FreeEndpointListBuf(struct usb_hcd * hcd,uint8_t force_free);

static void USB_disconnect_handle(struct usb_hcd *hcd);

static void MGC_Trace_Tx_State(MGC_LinuxCd * pThis,struct urb *pUrb);
//---------------------------------------------------------------------------
// Static variables
//---------------------------------------------------------------------------
static const char MUC_HcdName[] = "MtkUsbHcdHub";

static MUSB_LinuxController MUC_aLinuxController[] = 
{
#ifdef USB_SINGLE_PORT
#if (USB_SINGLE_PORT == 0)
{  /* Port 0 information. */
    .wType = MUSB_CONTROLLER_MHDRC,  
    .pBase = (void *) MUSB_BASE, 
    .dwIrq = MUSB_VECTOR_USB0, 
    .bSupport = FALSE, 
},
#elif (USB_SINGLE_PORT == 1)
{  /* Port 1 information. */
    .wType = MUSB_CONTROLLER_MHDRC, 
    .pBase = (void *) MUSB_BASE2, 
    .dwIrq = MUSB_VECTOR_USB1, 
    .bSupport = FALSE, 
},
#else
{  /* Port 0 information. */
    .wType = MUSB_CONTROLLER_MHDRC,  
    .pBase = (void *) MUSB_BASE, 
    .dwIrq = MUSB_VECTOR_USB0, 
    .bSupport = FALSE, 
},
{  /* Port 1 information. */
    .wType = MUSB_CONTROLLER_MHDRC, 
    .pBase = (void *) MUSB_BASE2, 
    .dwIrq = MUSB_VECTOR_USB1, 
    .bSupport = FALSE, 
},
#endif
#else
    {  /* Port 0 information. */
        .wType = MUSB_CONTROLLER_MHDRC,  
        .pBase = (void *) MUSB_BASE, 
        .dwIrq = MUSB_VECTOR_USB0, 
        .bSupport = FALSE, 
    },
    {  /* Port 1 information. */
        .wType = MUSB_CONTROLLER_MHDRC, 
        .pBase = (void *) MUSB_BASE2, 
        .dwIrq = MUSB_VECTOR_USB1, 
        .bSupport = FALSE, 
    },
#endif    
};

int usbsuspend(struct device *dev);
int usbresume(struct device *dev);
struct dev_pm_ops usbdev_pm_ops = {
        .prepare = NULL,
        .complete = NULL,
        .suspend = usbsuspend,
        .resume = usbresume,
        .freeze = NULL,     
        .thaw = NULL,
        .poweroff = NULL,
        .restore = NULL,
};

struct device_type usb_host_type = {
    .name       = "host",
    .release    = NULL,
    .groups     = NULL,
    .uevent     = NULL,
    .pm     = &usbdev_pm_ops,
};

static struct platform_device MUC_pdev[MUC_NUM_PLATFORM_DEV];
//static struct timer_list *plocaltimer[MUC_NUM_PLATFORM_DEV];//record mtkhcd's timer,and kill it when remove mtkhcd.KO
static struct timer_list *peventstimer[MUC_NUM_PLATFORM_DEV];

static struct usb_hcd *MUC_phcd[MUC_NUM_PLATFORM_DEV];

static struct platform_driver MUC_hcd_driver = 
{
        .probe                = MUC_hcd_probe,
        .remove                = MUC_hcd_remove,
        .suspend        = MUC_hcd_suspend,
        .resume                = MUC_hcd_resume,
        .driver                = {
                .name        = (char *) MUC_HcdName,
                .owner        = THIS_MODULE,
        },
};

static struct hc_driver MUC_hc_driver = 
{
    .description = MUC_HcdName,
    .hcd_priv_size = sizeof(MGC_LinuxCd),

    /*
     * generic hardware linkage
     */
    .irq = MUC_Irq,
    .flags = HCD_USB2 | HCD_MEMORY,

    /* Basic lifecycle operations */
    .start = MUC_start,
    .stop = MUC_stop,

    /*
     * managing i/o requests and associated device resources
     */
    .urb_enqueue = MUC_urb_enqueue,
    .urb_dequeue = MUC_urb_dequeue,
    .endpoint_disable = MUC_endpoint_disable,

    /*
     * periodic schedule support
     */
    .get_frame_number = MUC_get_frame,

    /*
     * root hub support
     */
    .hub_status_data = MUC_hub_status_data,
    .hub_control = MUC_hub_control,
    .bus_suspend = MUC_bus_suspend,
    .bus_resume = MUC_bus_resume,
    .reset_device = MUC_reset_device,
};

#ifdef MUSB_DEBUG
static int MGC_DebugLevel = 3;
static int MGC_DebugDisable = 0;
#endif

#ifdef USB_READ_WRITE_TEST
static struct proc_dir_entry *usb_rw_test_entry = NULL;
static struct proc_dir_entry *usb_stress_test_entry = NULL;
static struct proc_dir_entry *usb_proc_dir = NULL;
static struct proc_dir_entry *usb_log_en_entry = NULL;
static struct proc_dir_entry *usb_root_port_entry = NULL;
static struct proc_dir_entry *usb_setting_entry = NULL;

static unsigned int usb_test_log_en = 0x5;
#define USB_TEST_LOG(n, f, x...) if (usb_test_log_en & (n)){ printk(DRIVER_NAME ": <%s> " f, __func__,## x); }
#endif

#ifdef SIMULATE_USB_INSERT
static int simulate_insert_enable = SIMULATE_INSERT_DISABLE;
static int simulate_insert_usb = SIMULATE_INSERT_DISABLE;
#endif
static MGC_LinuxCd *pThisSimulate = NULL;
static struct kobject *usbdetect_kobj = NULL;

//#if CONFIG_USB_SUSPEND //2010.08.09 Toby, use mtk_hcd_cfg.c to set config
int usbsuspend(struct device *dev);
int usbsuspend(struct device *dev)
{
    pm_message_t state = { .event = PM_EVENT_USER_SUSPEND };

    struct platform_device *Platdev = to_platform_device(dev);
    if(!Platdev)
    {
        printk("unexpected usb error");
        return (-1);
    }
    if(Platdev->id >= MUC_NUM_PLATFORM_DEV)
    {
        printk("unexpected usb error");
        return (-1);
    }

    printk("[usb]suspend usb-hcd%d",Platdev->id);
#if 0   
 #ifdef UNIFIED_USB
    {
     uint32_t u4Reg = MGC_PHY_Read32(MUC_aLinuxController[Platdev->id].pBase,0x68);
     u4Reg |=  0x00040000; 
     MGC_PHY_Write32(MUC_aLinuxController[Platdev->id].pBase, 0x68, u4Reg);

    }

 #else
    MGC_PHY_Write32( MUC_aLinuxController[Platdev->id].pBase, 0x10, 0x00000000);
 #endif   
 #else
 MUC_hcd_suspend(Platdev,state);
 #endif
 
    return 0;
}
int usbresume(struct device *dev);
int usbresume(struct device *dev)
{
    struct platform_device *Platdev = to_platform_device(dev);
    if(!Platdev)
    {
        printk("unexpected usb error");
        return (-1);
    }
    if(Platdev->id >= MUC_NUM_PLATFORM_DEV)
    {
        printk("unexpected usb error");
        return (-1);
    }

    printk("[usb]resume usb-hcd%d",Platdev->id);    

#if 0   
 #ifdef UNIFIED_USB
    {
     uint32_t u4Reg = MGC_PHY_Read32(MUC_aLinuxController[Platdev->id].pBase,0x68);
     u4Reg &=   ~0x00040000; 
     MGC_PHY_Write32(MUC_aLinuxController[Platdev->id].pBase, 0x68, u4Reg);
    
    }


 #else   
    MGC_PHY_Write32( MUC_aLinuxController[Platdev->id].pBase, 0x10, 0x00010000);
 #endif   
 #else 
 MUC_hcd_resume(Platdev);
 #endif
    return 0;
}
//#endif //2010.08.09 Toby, use mtk_hcd_cfg.c to set config
//---------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------
static bool config_usb11(void)
{
    printk("%s\n", usb_protocol);
    if(!strcmp(usb_protocol, "usb1.1")) 
        return true;
    else  
        return false;
}

static inline MGC_LinuxCd *hcd_to_musbstruct(struct usb_hcd *hcd)
{
    return (MGC_LinuxCd *) (hcd->hcd_priv);
}

static inline struct usb_hcd *musbstruct_to_hcd(const MGC_LinuxCd * pThis)
{
    return container_of((void *) pThis, struct usb_hcd, hcd_priv);
}

static inline int MGC_IsPeriodicUrb(struct urb *pUrb)
{
    return (usb_pipeint(pUrb->pipe) || usb_pipeisoc(pUrb->pipe));
}

#ifdef MUSB_DEBUG
static char *MGC_DecodeUrbProtocol(struct urb *pUrb)
{
    static char buffer[8];

    if (!pUrb)
    {
        strcpy(&buffer[0], "NULL");

        return buffer;
    }

    buffer[0] = usb_pipein(pUrb->pipe) ? 'I' : 'O';

    if (usb_pipeint(pUrb->pipe))
    {
        strcpy(&buffer[1], " int");
    }
    else if (usb_pipeisoc(pUrb->pipe))
    {
        strcpy(&buffer[1], " isoc");
    }
    else if (usb_pipebulk(pUrb->pipe))
    {
        strcpy(&buffer[1], " bulk");
    }
    else if (usb_pipecontrol(pUrb->pipe))
    {
        strcpy(&buffer[0], " ctl");
    }

    return buffer;
}
#endif

#ifdef CONFIG_USB_QUEUE
static bool mtk_wifi_dev(struct usb_device *dev)
{
  int i;
  struct usb_device_id * dev_id = wifi_id_table;
  
  MUSB_ASSERT(dev);

  for(i=0; i<ARRAY_SIZE(wifi_id_table); i++)
  {
    if(dev_id->idVendor == dev->descriptor.idVendor 
        && dev_id->idProduct == dev->descriptor.idProduct)
    {
      return true;
    }
    
    dev_id++;
  }

  return false;
}

static inline bool mtk_ep_q_used(MGC_LinuxLocalEnd * pEnd)
{
      return pEnd->ep_q_used;
}
#endif

static bool  mtk_ep_q_expired(struct usb_device* dev , uint8_t pipe_type)
{
   #ifdef CONFIG_USB_QUEUE
      if(dev->speed == USB_SPEED_HIGH
        &&((!MGC_IsUacDevice(&dev->descriptor, dev)&&(pipe_type == USB_ENDPOINT_XFER_ISOC))
              || ((MGC_usb_setting & WIFI_USE_QUEUE) && (mtk_wifi_dev(dev)) 
                  && (pipe_type== USB_ENDPOINT_XFER_BULK)))){
           return TRUE;
      }
      else
   #endif   
      {
           return FALSE;
      } 
    
}

static bool mtk_alloc_ep_fifo(MGC_LinuxCd * pThis,MGC_LinuxLocalEnd * pEnd,u16 wMaxPacketSize,bool is_in)
{
      u16 ep_fifo_sz = (le16_to_cpu(wMaxPacketSize)&0x7ff) *(1 + ((le16_to_cpu(wMaxPacketSize) >> 11) & 0x03));
      u16 fifo_unit_nr = (ep_fifo_sz+511)/512;
      u16 fifosz = 0;
      u16 free_uint = 0;
      u16 i;
      u8 reversed;
      u8 free;
      u8 found = 0;
      u16 fifoaddr;   
      u8 index;   

      if(pEnd->used_num > 0)
      {
        printk("[usb]fifo have alloced\n");
        return true;
      } 
      printk("max packet size 0x%x\n",le16_to_cpu(wMaxPacketSize));
      
      reversed = (ep_fifo_sz<=512)?1:0;   
        
      for(i=0; i<pThis->ep_fifo_total_sz; i++)
      {
        if(reversed)
           free = !(pThis->ep_fifo & (1<<(pThis->ep_fifo_total_sz-1-i)));
        else
           free = !(pThis->ep_fifo &  (1<<i));
        
        if(free)
          free_uint++;
        else
          free_uint = 0;
     
         if(free_uint == fifo_unit_nr)
         {
           found =1;
           break;   
         } 
            
      }

      if(found == 0)
      {
         printk("[usb]fifo not enough!!!!!!!!!!!\n");
         return false;
      }  

    if(reversed)
      fifoaddr = pThis->ep_fifo_total_sz-1-i;
    else
      fifoaddr = i-(fifo_unit_nr-1);

      for(i=0; i< fifo_unit_nr; i++)
      {
          pThis->ep_fifo |= (1<<(fifoaddr+i));
      }
      
      if(ep_fifo_sz <= 512){
        fifosz = 6;
      }
      else if(ep_fifo_sz <= 1024){
        fifosz = 7;
      }
      else if(ep_fifo_sz <= 2048){
        fifosz = 8;
      }
      else if(ep_fifo_sz <= 4096){
        fifosz = 9;
      }  

      index = MGC_Read8(pThis->pRegs, MGC_O_HDRC_INDEX);
      MGC_SelectEnd(pThis->pRegs, pEnd->bEnd);
      
      printk("[usb]assign hwep: %d,rx: %d,fifoaddr: 0x%08x,fifosz: 0x%02x\n",pEnd->bEnd,
        is_in,fifoaddr, fifosz);    
      printk("[usb]ep fifo status: 0x%08x\n",pThis->ep_fifo);
      
      if(is_in)
      {
        MGC_Write8(pThis->pRegs, MGC_O_HDRC_RXFIFOSZ, fifosz);
        MGC_Write16(pThis->pRegs, MGC_O_HDRC_RXFIFOADD, 0x08+0x40*fifoaddr);  
      }
      else
      {
        MGC_Write8(pThis->pRegs, MGC_O_HDRC_TXFIFOSZ, fifosz);
        MGC_Write16(pThis->pRegs, MGC_O_HDRC_TXFIFOADD, 0x08+0x40*fifoaddr);    
      }
      pEnd->wMaxPacketSize = ep_fifo_sz; 
      MGC_Write8(pThis->pRegs, MGC_O_HDRC_INDEX, index);
    
      return true;    
}

static bool mtk_release_ep_fifo(MGC_LinuxCd * pThis,MGC_LinuxLocalEnd * pEnd,u16 wMaxPacketSize,bool is_in)
{
      u16 ep_fifo_sz = (le16_to_cpu(wMaxPacketSize)&0x7ff) *(1 + ((le16_to_cpu(wMaxPacketSize) >> 11) & 0x03));
      u16 fifo_unit_nr = (ep_fifo_sz+511)/512;
      u16 i;
      u16 fifoaddr;   
      u8 index; 

      if(pEnd->used_num != 0)
      {
        printk("[usb]fifo have still used by other ep\n");
        return true;
      }

      index = MGC_Read8(pThis->pRegs, MGC_O_HDRC_INDEX);
      MGC_SelectEnd(pThis->pRegs, pEnd->bEnd);


      if(is_in)
        fifoaddr = MGC_Read16(pThis->pRegs, MGC_O_HDRC_RXFIFOADD); 
      else
        fifoaddr = MGC_Read16(pThis->pRegs, MGC_O_HDRC_TXFIFOADD); 

      
      fifoaddr =  (fifoaddr-0x08)/0x40;     

      for(i=0; i< fifo_unit_nr; i++)
      {
          pThis->ep_fifo &= ~(1<<(fifoaddr+i));
      }

      printk("[usb]release ep: %d,in: %d,ep fifo status: 0x%08x\n",pEnd->bEnd,is_in,pThis->ep_fifo);

      if(is_in)
      {
          MGC_Write8(pThis->pRegs, MGC_O_HDRC_RXFIFOSZ, 0);
          MGC_Write16(pThis->pRegs, MGC_O_HDRC_RXFIFOADD, 0); 

      }
      else
      {
          MGC_Write8(pThis->pRegs, MGC_O_HDRC_TXFIFOSZ, 0);
          MGC_Write16(pThis->pRegs, MGC_O_HDRC_TXFIFOADD, 0); 

      }

      pEnd->wMaxPacketSize = 0; 
      MGC_Write8(pThis->pRegs, MGC_O_HDRC_INDEX, index);
      return true;      
}

static inline MGC_LinuxUrbList* MGC_AllocUrbList(MGC_LinuxUrbList* list_buf)
{
  int i;
  MGC_LinuxUrbList* pUrbList;
  for(i=0; i< USB_MAX_URB_NUM; i++)
  {
    pUrbList = list_buf+i;
    if(!pUrbList->used){
       pUrbList->used = 1;
       return pUrbList;
    }   
  }
  return NULL;
}

static inline MGC_LinuxUrbList* MGC_FreeUrbList(MGC_LinuxUrbList* pUrbList)
{
   pUrbList->used = 0;
   return pUrbList;
}
/// @brief musb Flush Endpoint for QMU
/// .
/// @param musb:   struct musb
/// @param EP_Num: Endpoint number mapping Queue
/// @param isRx: RxQ or TxQ
/// @param type: Transfer Type for the selected Queue
/// @param isHost: If the usb is host mode
void flush_ep_csr(MGC_LinuxCd *pThis, u8 EP_Num, u8 isRx){
    void *mbase = pThis->pRegs;
    void *epio = pThis->aLocalEnd[!isRx][EP_Num].regs;
    u16 csr, wCsr;
    
    if (epio == NULL)
        printk(KERN_ALERT "epio == NULL\n");

    if (isRx)
    {
        csr = MGC_Read16(epio, MGC_O_HDRC_RXCSR);
        csr |= MGC_M_RXCSR_FLUSHFIFO | MGC_M_RXCSR_RXPKTRDY;
        csr &= ~MGC_M_RXCSR_H_REQPKT;

        /* write 2x to allow double buffering */
        //CC: see if some check is necessary
        MGC_Write16(epio, MGC_O_HDRC_RXCSR, csr);
        MGC_Write16(epio, MGC_O_HDRC_RXCSR, csr | MGC_M_RXCSR_CLRDATATOG);
    }
    else
    {
        csr = MGC_Read16(epio, MGC_O_HDRC_TXCSR);
        if (csr&MGC_M_TXCSR_TXPKTRDY)
        {
            wCsr = csr | MGC_M_TXCSR_FLUSHFIFO | MGC_M_TXCSR_TXPKTRDY;
            MGC_Write16(epio, MGC_O_HDRC_TXCSR, wCsr);
        }
        
        MU_MB();
        csr |= MGC_M_TXCSR_FLUSHFIFO&~MGC_M_TXCSR_TXPKTRDY;
        MGC_Write16(epio, MGC_O_HDRC_TXCSR, csr);
        MGC_Write16(epio, MGC_O_HDRC_TXCSR, csr | MGC_M_TXCSR_CLRDATATOG);
        
        //CC: why is this special?
        MU_MB();
        MGC_Write16(mbase, MGC_O_HDRC_INTRTX, 1<<EP_Num);
    }
}

int MGC_IsEndIdle(MGC_LinuxLocalEnd * pEnd)
{
    if (pEnd->pCurrentUrb)
    {
        return FALSE;
    }

    return list_empty(&pEnd->list);
}

struct urb *MGC_GetNextUrb(MGC_LinuxLocalEnd * pEnd)
{
    MGC_LinuxUrbList *pUrbList;
    struct urb *pUrb = NULL;

    if (pEnd->pCurrentUrb)
    {
        //MUSB_ASSERT(list_empty(&pEnd->list) == TRUE);
         //printk("[usb]urb for dual core or wifi complex dev?\n");
        pUrb = pEnd->pCurrentUrb;        
    }
    else
    {
        MUSB_ASSERT(list_empty(&pEnd->list) == FALSE);
        pUrbList = list_entry(pEnd->list.next, MGC_LinuxUrbList, list);
        pUrb = pUrbList->pUrb;
        if (pUrb)
        {
            pEnd->pCurrentUrb = pUrb;
            list_del(&pUrbList->list);
            #if USB_USB_LIST_POOL
                MGC_FreeUrbList(pUrbList);
            #else
                kfree (pUrbList);
            #endif
        }        
    }

    /*
    if (pUrb)
    {
        printk("Next pUrb=0x%08X, size=%d.\n", (uint32_t)pUrb, pUrb->transfer_buffer_length);
    }
    else
    {
        printk("Next pUrb=NULL.\n");
    }
    */    

    return pUrb;
}

struct urb *MGC_GetCurrentUrb(MGC_LinuxLocalEnd * pEnd)
{
    return pEnd->pCurrentUrb;
}

static void MGC_ClearEnd(MGC_LinuxLocalEnd * pEnd)
{
    pEnd->dwOffset = 0;
    pEnd->dwRequestSize = 0;
    pEnd->dwIsoPacket = 0;
    pEnd->dwWaitFrame = 0;
    pEnd->bRetries = 0;
    pEnd->bTrafficType = 0;
}


static int MGC_EnqueueEndUrb(MGC_LinuxLocalEnd * pEnd, struct urb *pUrb)
{
    MGC_LinuxUrbList *pUrbList;

    if ((pEnd->pCurrentUrb == NULL) && (list_empty(&pEnd->list) == TRUE))
    {
        // Only put pUrb to pCurrentUrb when pCurrentUrb and list are both empty.
        pEnd->pCurrentUrb = pUrb;
    }
    else
    {
    //avoid to alloc memory in critical session
#if USB_USB_LIST_POOL
        if(!pEnd->list_buf)
        {
          pEnd->list_buf = (MGC_LinuxUrbList*)kmalloc(USB_MAX_URB_NUM*sizeof(MGC_LinuxUrbList), pEnd->mem_flags);
          if(!pEnd->list_buf)
          {
            return -ENOMEM; 
          }
          memset((void*)pEnd->list_buf, 0, USB_MAX_URB_NUM*sizeof(MGC_LinuxUrbList));
        }

        pUrbList = MGC_AllocUrbList(pEnd->list_buf);
        if (!pUrbList)
        {
            printk("[HCD]No memory to get urb pool list.\n");
            return -ENOMEM;
        }       
#else
            pUrbList = kmalloc (sizeof(MGC_LinuxUrbList), pEnd->mem_flags);
            if (!pUrbList)
            {
                printk("[HCD]No memory to get urb list.\n");
                return -ENOMEM;
            }
            memset(pUrbList, 0, sizeof(MGC_LinuxUrbList));
#endif
        pUrbList->pUrb = pUrb;
        list_add_tail (&pUrbList->list, &pEnd->list);     
    }
    // Add pUrb to pEnd structure.
    pUrb->hcpriv = pEnd;

    return 0;   
}

int MGC_DequeueEndurb(MGC_LinuxLocalEnd * pEnd, struct urb *pUrb)
{
    MGC_LinuxUrbList *pUrbList;
    MGC_LinuxUrbList *pNextUrbList;

    // Remove pUrb to pEnd structure.
    pUrb->hcpriv = NULL;

    if (pEnd->pCurrentUrb == pUrb)
    {   
        pEnd->pCurrentUrb = NULL;
        MGC_ClearEnd(pEnd);
        return 0;
    }
    else
    {
        list_for_each_entry_safe (pUrbList, pNextUrbList, &pEnd->list, list)
        {
            if (pUrbList->pUrb == pUrb)
            {
                list_del (&pUrbList->list);
                #if USB_USB_LIST_POOL
                    MGC_FreeUrbList(pUrbList);
                #else
                    kfree (pUrbList);
                #endif
                return 0;     
            }            
        }            
    }

    return -1;
}

static int MGC_CheckDequeueurb(MGC_LinuxLocalEnd * pEnd, struct urb *pUrb, int status)
{
    MGC_LinuxUrbList *pUrbList;
    MGC_LinuxUrbList *pNextUrbList;
    
    if(!(pEnd && pUrb))
    {
        return -EIDRM;
    }

    if (pEnd->pCurrentUrb == pUrb)
    {   
        return 0;
    }
    else
    {
        list_for_each_entry_safe (pUrbList, pNextUrbList, &pEnd->list, list)
        {
            if (pUrbList->pUrb == pUrb)
            {
                break;            
            }            
        }   
        if (pUrbList->pUrb != pUrb)
        {
            return -EIDRM;            
        }
            
        if (pUrb->unlinked)
        {
        #if 0   // fix 3G dongle open/close bugs.  2012/03/08
        return -EBUSY;
        #else
        return 0;
        #endif
    }
    
    pUrb->unlinked = status;        
    }

    return 0;
}

static void MGC_InitEnd(MGC_LinuxCd * pThis)
{
    uint8_t bEnd;
    uint8_t bTx;

    MGC_LinuxLocalEnd *pEnd;
    void *pBase = pThis->pRegs;
    uint16_t wFifoOffset = 64;

    /* use the defined end points */
#ifdef UNIFIED_USB
    pThis->bEndCount = MGC_PortEpNumConfig[pThis->bPhyIndex];
    if(pThis->bEndCount > MUSB_C_NUM_EPS)
    {
      printk("[usb]too many endpoints\n");
      return;
    }
#else
    pThis->bEndCount = MUSB_C_NUM_EPS;
#endif

    /* Dynamic FIFO sizing: use pre-computed values for EP0 */
    MGC_SelectEnd(pBase, 0);
    MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, 3);
    MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, 3);
    MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, 0);
    MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, 0);

    pEnd = &(pThis->aLocalEnd[EP0][0]);
    pEnd->bIsSharedFifo = TRUE;
    pEnd->bEnd = 0;
    pEnd->bIsTx = TRUE;
    pEnd->wMaxPacketSize = 64;
    pEnd->wPacketSize = 0;

    pThis->wEndMask = 1;
    /* reset the softstate */
    spin_lock_init(&pEnd->Lock);
    pEnd->pCurrentUrb = NULL;
    INIT_LIST_HEAD(&pEnd->list);

    pEnd->wPacketSize = 0;
    pEnd->bRemoteAddress = 0;
    pEnd->bRemoteEnd = 0;
    pEnd->bTrafficType = 0;
    pEnd->bUseTxRxFifo = 0;
    //queue mode 
    pEnd->regs = MUSB_EP_OFFSET(0,0) + pThis->pRegs;
    pEnd->ep_q_configed = 0;
    pEnd->list_buf = NULL;
    //end

    //patch  need init pEnd[ep0][1] listhead
    INIT_LIST_HEAD(&(pThis->aLocalEnd[1][0].list));
    //end

    /* take care of the remaining eps */
#ifdef UNIFIED_USB
    for (bEnd = 1; bEnd < pThis->bEndCount; bEnd++)
#else
    for (bEnd = 1; bEnd < MUSB_C_NUM_EPS; bEnd++)
#endif      
    {        
        MGC_SelectEnd(pBase, bEnd);
        // fifo size = 512, turn off double packet buffer.
        MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, 6);
        MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, 6);

        MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, wFifoOffset >> 3);
        DBG(3, "bEnd = %d, RX, fifo addr=%d.\n", bEnd, wFifoOffset);

        wFifoOffset += 512;

        MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, wFifoOffset >> 3);
        DBG(3, "bEnd = %d, TX, fifo addr=%d.\n", bEnd, wFifoOffset);

        wFifoOffset += 512;

        pThis->wEndMask |= (1 << bEnd);
        
        for (bTx=EPRX; bTx<=EPTX; bTx++)
        {
            pEnd = &(pThis->aLocalEnd[bTx][bEnd]);
            pEnd->bEnd = bEnd;
            pEnd->bIsTx = bTx;
            pEnd->wMaxPacketSize = 512;
            pEnd->wPacketSize = 0;
            pEnd->bIsSharedFifo = FALSE;
            /* reset the softstate */
            spin_lock_init(&pEnd->Lock);
            pEnd->pCurrentUrb = NULL;
            INIT_LIST_HEAD(&pEnd->list);

            pEnd->wPacketSize = 0;
            pEnd->bRemoteAddress = 0;
            pEnd->bRemoteEnd = 0;
            pEnd->bTrafficType = 0;
            pEnd->bUseTxRxFifo = 0;
            //queue mode
            pEnd->regs = MUSB_EP_OFFSET(bEnd,0) + pThis->pRegs;
            
            if(pThis->port_q_configed){
            pEnd->ep_q_configed = MGC_EpQConfig[bTx][bEnd];
            }
            else{
                pEnd->ep_q_configed = 0;
            }
            
            pEnd->used_num = 0;
            pEnd->list_buf = NULL;
            //end
        }
    }
}
#if 0
static void MGC_DelTimer(MGC_LinuxCd * pThis)
{
    del_timer_sync((&pThis->Timer));        /* make sure another timer is not running */
}

static void MGC_SetTimer(MGC_LinuxCd * pThis,
                         void (*pfFunc) (unsigned long), uint32_t pParam,
                         uint32_t millisecs)
{
    del_timer_sync((&pThis->Timer));        /* make sure another timer is not running */

    init_timer(&(pThis->Timer));
    pThis->Timer.function = pfFunc;
    pThis->Timer.data = (unsigned long) pParam;
    pThis->Timer.expires = jiffies + (HZ * millisecs) / 1000;
    add_timer(&(pThis->Timer));
}
#endif
static void MGC_UnlinkInvalidUrb(unsigned long pParam)
{
    MGC_LinuxCd *prThis = (MGC_LinuxCd *) pParam;
    struct usb_hcd * hcd = musbstruct_to_hcd(prThis);    
    int i,j,k = 0;
    struct urb *pUrb = NULL;
    struct usb_device *child_dev = NULL;

    if(!prThis)
    {
        return;
    }

    if (!hcd)
    {
        return;
    }
    
    #if(0)
    if(!hcd->self.root_hub->children[0] && !prThis->bInsert)
    {
        MGC_DelTimer(prThis);
        return;
    }
    #endif
    
    for(i = 0; i < 2; i ++)
    {
    #ifdef UNIFIED_USB
        for(j = 0; j < prThis->bEndCount; j ++)
    #else
        for(j = 0; j < MUSB_C_NUM_EPS; j ++)
    #endif      
        {
	    child_dev = usb_hub_find_child(hcd->self.root_hub, 1);
            pUrb = MGC_GetCurrentUrb(&(prThis->aLocalEnd[i][j]));
            if(!pUrb)
            {
                continue;
            }
            //if(usb_pipecontrol(pUrb->pipe))
            if(usb_pipecontrol(pUrb->pipe) || usb_pipebulk(pUrb->pipe))
            {
                if(pUrb->setup_packet)
                {
                    printk("to be unlink request is 0x%02X\n", ((struct usb_ctrlrequest*)(pUrb->setup_packet))->bRequest);
                }
                if ((uint32_t)pUrb->dev == (uint32_t)child_dev ||
                    (uint32_t)pUrb->dev->parent == (uint32_t)child_dev ||
                    (uint32_t)pUrb->dev->parent == (uint32_t)hcd->self.root_hub)
                {
                    MUC_urb_dequeue(hcd, pUrb, -EFAULT);
                    continue;
                }
                if(!child_dev)
                {
                    continue;
                }
                for(k = 0; k < child_dev->maxchild; k ++)
                {
                    if((uint32_t)usb_hub_find_child(child_dev, k+1) == (uint32_t)pUrb->dev)
                    {
                        MUC_urb_dequeue(hcd, pUrb, -EFAULT);
                        continue;
                    }
                }
            }
        }
    }
   
    MGC_FreeEndpointListBuf(hcd,FALSE);
   
//    if(!prThis->bInsert)
//    {
//        MGC_SetTimer(prThis, MGC_UnlinkInvalidUrb, (unsigned long) prThis, 20);
//    }
}

static void MGC_FreeEndpointListBuf(struct usb_hcd * hcd,uint8_t force_free)
{
  int i,j;
  MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);
  MGC_LinuxLocalEnd *pEnd;
  unsigned long flags;

  if(!pThis)
  {
    printk("[USB]pThis is NULL\n");
    return;
  }
  
  spin_lock_irqsave(&pThis->Lock, flags);
  for(i = 0; i < 2; i ++)
  {
      for(j = 0; j < pThis->bEndCount; j ++)
      {
        pEnd = &(pThis->aLocalEnd[i][j]);
        if(MGC_IsEndIdle(pEnd) || force_free)
        {
            if(pEnd->list_buf)
            {
                kfree(pEnd->list_buf);
                pEnd->list_buf = NULL;
            }
        }
        else{
                printk("[usb]endpoint#%d is busy... !!!\n",pEnd->bEnd);
                continue;
        }       
      }
  }
  spin_unlock_irqrestore(&pThis->Lock, flags);
  
}

static void MGC_StartTx(MGC_LinuxCd * pThis, uint8_t bEnd)
{
    uint16_t wCsr;

    uint8_t *pBase = (uint8_t *) pThis->pRegs;

    MGC_SelectEnd(pBase, bEnd);

    if (bEnd)
    {
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);

        wCsr |= MGC_M_TXCSR_TXPKTRDY;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsr);
    }
    else
    {
        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_H_NO_PING |
                       MGC_M_CSR0_H_SETUPPKT | MGC_M_CSR0_TXPKTRDY);
    }
}
#if 0
//for UVC class, ISO IN PIPE checking
static int MGC_IsUvcDevice(struct usb_device_descriptor *pDescriptor, 
                           struct usb_device *pDev)
{
    struct usb_host_config *config = NULL;
    struct usb_interface_assoc_descriptor *iad = NULL;
    int i = 0;
    
    if(!pDescriptor || !pDev)
    {
        return FALSE;
    }
    
    if(pDescriptor->bDeviceClass == 0xEF && //Multi-interface Function Code Device
       pDescriptor->bDeviceSubClass == 0x02 &&  //This is the Common Class Sub Class
       pDescriptor->bDeviceProtocol == 0x01) //This is the Interface Association Descriptor protocol
    {
        return TRUE;
    }

    if(pDev->config)
    {
        config = pDev->config;

        if(config)
        {
            for (i = 0; i < USB_MAXIADS; i++) 
            {
                iad = config->intf_assoc[i];
                if (iad == NULL)
                    break;


                if((iad->bFunctionClass == 0x0E || iad->bFunctionClass == 0xFF) &&
                   iad->bFunctionSubClass == 0x03)
                {
                    printk("IAD Video Interface Class\n");
                    return TRUE;
                }
            }
        }

    }

    return FALSE;
}
#endif
static int MGC_IsUacDevice(struct usb_device_descriptor *pDescriptor, 
                                              struct usb_device *pDev)
{
    struct usb_host_config *c = NULL;
   // struct usb_interface_assoc_descriptor *iad = NULL;
    int i = 0;
    
    if(!pDescriptor || !pDev)
    {
        return FALSE;
    }
    
    if(pDev->actconfig)
    {
        c = pDev->actconfig;

    if(c)
    {
        for(; i < c->desc.bNumInterfaces; i ++)
        {
            if(c->intf_cache[i]->altsetting->desc.bInterfaceClass == USB_CLASS_AUDIO)
            {
                return TRUE;
            }
        }
    }
    }
    return FALSE;
}

static int MGC_IsMFIDevice(struct usb_device_descriptor *pDescriptor, 
                                              struct usb_device *pDev)
{
    //struct usb_host_config *config = NULL;
    //struct usb_interface_assoc_descriptor *iad = NULL;
    //int i = 0;
    
    if(!MGC_IsUacDevice(pDescriptor,pDev))
    {
        return FALSE;
    }
    
    if(0x05AC == pDescriptor->idVendor &&  0x12 == (pDescriptor->idProduct >> 8))
    {
        return TRUE;
    }
    
    return FALSE;   
}

static int MGC_IsUvcDevice(struct usb_device_descriptor *pDescriptor,
                                                    struct usb_device *pDev)
{
    struct usb_host_config *c = NULL;
    int i = 0;
    
    if(!pDescriptor || !pDev)
    {
        return FALSE;
    }
    
    if(pDev->actconfig)
    {
        c = pDev->actconfig;

    if(c)
    {
        for(; i < c->desc.bNumInterfaces; i ++)
        {
            if(c->intf_cache[i]->altsetting->desc.bInterfaceClass == USB_CLASS_VIDEO)
            {
                return TRUE;
            }
        }
    }
    }
    return FALSE;
}

static int MGC_SkipEpCheck(struct usb_device *pDev)
{
    return FALSE;
}

static MGC_LinuxLocalEnd *MGC_FindEnd(MGC_LinuxCd * pThis, struct urb *pUrb)
{
    MGC_LinuxLocalEnd *pEnd = NULL;
    int nEnd = -1;
    unsigned int nOut = usb_pipeout(pUrb->pipe);
    //uint16_t wPacketSize = usb_maxpacket(pUrb->dev, pUrb->pipe, nOut);
    struct usb_device *dev = pUrb->dev;
    struct usb_host_endpoint *ep;   
    struct usb_device_descriptor *pDescriptor = &dev->descriptor;
    struct usb_interface_descriptor *d;
    struct usb_interface *intf = NULL;
    //struct usb_host_interface *iface_desc = NULL;
    //uint8_t *pBase = (uint8_t *) pThis->pRegs;
    uint8_t ep_end;
    uint8_t k;
    uint8_t q_expired;

    DBG(2, "<== pUrb=%p\n", pUrb);

    if(pDescriptor->idVendor == 0x1d6b &&
       pDescriptor->idProduct == 0x0002 &&
       pDescriptor->bDeviceClass == 0x09 &&
       usb_pipeint(pUrb->pipe))
    {
        printk("[USB] Root Hub found.\n");
        pEnd = &(pThis->aLocalEnd[1][EPRX]);  /*0: Rx, 1: Tx*/            
        return pEnd;
    }

    /* control is always EP0, and can always be queued */
    if (usb_pipecontrol(pUrb->pipe))
    {
        DBG(2, "==> is a control pipe use ep0\n");

        pEnd = &(pThis->aLocalEnd[EP0][0]);  /*0: Rx, 1: Tx*/            
        pEnd->bIsOccupy = TRUE;
        return pEnd;
    }

    /* Other EPx */
    ep = (nOut ? dev->ep_out : dev->ep_in)[usb_pipeendpoint(pUrb->pipe)];
    if (!ep) 
    {
        return NULL;
    }
    
    if (ep->hcpriv) 
    {
        pEnd = (MGC_LinuxLocalEnd *)ep->hcpriv;
        pEnd->bIsOccupy = TRUE;
        
        if(!pEnd->dev && dev)
        {
            pEnd->dev = dev;//this local ep should be occupied by check_free_ep
        }
        return pEnd;
    }

    q_expired = mtk_ep_q_expired(dev, (ep->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK));

    intf = pUrb->dev->config->interface[0];

    d = &dev->actconfig->interface[0]->cur_altsetting->desc;
    /* use a reserved one for bulk if any */
    #ifdef PHYEP_MAPPING_MULTI_LOCALEP
    if (usb_pipebulk(pUrb->pipe))
    {

        if ((pDescriptor->bDeviceClass == USB_CLASS_MASS_STORAGE) || 
            (d->bInterfaceClass == USB_CLASS_MASS_STORAGE))
        {
            nEnd = pThis->bEndCount -1;
            // use for mass storage device.
            pEnd = &(pThis->aLocalEnd[nOut][nEnd]);  /*0: Rx, 1: Tx*/        
            ep->hcpriv = (void *)pEnd;
			mtk_alloc_ep_fifo(pThis, pEnd, ep->desc.wMaxPacketSize, !nOut); 
			pEnd->used_num += 1;
			pEnd->dev = dev;
			if(q_expired && pEnd->ep_q_configed)
				pEnd->ep_q_used = 1;
			else
				pEnd->ep_q_used = 0;
			
            pEnd->bIsOccupy = TRUE;

            printk("[USB] Assign L_EP [%d: %s=%d] R_EP:0x%2x Queue: %s\n", 
                            nEnd, nOut?"TX":"RX", pEnd->bIsOccupy, ep->desc.bEndpointAddress, pEnd->ep_q_used?"Yes":"No");      
            return pEnd;
        }
    }
    #endif

    #ifdef PHYEP_MAPPING_MULTI_LOCALEP
    #ifdef UNIFIED_USB
       if(dev->level == 1 && d->bInterfaceClass != USB_CLASS_HUB)
		  ep_end = pThis->bEndCount;
	   else
	#endif 	
	      ep_end = pThis->bEndCount-1;
    #else
          ep_end = pThis->bEndCount;
    #endif

    /* scan, remembering exact match and best match */
    for (k = 1; k < ep_end; k++)
        {
        if(q_expired)
            nEnd = k;
        else
            nEnd = ep_end - k;


        pEnd = &(pThis->aLocalEnd[nOut][nEnd]);
  #if 0
      #if 1 
        if(MGC_IsUvcDevice(pDescriptor, dev) &&
           usb_pipeisoc(pUrb->pipe) &&
           usb_pipein(pUrb->pipe))
      #endif     
        {
            if((pEnd->dev == NULL) && 
               wPacketSize > pEnd->wMaxPacketSize && 
               wPacketSize < pEnd->wMaxPacketSize*2)
            {
                uint8_t bIndex = 0;               /* ep number */

                pEnd->dev = dev;
                ep->hcpriv = (void *)pEnd;
                pEnd->bIsOccupy = TRUE;

                /* save index */
                bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);

                MGC_SelectEnd(pBase, pEnd->bEnd);
                // fifo size = 1024
                MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, 7);
                MGC_SelectEnd(pBase, bIndex);

                pEnd->wMaxPacketSize = 1024;
                pEnd->bIsSharedFifo = TRUE;

                printk("[USB] Assigned L_EP [%d: %s=%d] to Webcam\n", 
                        nEnd, nOut?"TX":"RX", pEnd->bIsOccupy);                               

                return pEnd;
            }
        }
    #endif
        if (pEnd->bIsOccupy == FALSE)
        {
            MUSB_ASSERT(pEnd->pCurrentUrb == NULL);
            MUSB_ASSERT(list_empty(&pEnd->list) == TRUE);

            // Occupy this endpoint.
            pEnd->dev = dev;
            ep->hcpriv = (void *)pEnd;
            mtk_alloc_ep_fifo(pThis, pEnd, ep->desc.wMaxPacketSize, !nOut); 
            pEnd->used_num += 1;
            
            if(q_expired && pEnd->ep_q_configed)
                pEnd->ep_q_used = 1;
            else
                pEnd->ep_q_used = 0;
            
            pEnd->bIsOccupy = TRUE;

            printk("[USB] Assign L_EP [%d: %s=%d] R_EP:0x%2x Queue: %s\n", 
                nEnd, nOut?"TX":"RX", pEnd->bIsOccupy, ep->desc.bEndpointAddress, pEnd->ep_q_used?"Yes":"No");      

          
            return pEnd;
        }
    }
   
    return NULL;
}

int MGC_CheckFreeEndpoint(struct usb_device *dev, int configuration)
{
    MGC_LinuxCd *pThis = NULL;
    struct usb_hcd *pHcd = NULL;
    //MGC_LinuxCd *pThis = pMgcLinuxCd;
    struct usb_host_config *cp = NULL;
    MGC_LinuxLocalEnd *pEnd = NULL;
    //Todo: It should be for each port.
    int aIsLocalEndOccupy[2][MUSB_C_NUM_EPS]; /*0: Rx, 1: Tx*/
    uint16_t wPacketSize = 0;
    unsigned int nOut = 0;
    int nintf = 0; 
    int nEnd = -1;
    int i = 0;
    struct usb_device_descriptor *pDescriptor = &dev->descriptor;
    unsigned long flags = 0;

    if(!dev)
    {
        printk("usb_dev is NULL: %s(%d)\n", __FILE__, __LINE__);
        return -1;
    }

    if(!dev->bus)
    {
        printk("usb_bus is NULL: %s(%d)\n", __FILE__, __LINE__);
        return -1;
    }

    pHcd = bus_to_hcd(dev->bus);

    if(!dev->bus)
    {
        printk("usb_hcd is NULL: %s(%d)\n", __FILE__, __LINE__);
        return -1;
    }
    
    pThis = hcd_to_musbstruct(bus_to_hcd(dev->bus));

    if(!pThis)
    {
        printk("MGC_LinuxCd is NULL: %s(%d)\n", __FILE__, __LINE__);
        return -1;
    }

    if (dev->authorized == 0 || configuration == -1)
    {
        configuration = 0;
    }
    else 
    {
        for (i = 0; i < dev->descriptor.bNumConfigurations; i++) 
        {
            if (dev->config[i].desc.bConfigurationValue == configuration) 
            {
                cp = &dev->config[i];
                break;
            }
        }
    }
    if ((!cp && configuration != 0))
        return -EINVAL;

    if(pDescriptor->idVendor == 0x1d6b &&
       pDescriptor->idProduct == 0x0002 &&
       pDescriptor->bDeviceClass == 0x09)
    {
        printk("[USB CFE] Root Hub found.\n");
        return 0;
    }

    if(MGC_SkipEpCheck(dev))
    {
        return 0;
    }

    if(dev->parent)
    {
        printk("[USB:parent] level  = 0x%04X\n", dev->parent->level);
        printk("[USB:parent] idVendor  = 0x%04X\n", dev->parent->descriptor.idVendor);
        printk("[USB:parent] idProduct = 0x%04X\n", dev->parent->descriptor.idProduct);
        printk("[USB:parent] bDeviceClass = 0x%04X\n", dev->parent->descriptor.bDeviceClass);
    }

    spin_lock_irqsave(&pThis->Lock, flags);

    /* The USB spec says configuration 0 means unconfigured.
     * But if a device includes a configuration numbered 0,
     * we will accept it as a correctly configured state.
     * Use -1 if you really want to unconfigure the device.
     */
    if (cp && configuration == 0)
        dev_warn(&dev->dev, "config 0 descriptor??\n");

    if(cp)
    {
        nintf = cp->desc.bNumInterfaces;

        for (i = 0; i < nintf; ++i) 
        {
            struct usb_interface *intf = cp->interface[i];
            struct usb_host_interface *iface_desc;
            struct usb_endpoint_descriptor *ep_desc;
            int j = 0;
            int found = 0;
            int epnum = 0;
            int is_out = 0;
            struct usb_host_endpoint *ep = NULL;  
            int ep_end = 0;
            int k;
            uint8_t q_expired = 0;

        printk("[USB:interface] bNumEndpoints  = 0x%X\n", intf->cur_altsetting->desc.bNumEndpoints);
        printk("[USB:interface] bInterfaceClass  = 0x%X\n", intf->cur_altsetting->desc.bInterfaceClass);        

            /* scan, remembering exact match and best match */
            for (nEnd = 1; nEnd < pThis->bEndCount; nEnd++)
            {
                #if 1
                /*0: Rx, 1: Tx*/
                pEnd = &(pThis->aLocalEnd[0][nEnd]);
                aIsLocalEndOccupy[0][nEnd] = pEnd->bIsOccupy;
                pEnd = &(pThis->aLocalEnd[1][nEnd]);  
                aIsLocalEndOccupy[1][nEnd] = pEnd->bIsOccupy;
                #endif

                printk("[USB1] [%d: RX=%d TX= %d]\n", 
                        nEnd, aIsLocalEndOccupy[0][nEnd], aIsLocalEndOccupy[1][nEnd]);
            }

            iface_desc = intf->cur_altsetting;

            for (j = 0; j < iface_desc->desc.bNumEndpoints; ++j) 
            {
                ep_desc = &iface_desc->endpoint[j].desc;
                epnum = usb_endpoint_num(ep_desc);
                is_out = usb_endpoint_dir_out(ep_desc);

                //iso and wifi bulk use q   
                q_expired = mtk_ep_q_expired(dev, 
                      (ep_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK));


                printk("[USB] Type: 0x%02X\n", ep_desc->bDescriptorType);
                printk("[USB] Addr: 0x%02X\n", ep_desc->bEndpointAddress);
                printk("[USB] Attr: 0x%02X\n", ep_desc->bmAttributes);
                printk("[USB] Intr: 0x%02X\n", ep_desc->bInterval);
                printk("[USB] MaxP: 0x%04X\n", ep_desc->wMaxPacketSize);

                /*0: Rx, 1: Tx*/
                nOut = (ep_desc->bEndpointAddress & USB_DIR_IN)? 0 : 1;
                found = 0;
                
                if (is_out) 
                {
                    ep = dev->ep_out[epnum];
                }
                if (!is_out) 
                {
                    ep = dev->ep_in[epnum];
                }
                
#ifdef USB_DISABLE_MSD_FOR_IPOD
                //2009.10.19, for APPLE iPod MSC device
                if(dev->quirks == USB_QUIRK_NOT_SUPPORT_APPLE_MSD)
                {
                    if(iface_desc->desc.bInterfaceClass == USB_CLASS_MASS_STORAGE && 
                       iface_desc->desc.bInterfaceSubClass == 0x06)
                    {   
                        //printk("[USB] Set EP to 0xAA.\n");
                        goto NO_FOUND;
                    }
                }
#endif 
                if(ep && ep->hcpriv)
                {
                    pEnd = (MGC_LinuxLocalEnd *)ep->hcpriv;
                
                    found = 1;
                    continue;
                }
                       
                /* use a reserved one for bulk if any */
                #ifdef PHYEP_MAPPING_MULTI_LOCALEP
                if ((ep_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
                            == USB_ENDPOINT_XFER_BULK)
                {
                    if (iface_desc->desc.bInterfaceClass == USB_CLASS_MASS_STORAGE)
                    {
                        //nEnd = 1;
                        nEnd = pThis->bEndCount -1 ;
                        // use for mass storage device.
                        pEnd = &(pThis->aLocalEnd[nOut][nEnd]);  /*0: Rx, 1: Tx*/        
                        aIsLocalEndOccupy[nOut][nEnd] = TRUE;
                        mtk_alloc_ep_fifo(pThis, pEnd, ep_desc->wMaxPacketSize, !nOut); 
                        pEnd->used_num += 1;
                        pEnd->bIsOccupy = TRUE;
                        ep->hcpriv = (void *)pEnd;
                        pEnd->dev = dev;
                        if(q_expired && pEnd->ep_q_configed)
                            pEnd->ep_q_used = 1;
                        else
                            pEnd->ep_q_used = 0;
                        found = 1;
                        printk("[USB] L_EP [%d: %s=%d] R_EP:0x%2x Queue: %s\n", 
                            nEnd, nOut?"TX":"RX", pEnd->bIsOccupy, ep_desc->bEndpointAddress, pEnd->ep_q_used?"Yes":"No");          
                        continue;
                    }
                }
                #endif
                
            #ifdef PHYEP_MAPPING_MULTI_LOCALEP
                #ifdef UNIFIED_USB
                 if(dev->level == 1 && iface_desc->desc.bInterfaceClass != USB_CLASS_HUB)
				  ep_end = pThis->bEndCount;
				 else
				#endif 	
				  ep_end = pThis->bEndCount - 1;
			#else
                  ep_end = pThis->bEndCount;
			#endif


                for (k = 1; k < ep_end; k++)
                {
           
                    if(q_expired)
                        nEnd = k;
                    else
                        nEnd = ep_end - k;
                    
                    pEnd = &(pThis->aLocalEnd[nOut][nEnd]);
                    wPacketSize = le16_to_cpu(ep_desc->wMaxPacketSize);
                            
                    if (pEnd->bIsOccupy != TRUE)
                       {
                        // Occupy this endpoint.
                        aIsLocalEndOccupy[nOut][nEnd] = TRUE;
                        pEnd->bIsOccupy = TRUE;
                        mtk_alloc_ep_fifo(pThis, pEnd, ep_desc->wMaxPacketSize, !nOut);  
                        pEnd->used_num += 1;
                        ep->hcpriv = (void *)pEnd;
                        pEnd->dev = dev;
                        if(q_expired && pEnd->ep_q_configed)
                            pEnd->ep_q_used = 1;
                        else
                            pEnd->ep_q_used = 0;        
                        
                        printk("[USB] L_EP [%d: %s=%d] R_EP:0x%2x Queue: %s\n", 
                            nEnd, nOut?"TX":"RX", pEnd->bIsOccupy, ep_desc->bEndpointAddress, pEnd->ep_q_used?"Yes":"No");      
                        found = 1;
                        break;
                    }

                }
 
                if(found == 0)
                {
#ifdef USB_DISABLE_MSD_FOR_IPOD
                    NO_FOUND:
#endif
                    //printk("[USB] WARNING: Endpoint not enough!\n");
                    //Set Device Base Class as a miscellaneous
                    //Set InterfaceClass & InterfaceSubClass to 0xEE as an 
                    //unsupport device.
                    if(iface_desc)
                    {
                        ep_desc->bEndpointAddress = 0xEE;
#ifdef USB_DISABLE_MSD_FOR_IPOD
                        if(dev->quirks == USB_QUIRK_NOT_SUPPORT_APPLE_MSD)
                        {
                            ep_desc->bEndpointAddress = 0xAA;
                        }
#endif
                        iface_desc->desc.bInterfaceClass = 0xEE;
                        iface_desc->desc.bInterfaceSubClass = 0xEE;                    
                    }

                    for (nEnd = 1; nEnd < pThis->bEndCount; nEnd++)
                    {
                        printk("[USB2] [%d: RX=%d TX= %d]\n", 
                                nEnd, aIsLocalEndOccupy[0][nEnd], aIsLocalEndOccupy[1][nEnd]);
                    }
                    spin_unlock_irqrestore(&pThis->Lock, flags);
                    return -1;
                }
            }
        }
    }

    for (nEnd = 1; nEnd < pThis->bEndCount; nEnd++)
    {
        printk("[USB3] [%d: RX=%d TX= %d]\n", 
                nEnd, aIsLocalEndOccupy[0][nEnd], aIsLocalEndOccupy[1][nEnd]);
    }
    spin_unlock_irqrestore(&pThis->Lock, flags);
    //printk("[USB] EP is enough.\n");
    return 0;
}

void MGC_CallbackUrb(MGC_LinuxCd * pThis, struct urb *pUrb)
{
    unsigned int flag = URB_NO_TRANSFER_DMA_MAP | URB_DIR_IN;

    if (pUrb->status)
    {
        DBG(1, "Err: urb=0x%p,status=%d\n", pUrb, pUrb->status);
    }

    #if 0
        printk("=======================================\n");
        printk("transfer_flags        : 0x%08X\n",(uint32_t)pUrb->transfer_flags);
        printk("transfer_buffer       : 0x%08X\n",(uint32_t)pUrb->transfer_buffer);
        printk("transfer_dma          : 0x%08X\n",(uint32_t)pUrb->transfer_dma);
        printk("transfer_buffer_length: 0x%08X\n",(uint32_t)pUrb->transfer_buffer_length);
        printk("setup_packet          : 0x%08X\n",(uint32_t)pUrb->setup_packet);
        printk("setup_dma             : 0x%08X\n",(uint32_t)pUrb->setup_dma);
        printk("MGC_GetUrbBuffer      : 0x%08X\n",(uint32_t)MGC_GetUrbBuffer(pUrb));
        
        printk("=======================================\n");
    #endif

    //2010.05.31, Invalid Cache after RX
    if(flag == (pUrb->transfer_flags & flag))
    {
        //uint8_t *pBuf = MGC_GetUrbBuffer(pUrb);
        
        //remove to unload fifo... it's better.
        //if(pUrb->transfer_buffer_length < MGC_HSDMA_MIN_DMA_LEN)
        {
    //        BSP_CleanDCacheRange((uint32_t)pBuf, (uint32_t)pUrb->transfer_buffer_length); 
        }
    }

    usb_hcd_giveback_urb(musbstruct_to_hcd(pThis), pUrb, pUrb->status);

    if (pUrb->status)
    {
        DBG(1, "done completing pUrb=%p\n", pUrb);
    }
}
static void MGC_Trace_Tx_State(MGC_LinuxCd * pThis,struct urb *pUrb)
{
	if(trace_tx_cmd_status_flag =1)//when trace_tx_cmd_flag =1,begain trace
	{
		if(pUrb->status == USB_ST_NORESPONSE)//cmd timeout
		{
			if(no_response_count < 3)//get description will try 3 times in usb core hub.c
			{
				no_response_count++;
			}
			else	// will be reset when the first cmd timeout > 3 times& reset times < 3
			{
				if(reset_port_count < 3)
				{
					reset_port_count++;
					no_response_count = 0;
					usb1_vbus_power_reset();
				}
				else //stop trace state until trace_tx_cmd_flag = 1
				{
					trace_tx_cmd_status_flag = 0;
					no_response_count = 0;
					reset_port_count = 0;
				}
			}
		}
		else//if send cmd state is not timeout,stop trace state. 
		{
			trace_tx_cmd_status_flag = 0;
		}
	}

}

static int MGC_CompleteUrb(MGC_LinuxCd * pThis, MGC_LinuxLocalEnd * pEnd,
                           struct urb *pUrb)
{
    int status;

    DBG(2, "Completing URB=0x%p, on pEnd->bEnd=%d status=%d, proto=%s\n",
        pUrb, pEnd->bEnd, pUrb->status, MGC_DecodeUrbProtocol(pUrb));

    /* prevents locking&kickstarting */
    pEnd->bBusyCompleting = 1;

    // Unlock pEnd to prevent dead lock with user's submit urb in callback.
    spin_unlock(&pEnd->Lock);
#ifdef USB_IRQ_LOCK   
    spin_unlock(&pThis->Lock);
#endif
    //printk("Complete pUrb=0x%X.\n", (uint32_t)pUrb);

    MGC_CallbackUrb(pThis, pUrb);

#ifdef USB_IRQ_LOCK
    spin_lock(&pThis->Lock);
#endif
    // Lock pEnd to prevent race condition with other thread.
    spin_lock(&pEnd->Lock);

    //   check if there is queued urb before kickstart next. 
    status = MGC_IsEndIdle(pEnd);        /* kickstart next */

    /* allows locking&kickstarting again */
    pEnd->bBusyCompleting = 0;
    MGC_Trace_Tx_State(pThis,pUrb);
    return status;
}

static inline uint8_t MGC_GetTransferType(struct urb *pUrb)
{
    uint8_t bStdType = 0;

    const unsigned int nPipe = pUrb->pipe;

    if (usb_pipeisoc(nPipe))
    {
        bStdType = 1;
    }
    else if (usb_pipeint(nPipe))
    {
        bStdType = 3;
    }
    else if (usb_pipebulk(nPipe))
    {
        bStdType = 2;
    }

    return bStdType;
}

static void MGC_SetProtocol(MGC_LinuxCd * pThis, struct urb *pUrb,
                            uint8_t bEnd, unsigned int bXmt)
{
    uint8_t reg;

    uint8_t bStdType = MGC_GetTransferType(pUrb);
    unsigned int nPipe = pUrb->pipe;
    uint8_t *pBase = (uint8_t *) pThis->pRegs;

    reg = (bStdType << 4) | (((uint8_t) usb_pipeendpoint(nPipe)) & 0xf);

    switch (((uint8_t) pUrb->dev->speed))
    {
    case USB_SPEED_LOW:
        reg |= 0xc0;
        break;

    case USB_SPEED_FULL:
        reg |= 0x80;
        break;

    default:
        reg |= 0x40;
    }

    if (bXmt)
    {
        if (bEnd)
        {
            MGC_WriteCsr8(pBase, MGC_O_HDRC_TXTYPE, bEnd, reg);
        }
        else
        {
            MGC_WriteCsr8(pBase, MGC_O_HDRC_TYPE0, 0, reg & 0xc0);
        }
    }
    else
    {
        if (bEnd)
        {
            MGC_WriteCsr8(pBase, MGC_O_HDRC_RXTYPE, bEnd, reg);
        }
        else
        {
            MGC_WriteCsr8(pBase, MGC_O_HDRC_TYPE0, 0, reg & 0xc0);
        }
    }
}

static void MGC_SetAddress(MGC_LinuxCd * pThis, struct urb *pUrb,
                           uint8_t bEnd, unsigned int bXmt)
{
    uint8_t *pBase = (uint8_t *) pThis->pRegs;

    uint8_t bAddress = (uint8_t) usb_pipedevice(pUrb->pipe);
    uint8_t bHubAddr = 0, bHubPort = 0;

    /* NOTE: there is always a parent due to the virtual root hub */
    if (pUrb->dev->parent)
    {
        bHubAddr = (uint8_t) pUrb->dev->parent->devnum;
    }

    if (pUrb->dev->tt)
    {
        if(pUrb->dev->tt->multi)
    {
        bHubAddr |= 0x80;
    bHubPort = pUrb->dev->ttport;
        }
    }

    /* target addr & hub addr/port */
    if (bXmt)
    {
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_TXFUNCADDR),
                   bAddress);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_TXHUBADDR),
                   bHubAddr);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_TXHUBPORT),
                   bHubPort);
    }

    /* also, try Rx (this is a bug ion the core: I always need to to do 
     * both (at least for ep0), needs to be changed when the core is
     * fixed */

#ifdef UNIFIED_USB
    if(!bXmt)
    {
      
       if(!bEnd)
       {
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_TXFUNCADDR), 
            bAddress);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_TXHUBADDR), 
            bHubAddr);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_TXHUBPORT), 
            bHubPort);   
        
       }
       else
       {
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_RXFUNCADDR), 
            bAddress);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_RXHUBADDR), 
            bHubAddr);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_RXHUBPORT), 
            bHubPort);
       }
    }

    #else
    if ((bEnd == 0) ||(!bXmt))
    {
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_RXFUNCADDR),
                   bAddress);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_RXHUBADDR),
                   bHubAddr);
        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_MHDRC_RXHUBPORT),
                   bHubPort);
    }
    #endif

    //dbg(2,"[Yifan TEST]end %d, device %d, parent %d, port %d, multi-tt: %d, speed:%d\n",
    //    bEnd, pUrb->dev->devnum, bHubAddr, bHubPort, bIsMulti,
    //    pUrb->dev->speed);
}

static inline uint8_t *MGC_GetUrbBuffer(struct urb *pUrb)
{
    uint8_t *pBuffer = NULL;

    pBuffer = pUrb->transfer_buffer;

    if (!pBuffer)
    {
        pBuffer = (void *) phys_to_virt(pUrb->transfer_dma);
    }

    return pBuffer;
}

static void MGC_LoadFifo(const uint8_t * pBase, uint8_t bEnd,
                         uint16_t wCount, const uint8_t * pSource)
{
    uint32_t dwDatum = 0;

    uint32_t dwCount = wCount;
    uint8_t bFifoOffset = MGC_FIFO_OFFSET(bEnd);
    uint32_t dwBufSize = 4;

    DBG(2, "pBase=%p, bEnd=%d, wCount=0x%04x, pSrc=%p\n", pBase, bEnd,
        wCount, pSource);

#ifdef MUSB_PARANOID

    if (IS_INVALID_ADDRESS(pSource))
    {
        ERR("loading fifo from a null buffer; why did u do that????\n");

        return;
    }

#endif

    //  do not handle zero length data.
    if (dwCount == 0)
    {
        return;
    }

    /* byte access for unaligned */
    if ((dwCount > 0) && ((uint32_t) pSource & 3))
    {
        while (dwCount)
        {
#ifdef UNIFIED_USB
            if (3 == dwCount || 2 == dwCount)
            {
                dwBufSize = 2;
                MU_MB();
                // set FIFO byte count.
                MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 1);
            }
            else if(1 == dwCount)
            {
                dwBufSize = 1;
                MU_MB();
                // set FIFO byte count.
                MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 0);             
            }
            
            memcpy((void *)&dwDatum, (const void *)pSource, dwBufSize);
            MU_MB();
            MGC_Write32(pBase, bFifoOffset, dwDatum);
            
            dwCount -= dwBufSize;
            pSource += dwBufSize;


        #else
            if (dwCount < 4)
            {
                dwBufSize = dwCount;

                // set FIFO byte count.
                MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, dwCount);
            }

            memcpy((void *) &dwDatum, (const void *) pSource, dwBufSize);

            MGC_Write32(pBase, bFifoOffset, dwDatum);

            dwCount -= dwBufSize;
            pSource += dwBufSize;
        #endif  
        }
    }
    else                        /* word access if aligned */
    {
        while ((dwCount > 3) && !((uint32_t) pSource & 3))
        {
            MU_MB();
            MGC_Write32(pBase, bFifoOffset,
                        *((uint32_t *) ((void *) pSource)));

            pSource += 4;
            dwCount -= 4;
        }
#ifdef UNIFIED_USB
        if (3 == dwCount || 2 == dwCount)
        {
            MUSB_ASSERT(dwCount < 4);

            // set FIFO byte count.
            MU_MB();
            MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 1);

            MU_MB();
            MGC_Write32(pBase, bFifoOffset, *((uint32_t *)((void *)pSource)));
            
            dwCount -= 2;
            pSource += 2;
        }
        
        if(1 == dwCount)
        {
          MU_MB();
          MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 0);
          if((uint32_t)pSource & 3)
          {
            
            memcpy((void *)&dwDatum, (const void *)pSource, 1);
            MU_MB();
            MGC_Write32(pBase, bFifoOffset, dwDatum);           
          }
          else
          {
            MU_MB();
            MGC_Write32(pBase, bFifoOffset, *((uint32_t *)((void *)pSource)));          
          }
        }

#else

        if (dwCount > 0)
        {
            // set FIFO byte count.
            MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, dwCount);

            MGC_Write32(pBase, bFifoOffset,
                        *((uint32_t *) ((void *) pSource)));
        }
#endif      
    }

        MU_MB();

#ifdef UNIFIED_USB
        MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 2);
#else 
        MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 4);
#endif
        MU_MB();


    return;
}

void MGC_UnloadFifo(const uint8_t * pBase, uint8_t bEnd, uint16_t wCount,
                    uint8_t * pDest)
{
    uint32_t dwDatum = 0;

    uint32_t dwCount = wCount;
    uint8_t bFifoOffset = MGC_FIFO_OFFSET(bEnd);
    uint32_t i;

    DBG(2, "pBase=%p, bEnd=%d, wCount=0x%04x, pDest=%p\n", pBase, bEnd,
        wCount, pDest);

    //  do not handle zero length data.
    if (dwCount == 0)
    {
        return;
    }

    if (((uint32_t) pDest) & 3)
    {
        /* byte access for unaligned */
        while (dwCount > 0)
        {
            if (dwCount < 4)
            {
#ifdef  UNIFIED_USB
               if(3 == dwCount || 2 == dwCount)
               {
                   MU_MB();
                   MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 1);
                   MU_MB();
                   dwDatum = MGC_Read32(pBase, bFifoOffset);
                   
                   for (i = 0; i < 2; i++)
                   {
                       *pDest++ = ((dwDatum >> (i *8)) & 0xFF);
                   }
                   
                   dwCount -=2;
              }

              if(1 == dwCount) 
              {
                MU_MB();
                MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 0);
                MU_MB();
                dwDatum = MGC_Read32(pBase, bFifoOffset);
                *pDest++ = (dwDatum  & 0xFF);
                 dwCount -= 1;
              }
               
               MU_MB();
               // set FIFO byte count = 4 bytes.
               MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 2);
               MU_MB();
               
               dwCount = 0;

   #else            
                // set FIFO byte count.
                MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, dwCount);

                dwDatum = MGC_Read32(pBase, bFifoOffset);

                for (i = 0; i < dwCount; i++)
                {
                    *pDest++ = ((dwDatum >> (i * 8)) & 0xFF);
                }

                // set FIFO byte count = 4 bytes.
                MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 4);
                dwCount = 0;
    #endif          
            }
            else
            {
                MU_MB();
                dwDatum = MGC_Read32(pBase, bFifoOffset);

                for (i = 0; i < 4; i++)
                {
                    *pDest++ = ((dwDatum >> (i * 8)) & 0xFF);
                }

                dwCount -= 4;
            }
        }
    }
    else
    {
        /* word access if aligned */
        while (dwCount >= 4)
        {
            MU_MB();
            *((uint32_t *) ((void *) pDest)) =
                MGC_Read32(pBase, bFifoOffset);

            pDest += 4;
            dwCount -= 4;
        }

        if (dwCount > 0)
        {
#ifdef UNIFIED_USB
            if(3 == dwCount ||2 == dwCount )
            {
               MU_MB();
               MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 1);
               MU_MB();
               dwDatum = MGC_Read32(pBase, bFifoOffset);
                for (i = 0; i < 2; i++)
                {
                    *pDest++ = ((dwDatum >> (i *8)) & 0xFF);
                }
                dwCount -= 2;
            }

            if(1 == dwCount)
            {
               MU_MB();
               MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT,0);
               MU_MB();
               dwDatum = MGC_Read32(pBase, bFifoOffset);
               
                *pDest++ = (dwDatum & 0xFF);
                dwCount -= 1;              
            }
            
            // set FIFO byte count = 4 bytes.
            MU_MB();
            MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 2);
            MU_MB();


        #else        
            // set FIFO byte count.
            MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, dwCount);

            dwDatum = MGC_Read32(pBase, bFifoOffset);

            for (i = 0; i < dwCount; i++)
            {
                *pDest++ = ((dwDatum >> (i * 8)) & 0xFF);
            }

            // set FIFO byte count = 4 bytes.
            MGC_FIFO_CNT(pBase, M_REG_FIFOBYTECNT, 4);
        #endif  
        }
    }
    MU_MB();

    return;
}


static uint32_t MGC_Log2(uint32_t x)
{
    uint32_t i;

    for (i = 0; x > 1; i++)
    {
        x = x / 2;
    }

    return i;
}

void MGC_CleanDcache(struct urb* urb,uint32_t start,uint32_t len)
{
    uint32_t phy;
    if(!(urb->transfer_flags & URB_NO_TRANSFER_DMA_MAP) && virt_addr_valid(start))
    {  
       phy = dma_map_single(NULL,(void*)start, len, DMA_TO_DEVICE);
       dma_unmap_single(NULL,phy, len, DMA_TO_DEVICE);
             
    }
}

static void MGC_SetInterval(MGC_LinuxCd * pThis, struct urb *pUrb,
                            uint8_t bEnd)
{
    uint8_t *pBase = (uint8_t *) pThis->pRegs;

    unsigned int nPipe = pUrb->pipe;
    uint8_t bInterval = 0;
    uint8_t bIsMsc = 0;

    if (usb_pipeint(nPipe))
    {
        if (pThis->bRootSpeed == 1)        /* high speed */
        {
            bInterval = MGC_Log2(pUrb->interval) + 1;
        }
        else                        /* full or low speed */
        {
            bInterval = pUrb->interval;
        }
    }
    else if (usb_pipeisoc(nPipe))
    {
        if (pThis->bRootSpeed == 1)        /* high speed */
        {
            bInterval = MGC_Log2(pUrb->interval) + 1;
        }
        else if (pThis->bRootSpeed == 2)        /* full speed */
        {
            bInterval = MGC_Log2(pUrb->interval) + 1;
        }
    }
    else if (usb_pipebulk(nPipe) && (pUrb->interval >= 0))
    {
        struct usb_interface *intf = NULL;
        struct usb_host_interface *iface_desc = NULL;
        intf = pUrb->dev->config->interface[0];
            
        if(intf)
        {
            iface_desc = intf->cur_altsetting;

            if(iface_desc)
        {
                //printk("[USB DBG] bInterfaceClass: %d\n", 
                //        iface_desc->desc.bInterfaceClass);
                 /* Check whether is Mass Storage CLASS or not? */
                if(iface_desc->desc.bInterfaceClass == USB_CLASS_MASS_STORAGE && 
                   iface_desc->desc.bInterfaceSubClass == 0x06)
            {
                    //printk("[USB DBG] MSD Device\n");
                    bIsMsc = 1;
                }
            }
            }

        if (pThis->bRootSpeed == 1)        /* high speed */
        {
            if(bIsMsc)
            {
            /*
              *for some card reader with ZERO interval in it's EP descriptor
              *also turn on nak limit when process bulk transfer
              */
                bInterval = 14; //about 1s nak timeout
            }
            else if (pUrb->interval > 4096)
            {
                bInterval = 16;
            }
            else if(pUrb->interval > 0)
            {
                bInterval = MGC_Log2(pUrb->interval) + 1;
            }
        }
        else if (pThis->bRootSpeed == 2)        /* full speed */
        {
            if(bIsMsc)
            {
                bInterval = 11;
            }
            else if (pUrb->interval > 32768)
            {
                bInterval = 16;
            }
            else if(pUrb->interval > 0)
            {
                bInterval = MGC_Log2(pUrb->interval) + 1;
            }
        }
    }

    if (usb_pipeout(nPipe))
    {
      if(MGC_ReadCsr8(pBase, MGC_O_HDRC_TXINTERVAL, bEnd) != bInterval){
        MGC_WriteCsr8(pBase, MGC_O_HDRC_TXINTERVAL, bEnd, bInterval);
    }
    }
    else
    {
      if(MGC_ReadCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd) != bInterval){
        MGC_WriteCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd, bInterval);
        }
    }
}

/**
 * used ONLY in host mode, I'll be moved to musb_host
 * @param pPrivateData
 * @param bLocalEnd
 * @param bTransmit
 */
static uint8_t MGC_HsDmaChannelStatusChanged(void *pPrivateData,
                                             uint8_t bLocalEnd,
                                             uint8_t bTransmit)
{
    MGC_LinuxCd *pThis = (MGC_LinuxCd *) pPrivateData;

    if (!bLocalEnd)
    {
        MGC_ServiceDefaultEnd(pThis);
    }
    else
    {
        /* endpoints 1..15 */
        if (bTransmit)
        {
            MGC_ServiceTxAvail(pThis, bLocalEnd);
        }
        else
        {
            /* receive */
            MGC_ServiceRxReady(pThis, bLocalEnd);
        }
    }

    return TRUE;
}

static MGC_DmaChannel *MGC_HsDmaAllocateChannel(void *pPrivateData,
                                                uint8_t bLocalEnd,
                                                uint8_t bTransmit,
                                                uint8_t bProtocol,
                                                uint16_t wMaxPacketSize)
{
    uint8_t bBit;

    MGC_DmaChannel *pChannel = NULL;
    MGC_HsDmaChannel *pImplChannel = NULL;
    MGC_HsDmaController *pController =
        (MGC_HsDmaController *) pPrivateData;

#ifdef UNIFIED_USB

    for (bBit = 0; bBit < pController->bChannelCount; bBit++)

#else
    for (bBit = 0; bBit < MGC_HSDMA_CHANNELS; bBit++)
#endif      
    {
        if (!(pController->bmUsedChannels & (1 << bBit)))
        {
            pController->bmUsedChannels |= (1 << bBit);

            pImplChannel = &(pController->aChannel[bBit]);
            pImplChannel->pController = pController;
            pImplChannel->wMaxPacketSize = wMaxPacketSize;
            pImplChannel->bIndex = bBit;
            pImplChannel->bEnd = bLocalEnd;
            pImplChannel->bProtocol = bProtocol;
            pImplChannel->bTransmit = bTransmit;
            pChannel = &(pImplChannel->Channel);
            pChannel->pPrivateData = pImplChannel;
            pChannel->bStatus = MGC_DMA_STATUS_FREE;
            //dexiao, porting from nuleus code. 
            //pChannel->dwMaxLength = 0x10000;
            pChannel->dwMaxLength = 0xffffffff;
            /* Tx => mode 1; Rx => mode 0 */
            pChannel->bDesiredMode = bTransmit;
            break;
        }
    }
    return pChannel;
}

static void MGC_HsDmaReleaseChannel(MGC_DmaChannel * pChannel)
{
    MGC_HsDmaChannel *pImplChannel =
        (MGC_HsDmaChannel *) pChannel->pPrivateData;

    MGC_HsDmaController *pController = pImplChannel->pController;
    uint8_t *pBase = pController->pCoreBase;
    uint8_t bChannel = pImplChannel->bIndex;
    uint16_t wCsr = 0;
#ifdef UNIFIED_USB
    uint8_t bIntr = MGC_Read8(pBase, MGC_O_HSDMA_INTR); 
    
    
    if (bIntr & (1 << bChannel))
    {
        //  write clear interrupt register value.
        MGC_Write8(pBase, MGC_O_HSDMA_INTR, (1 << bChannel));
    }

#else
    uint32_t dwIntr = MGC_Read32(pBase, MGC_O_HSDMA_INTR); 


    if (dwIntr & (1 << bChannel))
    {
        //  write clear interrupt register value.
        MGC_Write32(pBase, MGC_O_HSDMA_INTR, (1 << bChannel));
    }
#endif

    MGC_Write32(pBase, 
        MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_CONTROL), 0);

    // clear TX/RX CSR register.
    if (pImplChannel->bTransmit)
    {
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd);

        if (wCsr &
            (MGC_M_TXCSR_AUTOSET | MGC_M_TXCSR_DMAENAB |
             MGC_M_TXCSR_DMAMODE))
        {
            wCsr &=
                ~(MGC_M_TXCSR_AUTOSET | MGC_M_TXCSR_DMAENAB );
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd, wCsr);

            wCsr &= ~MGC_M_TXCSR_DMAMODE;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd, wCsr);
        }
    }
    else
    {
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, pImplChannel->bEnd);

        if (wCsr &
            (MGC_M_RXCSR_AUTOCLEAR | MGC_M_RXCSR_H_AUTOREQ |
             MGC_M_RXCSR_DMAENAB | MGC_M_RXCSR_DMAMODE))
        {
            wCsr &=
                ~(MGC_M_RXCSR_AUTOCLEAR | MGC_M_RXCSR_H_AUTOREQ |
                  MGC_M_RXCSR_DMAENAB | MGC_M_RXCSR_DMAMODE);

            MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, pImplChannel->bEnd, wCsr);
        }
    }

    pImplChannel->pController->bmUsedChannels &= ~(1 << pImplChannel->bIndex);
    pImplChannel->Channel.bStatus = MGC_DMA_STATUS_FREE;
}

static uint8_t MGC_HsDmaProgramChannel(MGC_DmaChannel * pChannel,
                                       uint16_t wPacketSize, uint8_t bMode,
                                       const uint8_t * pBuffer,
                                       uint32_t dwLength)
{
    MGC_HsDmaChannel *pImplChannel =
        (MGC_HsDmaChannel *) pChannel->pPrivateData;

    MGC_HsDmaController *pController = pImplChannel->pController;
    uint8_t *pBase = pController->pCoreBase;
    uint32_t dwCsr = 
        (pImplChannel->bEnd << MGC_S_HSDMA_ENDPOINT) | (1 << MGC_S_HSDMA_ENABLE) | MGC_M_HSDMA_BURSTMODE;
    uint8_t bChannel = pImplChannel->bIndex;
    uint16_t wCsr = 0;

    /* reject below threshold (packet size) */
    if (dwLength < MGC_HSDMA_MIN_DMA_LEN)
    {
        return FALSE;
    }

    if (bMode)
    {
        dwCsr |= 1 << MGC_S_HSDMA_MODE1;
    }

    if (pImplChannel->bTransmit)
    {
        //  prevent client task and USB HISR race condition, set csr in MGC_HsDmaProgramChannel().
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd);

        if (bMode)
        {
            wCsr |=
                (MGC_M_TXCSR_AUTOSET | MGC_M_TXCSR_DMAENAB |
                 MGC_M_TXCSR_DMAMODE);
        }
        else
        {
            wCsr &=
                ~(MGC_M_TXCSR_AUTOSET | MGC_M_TXCSR_DMAENAB |
                  MGC_M_TXCSR_DMAMODE);
        }
#ifdef UNIFIED_USB
    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd,
               wCsr);

#else
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd,
                       wCsr | MGC_M_TXCSR_MODE);
#endif

        dwCsr |= 1 << MGC_S_HSDMA_TRANSMIT;
    }
    else
    {
        if (bMode)
        {
            wCsr =
                MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, pImplChannel->bEnd);

            wCsr |= (MGC_M_RXCSR_AUTOCLEAR | MGC_M_RXCSR_DMAENAB);
            
            if(0 == pImplChannel->wMaxPacketSize)
            {
                printk("unexpect error,packsize is zero!\n");
                return FALSE;
            }
            //  Request the actual number of packet to be received.
            MGC_DMA_Write32(pBase, M_REG_REQPKT(pImplChannel->bEnd),
                            (((dwLength + pImplChannel->wMaxPacketSize) -
                              1) / pImplChannel->wMaxPacketSize));

            wCsr &= ~MGC_M_RXCSR_RXPKTRDY;
            // host use MGC_M_RXCSR_DMAMODE.
            wCsr |=
                (MGC_M_RXCSR_H_AUTOREQ | MGC_M_RXCSR_DMAMODE |
                 MGC_M_RXCSR_H_REQPKT);
        }
    }

    dwCsr |= 1 << MGC_S_HSDMA_IRQENABLE;

    /* address/count */
    MGC_Write32(pBase,
                MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_ADDRESS),
                (uint32_t) pBuffer);
    MGC_Write32(pBase,
                MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_COUNT),
                dwLength);

    /* control (this should start things) */
    pChannel->dwActualLength = 0L;
    pImplChannel->dwStartAddress = (uint32_t) pBuffer;
    pImplChannel->dwCount = dwLength;

    MU_MB();
    MGC_Write32(pBase,
                MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_CONTROL),
                dwCsr);


    if (!pImplChannel->bTransmit)
    {
/*
    Note: 
        RxCSR should be set after DMA is configured. 
        This can prevent race condition between setuping DMA and data entering fifo.
*/   
        MU_MB();
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, pImplChannel->bEnd, wCsr);
    }

    return TRUE;
}

static MGC_DmaChannelStatus MGC_HsDmaGetChannelStatus(MGC_DmaChannel *
                                                      pChannel)
{
    uint32_t dwAddress;

    MGC_HsDmaChannel *pImplChannel =
        (MGC_HsDmaChannel *) pChannel->pPrivateData;
    MGC_HsDmaController *pController = pImplChannel->pController;
    uint8_t *pBase = pController->pCoreBase;
    uint8_t bChannel = pImplChannel->bIndex;
    uint32_t dwCsr = MGC_Read32(pBase,
                                MGC_HSDMA_CHANNEL_OFFSET(bChannel,
                                                         MGC_O_HSDMA_CONTROL));
    uint32_t dwCsrMask;

    if (dwCsr & (1 << MGC_S_HSDMA_BUSERROR))
    {
        return MGC_DMA_STATUS_BUS_ABORT;
    }

    //  handle last short packet in multiple packet DMA RX mode 1.  
    dwCsrMask = (1 << MGC_S_HSDMA_ENABLE) | (1 << MGC_S_HSDMA_MODE1) | 
        (1 << MGC_S_HSDMA_IRQENABLE);

    if ((dwCsr & dwCsrMask) == dwCsrMask)
    {
        /* most DMA controllers would update the count register for simplicity... */
        dwAddress =
            MGC_Read32(pBase,
                       MGC_HSDMA_CHANNEL_OFFSET(bChannel,
                                                MGC_O_HSDMA_ADDRESS));

        pImplChannel->Channel.dwActualLength =
            dwAddress - pImplChannel->dwStartAddress;

        MGC_Write32(pBase,
                    MGC_HSDMA_CHANNEL_OFFSET(bChannel,
                                             MGC_O_HSDMA_CONTROL), 0);
        return MGC_DMA_STATUS_MODE1_SHORTPKT;
    }

    /* most DMA controllers would update the count register for simplicity... */
    dwAddress =
        MGC_Read32(pBase,
                   MGC_HSDMA_CHANNEL_OFFSET(bChannel,
                                            MGC_O_HSDMA_ADDRESS));

    if (dwAddress < (pImplChannel->dwStartAddress + pImplChannel->dwCount))
    {
        return MGC_DMA_STATUS_BUSY;
    }

    return MGC_DMA_STATUS_FREE;
}

static uint8_t MGC_HsDmaControllerIsr(void *pPrivateData)
{
    uint8_t bChannel;

    uint32_t dwCsr;
    uint32_t dwAddress;
    MGC_HsDmaChannel *pImplChannel;
    MGC_HsDmaController *pController =
        (MGC_HsDmaController *) pPrivateData;
    uint8_t *pBase = pController->pCoreBase;
#ifdef UNIFIED_USB
    uint8_t bIntr,bIntrMask;
#else
    uint32_t dwIntr;
#endif

    uint8_t bEnd;

#ifdef UNIFIED_USB
    bIntr = MGC_Read8(pBase, MGC_O_HSDMA_INTR);
    bIntrMask = MGC_Read8(pBase, MGC_O_HSDMA_INTR_MASK);
    bIntr = (bIntr&bIntrMask);
#else

    dwIntr = MGC_Read32(pBase, MGC_O_HSDMA_INTR);
#endif

#ifdef UNIFIED_USB
        if (!bIntr)
#else
        if (!dwIntr)
#endif  

    {
        return FALSE;
    }

    //  write clear interrupt register value.
#ifdef UNIFIED_USB
    MGC_Write8(pBase, MGC_O_HSDMA_INTR, bIntr);  
#else
    MGC_Write32(pBase, MGC_O_HSDMA_INTR, dwIntr);    
#endif

#ifdef UNIFIED_USB
    for (bChannel = 0; bChannel < pController->bChannelCount; bChannel++)

#else
    for (bChannel = 0; bChannel < MGC_HSDMA_CHANNELS; bChannel++)
#endif      
    {
    #ifdef UNIFIED_USB
        if ((bIntr & (1 << bChannel)) == 0)

    #else
        if ((dwIntr & (1 << bChannel)) == 0)
    #endif      
            continue;

        pImplChannel = (MGC_HsDmaChannel *) & (pController->aChannel[bChannel]);
        dwCsr = MGC_Read32(pBase, 
            MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_CONTROL));

        if (dwCsr & (1 << MGC_S_HSDMA_BUSERROR))
        {
            pImplChannel->Channel.bStatus = MGC_DMA_STATUS_BUS_ABORT;
        }
        else
        {
            /* most DMA controllers would update the count register for simplicity... */
            dwAddress = MGC_Read32(pBase, 
                MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_ADDRESS));

            pImplChannel->Channel.bStatus = MGC_DMA_STATUS_FREE;
            pImplChannel->Channel.dwActualLength =
                dwAddress - pImplChannel->dwStartAddress;

            if (pImplChannel->bTransmit)
            {
                //  send last short packet in multiple packet transfer and tx single packet.
                if ((pImplChannel->dwCount % pImplChannel->wMaxPacketSize)
                    || (pImplChannel->dwCount <=
                        pImplChannel->wMaxPacketSize))
                {
                    bEnd = pImplChannel->bEnd;

                    MGC_SelectEnd(pBase, bEnd);

                    if (bEnd)
                    {
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                                       MGC_M_TXCSR_TXPKTRDY);
                    }
                    else
                    {
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0,
                                       MGC_M_CSR0_H_NO_PING |
                                       MGC_M_CSR0_H_SETUPPKT |
                                       MGC_M_CSR0_TXPKTRDY);
                    }

                    continue;
                }
            }

            pController->pfDmaChannelStatusChanged(pController->pDmaPrivate,
                pImplChannel->bEnd, pImplChannel->bTransmit);
        }
    }

    return TRUE;
}

static uint8_t MGC_TxPacket(MGC_LinuxCd * pThis, MGC_LinuxLocalEnd *pEnd)
{
    uint16_t wLength = 0;
    uint8_t bDone = FALSE;
    uint8_t *pBase;
    struct urb *pUrb;
    uint8_t *pBuffer;
    int nPipe;
    uint8_t bEnd;
    int i;
    struct usb_iso_packet_descriptor *packet;
    MGC_DmaChannel* pDmaChannel;
    uint8_t* pDmaBuffer;
    uint8_t bDmaOk;

    if ((!pThis) ||(!pEnd))
    {
        return TRUE;    
    }

    pBase = (uint8_t *) pThis->pRegs;
    pUrb = MGC_GetCurrentUrb(pEnd);
    if (!pUrb)
    {
        return TRUE;
    }    
    nPipe = pUrb ? pUrb->pipe : 0;    
    bEnd = pEnd->bEnd;
    pBuffer = MGC_GetUrbBuffer(pUrb);

    DBG(2, "<== bEnd=%d\n", bEnd);

    if (!pBuffer)
    {                                // abort the transfer
        ERR("***> no buffer was given, BAD things are happening (TM)!\n");

        return TRUE;
    }

    /* see if more transactions are needed */
    if (pEnd->pDmaChannel)
    {
        if (MGC_DMA_STATUS_FREE ==
            pThis->pDmaController->pfDmaGetChannelStatus(pEnd->pDmaChannel))
        {
            pEnd->dwOffset += pEnd->dwRequestSize;
            
            if (usb_pipeisoc(nPipe))
            {                              
                packet = &pUrb->iso_frame_desc[pEnd->dwIsoPacket];
                if (pEnd->dwRequestSize >= packet->length)
                {
                    pEnd->dwIsoPacket ++;
                }
                
                if (pEnd->dwIsoPacket < pUrb->number_of_packets)
                {
                    pDmaBuffer = (uint8_t *)pUrb->transfer_dma;                
                    pDmaBuffer += pUrb->iso_frame_desc[pEnd->dwIsoPacket].offset;
                    wLength = pUrb->iso_frame_desc[pEnd->dwIsoPacket].length;

                    pDmaChannel = pEnd->pDmaChannel;
                    if(pDmaChannel) 
                    {
                        // Iso is always single packet to send.
                        pDmaChannel->bDesiredMode = 0;
                        pDmaChannel->dwActualLength = 0L;
                        pEnd->dwRequestSize = min((uint32_t)wLength, pDmaChannel->dwMaxLength);
                        bDmaOk = pThis->pDmaController->pfDmaProgramChannel(pDmaChannel, 
                            pEnd->wPacketSize, pDmaChannel->bDesiredMode, pDmaBuffer, 
                            pEnd->dwRequestSize);
                        if(!bDmaOk) 
                        {        
                            pThis->pDmaController->pfDmaReleaseChannel(pDmaChannel);
                            pEnd->pDmaChannel = NULL;
                            pEnd->dwRequestSize = 0;
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                }    
                else
                {
                    for (i = 0; i < pUrb->number_of_packets; i++) 
                    {
                        packet = &pUrb->iso_frame_desc[i];
                        packet->status = 0;
                        packet->actual_length = packet->length;
                    }                    
                    // Urb finish.
                    pUrb->status=0;                            
                    return TRUE;
                }
            }
        }
    }
    else
    {
        pEnd->dwOffset += pEnd->dwRequestSize;
    }

    if (usb_pipeisoc(nPipe))
    {
        /* isoch case */
        if (pEnd->dwIsoPacket >= pUrb->number_of_packets)
        {
            for (i = 0; i < pUrb->number_of_packets; i++) 
            {
                packet = &pUrb->iso_frame_desc[i];
                packet->status = 0;
                packet->actual_length = packet->length;
            }
        
            bDone = TRUE;
        }
        else
        {
            pBuffer += pUrb->iso_frame_desc[pEnd->dwIsoPacket].offset;
            wLength = pUrb->iso_frame_desc[pEnd->dwIsoPacket].length;
            pEnd->dwIsoPacket ++;
        }
    }
    else
    {
        pBuffer += pEnd->dwOffset;

        wLength = min(pEnd->wPacketSize, 
            (uint16_t) (pUrb->transfer_buffer_length -pEnd->dwOffset));

        if (pEnd->dwOffset >= pUrb->transfer_buffer_length)
        {
            /* sent everything; see if we need to send a null */
            if ((pUrb->transfer_flags & USB_ZERO_PACKET) &&
                (pEnd->dwRequestSize > 0) && 
                ((pEnd->dwRequestSize % pEnd->wPacketSize) == 0))
            {
                // send null packet.
                pEnd->dwRequestSize = 0;
                bDone = FALSE;
            }
            else
            {
                bDone = TRUE;
            }
        }
    }

    if (bDone)
    {
        pUrb->status = 0;
    }
    else if (wLength)
    {                                /* @assert bDone && !wLength */
        MGC_LoadFifo(pBase, bEnd, wLength, pBuffer);

        pEnd->dwRequestSize = wLength;
    }

    return bDone;
}

static uint8_t MGC_RxPacket(MGC_LinuxCd * pThis, 
    MGC_LinuxLocalEnd *pEnd, uint16_t wRxCount)
{
    uint16_t wLength;
    uint8_t bDone = FALSE;
    uint8_t *pBase;
    uint8_t bEnd;
    struct urb *pUrb;
    uint8_t *pBuffer;
    int nPipe;
    uint16_t wPacketSize;
    uint16_t wCsr;
 //   uint8_t bStdType;
  //  uint8_t bDmaOk = FALSE;
  //  MGC_DmaChannel *pDmaChannel;
  //  MGC_DmaController *pDmaController;
  //  uint8_t *pDmaBuffer;

    if ((!pThis) ||(!pEnd))
    {
        return TRUE;    
    }

    pBase = (uint8_t *) pThis->pRegs;
    pUrb = MGC_GetCurrentUrb(pEnd);
    if (!pUrb)
    {
        return TRUE;
    }    
    nPipe = pUrb ? pUrb->pipe : 0;    
    bEnd = pEnd->bEnd;
    pBuffer = MGC_GetUrbBuffer(pUrb);

    DBG(2, "<== end %d RxCount=%04x\n", bEnd, wRxCount);

    DBG(3, "bEnd=%d, pUrb->transfer_flags=0x%x pUrb->transfer_buffer=%p\n",
        bEnd, pUrb->transfer_flags, pUrb->transfer_buffer);
    DBG(3,
        "pUrb->transfer_buffer_length=%d, pEnd->dwOffset=%d, wRxCount=%d\n",
        pUrb->transfer_buffer_length, pEnd->dwOffset, wRxCount);

    if (!pBuffer)
    {                                // abort the transfer
        ERR("***> pBuffer=NULL, BAD things are happening (TM)!\n");
        return TRUE;
    }

    /* unload FIFO */
    if (usb_pipeisoc(nPipe))
    {
        //printk("callback frames %d\n",frames);
        /* isoch case */
        pBuffer += pUrb->iso_frame_desc[pEnd->dwIsoPacket].offset;
        wLength =
            min((unsigned int) wRxCount,
                pUrb->iso_frame_desc[pEnd->dwIsoPacket].length);
        pUrb->actual_length += wLength;

        /* update actual & status */
        pUrb->iso_frame_desc[pEnd->dwIsoPacket].actual_length = wLength;
        pUrb->iso_frame_desc[pEnd->dwIsoPacket].status = USB_ST_NOERROR;

        /* see if we are done */
        bDone = (++pEnd->dwIsoPacket >= pUrb->number_of_packets);

        DBG(3,
            "pEnd->dwIsoPacket=%d, pUrb->number_of_packets=%d, wLength=%d\n",
            pEnd->dwIsoPacket, pUrb->number_of_packets, wLength);

        if (wLength)
        {
            MGC_UnloadFifo(pBase, bEnd, wLength, pBuffer);
            MGC_CleanDcache(pUrb,(uint32_t)pBuffer, (uint32_t)wLength);
        }

        if (bEnd && bDone)
        {
            pUrb->status = 0;
        }
        
        if(pEnd->dev && MGC_IsMFIDevice(&pEnd->dev->descriptor,pEnd->dev)) {
            MGC_Write16(pBase,MGC_END_OFFSET(bEnd, MGC_O_HDRC_RXCSR),MGC_M_RXCSR_H_REQPKT);
            wCsr = MGC_Read16(pBase,MGC_END_OFFSET(bEnd, MGC_O_HDRC_RXCSR));
            if(!(wCsr & MGC_M_RXCSR_H_REQPKT)) {
                MGC_Write16(pBase,MGC_END_OFFSET(bEnd, MGC_O_HDRC_RXCSR),MGC_M_RXCSR_H_FLUSHFIFO);
                MGC_Write16(pBase,MGC_END_OFFSET(bEnd, MGC_O_HDRC_RXCSR),MGC_M_RXCSR_H_REQPKT);
            }
        }
 
  #if 1      
        if(pEnd->dev)
        {
            if(MGC_IsUvcDevice(&pEnd->dev->descriptor,pEnd->dev) || 
               (MGC_IsUacDevice(&pEnd->dev->descriptor,pEnd->dev) && !MGC_IsMFIDevice(&pEnd->dev->descriptor,pEnd->dev)))

            {
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, MGC_M_RXCSR_H_REQPKT);                
            }
        }
  #endif
        return 1;
    }
    else
    {
        DBG(3,
            "(bEnd=%d), wRxCount=%d, pUrb->transfer_buffer_length=%d, pEnd->dwOffset=%d, pEnd->wPacketSize=%d\n",
            bEnd, wRxCount, pUrb->transfer_buffer_length, pEnd->dwOffset,
            pEnd->wPacketSize);

        /* non-isoch */
        pBuffer += pEnd->dwOffset;

        wLength =
            min((unsigned int) wRxCount,
                pUrb->transfer_buffer_length - pEnd->dwOffset);

        wPacketSize = usb_maxpacket(pUrb->dev, pUrb->pipe, FALSE);
        //dexiao,remove, if urb don't get dma channel,  never use dma for this urb.
        if (wLength > 0)
        {
            pUrb->actual_length += wLength;
            pEnd->dwOffset += wLength;

            MGC_UnloadFifo(pBase, bEnd, wLength, pBuffer);
            MGC_CleanDcache(pUrb,(uint32_t)pBuffer, (uint32_t)wLength);
        }
    }


    /* see if we are done */
    bDone = (pEnd->dwOffset >= pUrb->transfer_buffer_length)
        || (wRxCount < pEnd->wPacketSize);

    MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, 0);

    if (!bDone)
    {                   
        MU_MB();
        /* test for short packet */
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                       MGC_M_RXCSR_H_REQPKT);
    }

    if (bEnd && bDone)
    {
        pUrb->status = 0;
    }

    DBG(2, "==> bDone=%d\n", bDone);
    return bDone;
}

static void MGC_ProgramEnd(MGC_LinuxCd * pThis, MGC_LinuxLocalEnd *pEnd,
                           struct urb *pUrb, uint8_t bXmt, uint8_t * pBuffer, uint32_t dwLength)
{
    uint16_t wIntrTxE;
    uint32_t dwIntrTxE,dwIntrRxE;
    uint8_t *pBase = (uint8_t *) pThis->pRegs;
    unsigned int nPipe = pUrb->pipe;
    uint16_t wPacketSize = usb_maxpacket(pUrb->dev, nPipe, usb_pipeout(nPipe));
    uint8_t bInterval = 0;
    uint8_t bEnd = pEnd->bEnd;
    uint8_t bDmaOk = FALSE;
    MGC_DmaChannel *pDmaChannel;
    MGC_DmaController *pDmaController;
    uint16_t wFrame;
    uint16_t wCount = 0;
#ifdef UNIFIED_USB
    uint16_t wCsr = 0;

#else
    uint16_t wCsr = MGC_M_TXCSR_MODE;
#endif
    
    MGC_SelectEnd(pBase, bEnd);  
    
    if(usb_pipeisoc(pUrb->pipe) && !usb_pipeout(pUrb->pipe))
    {
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
        if (wCsr & MGC_M_RXCSR_H_REQPKT)
        {
            return;
        }    
        else
        {   
            bInterval = MGC_ReadCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd);
#if(1)
            if(4 == bInterval && 0x05AC == pEnd->dev->descriptor.idVendor &&  0x12 == (pEnd->dev->descriptor.idProduct >> 8))
            {
                //printk("sof %d\n",wFrame);
                //MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, MGC_M_RXCSR_H_REQPKT);//for iPod isoch rx
                //return;
                goto req_pkt_directly;
            }  
#endif     
        }       
    }

       if(usb_pipeint(pUrb->pipe) && !usb_pipeout(pUrb->pipe)) 
    {
        bInterval = MGC_ReadCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd);
        if(bInterval)
        {
            //MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, MGC_M_RXCSR_H_REQPKT);//for iPod isoch rx
            //return;
            goto req_pkt_directly;
        }
    }

    // setup packet in control pipe is always out.
    if (usb_pipecontrol(nPipe))
    {
        bXmt = TRUE;
        //unmap control
        //unmap_urb_for_dma(musbstruct_to_hcd(pThis),pUrb);
    }
   

    MGC_SetProtocol(pThis, pUrb, bEnd, bXmt);
    MGC_SetAddress(pThis, pUrb, bEnd, bXmt);

    if (bXmt)
    {                                /* transmit */
        //  This disable interrupt will clear INTRRX interrupt event. Because our 32 bits register access mode.
        /* disable interrupt in case we flush */
        wIntrTxE = MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);

        //MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE & ~(1 << bEnd));
        dwIntrTxE = MGC_Read32(pBase, MGC_O_HDRC_INTRRX);
        dwIntrTxE &= (wIntrTxE & ~(uint16_t) (1 << bEnd)) << 16;
        MGC_Write32(pBase, MGC_O_HDRC_INTRRX, dwIntrTxE);
        
        if (bEnd)
        {
            /* twice in case of double packet buffering */
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,MGC_M_TXCSR_CLRDATATOG);
        
            // get data toggle bit from logical ep.
            if (usb_gettoggle(pUrb->dev, pEnd->bRemoteEnd, 1))
            {
                // set data toggle bit to physical ep.
#ifdef UNIFIED_USB
        MU_MB();
                MGC_Write32(pBase, M_REG_TXDATATOG,
                                 M_REG_SET_DATATOG(bEnd, 1));
            MU_MB();
        //unified usb patch, dexiao
        MGC_Write32(pBase, M_REG_TXDATATOG,(1 << bEnd));

#else
                MGC_MISC_Write32(pBase, M_REG_TXDATATOG,
                                 M_REG_SET_DATATOG(bEnd, 1));
#endif
            }

            /* protocol/endpoint/interval/NAKlimit */
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXMAXP, bEnd, wPacketSize);

            //  Set interrupt polling interval must depend on high, or low/full speed.
            MGC_SetInterval(pThis, pUrb, bEnd);
        }
        else
        {
            /* endpoint 0: just flush */
            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, bEnd, MGC_M_CSR0_FLUSHFIFO);

            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, bEnd, MGC_M_CSR0_FLUSHFIFO);
        
            bInterval = MGC_Log2(pUrb->interval) + 1;
            bInterval = (bInterval > 16) ? 16 : ((bInterval <= 1) ? 0 : bInterval);
            
            /* protocol/endpoint/interval/NAKlimit */
            MGC_WriteCsr8(pBase, MGC_O_HDRC_NAKLIMIT0, 0, bInterval);
        }

        /* re-enable interrupt and write CSR to transmit */
        //MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE);             
        //may be disable by queue mode, confirm that interrupt is enable.
        dwIntrTxE = (wIntrTxE | (1 << bEnd)) << 16;
        MGC_Write32(pBase, MGC_O_HDRC_INTRRX, dwIntrTxE);

        if (bEnd)
        {
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsr);
        }

        // Check DMA condition.
        if ((bEnd) && (dwLength >= MGC_HSDMA_MIN_DMA_LEN))
        {
            pDmaController = pThis->pDmaController;

            pDmaChannel = pEnd->pDmaChannel;

            /* candidate for DMA */
            if (pDmaController && !pDmaChannel)
            {
                pDmaChannel =
                    pEnd->pDmaChannel =
                    pDmaController->pfDmaAllocateChannel(pDmaController->pPrivateData,
                                                         bEnd, TRUE,
                                                         MGC_GetTransferType(pUrb),
                                                         wPacketSize);
            }

            if (pDmaChannel)
            {
                //  set DMAReqMode by sending data size.                    
                pDmaChannel->bDesiredMode =
                    (dwLength > wPacketSize) ? 1 : 0;

                pDmaChannel->dwActualLength = 0L;
                pEnd->dwRequestSize =
                    min(dwLength, pDmaChannel->dwMaxLength);
                bDmaOk =
                    pDmaController->pfDmaProgramChannel(pDmaChannel,
                                                        wPacketSize,
                                                        pDmaChannel->bDesiredMode,
                                                        pBuffer,
                                                        pEnd->dwRequestSize);

                if (!bDmaOk)
                {
                    pDmaController->pfDmaReleaseChannel(pDmaChannel);

                    pEnd->pDmaChannel = NULL;
                    pEnd->dwRequestSize = 0;
                }
            }
        }

        if (!bDmaOk)
        {
            wCount = min((uint32_t) wPacketSize, dwLength);

            if (bEnd)
            {
                MGC_TxPacket(pThis, pEnd);
            }
            else
            {
                if (wCount)
                {
                    pEnd->dwRequestSize = wCount;

                    MGC_LoadFifo(pThis->pRegs, bEnd, wCount, pBuffer);
                }
            }

            /* determine if the time is right for a periodic transfer */
            if (usb_pipeisoc(nPipe) || usb_pipeint(nPipe))
            {
                DBG(3, "check whether there's still time for periodic Tx\n");

                wFrame = MGC_Read16(pBase, MGC_O_HDRC_FRAME);
                if ((pUrb->transfer_flags & USB_ISO_ASAP) || (wFrame >= pUrb->start_frame))
                {
                    pEnd->dwWaitFrame = 0;

                    MGC_StartTx(pThis, bEnd);
                }
                else
                {
                    pEnd->dwWaitFrame = pUrb->start_frame;
                    /* enable SOF interrupt so we can count down */
                    MGC_Write8(pBase, MGC_O_HDRC_INTRUSBE, 0xff);
                }
            }
            else
            {
                MGC_StartTx(pThis, bEnd);
            }
        }
    }
    else
    {  
        /*make sure interrupt is enable */
        dwIntrRxE = MGC_Read32(pBase,MGC_O_HDRC_INTRRXE);
        if(!(dwIntrRxE & (1<<bEnd)))
        {
           dwIntrRxE |= (1<<bEnd);
           dwIntrRxE &= 0xff00ffff; //mask usb common interrupt status
           MGC_Write32(pBase,MGC_O_HDRC_INTRRXE,dwIntrRxE);
        }
        
        /* receive */
        if (pEnd->bIsSharedFifo)
        {
            /* if was programmed for Tx, be sure it is ready for re-use */
            wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);
            if (wCsr & MGC_M_TXCSR_MODE)
            {
                DBG(1, "reprogramming ep%d for rx\n", bEnd);

                if (wCsr & MGC_M_TXCSR_FIFONOTEMPTY)
                {
                    /* this shouldn't happen */
                    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                                   MGC_M_TXCSR_FRCDATATOG);

                    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                                   MGC_M_TXCSR_FRCDATATOG);

                    ERR("*** switching end %d to Rx but Tx FIFO not empty\n",
                        bEnd);
                }

                /* clear mode (and everything else) to enable Rx */
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, 0);
            }
        }

        /* grab Rx residual if any */
        wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);

        if (wCsr & MGC_M_RXCSR_RXPKTRDY)
        {
            if(usb_pipeisoc(pUrb->pipe) && !usb_pipeout(pUrb->pipe) && 
                0x05AC == pEnd->dev->descriptor.idVendor &&  
                0x12 == (pEnd->dev->descriptor.idProduct >> 8))
            {
                printk("may cause iso issue\n");
            }        
            wCount = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, bEnd);
            MGC_RxPacket(pThis, pEnd, wCount);
            return;
        }

        /* protocol/endpoint/interval/NAKlimit */
        if (bEnd)
        {
            MGC_WriteCsr16(pBase, MGC_O_HDRC_RXMAXP, bEnd, wPacketSize);

            //  Set interrupt polling interval must depend on high, or low/full speed.
            MGC_SetInterval(pThis, pUrb, bEnd);
        }

        /* first time or re-program and shared FIFO, flush & clear toggle */
        /* twice in case of double packet buffering */
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                       MGC_M_RXCSR_CLRDATATOG);

        // HDRC will use this method.
        if (usb_gettoggle(pUrb->dev, pEnd->bRemoteEnd, 0))
        {
#ifdef UNIFIED_USB
            MU_MB();
            MGC_Write32(pBase, M_REG_RXDATATOG,
                         M_REG_SET_DATATOG(bEnd, 1));
                MU_MB();    
        //unified usb patch, dexiao
            MGC_Write32(pBase, M_REG_RXDATATOG,(1 << bEnd));            

#else
            MGC_MISC_Write32(pBase, M_REG_RXDATATOG,
                             M_REG_SET_DATATOG(bEnd, 1));
#endif
        }

        // Check DMA condition.
        if ((usb_pipebulk(nPipe)) && (dwLength >= wPacketSize))
        {
            pDmaController = pThis->pDmaController;

            pDmaChannel = pEnd->pDmaChannel;

            /* candidate for DMA */
            if (pDmaController && !pDmaChannel)
            {
                pDmaChannel =
                    pEnd->pDmaChannel =
                    pDmaController->pfDmaAllocateChannel(pDmaController->pPrivateData,
                                                         bEnd, FALSE,
                                                         MGC_GetTransferType(pUrb),
                                                         wPacketSize);
            }

            if (pDmaChannel)
            {
                /*
                    Note: 
                    Use "multiple packet RX, if size of data block not know" to handle rx data by dma.
                */                
                pDmaChannel->bDesiredMode = 1;

                pDmaChannel->dwActualLength = 0L;
                pEnd->dwRequestSize =
                    min(dwLength, pDmaChannel->dwMaxLength);
                bDmaOk =
                    pDmaController->pfDmaProgramChannel(pDmaChannel,
                                                        wPacketSize,
                                                        pDmaChannel->bDesiredMode,
                                                        pBuffer,
                                                        pEnd->dwRequestSize);

                if (!bDmaOk)
                {
                    pDmaController->pfDmaReleaseChannel(pDmaChannel);

                    pEnd->pDmaChannel = NULL;
                    pEnd->dwRequestSize = 0;
                }
                else
                {
                    // DMA ok.
                    return;
                }
            }
        }

        req_pkt_directly:
        //unmap bulk(not dma), interrupt, iso
        //unmap_urb_for_dma(musbstruct_to_hcd(pThis),pUrb);   

        /* kick things off */
        if (bEnd)
        {
            MU_MB();
            MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, MGC_M_RXCSR_H_REQPKT);
        }
        
    }
}

uint8_t MGC_UseDma(struct urb *pUrb)
{
  if(usb_pipeout(pUrb->pipe) && pUrb->transfer_buffer_length >=MGC_HSDMA_MIN_DMA_LEN)
  return TRUE;

  if(usb_pipein(pUrb->pipe) && 
    usb_pipebulk(pUrb->pipe)&&
    pUrb->transfer_buffer_length >= usb_maxpacket(pUrb->dev, pUrb->pipe, usb_pipeout(pUrb->pipe)))
  return TRUE;

  return FALSE;
    
}

static void MGC_KickUrb(MGC_LinuxCd * pThis, struct urb *pUrb, MGC_LinuxLocalEnd *pEnd)
{
    uint32_t dwLength;
    void *pBuffer;
    unsigned int nPipe;
    uint8_t bXmt;
    uint16_t wPacketSize;
    uint8_t bRemoteAddress, bRemoteEnd;

    if (!pUrb)
    {
        ERR("***> bEnd=%d, pUrb = NULL!\n", pEnd->bEnd);
        return;
    }

    nPipe = pUrb->pipe;
    bXmt = usb_pipeout(nPipe);
    wPacketSize = usb_maxpacket(pUrb->dev, nPipe, usb_pipeout(nPipe));
    bRemoteAddress = (uint8_t) usb_pipedevice(nPipe);
    bRemoteEnd = (uint8_t) usb_pipeendpoint(nPipe);

    DBG(2,
        "<== pUrb=%p, bEnd=%d, wPacketSize=%d, bRemoteAddress=%d, bRemoteEnd=%d, bXmt=%d\n",
        pUrb, pEnd->bEnd, wPacketSize, bRemoteAddress, bRemoteEnd, bXmt);

    /* indicate in progress */
    pUrb->actual_length = 0;
    pUrb->error_count = 0;

    /* remember software state - find_end() will use this - */
    pEnd->bRemoteAddress = bRemoteAddress;
    pEnd->bRemoteEnd = bRemoteEnd;
    pEnd->bTrafficType = (uint8_t) usb_pipetype(nPipe);

    /* init urb */
    pEnd->dwOffset = 0;
    pEnd->dwRequestSize = 0;
    pEnd->dwIsoPacket = 0;
    pEnd->dwWaitFrame = 0;
    pEnd->bRetries = 0;
    pEnd->wPacketSize = wPacketSize;

    if (usb_pipecontrol(nPipe))
    {
        /* control transfers always start with an OUT */
        bXmt = TRUE;
        pThis->bEnd0Stage = MGC_END0_START;
    }

    /* gather right source of data */
    if (usb_pipeisoc(nPipe))
    {
        dwLength = pUrb->iso_frame_desc[0].length;
        if (dwLength >= MGC_HSDMA_MIN_DMA_LEN)
        {
            pBuffer = (void *)
                ((uint32_t)pUrb->transfer_dma + pUrb->iso_frame_desc[0].offset);
        }                                    
        else
        {
            pBuffer = (void *)
                ((uint32_t)pUrb->transfer_buffer + pUrb->iso_frame_desc[0].offset);
        }
    }
    else if (usb_pipecontrol(nPipe))
    {
        dwLength = 8;
        pBuffer = (void *)pUrb->setup_packet;

    }
    else
    {
        // Only bulk transfer use DMA.
        dwLength = pUrb->transfer_buffer_length;
        if (dwLength >= MGC_HSDMA_MIN_DMA_LEN)
        {
            pBuffer = (void *) pUrb->transfer_dma;
        }
        else
        {
            pBuffer = (void *) pUrb->transfer_buffer;
        }
    }

    if (!pBuffer)
    {
        pBuffer = (void *) phys_to_virt(pUrb->transfer_dma);
    }

    if (!pBuffer)
    {                                // abort the transfer
        ERR("Rx requested but no buffer was given, BAD things are happening (TM)! aborting\n");
        return;
    }

    DBG(3,
        "(%p): dir=%s, type=%d, wPacketSize=%d, bRemoteAddress=%d, bRemoteEnd=%d, pBuffer=%p\n",
        pUrb, (bXmt) ? "out" : "in", usb_pipetype(nPipe), wPacketSize,
        bRemoteAddress, bRemoteEnd, pBuffer);
    
    #if(0)    
    if(usb_pipeisoc(nPipe) && !usb_pipeout(pUrb->pipe))//finetune for iPod isoch rx
    {
        MGC_LinuxUrbList *pUrbList;
        list_for_each_entry (pUrbList, &pThis->aLocalEnd[0][pEnd->bEnd].list, list) 
        {
            if(pUrbList->pUrb)
            {
                if(usb_pipeisoc(pUrbList->pUrb->pipe) && usb_pipeout(pUrbList->pUrb->pipe))
                {
                    return;
                }
            }
        }
    }
    #endif

    /* Configure endpoint */
    MGC_ProgramEnd(pThis, pEnd, pUrb, bXmt, pBuffer, dwLength);
}

#ifdef CONFIG_USB_QUEUE
static void MGC_Q_KickUrb(MGC_LinuxCd * pThis, struct urb *pUrb, MGC_LinuxLocalEnd *pEnd)
{
    uint32_t dwLength;
    void *pBuffer = NULL;
    unsigned int nPipe;
    uint8_t is_in;
    uint16_t wPacketSize;
    uint8_t bRemoteAddress, bRemoteEnd;
    int status = 0;
    int i;
    unsigned int offset;
    uint8_t zlp_enable;

    if (!pUrb)
    {
        ERR("***> bEnd=%d, pUrb = NULL!\n", pEnd->bEnd);
        return;
    }

    nPipe = pUrb->pipe;
    is_in = usb_pipein(nPipe);
    wPacketSize = usb_maxpacket(pUrb->dev, nPipe, usb_pipeout(nPipe));
    bRemoteAddress = (uint8_t) usb_pipedevice(nPipe);
    bRemoteEnd = (uint8_t) usb_pipeendpoint(nPipe);

   // printk(
   //     "<== pUrb=%p, bEnd=%d, wPacketSize=%d, bRemoteAddress=%d, bRemoteEnd=%d, bXmt=%d\n",
   //     pUrb, pEnd->bEnd, wPacketSize, bRemoteAddress, bRemoteEnd, !is_in);

    /* indicate in progress */
    pUrb->actual_length = 0;
    pUrb->error_count = 0;

    if(pUrb->transfer_flags & URB_NO_TRANSFER_DMA_MAP){
        pBuffer =(u8*)pUrb->transfer_dma;
    }

    if(!pBuffer)
    {
        pBuffer =(u8*)virt_to_phys(pUrb->transfer_buffer);
    }

    if (!pBuffer)
    {                                // abort the transfer
        ERR("Rx requested but no buffer was given, BAD things are happening (TM)! aborting\n");
        return;
    }

    if(pEnd->pCurrentUrb == pUrb){
      /* remember software state - find_end() will use this - */
      pEnd->bRemoteAddress = bRemoteAddress;
      pEnd->bRemoteEnd = bRemoteEnd;
      pEnd->bTrafficType = (uint8_t) usb_pipetype(nPipe);

      /* init urb */
      pEnd->dwOffset = 0;
      pEnd->dwRequestSize = 0;
      pEnd->dwIsoPacket = 0;
      pEnd->dwWaitFrame = 0;
      pEnd->bRetries = 0;
      pEnd->wPacketSize = wPacketSize;

    } 
    
    DBG(3,
        "(%p): dir=%s, type=%d, wPacketSize=%d, bRemoteAddress=%d, bRemoteEnd=%d, pBuffer=%p\n",
        pUrb, (!is_in) ? "out" : "in", usb_pipetype(nPipe), wPacketSize,
        bRemoteAddress, bRemoteEnd, pBuffer);
    
    /* insert GPD! */
    if(usb_pipeisoc(nPipe)){

        if(!mtk_is_qmu_enabled(pThis, pEnd->bEnd, is_in)){
            status = mtk_enable_q(pThis, bRemoteAddress, pEnd->bEnd, is_in, 
                      USB_ENDPOINT_XFER_ISOC, wPacketSize&0x7ff, MGC_Log2(pUrb->interval) + 1, bRemoteEnd, (is_in ? 0 : 1), 1, 0,1 + ((wPacketSize >> 11) & 0x03));
        }
        if(status < 0){
            printk(KERN_ALERT "Failed to enable queue, ISOC\n");
            return;
        }
    
        for(i=0; i<pUrb->number_of_packets; i++)
        {
            offset = pUrb->iso_frame_desc[i].offset;
            dwLength = pUrb->iso_frame_desc[i].length;
            //    printk("insert GPD[%d] 0x%x\n",i,(uint32_t)(pBuffer+offset));
            
                if(i==(pUrb->number_of_packets-1)){
                    status = mtk_qmu_insert_task(pThis,pEnd->bEnd, is_in, pBuffer+offset, dwLength, 1);
                }
                else{
                    status = mtk_qmu_insert_task(pThis,pEnd->bEnd, is_in, pBuffer+offset, dwLength, 0);
                }
        }
    
        if(status < 0){
            printk(KERN_ALERT "Insert Task Error! ISOC\n");
            return;
        }       
    }
    else if(usb_pipebulk(nPipe)){
        if(!mtk_is_qmu_enabled(pThis, pEnd->bEnd, is_in)){
            zlp_enable = (pUrb->transfer_flags & URB_ZERO_PACKET);
            status = mtk_enable_q(pThis, bRemoteAddress, pEnd->bEnd, is_in, USB_ENDPOINT_XFER_BULK, 
                         wPacketSize&0x7ff, 0, bRemoteEnd, zlp_enable, 1, 0, 0);
            if(status < 0){
                printk(KERN_ALERT "Failed to enable queue, BULK\n");
                return;
            }
        }
        
        dwLength = pUrb->transfer_buffer_length;
        /* process 65536 urb, delete?*/
        if(dwLength >= 65536){
            printk(KERN_ALERT "URB SIZE >=  65536\n");
            status = mtk_qmu_insert_task(pThis,pEnd->bEnd, is_in, pBuffer, 512, 1);
            status = mtk_qmu_insert_task(pThis,pEnd->bEnd, is_in, pBuffer + 512, 65024, 1);
        }
        else{
            status = mtk_qmu_insert_task(pThis,pEnd->bEnd, is_in, pBuffer, dwLength, 1);
        }
    
        if(status < 0){
            printk(KERN_ALERT "Insert Task Error! BULK\n");
            return;
        }
    }   
}
#endif

static int MGC_ScheduleUrb(MGC_LinuxCd *pThis, MGC_LinuxLocalEnd *pEnd, 
    struct urb *pUrb)
{
    int idle;
    if(!pUrb->ep->enabled)
        return -ENODEV;

    /* async unlink?? */
    if (pUrb->status != (-EINPROGRESS))
    {
        MGC_CompleteUrb(pThis, pEnd, pUrb);
        return 0;
    }

    idle = MGC_IsEndIdle(pEnd);

    if (MGC_EnqueueEndUrb(pEnd, pUrb) != 0)
    {
        ERR("**>cannot queue pUrb=%p to pEnd=%p! this is bad (TM)\n", pUrb,
            pEnd);
        return -EBUSY;
    }

    DBG(3,
        "queued URB %p (current %p) to end %d (bRemoteAddress=%d, bRemoteEnd=%d proto=%d) (idle=%d) pEnd->bBusyCompleting=%d\n",
        pUrb, MGC_GetCurrentUrb(pEnd), pEnd->bEnd,
        (uint8_t) usb_pipedevice(pUrb->pipe),
        (uint8_t) usb_pipeendpoint(pUrb->pipe), usb_pipetype(pUrb->pipe),
        idle, pEnd->bBusyCompleting);

    //  check pEnd->bBusyCompleting to prevent double kickstart the same urb.           
  #ifdef CONFIG_USB_QUEUE  
    if(mtk_ep_q_used(pEnd)){
      MGC_Q_KickUrb(pThis, pUrb, pEnd);
    }
    else
  #endif        
    if (idle && (pEnd->bBusyCompleting == 0))
    {
        MGC_KickUrb(pThis, MGC_GetCurrentUrb(pEnd), pEnd);
    }

    return 0;
}

static void MGC_CompleteEndUrb(MGC_LinuxCd * pThis,
                               MGC_LinuxLocalEnd * pEnd, struct urb *pUrb,
                               int toggle)
{
    if (pUrb->status == USB_ST_STALL)
    {
        toggle = 0;
    }

    /* save data toggle */
    usb_settoggle(pUrb->dev, pEnd->bRemoteEnd, (pEnd->bIsTx) ? 1 : 0,
                  toggle);

    if (pUrb->status)
    {
        DBG(1, "completing Tx URB=%p, status=%d, len=%x\n",
            pUrb, pUrb->status, pUrb->actual_length);
    }

    if (MGC_DequeueEndurb(pEnd, pUrb) == 0)
    {
        if (MGC_CompleteUrb(pThis, pEnd, pUrb) == 0)
        {            
            MGC_KickUrb(pThis, MGC_GetNextUrb(pEnd), pEnd);
        }        
        else
        if(usb_pipeisoc(pUrb->pipe) && !usb_pipeout(pUrb->pipe))     
        {
            printk("may cause isoin token issue\n");
        }       
    }
    else
    {
        ERR("*** pUrb=%p is not queued to bEnd=%d, this is BAD!\n", pUrb,
            pEnd->bEnd);
    }
}

static int MGC_ServiceUSBIntr(MGC_LinuxCd * pThis, uint8_t bIntrUSB)
{
    int handled = 0;

    uint8_t bSpeed = 1;
    uint8_t devctl;
    uint8_t *pBase = (uint8_t *) pThis->pRegs;
    MGC_LinuxLocalEnd *pEnd;
    uint32_t dwVirtualHubPortStatus = 0;
    uint32_t dummy1;
    uint8_t bEnd;
    uint16_t wFrame;
    uint8_t power;
    

    devctl = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);
    power = MGC_Read8(pBase, MGC_O_HDRC_POWER);

    if (bIntrUSB & MGC_M_INTR_RESUME)
    {
        handled++;

        DBG(2, "RESUME\n");
    }

    if (bIntrUSB & MGC_M_INTR_SESSREQ)
    {
        handled++;

        DBG(2, "SESSION_REQUEST\n");
    }

    if (bIntrUSB & MGC_M_INTR_VBUSERROR)
    {
        handled++;

        DBG(2, "V_BUS ERROR??? this is bad (TM)\n");
    }

    if (bIntrUSB & MGC_M_INTR_CONNECT)
    {
        handled++;

        DBG(2, "CONNECT\n");

        pThis->pRootDevice = NULL;
        pThis->bEnd0Stage = MGC_END0_START;

        if (devctl & MGC_M_DEVCTL_LSDEV)
        {
            bSpeed = MGC_SPEED_LS;

            dwVirtualHubPortStatus = (1 << USB_PORT_FEAT_C_CONNECTION) |
                (USB_PORT_STAT_LOW_SPEED) | (1 << USB_PORT_FEAT_POWER)
                | (1 << USB_PORT_FEAT_CONNECTION);
        }
        else if (devctl & MGC_M_DEVCTL_FSDEV)
        {
            /* NOTE: full-speed is "speculative" until reset */
            //bSpeed = 2;
            bSpeed = (power & MGC_M_POWER_HSMODE) ? MGC_SPEED_HS : MGC_SPEED_FS;

            dwVirtualHubPortStatus = (1 << USB_PORT_FEAT_C_CONNECTION) |
                (1 << USB_PORT_FEAT_POWER) | (1 <<
                                              USB_PORT_FEAT_CONNECTION);
        }

        pThis->bRootSpeed = bSpeed;
        pThis->dwVirtualHubPortStatus = dwVirtualHubPortStatus;
        usb_hcd_poll_rh_status(musbstruct_to_hcd(pThis));
    }

    /* saved one bit: bus reset and babble share the same bit;
     * If I am host is a babble! i must be the only one allowed
     * to reset the bus; when in otg mode it means that I have 
     * to switch to device
     */
    if (bIntrUSB & MGC_M_INTR_RESET)
    {
        handled++;

        printk("[usb]BUS babble\n");

        //  Solve the busy usb babble interrupt caused by babble device
        //  if device is attached
        if (devctl & (MGC_M_DEVCTL_FSDEV | MGC_M_DEVCTL_LSDEV))
        {
        #ifdef UNIFIED_USB
            dummy1 = MGC_Read8(pBase,M_REG_PERFORMANCE3);
        #else
            DBG(1, "MT539x ");

            dummy1 = MGC_MISC_Read32(pBase, MGC_O_HDRC_DUMMY1);
            DBG(1, "device hooked , addr %08x (dummy1) =%08x\n",
                (uint32_t) pBase + MUSB_MISCBASE + MGC_O_HDRC_DUMMY1,
                dummy1);
        #endif
            if (dummy1 & MGC_M_DUMMY1_SOFFORCE)
            {
                DBG(1, "SOF is still transmit during babble\n");

                bIntrUSB &= ~MGC_M_INTR_RESET;
            }
        }

        handled++;
    }

    if (bIntrUSB & MGC_M_INTR_SOF)
    {
        DBG(2, "START_OF_FRAME\n");

        handled++;

        /* start any periodic Tx transfers waiting for current frame */
        wFrame = MGC_Read16(pBase, MGC_O_HDRC_FRAME);

        for (bEnd = 1;
             (bEnd < pThis->bEndCount) && (pThis->wEndMask >= (1 << bEnd));
             bEnd++)
        {
            pEnd = &(pThis->aLocalEnd[EPTX][bEnd]); /* 1: Tx */
            if ((pEnd->dwWaitFrame) && (pEnd->dwWaitFrame >= wFrame))
            {
                pEnd->dwWaitFrame = 0;

                MGC_StartTx(pThis, bEnd);
            }
        }
    }

    /* p35 MUSBHDRC manual for the order of the tests */
    if (bIntrUSB & MGC_M_INTR_DISCONNECT)
    {
        DBG(2, "DISCONNECT\n");

        handled++;

        pThis->dwVirtualHubPortStatus =
            (1 << USB_PORT_FEAT_C_CONNECTION) | (1 << USB_PORT_FEAT_POWER);

        pThis->pRootDevice = NULL;

        // Set UTMI+PHY to low power mode and stop XCLK.
        power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
        power |= (MGC_M_POWER_SUSPENDM |MGC_M_POWER_ENSUSPEND);    
        MGC_Write8(pBase, MGC_O_HDRC_POWER, power);        

        MGC_PHY_Write32(pBase, 0x6C, (MGC_PHY_Read32(pBase,0x6C) &(~0x3F3F)) | 0x3C10); 
        MGC_PHY_Write32(pBase, 0x6C, (MGC_PHY_Read32(pBase,0x6C) &(~0x3F3F)) | 0x3E10); 

        mdelay(10);

        MGC_PHY_Write32(pBase, 0x6C, (MGC_PHY_Read32(pBase,0x6C) & (~0x3F3F)) |  0x3E2C); 
        MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, MGC_Read8(pBase,MGC_O_HDRC_DEVCTL)|MGC_M_DEVCTL_SESSION);
    }

    /* I cannot get suspend while in host mode! go to error mode and ignore 
     * the other signals; need to be last (see manual p35)s  */
    if (bIntrUSB & MGC_M_INTR_SUSPEND)
    {
        DBG(2, "RECEIVED SUSPEND\n");

        handled++;
    }

    return handled;
}

static void MGC_CheckConnect(unsigned long pParam)
{
    MGC_LinuxCd *pThis = (MGC_LinuxCd *) pParam;

    void *pBase = pThis->pRegs;
    uint32_t dwLineState;

#ifdef UNIFIED_USB
    
    dwLineState = MGC_Read32(pBase,M_REG_DEBUG_PROBE);
#else

    dwLineState = MGC_PHY_Read32(pBase, M_REG_PHYC5);
#endif  

    dwLineState &= M_REG_LINESTATE_MASK;


    if (dwLineState == LINESTATE_DISCONNECT)
    {
#if CONFIG_USB_DETECT_IMPROVE
    
        printk("[USB] check connect fail, disconnect handle\n");
        //pThis->fgIsWantFullSpeed = FALSE;
        pThis->fgIsWantFullSpeed = config_usb11() ? TRUE : FALSE;
        //2009.09.21
        printk("[USB] update hub status\n");
    
        pThis->dwVirtualHubPortStatus = (1 << USB_PORT_FEAT_POWER)
            | (1 << USB_PORT_FEAT_C_CONNECTION);
        pThis->dwVirtualHubPortStatus &= ~(1 << USB_PORT_FEAT_CONNECTION);
        usb_hcd_poll_rh_status(musbstruct_to_hcd(pThis));
    
        pThis->bInsert = FALSE;
    
        MGC_ServiceUSBIntr(pThis, MGC_M_INTR_DISCONNECT);
#endif
        return;
    }

    if (dwLineState == LINESTATE_HWERROR)
    {
#if 0
        //  Move MGC_dwCheckInsert inside MGC_LinuxCd port structure
        pThis->bCheckInsert++;

        if (pThis->bCheckInsert > 5)
        {
            DBG(2, "LINESTATE_HWERROR !!!\n");

            pThis->bCheckInsert = 0;
            return;
        }

        MGC_SetTimer(pThis, MGC_CheckConnect, (unsigned long) pThis,
                     MGC_CHECK_INSERT_DEBOUNCE);
#endif
        return;
    }

    pThis->bInsert = TRUE;

    DBG(2, "Connect interrupt !!!\n");
    MGC_UnlinkInvalidUrb( (unsigned long) pThis);
    // handle device connect.
    MGC_ServiceUSBIntr(pThis, MGC_M_INTR_CONNECT);
}

static uint8_t MGC_ServiceHostDefault(MGC_LinuxCd * pThis, uint16_t wCount,
                                      struct urb *pUrb)
{
    uint8_t bMore = FALSE;
    uint8_t *pFifoDest = NULL;
    uint16_t wFifoCount = 0;
    uint8_t *pBase = (uint8_t *) pThis->pRegs;
    MGC_LinuxLocalEnd *pEnd = &(pThis->aLocalEnd[EP0][0]);
    MUSB_DeviceRequest *pRequest = (MUSB_DeviceRequest *) pUrb->setup_packet;

    DBG(2, "<== (wCount=%04x, pUrb=%lx, bStage=%02x)\n",
        wCount, (unsigned long) pUrb, pThis->bEnd0Stage);

    if (MGC_END0_IN == pThis->bEnd0Stage)
    {
        /* we are receiving from peripheral */
        pFifoDest = pUrb->transfer_buffer + pUrb->actual_length;
        wFifoCount =
            min(wCount,
                ((uint16_t)
                 (pUrb->transfer_buffer_length - pUrb->actual_length)));

        DBG(3, "Receiving %d bytes in &%p[%d] (pUrb->actual_length=%u)\n",
            wFifoCount, pUrb->transfer_buffer,
            (unsigned int) pUrb->actual_length, pUrb->actual_length);

        MGC_UnloadFifo(pBase, 0, wFifoCount, pFifoDest);
        MGC_CleanDcache(pUrb,(uint32_t)pFifoDest, (uint32_t)wFifoCount);
        pUrb->actual_length += wFifoCount;
        if ((pUrb->actual_length < pUrb->transfer_buffer_length)
            && (wCount == pEnd->wPacketSize))
        {
            bMore = TRUE;
        }
    }
    else
    {
        /* we are sending to peripheral */
        if ((MGC_END0_START == pThis->bEnd0Stage)
            && (pRequest->bmRequestType & USB_DIR_IN))
        {
            DBG(3, "just did setup, switching to IN\n");

            /* this means we just did setup; switch to IN */
            pThis->bEnd0Stage = MGC_END0_IN;
            bMore = TRUE;

        }
        else if (pRequest->wLength
                 && (MGC_END0_START == pThis->bEnd0Stage))
        {
            pThis->bEnd0Stage = MGC_END0_OUT;
        }
        
        if (MGC_END0_OUT == pThis->bEnd0Stage)
        {
            pFifoDest =
            (uint8_t *) (pUrb->transfer_buffer + pUrb->actual_length);
            wFifoCount =
            min(pEnd->wPacketSize,
                   ((uint16_t)(pUrb->transfer_buffer_length -pUrb->actual_length)));
            DBG(3, "Sending %d bytes to %p\n", wFifoCount, pFifoDest);
            MGC_LoadFifo(pBase, 0, wFifoCount, pFifoDest);

            pEnd->dwRequestSize = wFifoCount;
            pUrb->actual_length += wFifoCount;
            if (wFifoCount)
            {
                bMore = TRUE;
            }
        }        
    }

    return bMore;
}

static void MGC_ServiceDefaultEnd(MGC_LinuxCd * pThis)
{
    struct urb *pUrb;
    uint16_t wCsrVal, wCount;
    int status = USB_ST_NOERROR;
    uint8_t *pBase = (uint8_t *) pThis->pRegs;
    MGC_LinuxLocalEnd *pEnd = &(pThis->aLocalEnd[EP0][0]);
    uint8_t bVal, bOutVal = 0, bComplete = FALSE, bError = FALSE;

    spin_lock(&pEnd->Lock);
    pUrb = MGC_GetCurrentUrb(pEnd);
    if (!pUrb)
    {
        spin_unlock(&pEnd->Lock);    
        return ;
    }    

    MGC_SelectEnd(pBase, 0);
    wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
    wCount = MGC_ReadCsr8(pBase, MGC_O_HDRC_COUNT0, 0);
    bVal = (uint8_t) wCsrVal;

    DBG(2, "<== CSR0=%04x, wCount=%04x\n", wCsrVal, wCount);

    /* if we just did status stage, we are done */
    if (MGC_END0_STATUS == pThis->bEnd0Stage)
    {
        bComplete = TRUE;
    }

    /* prepare status */
    if ((MGC_END0_START == pThis->bEnd0Stage) && !wCount
        && (wCsrVal & MGC_M_CSR0_RXPKTRDY))
    {
        printk("[usb]missed data\n");

        /* just started and got Rx with no data, so probably missed data */
        status = USB_ST_SHORT_PACKET;
        bError = TRUE;

        //bComplete = TRUE;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
    }

    if (bVal & MGC_M_CSR0_H_RXSTALL)
    {
        printk("[usb]STALLING ENDPOINT 0\n");
        status = USB_ST_STALL;
        bError = TRUE;
    }
    else if (bVal & MGC_M_CSR0_H_ERROR)
    {
        printk("[usb]ep0 no response (error)\n");

        status = USB_ST_NORESPONSE;
        bError = TRUE;

    }
    else if (bVal & MGC_M_CSR0_H_NAKTIMEOUT)
    {
        printk("[usb]ep0 NAK timeout pEnd->bRetries=%d\n", pEnd->bRetries);

        if (++pEnd->bRetries < MUSB_MAX_RETRIES)
        {
            /* cover it up if retries not exhausted */
            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
        }
        else
        {
            printk("[usb]no response (NAK timeout)\n");
            pEnd->bRetries = 0;
            status = USB_ST_NORESPONSE;
            bError = TRUE;
        }
    }

    if (USB_ST_NORESPONSE == status)
    {
        printk("[usb]ep0 aborting\n");

        /* use the proper sequence to abort the transfer */
        if (bVal & MGC_M_CSR0_H_REQPKT)
        {
            bVal &= ~MGC_M_CSR0_H_REQPKT;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
            bVal &= ~MGC_M_CSR0_H_NAKTIMEOUT;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
        }
        else
        {
            bVal |= MGC_M_CSR0_FLUSHFIFO;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
            bVal &= ~MGC_M_CSR0_H_NAKTIMEOUT;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
        }

        MGC_WriteCsr8(pBase, MGC_O_HDRC_NAKLIMIT0, 0, 0);
    }

    if (bError)
    {
        printk("[usb]ep0 handling error\n");

        /* clear it */
        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
    }

    if (!pUrb)
    {
        /* stop endpoint since we have no place for its data, this 
         * SHOULD NEVER HAPPEN! */
        ERR("no URB for end 0\n");

        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);

        /* start next URB that might be queued for it */
        spin_unlock(&pEnd->Lock);
        return;
    }

    if (!bComplete && !bError)
    {

        /* call common logic and prepare response */
        if (MGC_ServiceHostDefault(pThis, wCount, pUrb))
        {
            /* more packets required */
            bOutVal = (MGC_END0_IN == pThis->bEnd0Stage) ?
                MGC_M_CSR0_H_REQPKT : MGC_M_CSR0_TXPKTRDY;
            DBG(3, "Need more bytes bOutVal=%04x\n", bOutVal);
        }
        else
        {
            /* data transfer complete; perform status phase */
            bOutVal = MGC_M_CSR0_H_STATUSPKT |
                (usb_pipeout(pUrb->pipe) ? MGC_M_CSR0_H_REQPKT :
                 MGC_M_CSR0_TXPKTRDY);

            /* flag status stage */
            pThis->bEnd0Stage = MGC_END0_STATUS;
            DBG(3, "Data transfer complete, status phase bOutVal=%04x\n",
                bOutVal);
        }
    }

    /* write CSR0 if needed */
    if (bOutVal)
    {
        MU_MB();
        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bOutVal);
    }

    /* call completion handler if done */
    if (bComplete || bError)
    {
        DBG(3, "completing cntrl URB %p, status=%d, len=%x\n", pUrb,
            status, pUrb->actual_length);

        if (MGC_DequeueEndurb(pEnd, pUrb) == 0)
        {
            pUrb->status = status;
            if (MGC_CompleteUrb(pThis, pEnd, pUrb) == 0)
            {
                /* Endpoint is not idle. */
                MGC_KickUrb(pThis, MGC_GetNextUrb(pEnd), pEnd);
            }
        }
        else
        {
            ERR("*** pUrb=%p is not queued to bEnd=%d\n", pUrb,
                pEnd->bEnd);
        }
    }

    spin_unlock(&pEnd->Lock);
}

static void MGC_ServiceTxAvail(MGC_LinuxCd * pThis, uint8_t bEnd)
{
    int skip = 0;
    struct urb *pUrb;
    uint16_t wTxCsrVal;
    uint16_t wVal = 0;
    MGC_LinuxLocalEnd *pEnd = &(pThis->aLocalEnd[EPTX][bEnd]);  /*1: Tx*/
    uint8_t *pBase = (uint8_t *) pThis->pRegs;
    int toggle;
    uint32_t status = 0;

    DBG(1, "<==\n");

    spin_lock(&pEnd->Lock);
    pUrb = MGC_GetCurrentUrb(pEnd);
    if (!pUrb)
    {
        //printk("Killed Tx pUrb=0x%X.\n", (uint32_t)pUrb);   
        spin_unlock(&pEnd->Lock);
        return;
    }

    MGC_SelectEnd(pBase, bEnd);
    wVal = wTxCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);

    DBG(3, "end %d wTxCsrVal=%04x\n", bEnd, wTxCsrVal);

    /* check for errors */
    if (wTxCsrVal & MGC_M_TXCSR_H_RXSTALL)
    {
        printk("[usb]TX end %d stall\n", bEnd);
        status = USB_ST_STALL;
    }
    else if (wTxCsrVal & MGC_M_TXCSR_H_ERROR)
    {
        printk("[usb]TX data error on ep=%d\n", bEnd);

        status = USB_ST_NORESPONSE;

        /* do the proper sequence to abort the transfer */
        wVal &= ~MGC_M_TXCSR_FIFONOTEMPTY;
        wVal |= MGC_M_TXCSR_FLUSHFIFO;

        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);

    }
    else if (wTxCsrVal & MGC_M_TXCSR_H_NAKTIMEOUT)
    {
        /* cover it up if retries not exhausted */
        if (pUrb->status == (-EINPROGRESS)
            && ++pEnd->bRetries < MUSB_MAX_RETRIES)
        {
            wVal &= ~MGC_M_TXCSR_H_NAKTIMEOUT;
            wVal |= MGC_M_TXCSR_TXPKTRDY;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
            /* reset error bits by flush fifo */
            //MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, MGC_M_TXCSR_FLUSHFIFO);
            //MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, MGC_M_TXCSR_FLUSHFIFO);    
            //MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, MGC_M_TXCSR_MODE);
            spin_unlock(&pEnd->Lock);
            printk("[usb]==> cover tx error,retry times is %d\n",pEnd->bRetries);
            return;
        }

        if (pUrb->status == (-EINPROGRESS))
        {
            status = -ECONNRESET;
        }

        printk("[usb]device not responding on ep=%d\n", bEnd);

        /* do the proper sequence to abort the transfer */
        wVal &= ~MGC_M_TXCSR_FIFONOTEMPTY;
        wVal |= MGC_M_TXCSR_FLUSHFIFO;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
        MGC_WriteCsr8(pBase, MGC_O_HDRC_TXINTERVAL, bEnd, 0);

        pEnd->bRetries = 0;
    }
    else if (wTxCsrVal & MGC_M_TXCSR_FIFONOTEMPTY)
    {
        printk("[usb]TX MGC_M_TXCSR_FIFONOTEMPTY\n");
        /* whopps, dbould buffering better be enabled */
        skip = TRUE;
    }

    if (status)
    {
        pUrb->status = status;        /* */

        // disable dma firstly
        if (pEnd->pDmaChannel)
        {
            /* release previously-allocated channel */
            pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
            pEnd->pDmaChannel = NULL;
        }
        if (USB_ST_STALL != status)
        {
            printk("Tx error\n");
            //abort
            wTxCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);
            wTxCsrVal |= (MGC_M_TXCSR_TXPKTRDY|MGC_M_TXCSR_FLUSHFIFO);
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wTxCsrVal);

            udelay(125);
            //flush
            wTxCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);
            wTxCsrVal |= MGC_M_TXCSR_FLUSHFIFO;
            wTxCsrVal &= ~MGC_M_TXCSR_TXPKTRDY;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wTxCsrVal);
        }

        /* reset error bits */
        wVal &=
        ~(MGC_M_TXCSR_AUTOSET
          | MGC_M_TXCSR_DMAENAB
          | MGC_M_TXCSR_H_ERROR
          | MGC_M_TXCSR_H_RXSTALL
          | MGC_M_TXCSR_H_NAKTIMEOUT
          | MGC_M_TXCSR_FIFONOTEMPTY);
        wVal |= MGC_M_TXCSR_FRCDATATOG;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
        MGC_WriteCsr8(pBase, MGC_O_HDRC_TXINTERVAL, bEnd, 0);
    }

    if (!skip && pUrb->status == (-EINPROGRESS))
    {
        MGC_TxPacket(pThis, pEnd);
    }

    /* complete the current request or start next tx transaction */
    if (pUrb->status != (-EINPROGRESS))
    {
        //  set data toggle in device basis for supporting Hub.
        // get data toggle bit from physical ep.

#ifdef UNIFIED_USB
        toggle =
            ((MGC_Read32(pBase, M_REG_TXDATATOG)) >> bEnd) & 0x01;

#else
        toggle =
            ((MGC_MISC_Read32(pBase, M_REG_TXDATATOG)) >> bEnd) & 0x01;
#endif

        // release dma channel.
        if (pEnd->pDmaChannel)
        {
            /* release previously-allocated channel */
            pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
            pEnd->pDmaChannel = NULL;
        }
        pUrb->actual_length = pEnd->dwOffset;
/*        
        if (pUrb->actual_length != pUrb->transfer_buffer_length)
        {
            printk("Tx: transfer_buffer_length =%d, actual_length=%d.\n", 
                pUrb->transfer_buffer_length, pUrb->actual_length);
        }
*/
        MGC_CompleteEndUrb(pThis, pEnd, pUrb, toggle);
    }
    else
    {
        if (!skip)
        {
            MU_MB();
#ifdef UNIFIED_USB
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
               (MGC_M_TXCSR_TXPKTRDY));

#else
            MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                   (MGC_M_TXCSR_MODE | MGC_M_TXCSR_TXPKTRDY));
#endif
        }
        DBG(1, "==>\n");
    }

    spin_unlock(&pEnd->Lock);
}

static void MGC_ServiceRxReady(MGC_LinuxCd * pThis, uint8_t bEnd)
{
    struct urb *pUrb;
    uint16_t wRxCount, wRxCsrVal, wVal = 0;
    uint8_t bIsochError = FALSE;
    MGC_LinuxLocalEnd *pEnd = &(pThis->aLocalEnd[EPRX][bEnd]); /*0: Rx*/
    uint8_t *pBase = (uint8_t *) pThis->pRegs;
    uint8_t bDone = FALSE;
    uint32_t dwLength;
    MGC_DmaChannelStatus bStatus;
    uint8_t bDmaOk = FALSE;
    uint8_t *pDmaBuffer;
    int toggle;
    uint32_t status = 0;
    //uint16_t wFrame = 0;

    

    DBG(2, "<== end%d\n", bEnd);
    spin_lock(&pEnd->Lock);
    DBG(3, "locked end%d, pUrb=%p\n", bEnd, MGC_GetCurrentUrb(pEnd));

    pUrb = MGC_GetCurrentUrb(pEnd);
    if (!pUrb)
    {
        //printk("Killed Rx pUrb=0x%X.\n", (uint32_t)pUrb);
        //patch from lawrance     
        //printk("Urb has been freed before packet comeback !!");
        MGC_SelectEnd(pBase, bEnd);
#if 0
        if(pEnd->dev)
        {
            if(0x05AC == pEnd->dev->descriptor.idVendor &&  0x12 == (pEnd->dev->descriptor.idProduct >> 8))
            {
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, MGC_M_RXCSR_H_REQPKT);//for iPod isoch rx
                spin_unlock(&pEnd->Lock);
                return;
            }
        }
#endif
        printk("<UN:0x%x>", MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd));
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, 0);
        //end      
        spin_unlock(&pEnd->Lock);
        return;
    }

    MGC_SelectEnd(pBase, bEnd);
    wRxCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
    wVal = wRxCsrVal; 
    wRxCount = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, bEnd);

    DBG(3, "end %d wRxCsrVal=%04x, wRxCount=%d, pUrb->actual_length=%d\n",
        bEnd, wRxCsrVal, wRxCount, pUrb->actual_length);

    /* check for errors, concurrent stall & unlink is not really
     * handled yet! */
    if (wRxCsrVal & MGC_M_RXCSR_H_RXSTALL)
    {
        printk("[usb]RX end %d STALL\n", bEnd);
        status = USB_ST_STALL;//clearing endpoint halt for bulk pipe later 
    }
    else if (wRxCsrVal & MGC_M_RXCSR_H_ERROR)
    {
        //printk("[usb]end %d Rx error\n", bEnd);
        if ((-EINPROGRESS)  == pUrb->status && PIPE_BULK == pEnd->bTrafficType 
            && ++pEnd->bRetries < MUSB_MAX_ERR_RETRIES)
        {
                wRxCsrVal &= ~MGC_M_RXCSR_H_ERROR;
                wRxCsrVal &= ~MGC_M_RXCSR_RXPKTRDY;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                               wRxCsrVal | MGC_M_RXCSR_H_REQPKT);

                spin_unlock(&pEnd->Lock);
                DBG(0, "==> cover rx error\n");
                return;
            
        }
        else
        {
            status = -ECONNRESET;
        }

        /* do the proper sequence to abort the transfer */
        wVal &= ~MGC_M_RXCSR_H_REQPKT;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
        MGC_WriteCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd, 0);
        
        pEnd->bRetries = 0;
        DBG(3, "end=%d device not responding,RX error\n", bEnd);

    }
    else if (wRxCsrVal & MGC_M_RXCSR_DATAERR)
    {
        printk("[usb]end %d Rx data error\n", bEnd);
        if (PIPE_BULK == pEnd->bTrafficType)
        {
            /* cover it up if retries not exhausted, slow devices might  
             * not answer quickly enough: I was expecting a packet but the 
             * packet didn't come. The interrupt is generated after 3 failed
             * attempts, it make MUSB_MAX_RETRIESx3 attempts total..
             */
            if (pUrb->status == (-EINPROGRESS)
                && ++pEnd->bRetries < MUSB_MAX_RETRIES)
            {
                /* silently ignore it */
                wRxCsrVal &= ~MGC_M_RXCSR_DATAERR;
                wRxCsrVal &= ~MGC_M_RXCSR_RXPKTRDY;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                               wRxCsrVal | MGC_M_RXCSR_H_REQPKT);

                spin_unlock(&pEnd->Lock);
                DBG(2, "==> cover rx error\n");
                return;
            }

            if (pUrb->status == (-EINPROGRESS))
            {
                DBG(-1, "urb=%p, protocol=%s timed out\n", pUrb,
                    MGC_DecodeUrbProtocol(pUrb));
                status = USB_ST_TIMEOUT;//Nak timeout is not an error
            }

            wVal &= ~MGC_M_RXCSR_H_REQPKT;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
            wVal &= ~MGC_M_RXCSR_DATAERR;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
            MGC_WriteCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd, 0);
            pEnd->bRetries = 0;

            /* do the proper sequence to abort the transfer; 
             * am I dealing with a slow device maybe? */
            DBG(3, "end=%d device not responding,Nak timeout\n", bEnd);

        }
        else if (PIPE_ISOCHRONOUS == pEnd->bTrafficType)
        {
            DBG(3, "bEnd=%d Isochronous error\n", bEnd);
            bIsochError = TRUE;
        }
    }

    /* an error won't process the data */
    if (status)
    {
        pUrb->status = status;

        /* data errors are signaled */
        if (USB_ST_STALL != status)
        {
            DBG(3, "end %d Rx error, status=%d\n", bEnd, status);
        }
        else
        {
            /* twice in case of double packet buffering */
            MGC_WriteCsr16((uint8_t *) pThis->pRegs, MGC_O_HDRC_RXCSR,
                           bEnd,
                           MGC_M_RXCSR_FLUSHFIFO | MGC_M_RXCSR_CLRDATATOG);
            MGC_WriteCsr16((uint8_t *) pThis->pRegs, MGC_O_HDRC_RXCSR,
                           bEnd,
                           MGC_M_RXCSR_FLUSHFIFO | MGC_M_RXCSR_CLRDATATOG);

        }

        DBG(3, "clearing all error bits, right away\n");
        wVal &=
            ~(MGC_M_RXCSR_H_ERROR | MGC_M_RXCSR_DATAERR |
              MGC_M_RXCSR_H_RXSTALL);
        wVal &= ~MGC_M_RXCSR_RXPKTRDY;
        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
    }

    /* no errors, unload... */
    if (pUrb->status == (-EINPROGRESS))
    {
        /* be sure a packet is ready for unloading */
        if (!wRxCsrVal & MGC_M_RXCSR_RXPKTRDY)
        {
            pUrb->status = USB_ST_INTERNALERROR;

            /* do the proper sequence to abort the transfer */
            wVal &= ~MGC_M_RXCSR_H_REQPKT;
            MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
            DBG(3, "Rx interrupt with no errors or packet!\n");
        }
        else
        {
            /* we are expecting traffic */
            if (pEnd->pDmaChannel)
            {
                bStatus =
                    pThis->pDmaController->pfDmaGetChannelStatus(pEnd->pDmaChannel);
                pEnd->dwOffset += pEnd->pDmaChannel->dwActualLength;
                pUrb->actual_length += pEnd->pDmaChannel->dwActualLength;

                if (MGC_DMA_STATUS_FREE == bStatus)
                {
                    /* see if we are done */
                    bDone =
                        (pEnd->dwOffset >= pUrb->transfer_buffer_length)
                        || (pEnd->pDmaChannel->dwActualLength <
                            pEnd->wPacketSize);

                    DEBUG_CODE(3, if (bDone)
                               {
                               INFO
                               ("will complete URB; pUrb=%p (%s) len=%x, errors=%d\n",
                                pUrb, MGC_DecodeUrbProtocol(pUrb),
                                pUrb->actual_length, pUrb->error_count);}
                    );

                    if (bEnd && bDone)
                    {
                        // clear rxpktrdy.
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, 0);
                        pUrb->status = 0;
                    }
                    else
                    {
                        dwLength =
                            pUrb->transfer_buffer_length - pEnd->dwOffset;
                        if (dwLength > pEnd->wPacketSize)
                        {
                            pDmaBuffer = (uint8_t *) pUrb->transfer_dma;
                            pDmaBuffer += pEnd->dwOffset;
                            pEnd->pDmaChannel->bDesiredMode = 1;
                            pEnd->pDmaChannel->dwActualLength = 0L;
                            pEnd->dwRequestSize = dwLength;
                            bDmaOk =
                                pThis->pDmaController->pfDmaProgramChannel(pEnd->pDmaChannel,
                                                    pEnd->wPacketSize,
                                                    pEnd->pDmaChannel->bDesiredMode,
                                                    pDmaBuffer,
                                                    pEnd->dwRequestSize);
                            if (!bDmaOk)
                            {
                                pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
                                pEnd->pDmaChannel = NULL;
                                pEnd->dwRequestSize = 0;
                            }
                        }
                        else
                        {
                            /* release previously-allocated channel */
                            pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
                            pEnd->pDmaChannel = NULL;

                            MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                                           MGC_M_RXCSR_H_REQPKT);
                        }
                    }
                }
                else if (MGC_DMA_STATUS_MODE1_SHORTPKT == bStatus)
                {
                    //unmap when get short packet
                    //unmap_urb_for_dma(musbstruct_to_hcd(pThis),pUrb);
                    bDone = MGC_RxPacket(pThis, pEnd, wRxCount);
		    outer_clean_range(pUrb->transfer_dma, pUrb->transfer_dma + wRxCount);
                }
            }
            else
            {
                bDone = MGC_RxPacket(pThis, pEnd, wRxCount);
            }
        }
    }

    /* complete the current request or start next one clearing RxPktRdy 
     * and setting ReqPkt */
    if (pUrb->status != (-EINPROGRESS))
    {
        //  set data toggle in device basis for supporting Hub.
        // get data toggle bit from physical ep.
#ifdef UNIFIED_USB
        toggle = ((MGC_Read32(pBase, M_REG_RXDATATOG)) >> bEnd) & 0x01;

#else
        toggle = ((MGC_MISC_Read32(pBase, M_REG_RXDATATOG)) >> bEnd) & 0x01;

#endif
        // release dma channel.
        if (pEnd->pDmaChannel)
        {
            /* release previously-allocated channel */
            pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
            pEnd->pDmaChannel = NULL;
        }

/*
        if (pUrb->actual_length != pUrb->transfer_buffer_length)
        {
            printk("Rx: transfer_buffer_length =%d, actual_length=%d.\n", 
                pUrb->transfer_buffer_length, pUrb->actual_length);
        }
*/

        MGC_CompleteEndUrb(pThis, pEnd, pUrb, toggle);
    }

    spin_unlock(&pEnd->Lock);
}

static void MGC_ResetOff(unsigned long param)
{
    uint8_t power;
    MGC_LinuxCd *pThis = (MGC_LinuxCd *) param;
    void *pBase = pThis->pRegs;
    unsigned long flags;
    //uint32_t u4UsbIrqEnable = 0;

    //spin_lock_irqsave(&pThis->Lock, flags);

    power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
    MGC_Write8(pBase, MGC_O_HDRC_POWER, power & ~MGC_M_POWER_RESET);

    /* check for high-speed and set in root device if so */
    power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
    if (power & MGC_M_POWER_HSMODE)
    {
        DBG(3, "high-speed device connected\n");
        printk("[USB] HS device connected\n");

        pThis->bRootSpeed = MGC_SPEED_HS;
    }

    // disable reset status.
    pThis->dwVirtualHubPortStatus &= ~(1 << USB_PORT_FEAT_RESET);

    // reset complete. device still connect, set port enable.
    pThis->dwVirtualHubPortStatus |=
        (1 << USB_PORT_FEAT_C_RESET) | (1 << USB_PORT_FEAT_ENABLE);
    
    //pThis->dwVirtualHubPortStatus &= ~(USB_PORT_STAT_SUPER_SPEED);

    if (pThis->bRootSpeed == MGC_SPEED_HS)
    {
        pThis->dwVirtualHubPortStatus &= ~(USB_PORT_STAT_LOW_SPEED);
        pThis->dwVirtualHubPortStatus |= (USB_PORT_STAT_HIGH_SPEED);
    }
    else if (pThis->bRootSpeed == MGC_SPEED_FS)
    {
        pThis->dwVirtualHubPortStatus &=
            ~((USB_PORT_STAT_LOW_SPEED) |
              (USB_PORT_STAT_HIGH_SPEED));
    }
    else
    {
        pThis->dwVirtualHubPortStatus &= ~(USB_PORT_STAT_HIGH_SPEED);
        pThis->dwVirtualHubPortStatus |= (USB_PORT_STAT_LOW_SPEED);
    }

    //spin_unlock_irqrestore(&pThis->Lock, flags);
}

static int MGC_ResetRootPort(uint8_t bPortNumb, uint8_t fgIsWantFullSpeed, uint32_t u4mdelay)
{
    struct usb_hcd *hcd = NULL;
    MGC_LinuxCd *pThis = NULL;
    uint8_t *pBase = NULL;
    uint32_t u4Reg = 0;
    if(bPortNumb >= MUC_NUM_PLATFORM_DEV)
    {
        return -ENODEV;
    }
        
    hcd = MUC_phcd[bPortNumb];
    printk("MUC_phcd[%d] = 0x%08X\n", bPortNumb, (uint32_t)hcd);

    if(!hcd)
    {
        return -ENODEV;
    }

    pThis = hcd_to_musbstruct(hcd);

    pBase = (uint8_t *) pThis->pRegs;
#ifdef UNIFIED_USB
        //force disable R45
        printk("[USB]reset root port\n");
        u4Reg = MGC_PHY_Read32(pBase, 0x1C);
        MGC_PHY_Write32( pBase, 0x1C, u4Reg | 0x00004000);
    
        mdelay(u4mdelay);
    
        u4Reg = MGC_PHY_Read32( pBase, 0x1C);
        MGC_PHY_Write32(pBase, 0x1C, u4Reg | 0x00002000);
    
        mdelay(u4mdelay);
    
        //enable R45
        u4Reg = MGC_PHY_Read32(pBase, 0x1C);
        MGC_PHY_Write32(pBase, 0x1C, u4Reg & ~0x00006000);
#else
    //disable & enable R45
    //RG_HS_TERM_EN_MODE[1:0] set to 10, force disable
    u4Reg = MGC_AnaPhy_Read32(pBase, 0x08);
    u4Reg |=  ((uint32_t)(0x1L << 25)); //RG_FSLS_ENBGRI set to 1. 
    MGC_AnaPhy_Write32(pBase, 0x08, u4Reg);

#if 0
    // set 0E410H, REL_SUSP turn-off
    u4Reg = MGC_PHY_Read32(pBase, 0x10);
    u4Reg &= ~(0x00010000); 
    MGC_PHY_Write32(pBase, 0x10, u4Reg);
#endif

    mdelay(u4mdelay);

    //disable & enable R45
    //RG_HS_TERM_EN_MODE[1:0] set to 10, force disable
    u4Reg = MGC_AnaPhy_Read32(pBase, 0x08);
    u4Reg |=  ((uint32_t)(0x1L << 24)); //RG_FSLS_ENBGRI set to 1. 
    MGC_AnaPhy_Write32(pBase, 0x08, u4Reg);

    mdelay(u4mdelay);

    //RG_HS_TERM_EN_MODE[1:0] set to 00, turn-on
    u4Reg = MGC_AnaPhy_Read32(pBase, 0x08);
    u4Reg &=  ~((uint32_t)(0x3L << 24)); //RG_FSLS_ENBGRI set to 0. 
    MGC_AnaPhy_Write32(pBase, 0x08, u4Reg);
    
#if 0
    // set 0E410H, REL_SUSP turn-on
    u4Reg = MGC_PHY_Read32(pBase, 0x10);
    u4Reg |= 0x00010000; 
    MGC_PHY_Write32(pBase, 0x10, u4Reg);
#endif
#endif

    //Set Speed
    pThis->fgIsWantFullSpeed = fgIsWantFullSpeed;

    return 0;        
}

static int MGC_UnlinkUrb(MGC_LinuxCd * pThis, struct urb *pUrb)
{
    unsigned long flags;
//    uint32_t u4UsbIrqEnable = 0;
    MGC_LinuxLocalEnd *pEnd;
    const void *pBase = pThis->pRegs;
    uint8_t bIndex;
    uint16_t wIntr;
    uint8_t toggle;
    
    DBG(-1, "<== pUrb=%p, pUrb->hcpriv=%p proto=%s \n", pUrb, pUrb->hcpriv,
        MGC_DecodeUrbProtocol(pUrb));

    //spin_lock_irqsave(&pThis->Lock, flags);

    //printk("Unlink pUrb=0x%X.\n", (uint32_t)pUrb);

    pEnd = (MGC_LinuxLocalEnd *)pUrb->hcpriv;
    if (pEnd)
    {
        spin_lock(&pEnd->Lock);
        if (pEnd->pCurrentUrb == pUrb)
        {
            MGC_DequeueEndurb(pEnd, pUrb);        
#if 0
            if(0x05AC == pUrb->dev->descriptor.idVendor &&  0x12 == (pUrb->dev->descriptor.idProduct >> 8) && usb_pipeisoc(pUrb->pipe) && !pEnd->bIsTx)
            {
                spin_unlock(&pEnd->Lock);
				spin_unlock_irqrestore(&pThis->Lock, flags);
                MGC_CallbackUrb(pThis, pUrb);
				spin_lock_irqsave(&pThis->Lock, flags);
                spin_lock(&pEnd->Lock);
                bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);
                MGC_SelectEnd((uint8_t *) pThis->pRegs, pEnd->bEnd);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, MGC_M_RXCSR_H_REQPKT);
                MGC_Write8(pBase, MGC_O_HDRC_INDEX, bIndex);
                spin_unlock(&pEnd->Lock);
                spin_unlock_irqrestore(&pThis->Lock, flags);
                return 0;
            }  
#endif
            bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);
            MGC_SelectEnd((uint8_t *) pThis->pRegs, pEnd->bEnd);
            // release dma channel.
            if (pEnd->pDmaChannel)
            {
                /* release previously-allocated channel */
                pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
                pEnd->pDmaChannel = NULL;
            }

            if (pEnd->bEnd)
            {
                if (!pEnd->bIsTx)
                {
                    uint16_t wVal;
                    wVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
                    wVal &= ~MGC_M_RXCSR_H_REQPKT;
                    MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);                
                    /* twice in case of double packet buffering */
                    MGC_WriteCsr16((uint8_t *) pThis->pRegs, MGC_O_HDRC_RXCSR,
                                   pEnd->bEnd,
                                   MGC_M_RXCSR_FLUSHFIFO);
                    MGC_WriteCsr16((uint8_t *) pThis->pRegs, MGC_O_HDRC_RXCSR,
                                   pEnd->bEnd,
                                   MGC_M_RXCSR_FLUSHFIFO);

                    // Clear interrupt.
                    wIntr = MGC_Read16(pBase, MGC_O_HDRC_INTRRX);
                    if (wIntr & (1 <<pEnd->bEnd))
                    {
                        MGC_Write16(pBase, MGC_O_HDRC_INTRRX, (1 <<pEnd->bEnd));
                    }                    

#ifdef UNIFIED_USB
                      toggle = ((MGC_Read32(pBase, M_REG_RXDATATOG)) >> pEnd->bEnd) & 0x01;
#else
                      toggle = ((MGC_MISC_Read32(pBase, M_REG_RXDATATOG)) >> pEnd->bEnd) & 0x01;
#endif
                }
                else
                {
                    /* twice in case of double packet buffering */
                    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pEnd->bEnd,
                                   MGC_M_TXCSR_FLUSHFIFO);
                    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pEnd->bEnd,
                                   MGC_M_TXCSR_FLUSHFIFO);

                    // Clear interrupt.
                    wIntr = MGC_Read16(pBase, MGC_O_HDRC_INTRTX);
                    if (wIntr & (1 <<pEnd->bEnd))
                    {
                        MGC_Write16(pBase, MGC_O_HDRC_INTRTX, (1 <<pEnd->bEnd));
                    }                    
                    
#ifdef UNIFIED_USB
                      toggle = ((MGC_Read32(pBase, M_REG_TXDATATOG)) >> pEnd->bEnd) & 0x01;
#else
                      toggle = ((MGC_MISC_Read32(pBase, M_REG_TXDATATOG)) >> pEnd->bEnd) & 0x01;
#endif                  
                }
                
            /*patch ,shouldn't clear toggle, save toggle*/
                usb_settoggle(pUrb->dev, pEnd->bRemoteEnd, (pEnd->bIsTx) ? 1 : 0,
                  toggle);
            }
            else
            {
                /* endpoint 0: just flush */
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, pEnd->bEnd,
                               MGC_M_CSR0_FLUSHFIFO);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, pEnd->bEnd,
                               MGC_M_CSR0_FLUSHFIFO);

                wIntr = MGC_Read16(pBase, MGC_O_HDRC_INTRTX);
                if (wIntr & 1)
                {
                    MGC_Write16(pBase, MGC_O_HDRC_INTRTX, 1);
                }                                    
            }

            /* restore index */
            MGC_Write8(pBase, MGC_O_HDRC_INDEX, bIndex);
        }
        else
        {
#if 0
            if(0x05AC == pUrb->dev->descriptor.idVendor &&  0x12 == (pUrb->dev->descriptor.idProduct >> 8) && usb_pipeisoc(pUrb->pipe) && !pEnd->bIsTx)
            {
                spin_unlock(&pEnd->Lock);
				spin_unlock_irqrestore(&pThis->Lock, flags);
                MGC_CallbackUrb(pThis, pUrb);
				spin_lock_irqsave(&pThis->Lock, flags);
                spin_lock(&pEnd->Lock);
                bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);
                MGC_SelectEnd((uint8_t *) pThis->pRegs, pEnd->bEnd);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, MGC_M_RXCSR_H_REQPKT);
                MGC_Write8(pBase, MGC_O_HDRC_INDEX, bIndex);
                MGC_DequeueEndurb(pEnd, pUrb);
                spin_unlock(&pEnd->Lock);
                spin_unlock_irqrestore(&pThis->Lock, flags);
                return 0;
            }          
#endif			
            MGC_DequeueEndurb(pEnd, pUrb);
        }
        spin_unlock(&pEnd->Lock);
    } 
    //spin_unlock_irqrestore(&pThis->Lock, flags);
    MGC_CallbackUrb(pThis, pUrb);

    return 0;
}


//-------------------------------------------------------------------------
/** MUC_ResetPhy
 *  host controller register reset and initial.
 *  @param  void 
 *  @return  void
 */
//-------------------------------------------------------------------------
static int MUC_ResetPhy(void *pBase)
{
    uint32_t u4Reg = 0;
//  uint8_t nDevCtl = 0;
    MUSB_ASSERT(pBase);
    
    u4Reg  = MGC_CKGEN_Read32(0x284);
    u4Reg &= ~0x00000001; 
    MGC_CKGEN_Write32(0x284, u4Reg);

    u4Reg = MGC_CKGEN_Read32(0xA0);
    u4Reg |= (0x1 << 13);
    MGC_CKGEN_Write32(0xA0, u4Reg);

    u4Reg  = MGC_PHY_Read32(pBase,0);
    if(!(u4Reg&0x00004000)){
        u4Reg |= 0x00004000;
        MGC_PHY_Write32(pBase,0,u4Reg);
    }

    //Reset USB
    MGC_PHY_Write32(pBase,0x68,MGC_PHY_Read32(pBase,0x68) |   0x00004000);
    MGC_PHY_Write32(pBase,0x68,MGC_PHY_Read32(pBase,0x68) & (~0x00004000));

    //otg bit setting
    u4Reg = MGC_PHY_Read32(pBase,0x6C);
    u4Reg &= ~0x3F3F;
    u4Reg |=  0x3E2C;
    MGC_PHY_Write32(pBase, 0x6C,u4Reg);

    //suspendom control
    u4Reg = MGC_PHY_Read32(pBase,0x68);
    u4Reg &=  ~0x00040000; 
    MGC_PHY_Write32(pBase,0x68,u4Reg);

    //PLL setting to reduce clock jitter
    u4Reg = MGC_PHY_Read32(pBase, 0x0);
    u4Reg &= ~0x70000000;
    MGC_PHY_Write32(pBase, 0x0, u4Reg);

    //hs eye finetune
    u4Reg = MGC_PHY_Read32(pBase,0x10);
    u4Reg &=  ~0x00070000;
    u4Reg |= 0x00050000;
    MGC_PHY_Write32(pBase, 0x10, u4Reg);

    //PLL setting to reduce clock jitter
    u4Reg = MGC_PHY_Read32(pBase, 0x0);
    u4Reg &= ~0x70000000;
    u4Reg |= 0x20000000;
    MGC_PHY_Write32(pBase, 0x0, u4Reg);

    u4Reg = MGC_PHY_Read32(pBase, 0x4);
    u4Reg |= 0x3;
    MGC_PHY_Write32(pBase, 0x4, u4Reg);
    //End of PLL setting

    //For FS/LS eye pattern fine-tune
    u4Reg = MGC_PHY_Read32(pBase, 0x10);
    u4Reg &= ~0x00007707;
    u4Reg |=  0x00005503;
    MGC_PHY_Write32(pBase, 0x10, u4Reg);

    u4Reg = MGC_Read8(pBase,M_REG_PERFORMANCE3);
    u4Reg |=  MGC_M_BUSPERF3_NOISESTILL_SOF;
    u4Reg &= ~MGC_M_BUSPERF3_BAB_CLR_EN;
    MGC_Write8(pBase, M_REG_PERFORMANCE3, (u8)u4Reg);

    //USB disconnect debounce setting
    u4Reg = MGC_Read16(pBase, 0x604);
    u4Reg |= 0xF0;
    MGC_Write16(pBase, 0x604, (u16)u4Reg);
    
    // RG_USB20_DISCTH[3:0], Default value 1000 mean 560mV ,change to 1100 mean 680mV, for fix extension cable issue.
    MGC_PHY_Write32(pBase, 0x18, MGC_PHY_Read32(pBase, 0x18) | 0x000E0000);

    return 0;
}

extern void ac83xx_mask_ack_bim_irq(unsigned int irq);
//-------------------------------------------------------------------------
/** MUC_Irq
 *  system usb irq entry point.
 *  @param   irq                interrupt number.
 *  @param   *_prHcd       point to mtkhcd data structure.
 *  @param   *regs           cpu register before interrupt.
 *  @return  irqreturn_t    irq status: IRQ_HANDLED means successfully irq handled.
 */
//-------------------------------------------------------------------------
static irqreturn_t MUC_Irq(struct usb_hcd *hcd)
{
    MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);

    const void *pBase = pThis->pRegs;
    uint32_t nSource = 0;
    uint8_t bIntrUsbValue;
    uint16_t wIntrTxValue;
    uint16_t wIntrRxValue;
    uint32_t dwLineState;
    uint8_t bIndex;
    uint8_t bShift = 0;
    uint32_t reg;

#ifdef UNIFIED_USB
#if USB_SUSPEND_TEST
   uint32_t u4IntrL1ints;
#endif   
   uint32_t dwDmaIntr;// = MGC_Read8(pBase, MGC_O_HSDMA_INTR);
//#else
//   uint32_t dwDmaIntr = MGC_Read32(pBase, MGC_O_HSDMA_INTR);
#endif
#ifdef USB_IRQ_LOCK
   unsigned long flags = 0;
   spin_lock_irqsave(&pThis->Lock, flags);
#endif


#ifdef UNIFIED_USB
   dwDmaIntr = MGC_Read8(pBase, MGC_O_HSDMA_INTR);
#else
   dwDmaIntr = MGC_Read32(pBase, MGC_O_HSDMA_INTR);
#endif

#ifdef UNIFIED_USB
    #if USB_SUSPEND_TEST
        u4IntrL1ints = MGC_Read32(pBase,M_REG_INTRLEVEL1);
        DBG(2, "Level1 Init: 0x%08X\n", u4IntrL1ints);
    #endif
#endif  

    bIntrUsbValue = MGC_Read8(pBase, MGC_O_HDRC_INTRUSB);
    wIntrTxValue = MGC_Read16(pBase, MGC_O_HDRC_INTRTX);
    wIntrRxValue = MGC_Read16(pBase, MGC_O_HDRC_INTRRX);
#ifdef CONFIG_USB_QUEUE
    pThis->int_queue = MGC_Read32(pBase, M_REG_QISAR);
#endif
    
#ifdef UNIFIED_USB
    #if USB_SUSPEND_TEST
        u4IntrL1ints = MGC_Read32(pBase, M_REG_INTRLEVEL1);
        u4IntrL1ints &= MGC_Read32(pBase, M_REG_INTRLEVEL1EN);
        
        if(MGC_M_L1INTS_DPDM_INT & u4IntrL1ints)
        {
            reg = MGC_Read32(pBase,(uint32_t)M_REG_INTRLEVEL1EN);
            reg &= ~((uint32_t)0x80);
            MGC_Write32(pBase, M_REG_INTRLEVEL1EN, reg);
        }
    #endif
#endif  
    
    /* save index */
    bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);

    /* ### DMA intr handler added */
    if ((pThis->pDmaController) &&
        (pThis->pDmaController->pfDmaControllerIsr))
    {
        if (pThis->pDmaController->pfDmaControllerIsr(pThis->pDmaController->pPrivateData))
        {
            nSource |= 1;
        }
    }

    if (bIntrUsbValue)
    {
        MGC_Write8(pBase, MGC_O_HDRC_INTRUSB, bIntrUsbValue);

        bIntrUsbValue &= MGC_Read8(pBase, MGC_O_HDRC_INTRUSBE);
    }

    if (wIntrTxValue)
    {
        MGC_Write16(pBase, MGC_O_HDRC_INTRTX, wIntrTxValue);

        wIntrTxValue &= MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);
    }

    if (wIntrRxValue)
    {
        MGC_Write16(pBase, MGC_O_HDRC_INTRRX, wIntrRxValue);

        wIntrRxValue &= MGC_Read16(pBase, MGC_O_HDRC_INTRRXE);
    }

#ifdef CONFIG_USB_QUEUE
    if(pThis->int_queue)
    {
        MGC_Write32(pBase, M_REG_QISAR, pThis->int_queue);
        pThis->int_queue &= ~(MGC_Read32(pBase, M_REG_QIMR));
    }
#endif

    if(!dwDmaIntr && !bIntrUsbValue && !wIntrTxValue && !wIntrRxValue
#ifdef UNIFIED_USB
#if USB_SUSPEND_TEST
       && !u4IntrL1ints    
#endif
#ifdef CONFIG_USB_QUEUE
       && !pThis->int_queue
#endif
#endif
      ){
        printk("[%d] NULL Interrupt\n", pThis->nIrq);
      }

#ifdef UNIFIED_USB
#if USB_SUSPEND_TEST
    if(MGC_M_L1INTS_DPDM_INT & u4IntrL1ints)
    {       
        DBG(2, "[USB INTR] DPDM_INT\n");
        reg = MGC_PHY_Read32(pBase,(uint32_t)0x68);
        reg &=  ~0x00040000; //Resume PHY
        MGC_PHY_Write32(pBase, (uint32_t)0x68, reg);

        if( bIntrUsbValue || wIntrTxValue || wIntrRxValue)
        {
            printk("Resume PHY\n");       
            DBG(2, "bIntrUsbValue: 0x%08X\n", bIntrUsbValue);       
            DBG(2, "wIntrTxValue : 0x%08X\n", wIntrTxValue);       
            DBG(2, "wIntrRxValue : 0x%08X\n", wIntrRxValue);       
        }
        else
        {
            printk("Resume PHY ONLY\n"); 
        #ifdef USB_IRQ_LOCK 
            spin_unlock_irqrestore(&pThis->Lock, flags);
        #endif  
            ac83xx_mask_ack_bim_irq(pThis->nIrq);
            return 0;
        }
    }
#endif
#endif

    if (bIntrUsbValue == (MGC_M_INTR_CONNECT | MGC_M_INTR_DISCONNECT))
    {
        DBG(2, "Connect/Disconnet interrupt = 0x%X.\n", bIntrUsbValue);

        // treat it as disconnect interrupt only.
        bIntrUsbValue &= ~MGC_M_INTR_CONNECT;
    }

    if (bIntrUsbValue & MGC_M_INTR_CONNECT)
    {
        printk("[USB]-------------------- Start Mount USB Device on port1 --------------------\n");
     #ifdef UNIFIED_USB
        dwLineState = MGC_Read32(pBase,M_REG_DEBUG_PROBE);
        //printk("[usb]EMULATION: software set ok linestate directly\n");
 
     #else  
        dwLineState = MGC_PHY_Read32(pBase, M_REG_PHYC5);
     #endif 
        dwLineState &= M_REG_LINESTATE_MASK;


        printk("[USB] Device is connected\n");

        DBG(2, "Connect interrupt  = 0x%X.\n", bIntrUsbValue);

        //kill timer.
        //MGC_DelTimer(pThis);

        if ((!pThis->bInsert) && (dwLineState != LINESTATE_DISCONNECT))
        {
            pThis->bCheckInsert = 0;
            //MGC_SetTimer(pThis, MGC_CheckConnect, (unsigned long) pThis,
            //             MGC_CHECK_INSERT_DEBOUNCE);
            
            mod_timer(&pThis->events_timer, jiffies
                    + msecs_to_jiffies(
                            MGC_CHECK_INSERT_DEBOUNCE));
            pThis->events = USB_EVENTS_CHECK_CONNECT;
        }

        // When connect and disconnet happen at the same time,
        // we only handle connect event. This will happen at Vbus not stable when cable is  inserted.
        bIntrUsbValue &= ~(MGC_M_INTR_CONNECT);
    }
    else if (bIntrUsbValue & MGC_M_INTR_DISCONNECT)        // disconnect event.   
    {
            uint16_t regval; 
        
            udelay(30);
            regval = MGC_Read32(pBase,0x630);
            if (!((regval >> 5) & 0x1)) {
        		printk("[USB] Device is not real disconnected\n");
        		bIntrUsbValue &= ~(MGC_M_INTR_DISCONNECT);
#ifdef USB_IRQ_LOCK 
          		spin_unlock_irqrestore(&pThis->Lock, flags);
#endif  
                ac83xx_mask_ack_bim_irq(pThis->nIrq);
        		return IRQ_HANDLED;
            }
        
            printk("[USB] Device is disconnected\n");
            DBG(2, "Disconnect interrupt  = 0x%X.\n", bIntrUsbValue);

#if CONFIG_USB_DETECT_IMPROVE
        if(pThis->bInsert || !MGC_IsDevAttached(pBase))
#endif
        {
            //pThis->fgIsWantFullSpeed = FALSE;
            pThis->fgIsWantFullSpeed = config_usb11() ? TRUE : FALSE;
            //2009.09.21
            printk("[USB] update hub status\n");
            
            pThis->dwVirtualHubPortStatus = (1 << USB_PORT_FEAT_POWER)
                                            | (1 << USB_PORT_FEAT_C_CONNECTION);            
            pThis->dwVirtualHubPortStatus &= ~(1 << USB_PORT_FEAT_CONNECTION);
            usb_hcd_poll_rh_status(hcd);

            //kill timer.
  //          MGC_DelTimer(pThis);

            if (!pThis->bInsert)
            {
                // no need to handle this disconnect event.
                bIntrUsbValue &= ~(MGC_M_INTR_DISCONNECT);
            }
            else
            {
                pThis->bInsert = FALSE;
            }
            
            //MGC_SetTimer(pThis, MGC_UnlinkInvalidUrb, (unsigned long) pThis, 20);
            mod_timer(&pThis->events_timer, jiffies
                    + msecs_to_jiffies(
                            20));
            pThis->events = USB_EVENTS_UNLINK_INVALID_URB;
#ifdef UNIFIED_USB
            #if USB_SUSPEND_TEST
                printk("Suspend USB PHY\n");
                reg = MGC_PHY_Read32(pBase,(uint32_t)0x68);
                reg |=  0x00040000; // Suspend USB PHY.
                MGC_PHY_Write32(pBase, (uint32_t)0x68, reg);

                reg = MGC_Read32(pBase,(uint32_t)0xA4); //M_REG_INTRLEVEL1EN
                reg |= ((uint32_t)0x80); //Enable DPDM_INT
                MGC_Write32(pBase, 0xA4, reg); //M_REG_INTRLEVEL1EN
            #endif
#endif      
}
#if CONFIG_USB_DETECT_IMPROVE
            else
            {
                bIntrUsbValue &= ~(MGC_M_INTR_DISCONNECT);
                printk("[USB]check linestate later!\n");
            }
        #endif
}

#ifdef CONFIG_USB_QUEUE
    nSource |= bIntrUsbValue | wIntrTxValue | wIntrRxValue | pThis->int_queue;
#else
    nSource |= bIntrUsbValue | wIntrTxValue | wIntrRxValue;
#endif

    if (!nSource)
    {
        /* restore index */
        MGC_Write8(pBase, MGC_O_HDRC_INDEX, bIndex);
   #ifdef USB_IRQ_LOCK      
        spin_unlock_irqrestore(&pThis->Lock, flags);
   #endif
        ac83xx_mask_ack_bim_irq(pThis->nIrq);
        return IRQ_HANDLED;
    }

    if (bIntrUsbValue)
    {
        MGC_ServiceUSBIntr(pThis, bIntrUsbValue);
    }

#ifdef CONFIG_USB_QUEUE
    /* process generic queue interrupt*/
    if (pThis->int_queue){
        mtk_q_interrupt(pThis);
    }   
#endif  

    /* handle tx/rx on endpoints; each bit of wIntrTxValue is an endpoint, 
     * endpoint 0 first (p35 of the manual) bc is "SPECIAL" treatment; 
     * WARNING: when operating as device you might start receving traffic 
     * to ep0 before anything else happens so be ready for it */

    reg = wIntrTxValue;
    if (reg & 1)
    {                                /* EP0 */
        MGC_ServiceDefaultEnd(pThis);
    }

    /* TX on endpoints 1-15 */
    bShift = 1;
    reg >>= 1;

    while (reg)
    {
        if (reg & 1)
        {
            MGC_ServiceTxAvail(pThis, bShift);
        }

        reg >>= 1;
        bShift++;
    }
	//MGC_Write16(pBase, MGC_O_HDRC_INTRTX, wIntrTxValue);

    /* RX on endpoints 1-15 */
    reg = wIntrRxValue;
    bShift = 1;
    reg >>= 1;

    while (reg)
    {
        if (reg & 1)
        {
            MGC_ServiceRxReady(pThis, bShift);
        }

        reg >>= 1;
        bShift++;
    }
	//MGC_Write16(pBase, MGC_O_HDRC_INTRRX, wIntrRxValue);

    /* restore index */
    MGC_Write8(pBase, MGC_O_HDRC_INDEX, bIndex);
#ifdef USB_IRQ_LOCK   
      spin_unlock_irqrestore(&pThis->Lock, flags);
#endif

    //Clear system interrupt status.
    ac83xx_mask_ack_bim_irq(pThis->nIrq);
    return IRQ_HANDLED;
}

//-------------------------------------------------------------------------
/** MUC_urb_enqueue
 *  queue and process user's urb sequentially.
 *  @param   *hcd           system usb structure.
 *  @param   *urb             user's request block. 
 *  @param   mem_flags    malloc flag. 
 *  @retval   0            Success
 *  @retval  others Fail
 */
//-------------------------------------------------------------------------
static int MUC_urb_enqueue(struct usb_hcd *hcd, struct urb *pUrb, gfp_t mem_flags)
{
    MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);
    int rc;
    //uint32_t u4UsbIrqEnable = 0;
    unsigned long flags = 0;
    MGC_LinuxLocalEnd *pEnd = NULL;
    unsigned int pipe;

    DBG(2, "pUrb=0x%p, pUrb->hcpriv=%p proto=%s\n",
        pUrb, pUrb->hcpriv, MGC_DecodeUrbProtocol(pUrb));

    //SPIN_LOCK_IRQSAVE(&pThis->Lock, flags, pThis->nIrq);
    spin_lock_irqsave(&pThis->Lock, flags);

    /* find appropriate local endpoint to do it */
    pEnd = MGC_FindEnd(pThis, pUrb);

    if (pEnd == NULL)
    {
        pipe = pUrb ? pUrb->pipe : 0;
        pUrb->status = USB_ST_URB_REQUEST_ERROR;
        ERR("==> no resource for proto=%d, addr=%d, end=%d\n",
            usb_pipetype(pipe), usb_pipedevice(pipe),
            usb_pipeendpoint(pipe));

        //SPIN_UNLOCK_IRQRESTORE(&pThis->Lock, flags, pThis->nIrq);
        spin_unlock_irqrestore(&pThis->Lock, flags);
        return USB_ST_URB_REQUEST_ERROR;
    }

    /* if no root device, assume this must be it */
    if (!pThis->pRootDevice)
    {
        pThis->pRootDevice = pUrb->dev;
    }

    //printk("Enqueue pUrb=0x%X.\n", (uint32_t)pUrb);
    spin_lock(&pEnd->Lock);

    pEnd->mem_flags = mem_flags;
   

    rc = MGC_ScheduleUrb(pThis, pEnd, pUrb);

    spin_unlock(&pEnd->Lock);

    //SPIN_UNLOCK_IRQRESTORE(&pThis->Lock, flags, pThis->nIrq);
    spin_unlock_irqrestore(&pThis->Lock, flags);

    return rc;
}

//-------------------------------------------------------------------------
/** MUC_urb_dequeue
 *  kill user's urb.
 *  @param   *hcd           system usb structure.
 *  @param   *urb             user's request block. 
 *  @retval   0            Success
 *  @retval  others Fail
 */
//-------------------------------------------------------------------------
static int MUC_urb_dequeue(struct usb_hcd *hcd, struct urb *pUrb, int status)
{
    MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);
    int result = 0;
    MGC_LinuxLocalEnd *pEnd;
    unsigned long flags = 0;

    DBG(2, "<== pUrb=%p\n", pUrb);

    if (!pUrb)
    {
        DBG(2, "==> invalid urb%p, pUrb->hcpriv=%p\n", pUrb,
            (pUrb) ? pUrb->hcpriv : NULL);
        return -EINVAL;
    }

    if (!pUrb->dev || !pUrb->dev->bus)
    {
        DBG(2, "==>\n");
        return -ENODEV;
    }

    if (!pThis)
    {
        ERR("==> pThis is null: stopping before unlink\n");
        return -ENODEV;
    }

    pEnd = (MGC_LinuxLocalEnd *)pUrb->hcpriv;
    
    if(!pEnd)
    {
        ERR("==> pEnd is null: stopping before unlink\n");
        return -ENODEV;   
    }
    spin_lock_irqsave(&pThis->Lock, flags);
    printk("[usb]dequeue urb(0x%x) L_EP:%d %s\n",(uint32_t)pUrb,
        pEnd->bEnd,pEnd->bIsTx?"TX":"RX");
    if(result = MGC_CheckDequeueurb(pEnd, pUrb, status))
    {
        pUrb->status = status;
        spin_unlock_irqrestore(&pThis->Lock, flags);
        return result;
    }
    
    pUrb->status = status;
  #ifdef CONFIG_USB_QUEUE   
    if(mtk_ep_q_used(pEnd)){
      
       spin_lock(&pEnd->Lock);  
       //unlink all urbs on this ep
         while(!MGC_IsEndIdle(pEnd)){   
            pUrb = MGC_GetNextUrb(pEnd);
            MGC_DequeueEndurb(pEnd, pUrb);
            
            pUrb->status = status;
            
            spin_unlock(&pEnd->Lock);
			spin_unlock_irqrestore(&pThis->Lock, flags); 
            MGC_CallbackUrb(pThis, pUrb);
			spin_lock_irqsave(&pThis->Lock, flags);
            spin_lock(&pEnd->Lock); 
         } 
         
       spin_unlock(&pEnd->Lock);
       //disable Q        
       mtk_disable_q(pThis, pEnd->bEnd, !pEnd->bIsTx);
       
    }
    else
  #endif        
    {
    result = MGC_UnlinkUrb(pThis, pUrb);
    }
    spin_unlock_irqrestore(&pThis->Lock, flags); 
    return result;
}

//-------------------------------------------------------------------------
/** MUC_endpoint_disable
 *  disable user's ep.
 *  @param   *hcd             system usb hcd structure.
 *  @param   *hdev           system hcd device structure.
 *  @param   epnum          ep number.
 *  @return   void
 */
//-------------------------------------------------------------------------
static void MUC_endpoint_disable(struct usb_hcd *hcd, 
    struct usb_host_endpoint *hep)
{
    MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);
//    uint32_t u4UsbIrqEnable = 0;
    unsigned long flags = 0;
    MGC_LinuxLocalEnd *pEnd;
    uint8_t *pBase = (uint8_t *) pThis->pRegs;

    if (hep && hep->hcpriv)
    {
        spin_lock_irqsave(&pThis->Lock, flags);

        pEnd = (MGC_LinuxLocalEnd *)hep->hcpriv;
        if (pEnd)
        {
            spin_lock(&pEnd->Lock);

            printk("[usb]Disable h/w %s ep%d.\n", (pEnd->bIsTx ? "Out": "In"), pEnd->bEnd);           
            if (pEnd->dev || pEnd->bIsOccupy)            
            {
                uint8_t bIndex = 0;
#if 0
                if(pEnd->bIsSharedFifo == TRUE)
                {
                    /* save index */
                    bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);
                    MGC_SelectEnd(pBase, pEnd->bEnd);
                    // fifo size = 512
                    MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, 6);
                    /* restore index */
                    MGC_SelectEnd(pBase, bIndex);
                    pEnd->wMaxPacketSize = 512;
                    pEnd->bIsSharedFifo = FALSE;
               }
                
                 if(pEnd->bUseTxRxFifo == TRUE)
                 {
                     /* save index */
                     bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);
                     MGC_SelectEnd(pBase, pEnd->bEnd);
                     // fifo size = 512
                     if(pEnd->bIsTx)
                     {
                       MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, 6);
                       MGC_Write8(pBase, MGC_O_HDRC_TXFIFOADD, 
                        MGC_Read8(pBase, MGC_O_HDRC_RXFIFOADD) + 0x40);
                       //release rx 
                       pThis->aLocalEnd[0][pEnd->bEnd].bIsOccupy = FALSE;
                       pThis->aLocalEnd[0][pEnd->bEnd].dev = NULL;
                       
                     }
                     else
                     {
                       MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, 6);
                       //release tx
                       pThis->aLocalEnd[1][pEnd->bEnd].bIsOccupy = FALSE;
                       pThis->aLocalEnd[1][pEnd->bEnd].dev = NULL;                     
                     }
                     
                     /* restore index */
                     MGC_SelectEnd(pBase, bIndex);
                     pEnd->wMaxPacketSize = 512;
                     pEnd->bUseTxRxFifo= FALSE;
                }
#endif
                if(pEnd->dev)
                {
                    //if(0x05AC == pEnd->dev->descriptor.idVendor &&  0x12 == (pEnd->dev->descriptor.idProduct >> 8))
                    {
                        bIndex = MGC_Read8(pBase, MGC_O_HDRC_INDEX);
                        MGC_SelectEnd(pBase, pEnd->bEnd);
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd, 0);
                        MGC_SelectEnd(pBase, bIndex);
                    }
                }
#ifdef CONFIG_USB_QUEUE
                if(!mtk_ep_q_used(pEnd))
#endif      
                {
                  flush_ep_csr(pThis, pEnd->bEnd, !pEnd->bIsTx);
                }

                if(pEnd->used_num > 0)
                    pEnd->used_num -= 1;

                if(pEnd->used_num == 0)
                {
                         struct urb* pUrb;
                       //unlink all urbs on this ep
                         while(!MGC_IsEndIdle(pEnd)){
                            pUrb = MGC_GetNextUrb(pEnd);
                            MGC_DequeueEndurb(pEnd, pUrb);
                            
                            pUrb->status = -ESHUTDOWN;
                  
                            spin_unlock(&pEnd->Lock);
                            spin_unlock_irqrestore(&pThis->Lock, flags);
                            MGC_CallbackUrb(pThis, pUrb);
                            spin_lock_irqsave(&pThis->Lock, flags);
                            spin_lock(&pEnd->Lock); 
                }
                         
#ifdef CONFIG_USB_QUEUE
                    if(mtk_ep_q_used(pEnd))
                    {
                       //disable Q        
                       mtk_disable_q(pThis, pEnd->bEnd, !pEnd->bIsTx);             
                }
                  #endif
                    if(MGC_IsEndIdle(pEnd)){
                        if(pEnd->list_buf){
                          kfree(pEnd->list_buf);
                          pEnd->list_buf = NULL;
                        }
                    }
                    else{
                        printk("[usb]endpoint#%d is busy !!!\n",pEnd->bEnd);
                    }
                    pEnd->dev = NULL;
                    pEnd->bIsOccupy = FALSE;
                    //aIsLocalEndOccupy[pEnd->bIsTx][pEnd->bEnd] = FALSE;
                    mtk_release_ep_fifo(pThis,pEnd,hep->desc.wMaxPacketSize,!pEnd->bIsTx);
                }
                
            }

            spin_unlock(&pEnd->Lock);
        }
        
        hep->hcpriv = NULL;

        spin_unlock_irqrestore(&pThis->Lock, flags);       
    }
    return;
}

//-------------------------------------------------------------------------
/** MUC_get_frame
 *  Get sof frame variable.
 *  @param   *hcd             system usb hcd structure.
 *  @return   frame number
 */
//-------------------------------------------------------------------------
static int MUC_get_frame(struct usb_hcd *hcd)
{
    MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);

    /* wrong except while periodic transfers are scheduled;
     * never matches the on-the-wire frame;
     * subject to overruns.
     */
    //return pThis->frame;
    (void) pThis;
    return 0;
}

//-------------------------------------------------------------------------
/** MUC_hub_status_data
 *  Virtual root hub status api.
 *  @param   *hcd             system usb hcd structure.
 *  @param   *buf             point to status buffer.
 *  @retval   0            No change hub status.
 *  @retval   1      Change hub status.
 */
//-------------------------------------------------------------------------
//caller may be the irq
static int MUC_hub_status_data(struct usb_hcd *hcd, char *buf)
{
    MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);
    //unsigned long flags;
    int32_t retval;
    //uint32_t u4UsbIrqEnable = 0;
    
    //spin_lock_irqsave(&pThis->Lock, flags);

    if (!(pThis->dwVirtualHubPortStatus & MTK_PORT_C_MASK))
    {
        retval = 0;
    }
    else
    {
        // Hub port status change. Port 1 change detected.
        *buf = (1 << 1);

        DBG(3, "port status 0x%08X has changes\n",
            pThis->dwVirtualHubPortStatus);
        retval = 1;
    }

    //spin_unlock_irqrestore(&pThis->Lock, flags);

    return retval;
}

//-------------------------------------------------------------------------
/** MUC_hub_control
 *  Virtual root hub control api.
 *  @param   *hcd             system usb hcd structure.
 *  @param   typeReq        request code.
 *  @param   wValue         request value.
 *  @param   wIndex         request index.
 *  @param   *buf             point to status buffer.
 *  @param   wLength       data length.
 *  @retval   0            Success.
 *  @retval   others   Fail.
 */
//-------------------------------------------------------------------------
static int MUC_hub_control(struct usb_hcd *hcd, uint16_t typeReq,
                           uint16_t wValue, uint16_t wIndex, char *buf,
                           uint16_t wLength)
{
    MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);
    void *pBase = pThis->pRegs;
    int32_t retval = 0;
    unsigned long flags;
    //uint32_t u4UsbIrqEnable = 0;
    uint8_t power;

    DBG(3, "= 0x%X, 0x%X.\n", typeReq, wValue);

    spin_lock_irqsave(&pThis->Lock, flags);

    switch (typeReq)
    {
    case ClearHubFeature:
    case SetHubFeature:
        switch (wValue)
        {
        case C_HUB_OVER_CURRENT:
        case C_HUB_LOCAL_POWER:
            break;

        default:
            goto error;
        }

        break;

    case ClearPortFeature:
        if (wIndex != 1 || wLength != 0)
            goto error;

        switch (wValue)
        {
        case USB_PORT_FEAT_ENABLE:
         //   pThis->dwVirtualHubPortStatus &= (1 << USB_PORT_FEAT_POWER);
            break;

        case USB_PORT_FEAT_SUSPEND:
            if (!
                (pThis->
                 dwVirtualHubPortStatus & (1 << USB_PORT_FEAT_SUSPEND)))
                break;

            /* 20 msec of resume/K signaling, other irqs blocked */
            DBG(3, "start resume...\n");
            break;

        case USB_PORT_FEAT_POWER:
            break;

        case USB_PORT_FEAT_C_ENABLE:
        case USB_PORT_FEAT_C_SUSPEND:
        case USB_PORT_FEAT_C_CONNECTION:
        case USB_PORT_FEAT_C_OVER_CURRENT:
        case USB_PORT_FEAT_C_RESET:
            break;

        default:
            goto error;
        }

        pThis->dwVirtualHubPortStatus &= ~(1 << wValue);
        break;

    case GetHubDescriptor:
        MUC_hub_descriptor(pThis, (struct usb_hub_descriptor *) buf);
        break;

    case GetHubStatus:
        *(__le32 *) buf = cpu_to_le32(0);
        break;

    case GetPortStatus:
        if (wIndex != 1)
            goto error;
        if ((pThis->dwVirtualHubPortStatus & (1<<USB_PORT_FEAT_RESET)) && time_after_eq(jiffies, reset_done))
        {
    	    reset_done = 0;
			MGC_ResetOff(pThis);
        }

        *(__le32 *) buf = cpu_to_le32(pThis->dwVirtualHubPortStatus);

#ifndef VERBOSE

        if (*(uint16_t *) (buf + 2))        /* only if wPortChange is interesting */
#endif

            DBG(3, "GetPortStatus = 0x%08X.\n",
                pThis->dwVirtualHubPortStatus);

        break;

    case SetPortFeature:
        if (wIndex != 1 || wLength != 0)
            goto error;

        switch (wValue)
        {
        case USB_PORT_FEAT_ENABLE:
            break;

        case USB_PORT_FEAT_SUSPEND:
            if (pThis->dwVirtualHubPortStatus & (1 << USB_PORT_FEAT_RESET))
                goto error;

            if (!
                (pThis->
                 dwVirtualHubPortStatus & (1 << USB_PORT_FEAT_ENABLE)))
                goto error;

            DBG(3, "suspend...\n");
            break;

        case USB_PORT_FEAT_POWER:
            break;

        case USB_PORT_FEAT_RESET:
            if (pThis->
                dwVirtualHubPortStatus & (1 << USB_PORT_FEAT_SUSPEND))
                goto error;

            if (!
                (pThis->
                 dwVirtualHubPortStatus & (1 << USB_PORT_FEAT_POWER)))
                break;

            // reset port.
            power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
            power &= ~(MGC_M_POWER_SUSPENDM |MGC_M_POWER_ENSUSPEND);    

            if(pThis->fgIsWantFullSpeed)
            {
                printk("[USB] FS Reset\n");
                power &= ~MGC_M_POWER_HSENAB;
            }
            else
            {
                printk("[USB] HS Reset\n");
                power |= MGC_M_POWER_HSENAB;
            }
	    if (pThis->dwVirtualHubPortStatus & (1 << USB_PORT_FEAT_C_CONNECTION))
		pThis->dwVirtualHubPortStatus &=~(1 << USB_PORT_FEAT_C_CONNECTION);
            MGC_Write8(pBase, MGC_O_HDRC_POWER, power | MGC_M_POWER_RESET);
            //mod_timer(&pThis->events_timer, jiffies + msecs_to_jiffies(50));
            //pThis->events = USB_EVENTS_RESET_OFF;
            reset_done = jiffies + msecs_to_jiffies(50);

            break;

        default:
            goto error;
        }

        pThis->dwVirtualHubPortStatus |= 1 << wValue;
        break;

    default:
      error:
        /* "protocol stall" on error */
        retval = -EPIPE;
    }

    spin_unlock_irqrestore(&pThis->Lock, flags);
    return retval;
}

#if 0
static void vbus_power_init(void)
{
#if 0
    /* config power-controller gpio */
    GPIO_MultiFun_Set(PIN_2_GPIO2, PINMUX_LEVEL_GPIO_END_FLAG);
    
    gpio_request(PIN_2_GPIO2, "usb_power_switch");
    gpio_direction_output(PIN_2_GPIO2, 0);
    gpio_set_value(PIN_2_GPIO2, 0);
#endif
}

static void vbus_power_reset(void)
{
#if 0
    gpio_set_value(PIN_2_GPIO2, 1);
    msleep(200);
    gpio_set_value(PIN_2_GPIO2, 0);
#endif
}
#endif
static int MUC_reset_device(struct usb_hcd *hcd, struct usb_device *dev)
{
    /* power-down usb device */
    printk("++ MUC_reset_device ++\n");
    usb1_vbus_power_reset();
    return 1;
}

//-------------------------------------------------------------------------
/** MUC_bus_suspend
 *  Virtual root hub suspend.
 *  @param   *hcd             system usb hcd structure.
 *  @retval   0           Success.
 *  @retval   1      Fail.
 */
//-------------------------------------------------------------------------
static int MUC_bus_suspend(struct usb_hcd *hcd)
{
// SOFs off
    return 0;
}

//-------------------------------------------------------------------------
/** MUC_bus_resume
 *  Virtual root hub resume.
 *  @param   *hcd             system usb hcd structure.
 *  @retval   0           Success.
 *  @retval   1      Fail.
 */
//-------------------------------------------------------------------------
static int MUC_bus_resume(struct usb_hcd *hcd)
{
// SOFs on
    return 0;
}

//-------------------------------------------------------------------------
/** MUC_hub_descriptor
 *  get root hub descriptor.
 *  @param  *pHcd           mtkhcd pointer.
 *  @param   *desc             point to hub descriptor buffer.
 *  @return   void
 */
//-------------------------------------------------------------------------
static void MUC_hub_descriptor(MGC_LinuxCd * pThis,
                               struct usb_hub_descriptor *desc)
{
    uint16_t temp = 0;

    desc->bDescriptorType = 0x29;
    desc->bHubContrCurrent = 0;

    desc->bNbrPorts = 1;
    desc->bDescLength = 9;

    /* per-port power switching (gang of one!), or none */
    desc->bPwrOn2PwrGood = 10;

    /* no overcurrent errors detection/handling */
    temp = 0x0011;

    desc->wHubCharacteristics = (__force __u16) cpu_to_le16(temp);

    /* two bitmaps:  ports removable, and legacy PortPwrCtrlMask */
    desc->u.hs.DeviceRemovable[0] = 0x02;   /* port 1 */
    desc->u.hs.DeviceRemovable[1] = 0xff;

}

/*-------------------------------------------------------------------------*/
static int MUC_start(struct usb_hcd *hcd)
{
    DBG(3, "hcd = 0x%08X.\n", (uint32_t)hcd);

    hcd->state = HC_STATE_RUNNING;

    return 0;
}

/*-------------------------------------------------------------------------*/
static void MUC_stop(struct usb_hcd *hcd)
{
    DBG(3, "hcd = 0x%08X.\n", (uint32_t)hcd);
}

/*-------------------------------------------------------------------------*/
/* Check device is attached */
static uint8_t MGC_IsDevAttached(uint8_t *pBase)
{
    uint8_t devctl;

    MUSB_ASSERT(pBase);
    /* bConnectorId, bIsSession, bIsHost */
    devctl = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);

    if (devctl & (MGC_M_DEVCTL_FSDEV | MGC_M_DEVCTL_LSDEV))
    {
        return TRUE;
    }

    return FALSE;
}

#ifdef UNIFIED_USB
#if USB_SUSPEND_TEST
static void MGC_SuspendPhy(unsigned long pParam)
{
    uint8_t *pBase = (uint8_t *)pParam;
    uint32_t u4Reg = 0;

    if(!MGC_IsDevAttached(pBase))
    {
        printk("Suspend USB PHY\n");
        u4Reg = MGC_PHY_Read32(pBase,(uint32_t)0x68);
        u4Reg |=  0x00040000; // Suspend USB PHY.
        MGC_PHY_Write32(pBase, (uint32_t)0x68, u4Reg);

        u4Reg = MGC_Read32(pBase,(uint32_t)0xA4); //M_REG_INTRLEVEL1EN
        u4Reg |= ((uint32_t)0x80); //Enable DPDM_INT
        MGC_Write32(pBase, 0xA4, u4Reg); //M_REG_INTRLEVEL1EN
    }
}
#endif
#endif

static void MGC_hareware_down(void)
{
#if 0
   uint32_t u4Reg;
   uint8_t* pBase = (uint8_t*)MUSB_BASE3;
   #if 1
   if(MGC_IsDevAttached(pBase))
   {
        printk("[usb]suspend device on pBase 0x%x\n",(uint32_t)pBase);
        u4Reg = MGC_PHY_Read32(pBase, 0x68);
        u4Reg |=  0x00040008; 
        MGC_PHY_Write32(pBase, 0x68, u4Reg);  

        u4Reg = MGC_Read8(pBase, MGC_O_HDRC_POWER);
        u4Reg |= (MGC_M_POWER_SUSPENDM|MGC_M_POWER_ENSUSPEND);
        MGC_Write8(pBase, MGC_O_HDRC_POWER, u4Reg);
   }
   else
   #endif   
   {
        printk("[usb]suspend host hardware on pBase 0x%x\n",(uint32_t)pBase);
        u4Reg = MGC_PHY_Read32(pBase, 0x68);
        u4Reg |=  0x00040000; 
        MGC_PHY_Write32(pBase, 0x68, u4Reg);      
   }
    

#endif
}


static void usb_events_timer_func (unsigned long _pThis)
{
    MGC_LinuxCd *pThis = (MGC_LinuxCd *)_pThis;

    if(pThis->events & USB_EVENTS_UNLINK_INVALID_URB)
    {
      printk("[usb]USB_EVENTS_UNLINK_INVALID_URB\n");
      MGC_UnlinkInvalidUrb(_pThis);
      pThis->events &= ~USB_EVENTS_UNLINK_INVALID_URB;
    }
    
    if(pThis->events & USB_EVENTS_CHECK_CONNECT)
    {
      printk("[usb]USB_EVENTS_CHECK_CONNECT\n");
      MGC_CheckConnect(_pThis);
      usb_hcd_resume_root_hub(musbstruct_to_hcd(_pThis));
      mod_timer(&musbstruct_to_hcd(_pThis)->rh_timer, jiffies + msecs_to_jiffies(25));
      pThis->events &= ~USB_EVENTS_CHECK_CONNECT;
    }
	
    if (pThis->events & USB_RESUME_CHECK_STATUS)
    {
        if(pThis->bInsert == FALSE)
        {
			USB_disconnect_handle(musbstruct_to_hcd(_pThis));
			MGC_ServiceUSBIntr(pThis, MGC_M_INTR_DISCONNECT);
			pThis->dwVirtualHubPortStatus &= ~(1 << USB_PORT_FEAT_CONNECTION);
        }
        pThis->events &= ~USB_RESUME_CHECK_STATUS;
    }

    return;
}


/*-------------------------------------------------------------------------*/
static int MUC_hcd_probe(struct platform_device *pdev)
{
    uint32_t id = 0;
    int32_t retval;
    MUSB_LinuxController *pController;
    struct usb_hcd *hcd;
    MGC_LinuxCd *pThis;
    uint8_t *pBase;
    uint32_t u4Reg = 0;
    MGC_HsDmaController *pDmaController;
#ifdef UNIFIED_USB
    uint8_t intrmask = 0;
    uint8_t i;
#endif  

    DBG(3, "pdev = 0x%08X.\n", (uint32_t)pdev);
    trace_tx_cmd_status_flag = 1;//to began trace the first cmd sending to device state
    if (!pdev)
    {
        return -ENODEV;
    }
    
    id = pdev->id;
    if (id >= (sizeof(MUC_aLinuxController)/sizeof(MUSB_LinuxController)))
    {
        return -ENODEV;
    }
    pController = &MUC_aLinuxController[id];

    retval = MUC_ResetPhy(pController->pBase);
    if (retval < 0)
    {
        return -ENODEV;
    }

    pController->bSupport = TRUE;

    /* allocate and initialize hcd */
    hcd = usb_create_hcd(&MUC_hc_driver, &pdev->dev, dev_name(&pdev->dev));

    printk("MUC_phcd[%d] = 0x%08X @[0x%08X]\n", id, (uint32_t)hcd, (uint32_t)(pController->pBase));
    MUC_phcd[id] = hcd;
        
    if (!hcd)
    {
        return -ENOMEM;
    }
    
    hcd->has_tt = 1;
    //add for endpoint checking when enumeration.
   // hcd->check_free_ep = MGC_CheckFreeEndpoint;

    hcd->rsrc_start = (uint32_t) pController->pBase;
    pThis = hcd_to_musbstruct(hcd);
    pThis->pRegs = pController->pBase;
    pThis->nIrq = pController->dwIrq;
   // pThis->fgIsWantFullSpeed = FALSE;
    pThis->fgIsWantFullSpeed = config_usb11() ? TRUE : FALSE;
    //plocaltimer[id] = &pThis->Timer;
    spin_lock_init(&pThis->Lock);
    pThis->bPhyIndex = id;//dexiao add for mt8560

    init_timer(&pThis->events_timer);
    pThis->events_timer.function = usb_events_timer_func;
    pThis->events_timer.data = (unsigned long) pThis;   
    pThis->events = 0;
    peventstimer[id] = &pThis->events_timer;

    //queue mode
    pThis->port_q_configed = MGC_PortQConfig[id];
    pThis->ep_fifo_total_sz = ep_total_fifo_sz_cfg[id];
    pThis->ep_fifo = 0;
    //end
    
    pThisSimulate = pThis;

    // init endpoint, fifo.
    MGC_InitEnd(pThis);

    retval = usb_add_hcd(hcd, pController->dwIrq, IRQF_SHARED);
    if (retval != 0)
    {
        return -ENOMEM;
    }

    pBase = (uint8_t *) pThis->pRegs;

    MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, pThis->wEndMask);

    // IntrRxE, IntrUSB, and IntrUSBE are the same 32 bits group.
    // Tricky: Set 0 in all write clear field in IntrUSB field. Prevent to clear IntrUSB.
    u4Reg =
        MGC_M_INTR_SUSPEND | MGC_M_INTR_RESUME | MGC_M_INTR_BABBLE |
        /* MGC_M_INTR_SOF | */
        MGC_M_INTR_CONNECT | MGC_M_INTR_DISCONNECT | MGC_M_INTR_SESSREQ
        | MGC_M_INTR_VBUSERROR;
    u4Reg = (u4Reg << 24) | (pThis->wEndMask & 0xfffe);
    MGC_Write32(pBase, MGC_O_HDRC_INTRRXE, u4Reg);

   #ifdef UNIFIED_USB
   u4Reg = MGC_Read32(pBase, M_REG_INTRLEVEL1EN);
   u4Reg |= 0x0f;
   #ifdef CONFIG_USB_QUEUE
   u4Reg |= 0x20;
   #endif
   MGC_Write32(pBase, M_REG_INTRLEVEL1EN, u4Reg);
   printk ("[usb]Setting level1En to 0x%08x\n",u4Reg);

   #endif   

    DBG(1, "INTRUSBE reg:0x0x%X \n",
        MGC_Read8(pBase, MGC_O_HDRC_INTRUSBE));
    DBG(1, "INTRTXE  reg:0x0x%X \n",
        MGC_Read8(pBase, MGC_O_HDRC_INTRTXE));
    DBG(1, "INTRRXE  reg:0x0x%X \n",
        MGC_Read8(pBase, MGC_O_HDRC_INTRRXE));

    /* enable high-speed/low-power and start session */
    MGC_Write8(pBase, MGC_O_HDRC_POWER,
               MGC_M_POWER_SOFTCONN | MGC_M_POWER_HSENAB | 
               MGC_M_POWER_SUSPENDM |MGC_M_POWER_ENSUSPEND);

    /* enable high-speed/low-power and start session & suspend IM host */
    MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, MGC_M_DEVCTL_SESSION);

    // DMA controller init.
    pDmaController = &pController->rDma;
    memset(pDmaController, 0, sizeof(MGC_HsDmaController));
#ifdef UNIFIED_USB
    pDmaController->bChannelCount = MGC_DMAChannelNumConfig[id];

    for(i=0; i<pDmaController->bChannelCount; i++)
    {
        intrmask += (1<<i); 
    }   
    MGC_Write8(pBase, MGC_O_HSDMA_INTR_UNMASK_SET, intrmask);  
#else
    pDmaController->bChannelCount = MGC_HSDMA_CHANNELS;
#endif
    pDmaController->pfDmaChannelStatusChanged =
        MGC_HsDmaChannelStatusChanged;
    pDmaController->pDmaPrivate = (void *) pThis;
    pDmaController->pCoreBase = (void *) pBase;
    pDmaController->Controller.pPrivateData = pDmaController;
    pDmaController->Controller.pfDmaAllocateChannel =
        MGC_HsDmaAllocateChannel;
    pDmaController->Controller.pfDmaReleaseChannel =
        MGC_HsDmaReleaseChannel;
    pDmaController->Controller.pfDmaProgramChannel =
        MGC_HsDmaProgramChannel;
    pDmaController->Controller.pfDmaGetChannelStatus =
        MGC_HsDmaGetChannelStatus;
    pDmaController->Controller.pfDmaControllerIsr =
        MGC_HsDmaControllerIsr;
    pThis->pDmaController = &pDmaController->Controller;

#ifdef CONFIG_USB_QUEUE
    //reserve queue dma channel
    if(pThis->port_q_configed){
      pDmaController->bmUsedChannels |= (1 << (pDmaController->bChannelCount-1));
      mtk_q_dma_select(pThis,pDmaController->bChannelCount-1,3);
    }           
#endif

#ifdef UNIFIED_USB
    #if USB_SUSPEND_TEST
    MGC_SetTimer(pThis, MGC_SuspendPhy, (unsigned long) pBase, 100);
    #endif
#endif

    usb1_vbus_power_init();

    //Check whether is device attached or not?
    if(MGC_IsDevAttached(pBase))
    {
        uint8_t bIntrUsbValue = MGC_Read8(pBase, MGC_O_HDRC_INTRUSB);

        printk("[USB] device attached on init.\n");
        
        if ( !(bIntrUsbValue & (MGC_M_INTR_CONNECT)) )
        {
            //There is a device attached, but NO interrupt event occurred.
            //It should be handle device connection event.
            printk("[USB] handle device connect on init.\n");
            MGC_ServiceUSBIntr(pThis, MGC_M_INTR_CONNECT);            
        }
    }

    return 0;        
}

/*-------------------------------------------------------------------------*/
static int MUC_hcd_remove (struct platform_device *pdev)
{
        struct usb_hcd                *hcd;

        DBG(3, "pdev = 0x%08X.\n", (uint32_t)pdev);
        printk("hcd remove\n");

        hcd = platform_get_drvdata (pdev);
        usb_remove_hcd (hcd);
        //remove urblist buffer
        MGC_FreeEndpointListBuf(hcd,TRUE);
        usb_put_hcd (hcd);
        
        return 0;

}

static void hw_phy_power_on(struct platform_device *pdev, bool fgPowerOn)
{
    uint32_t  u4Reg;
    MUSB_LinuxController *pController;
	
    DBG(3, "pdev = 0x%08X.\n", (uint32_t)pdev);

    pController = &MUC_aLinuxController[pdev->id];

    if(fgPowerOn){
        MUC_ResetPhy(pController->pBase);
    } else {
        //Reset USB
        MGC_PHY_Write32(pController->pBase,0x68,MGC_PHY_Read32(pController->pBase,0x68) |   0x00004000);
        MGC_PHY_Write32(pController->pBase,0x68,MGC_PHY_Read32(pController->pBase,0x68) & (~0x00004000));

        u4Reg = MGC_CKGEN_Read32(0xA0);
        u4Reg &= ~(0x1 << 13);
        MGC_CKGEN_Write32(0xA0, u4Reg);
    }

	// RG_USB20_DISCTH[3:0], Default value 1000 mean 560mV ,change to 1100 mean 640mV, for fix extension cable issue.
	MGC_PHY_Write32(pController->pBase, 0x18, MGC_PHY_Read32(pController->pBase, 0x18) | 0x000C0000);
}

static int hw_initialize(struct platform_device *pdev)
{
    uint32_t dwResult = 0;
    uint32_t dwEndPoint;
    uint32_t dwFifoOffset = 64;
    uint32_t u4Reg = 0;
    uint16_t usb_status;
    uint8_t bEndPoint;
    MUSB_LinuxController *pController;

    pController = &MUC_aLinuxController[pdev->id];

    hw_phy_power_on(pdev, TRUE);

    u4Reg = MGC_Read32(pController->pBase, M_REG_INTRLEVEL1EN);
    u4Reg |= MGC_M_HDRC_L1INTM_DMA_INT_UNMASK  | MGC_M_HDRC_L1INTM_USBCOM_INT_UNMASK \
        | MGC_M_HDRC_L1INTM_RX_INT_UNMASK | MGC_M_HDRC_L1INTM_TX_INT_UNMASK; 
    MGC_Write32(pController->pBase, M_REG_INTRLEVEL1EN, u4Reg);


    /* Dynamic FIFO sizing: use pre-computed values for EP0 */
    MGC_SelectEnd(pController->pBase, 0);
    MGC_Write8 (pController->pBase, MGC_O_HDRC_TXFIFOSZ , 3);
    MGC_Write8 (pController->pBase, MGC_O_HDRC_RXFIFOSZ , 3);
    MGC_Write16(pController->pBase, MGC_O_HDRC_TXFIFOADD, 0);
    MGC_Write16(pController->pBase, MGC_O_HDRC_RXFIFOADD, 0);

    bEndPoint	  = MGC_Read8 (pController->pBase, MGC_O_HDRC_EPINFO)&0x0F; 
    for (dwEndPoint = 1; dwEndPoint <= bEndPoint; dwEndPoint++) {
        MGC_SelectEnd(pController->pBase, dwEndPoint);
        // fifo size = 512, turn off double packet buffer.
        MGC_Write8(pController->pBase, MGC_O_HDRC_TXFIFOSZ, 6);
        MGC_Write8(pController->pBase, MGC_O_HDRC_RXFIFOSZ, 6);
        MGC_Write16(pController->pBase, MGC_O_HDRC_TXFIFOADD, dwFifoOffset >> 3);

        dwFifoOffset += 512;
        MGC_Write16(pController->pBase, MGC_O_HDRC_RXFIFOADD, dwFifoOffset >> 3);
        dwFifoOffset += 512;
    }

    // Disable interrupts
    MGC_Write16(pController->pBase, MGC_O_HDRC_INTRTXE, 0);
    MGC_Write16(pController->pBase, MGC_O_HDRC_INTRRXE, 0);
    MGC_Write8(pController->pBase, MGC_O_HDRC_INTRUSBE, 0);

    // Read & clear interupt
    usb_status = MGC_Read16(pController->pBase, MGC_O_HDRC_INTRRX);
    MGC_Write16(pController->pBase, MGC_O_HDRC_INTRRX,usb_status);
    
    usb_status = MGC_Read16(pController->pBase, MGC_O_HDRC_INTRTX);
    MGC_Write16(pController->pBase, MGC_O_HDRC_INTRTX,usb_status);
    
    usb_status = MGC_Read8(pController->pBase, MGC_O_HDRC_INTRUSB);
    MGC_Write8(pController->pBase, MGC_O_HDRC_INTRUSB,usb_status);

    // initialize FRNUM register with index 0 of frame list
    MGC_Write16(pController->pBase, MGC_O_HDRC_FRAME,0);
    
    return dwResult;
}

static void hw_start_host_controller(struct platform_device *pdev)
{
    uint8_t Power,ToPower;
    uint8_t DevCtl,ToDevCtl;
    uint32_t i;
    uint32_t dwDmaChannelNumber;
    MUSB_LinuxController *pController;
    struct usb_hcd *hcd;
    MGC_LinuxCd *pThis;
    uint32_t u4Reg;

    pController = &MUC_aLinuxController[pdev->id];
    hcd = MUC_phcd[pdev->id];
    pThis = hcd_to_musbstruct(hcd);

	// IntrRxE, IntrUSB, and IntrUSBE are the same 32 bits group.
    // Tricky: Set 0 in all write clear field in IntrUSB field. Prevent to clear IntrUSB.
    u4Reg =
        MGC_M_INTR_SUSPEND | MGC_M_INTR_RESUME | MGC_M_INTR_BABBLE |
        /* MGC_M_INTR_SOF | */
        MGC_M_INTR_CONNECT | MGC_M_INTR_DISCONNECT | MGC_M_INTR_SESSREQ
        | MGC_M_INTR_VBUSERROR;

    u4Reg = (u4Reg << 24) | (pThis->wEndMask & 0xfffe);
    MGC_Write32(pController->pBase, MGC_O_HDRC_INTRRXE, u4Reg);
    MGC_Write8(pController->pBase, MGC_O_HSDMA_INTR, 0xFF);

    dwDmaChannelNumber = MGC_Read8(pController->pBase, MGC_O_HDRC_RAMINFO)>>4;
    u4Reg =0;
    for(i=0; i<dwDmaChannelNumber; i++){
        u4Reg |= (1<<i); 
    }

    MGC_Write8(pController->pBase, MGC_O_HSDMA_INTR_UNMASK_CLEAR, ~u4Reg);
    MGC_Write8(pController->pBase, MGC_O_HSDMA_INTR_UNMASK_SET, u4Reg);

    u4Reg = MGC_Read32(pController->pBase, M_REG_INTRLEVEL1EN);
    u4Reg |=  MGC_M_HDRC_L1INTM_DMA_INT_UNMASK  | MGC_M_HDRC_L1INTM_USBCOM_INT_UNMASK \
        | MGC_M_HDRC_L1INTM_RX_INT_UNMASK | MGC_M_HDRC_L1INTM_TX_INT_UNMASK;
    MGC_Write32(pController->pBase, M_REG_INTRLEVEL1EN, u4Reg);

    MGC_Write8(pController->pBase, MGC_O_HDRC_POWER, MGC_M_POWER_SOFTCONN|MGC_M_POWER_HSENAB|MGC_M_POWER_SUSPENDM|MGC_M_POWER_ENSUSPEND);

    MGC_Write8(pController->pBase, MGC_O_HDRC_DEVCTL, MGC_M_DEVCTL_SESSION);


#if 1
    printk("!!!! reusme : %d \n", plug_status_pm);
    if(plug_status_pm == TRUE) {
		mod_timer(&pThis->events_timer, jiffies
				+ msecs_to_jiffies(
						200));
		pThis->events = USB_RESUME_CHECK_STATUS;
    }
#endif
}

/*-------------------------------------------------------------------------*/
static int MUC_hcd_suspend (struct platform_device *pdev, pm_message_t state)
{


    uint32_t dwCnt = 5000, dwSOFCheck = 5,dwTxVail = 15;
    uint8_t bUsbBak,bUsbCommon;
    uint8_t bPower;
    MUSB_LinuxController *pController=NULL;
    struct usb_hcd *hcd;
    MGC_LinuxCd *pThis;

    DBG(3, "pdev = 0x%08X, state.event=%d.\n", (uint32_t)pdev, state.event);
    
    DBG(3, "pdev = 0x%08X.\n", (uint32_t)pdev);

    if (!pdev)
    {
        return -ENODEV;
    }

    if ( pdev->id >= (sizeof(MUC_aLinuxController)/sizeof(MUSB_LinuxController)))
    {
        return -ENODEV;
    }
    pController = &MUC_aLinuxController[pdev->id];
    hcd = MUC_phcd[pdev->id];
    pThis = hcd_to_musbstruct(hcd);

#ifdef CONFIG_USB_HIBERNATION
    //hw_phy_power_on(pdev, FALSE);
    plug_status_pm = pThis->bInsert;
    pThis->bInsert = FALSE;

    printk("++ MUC_hcd_suspend = 0x%x ++\n", state.event);
    bUsbBak = bUsbCommon  = MGC_Read8(pController->pBase,MGC_O_HDRC_INTRUSBE);
    bUsbCommon &= ~MGC_M_INTR_SOF;
    MGC_Write8(pController->pBase,MGC_O_HDRC_INTRUSBE,bUsbCommon);
    bUsbCommon  = MGC_Read8(pController->pBase,MGC_O_HDRC_INTRUSB);
    MGC_Write8(pController->pBase, MGC_O_HDRC_INTRUSB,bUsbCommon);

    bPower  = MGC_Read8(pController->pBase, MGC_O_HDRC_POWER);
    bPower &= ~(MGC_M_POWER_ENSUSPEND|MGC_M_POWER_SUSPENDM|MGC_M_POWER_RESUME);
    bPower |= (MGC_M_POWER_ENSUSPEND|MGC_M_POWER_SUSPENDM);

    do{
       bUsbCommon  = MGC_Read8(pController->pBase,MGC_O_HDRC_INTRUSB);
    }while((bUsbCommon & MGC_M_INTR_SOF) && (dwSOFCheck --));

    do{
       bUsbCommon  = MGC_Read8(pController->pBase,0x631);
       if((bUsbCommon & 0x20) == 0){
       dwTxVail --;
       }
       else {
       dwTxVail = 15;
            do{
                bUsbCommon  = MGC_Read8(pController->pBase,MGC_O_HDRC_INTRUSB);
            }while((bUsbCommon & MGC_M_INTR_SOF) && (dwSOFCheck --));
            dwSOFCheck  = 5;
            bUsbCommon &= ~MGC_M_INTR_SOF;
            MGC_Write8(pController->pBase, MGC_O_HDRC_INTRUSB,bUsbCommon);
       }
       dwCnt --;
    }while(dwTxVail && dwCnt);
    MGC_Write8(pController->pBase,MGC_O_HDRC_POWER, bPower);
    MGC_Write8(pController->pBase,MGC_O_HDRC_INTRUSBE,bUsbBak);     
    printk("-- MUC_hcd_suspend --\n");
#endif	
    
    return 0;
}

/*-------------------------------------------------------------------------*/
static int MUC_hcd_resume (struct platform_device *pdev)
{
    /* Automatically process */
    uint8_t bPower;
    MUSB_LinuxController *pController;
    struct usb_hcd *hcd;
    MGC_LinuxCd *pThis;
    hcd = MUC_phcd[pdev->id];
    pThis = hcd_to_musbstruct(hcd);
    trace_tx_cmd_status_flag = 1;//to began trace the first cmd sending to device state
    DBG(3, "pdev = 0x%08X.\n", (uint32_t)pdev);
    if (!pdev)
    {
        return -ENODEV;
    }
    
    if ( pdev->id >= (sizeof(MUC_aLinuxController)/sizeof(MUSB_LinuxController)))
    {
        return -ENODEV;
    }
    pController = &MUC_aLinuxController[pdev->id];
    
    DBG(3, "pdev = 0x%08X.\n", (uint32_t)pdev);

	if(MGC_PHY_Read32(pController->pBase,0x6c)) {
		printk("resume warm-reset\n");
		pThis->bInsert = plug_status_pm;

		bPower	= MGC_Read8(pController->pBase, MGC_O_HDRC_POWER);
		bPower &= ~(MGC_M_POWER_ENSUSPEND|MGC_M_POWER_SUSPENDM|MGC_M_POWER_RESUME);
		bPower |= MGC_M_POWER_RESUME;
		MGC_Write8(pController->pBase,MGC_O_HDRC_POWER, bPower);
	
		mdelay(20);
		
		MGC_Write8(pController->pBase,MGC_O_HDRC_POWER, bPower&(~MGC_M_POWER_RESUME));
		return 0;
    }

#ifdef CONFIG_USB_HIBERNATION
	hw_initialize(pdev);
	hw_start_host_controller(pdev);
#endif

    return 0;        
}

/*-------------------------------------------------------------------------*/
static void MUC_hcd_release (struct device *dev) 
{
    DBG(3, "dev = 0x%08X.\n", (uint32_t)dev);
}
static void USB_disconnect_handle(struct usb_hcd *hcd)
{
	MGC_LinuxCd *pThis = hcd_to_musbstruct(hcd);
        uint16_t regval; 
        const void *pBase = pThis->pRegs;  
        printk("[USB] Device is disconnected\n");

        //pThis->fgIsWantFullSpeed = FALSE;
        pThis->fgIsWantFullSpeed = config_usb11() ? TRUE : FALSE;
        //2009.09.21
        printk("[USB] update hub status\n");
        
        pThis->dwVirtualHubPortStatus = (1 << USB_PORT_FEAT_POWER)
                                        | (1 << USB_PORT_FEAT_C_CONNECTION);            
        pThis->dwVirtualHubPortStatus &= ~(1 << USB_PORT_FEAT_CONNECTION);
	
        usb_hcd_poll_rh_status(hcd);
	
        //MGC_SetTimer(pThis, MGC_UnlinkInvalidUrb, (unsigned long) pThis, 20);
        mod_timer(&pThis->events_timer, jiffies
                + msecs_to_jiffies(
                        20));
        pThis->events = USB_EVENTS_UNLINK_INVALID_URB; 
       
        
}
/*-------------------------------------------------------------------------*/

#ifdef USB_READ_WRITE_TEST

u32 Str2Hex(char *inp_buf, unsigned int count)
{
    u32 mCh;
    int i;
    
    // if input HEX format 0xXX
    if ((inp_buf[0] == '0') && (inp_buf[1] == 'x'))
    {
        mCh = 0;
        for (i = 2; i < count; i ++)
        {
            if ((inp_buf[i] >= '0') && (inp_buf[i] <= '9'))
            {
                mCh <<= 4;
                mCh += (inp_buf[i] - '0');
            }
            else if ((inp_buf[i] >= 'a') && (inp_buf[i] <= 'f'))
            {
                mCh <<= 4;
                mCh += ((inp_buf[i] - 'a') + 10);
            }
            else if ((inp_buf[i] >= 'A') && (inp_buf[i] <= 'F'))
            {
                mCh <<= 4;
                mCh += ((inp_buf[i] - 'A') + 10);
            }
        }
    }
    else
    {
        mCh = 0;
        for (i = 0; i < count; i ++)
        {
            if ((inp_buf[i] >= '0') && (inp_buf[i] <= '9'))
            {
                mCh *= 10;
                mCh += (inp_buf[i] - '0');
            }
        }
    }

    return mCh;
}

static int proc_read_usb_log_en(char *page, char **start,
                             off_t off, int count,
                             int *eof, void *data)
{
    int  len;

    len = sprintf(page, "[USB TEST] Current USB Log Level: 0x%x\n", (u32)usb_test_log_en);

    return len;
}

static int proc_write_usb_log_en(struct file *file,
                             const char *buffer,
                             unsigned long count,
                             void *data)
{
    int i;
    int tmp;
    char *buf;

    buf = kmalloc(count + 1, GFP_KERNEL);

    if(copy_from_user(buf, buffer, count))
            return -EFAULT;

    buf[count] = '\0';

    if ((buf[0] == '0') && (buf[1] == 'x'))
    {
        tmp = 0;
        for (i = 2; i < count; i ++)
        {
            if ((buf[i] >= '0') && (buf[i] <= '9'))
            {
                tmp <<= 4;
                tmp += (buf[i] - '0');
            }
            else if ((buf[i] >= 'a') && (buf[i] <= 'f'))
            {
                tmp <<= 4;
                tmp += ((buf[i] - 'a') + 10);
            }
            else if ((buf[i] >= 'A') && (buf[i] <= 'F'))
            {
                tmp <<= 4;
                tmp += ((buf[i] - 'A') + 10);
            }
        }
        usb_test_log_en = tmp;
        
        printk("[USB TEST] New USB Log Level: 0x%x\n", usb_test_log_en);
    }
    else
    {
        printk("Format : echo 0xZZ > log_en\n");
    }

    kfree(buf);

    return count;
}

static int proc_read_usb_setting(char *page, char **start,
                             off_t off, int count,
                             int *eof, void *data)
{
    int  len;

    len = sprintf(page, "[USB TEST] Current USB setting: 0x%x\n", (u32)MGC_usb_setting);

    return len;
}

static int proc_write_usb_setting(struct file *file,
                             const char *buffer,
                             unsigned long count,
                             void *data)
{ 
    char *inp_buf;
    u32 mHex;

    // Retrive magic char from echo 0xXX > /proc/driver/mmc/rw_test    
    inp_buf = kmalloc(count + 1, GFP_KERNEL);

    if (!inp_buf)
    {
        printk("[%s] inp_buf allocate failed !!\n", __func__);
        return 0;
    }
        
    if(copy_from_user(inp_buf, buffer, count))
            return -EFAULT;

    inp_buf[count] = '\0';

    mHex = Str2Hex(inp_buf, count);    
    kfree(inp_buf);

    if(mHex & USB_SETTING_CLEAR){
      mHex &= ~USB_SETTING_CLEAR;
      MGC_usb_setting &= ~mHex;
    }
    else{
      MGC_usb_setting |= mHex;
    }

    printk("[USB] New USB setting: 0x%x\n", MGC_usb_setting);
    return count;
}    


static int proc_read_test_pressed(char *page, char **start,
                             off_t off, int count,
                             int *eof, void *data)
{
    struct file *filp;
    mm_segment_t old_fs;
    u32 file_buf_len;
    int  len = 0;    
    
    if (off > 0) 
    {
        *eof = 1;
         return 0;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    file_buf_len = USB_DMA_BUFFER_SIZE;
  
    filp = filp_open("/dev/sda", O_RDWR | O_NONBLOCK | O_SYNC, 0);
        
    if (IS_ERR(filp))
    {
        len = sprintf(page, "[%s]Open /dev/sda failed: %d !!\n", __func__, (int)filp);
    }    
    else
    {
        u8  *file_buf;
        u32 rd_length;

        file_buf = (u8 *)kmalloc(file_buf_len, GFP_KERNEL | GFP_DMA);

        if (file_buf)
        {
            rd_length = filp->f_op->read(filp, file_buf, file_buf_len, &filp->f_pos);
            
            len = sprintf(page, "[%s] /dev/sda : file_buf = 0x%x, rd_length = %d\n", __func__, (u32)file_buf, rd_length);
            
            kfree(file_buf);
        }
        else
        {
            printk("[%s] Can't allocate buffer size = 0x%x\n", __func__, file_buf_len);
        }
                
        filp_close(filp, NULL);
    }

    set_fs(old_fs);

    return len;
}

static void vBdpDramBusy(unsigned char fgBool)
{   
    static unsigned char *pcDramBusyMemAddr = NULL;
    const unsigned int u4BufLength = 64*1024;
    unsigned int u4Reg = 0;
    
    if(fgBool)
    {
        if(!pcDramBusyMemAddr)
        {
            pcDramBusyMemAddr = (unsigned char*)kmalloc(u4BufLength, GFP_KERNEL);
        }

        if(pcDramBusyMemAddr)
        {
            printk("Start DRAM Test Agent.\n");
            //High priority¡G
            //RISCWrite 0x007104 0x0xxxxxxx
            u4Reg = IO_READ32(IO_VIRT, 0x7104);
            u4Reg &= 0x0FFFFFFF;
            IO_WRITE32(IO_VIRT, (0x7104), u4Reg);            

            //007210 (PHY Addr)
            IO_WRITE32(IO_VIRT, (0x7210), (unsigned int)pcDramBusyMemAddr);
            
            //007214 (Length, must be greater than 0x1000)
            IO_WRITE32(IO_VIRT, (0x7214), u4BufLength);           
            
            //007218 0x840F110D (once)
            //       0x860F110D (repeat)
            IO_WRITE32(IO_VIRT, (0x7218), 0x860F110D);           
        }
    }
    else
    {
        //Disable
        //007218 0x060F110D (once)
        u4Reg= IO_READ32(IO_VIRT, 0x7218);
        u4Reg &= 0x0FFFFFFF;
        IO_WRITE32(IO_VIRT, (0x7218), u4Reg);        
    }
}


static int proc_write_test_pressed(struct file *file,
                             const char *buffer,
                             unsigned long count,
                             void *data)
{
    struct file *filp;
    mm_segment_t old_fs;
    u32 file_buf_len;
    
    char *inp_buf;
    u32 mHex;

    // Retrive magic char from echo 0xXX > /proc/driver/mmc/rw_test    
    inp_buf = kmalloc(count + 1, GFP_KERNEL);

    if (!inp_buf)
    {
        printk("[%s] inp_buf allocate failed !!\n", __func__);
        return 0;
    }
        
    if(copy_from_user(inp_buf, buffer, count))
            return -EFAULT;

    inp_buf[count] = '\0';

    mHex = Str2Hex(inp_buf, count);    
    kfree(inp_buf);
    
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    file_buf_len = USB_DMA_BUFFER_SIZE;
  
    filp = filp_open("/dev/sda", O_RDWR | O_NONBLOCK | O_SYNC, 0);
    
    if (IS_ERR(filp))
    {
        printk("[%s]Open /dev/sda failed: %d !!\n", __func__, (int)filp);
    }
    else
    {
        u8  *file_buf;
        u32 wt_length;

        file_buf = (u8 *)kmalloc(file_buf_len, GFP_KERNEL | GFP_DMA);

        if (file_buf)
        {
            memset(file_buf, mHex, file_buf_len);
            wt_length = filp->f_op->write(filp, file_buf, file_buf_len, &filp->f_pos);
            printk("[%s] /dev/sda : file_buf = 0x%x, wt_length = %d, Magic Char = 0x%x\n", __func__, (u32)file_buf, wt_length, mHex);
            
            kfree(file_buf);
        }
        else
        {
            printk("[%s] Can't allocate buffer size = 0x%x\n", __func__, file_buf_len);
        }
                
        filp_close(filp, NULL);
    }

    set_fs(old_fs);

    return count;
}

static void print_Buffer(unsigned int u4BufSz, unsigned char *pu1Buf)
{
    unsigned int u4Idx = 0;
    unsigned char  *pu1TmpBuf = pu1Buf;

    for (u4Idx = 0; u4Idx < (u4BufSz); u4Idx ++)
    {
      if (u4Idx % 16 == 0)
      {
          printk("[0x%04X]", u4Idx);
      }

      printk("%02X", pu1TmpBuf[0]);

      if (u4Idx % 16 == 3 || u4Idx % 16 == 7 || u4Idx % 16 == 11)
      {
          printk(" ");
      }
      else if (u4Idx % 16 == 15)
      {
          printk("\n");
      }
      
      pu1TmpBuf ++;
    }    
}

static bool fgCompareFunc(unsigned int *pu4Buf1, unsigned int *pu4Buf2, unsigned int u4MemLen)
{
    unsigned int i;

    // Compare the result
    for(i=0; i<u4MemLen; i+=4)
    {
        if(pu4Buf1[i/4] != pu4Buf2[i/4])
        {
            printk("Compare Failed : rd_addr : 0x%x != wt_addr : 0x%x, inconsistant at element %u \n", (((unsigned int)pu4Buf2)+i), (((unsigned int)pu4Buf1)+i), i/4);
            MUC_ASSERT(0);
            break;
        }
    }

    if(i == u4MemLen)
    {
        return TRUE;
    }

    printk("[Debug] Write Buf = 0x%x\n", (u32)pu4Buf1);
    print_Buffer(u4MemLen, (unsigned char *)pu4Buf1);
    printk("[Debug] Read Buf = 0x%x\n", (u32)pu4Buf2);    
    print_Buffer(u4MemLen, (unsigned char *)pu4Buf2);

    return FALSE;
}

static int proc_stress_test_pressed(struct file *file,
                             const char *buffer,
                             unsigned long count,
                             void *data)
{
    struct file *filp;
    mm_segment_t old_fs;
    u32 file_buf_len;
    
    char *inp_buf = NULL;
    char *token = NULL;
    char **stringp = NULL;
    int len = 0;
    u32 mTestCount;
    char file_name[9] = {0};
    char disk_no = 'a';

    // Retrive magic char from 
    //echo 1000,a > /proc/driver/usb/stress
    inp_buf = kmalloc(count + 1, GFP_KERNEL);

    if (!inp_buf)
    {
        printk("[%s] inp_buf allocate failed !!\n", __func__);
        return 0;
    }
        
    if(copy_from_user(inp_buf, buffer, count))
            return -EFAULT;

    inp_buf[count] = '\0';

    stringp = &inp_buf;

    token = strsep(stringp, ",;"); //separate the argument with ',' and ';'
    len = strlen(token);

    mTestCount = Str2Hex(token, len);    

    while(token != NULL)
    {
        token = strsep(stringp, ",;"); //separate the argument with ',' and ';   
        if(token)
        {
            int c = (int)*token;
            if(isalpha(c))
            {
                disk_no = *token;
                printk("[USB TEST] test on /dev/sd%c \n", disk_no);
            }
        }
    }

    kfree(inp_buf);
    
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    
    file_buf_len = USB_DMA_BUFFER_SIZE;

    sprintf(file_name, "/dev/sd%c", disk_no);

    printk("[USB TEST] open file: %s\n", file_name);
    
    filp = filp_open(file_name, O_RDWR | O_NONBLOCK | O_SYNC, 0);
    
    if (IS_ERR(filp))
    {
        printk("[%s]Open %s failed: %d !!\n", __func__, file_name, (int)filp);
    }
    else
    {
        u8  *wt_buf = NULL;
        u8  *rd_buf = NULL;
        u32 wt_length;
        u32 rd_length;
        loff_t seekpos, seekresult, seekendpos;
        int   i, j;

        wt_buf = (u8 *)kmalloc(file_buf_len, GFP_KERNEL | GFP_DMA);
        rd_buf = (u8 *)kmalloc(file_buf_len, GFP_KERNEL | GFP_DMA);

        vBdpDramBusy(TRUE);

        seekendpos = filp->f_op->llseek(filp, 0, SEEK_END);
          
        if (wt_buf && rd_buf && seekendpos)
        {               
            unsigned long randCh;

            printk("\n **** Total test Count : %d, file length : %d KB = %d Test Unit (Test Unit : %d bytes) **** \n\n", mTestCount, (u32)seekendpos / 1024, (u32)seekendpos / file_buf_len, file_buf_len);

            // Start stress test for specific test count
            for (i = 0; i < mTestCount; i ++)
            {
                int loopnum = 0;
                
                seekpos = file_buf_len * i;

                // Avoid out of file range
                while (seekpos >= seekendpos-1)
                {
                    seekpos -= seekendpos;
                    loopnum++;
                }

                if (loopnum > 0)
                {
                    printk("\n **** Loop %d **** \n", loopnum);
                }
                
                // Seek to target position (Unit : USB_DMA_BUFFER_SIZE)
                seekresult = filp->f_op->llseek(filp, seekpos, SEEK_SET);
                if ( seekresult != seekpos)
                {                
                    printk("[%s] %s seek failed before write !! TestCount : %d, seekresult : 0x%x, seekpos : 0x%x\n", __func__, file_name, mTestCount, (u32)seekresult, (u32)seekpos);
                    break;
                }
                
                // Fill the write buffer by random number
                get_random_bytes(&randCh, sizeof(randCh));
                
                for (j = 0; j < file_buf_len; j += 4)
                {
                    *((unsigned int*)(((unsigned int)wt_buf)+j)) = (j+randCh);
                }
                // Write the buffer contents to card
                wt_length = filp->f_op->write(filp, wt_buf, file_buf_len, &filp->f_pos);

                // Close the file to avoid cache
                filp_close(filp, NULL);

                // Re-open the file for read back checking
                filp = filp_open(file_name, O_RDWR | O_NONBLOCK | O_SYNC, 0);
                
                if (IS_ERR(filp))
                {
                    printk("[%s]Open %s failed: %d !! TestCount = %d\n\n", __func__, file_name, (int)filp, mTestCount);
                    break;
                }
                
                printk("[%03d] %s : wt_buf : 0x%x, len : %d, pos : 0x%08X, Rand : 0x%08X\n", i, file_name, (u32)wt_buf, wt_length, (u32)seekpos, (u32)randCh);
                
                // Seek to target position (Unit : USB_DMA_BUFFER_SIZE)
                seekresult = filp->f_op->llseek(filp, seekpos, SEEK_SET);
                if ( seekresult != seekpos)
                {                
                    printk("[%s] %s seek failed before read !! TestCount : %d, seekresult : 0x%x, seekpos : 0x%x\n", __func__, file_name, mTestCount, (u32)seekresult, (u32)seekpos);
                    MUC_ASSERT(0);
                    break;
                }

                // Read back for verify
                rd_length = filp->f_op->read(filp, rd_buf, file_buf_len, &filp->f_pos);
                
                if(wt_length != rd_length) 
                {
                    printk("ERROR: wt_length != rd_length\n");
                    MUC_ASSERT(0);
                }
                
                printk("[%03d] %s : rd_buf : 0x%x, len : %d, pos : 0x%08X\n", i, file_name, (u32)rd_buf, rd_length, (u32)seekpos);    

                // Compare read/write result
                if (fgCompareFunc((unsigned int*)wt_buf, (unsigned int*)rd_buf, file_buf_len))
                {
                    printk("[%03d] %s : Compare OK.\n", i, file_name);
                }
                else
                {
                    printk("[%03d] %s : Compare NG !!.\n", i, file_name);
                    MUC_ASSERT(0);
                    break;
                }
            }
            
            printk("\n **** Test End : count : %d **** \n", i);
                    
            kfree(wt_buf);
        }
        else
        {
            if(seekendpos)
            {
                printk("[USB TEST] capacity of %s is 0\n", file_name);
            }
            else
            {
                printk("[%s] Can't allocate buffer size = 0x%x\n", __func__, file_buf_len);
            }
        }
                
        filp_close(filp, NULL);
    }

    set_fs(old_fs);

    return count;
}


static int proc_root_port_ctnl(struct file *file,
                             const char *buffer,
                             unsigned long count,
                             void *data)
{
    char *inp_buf = NULL;
    char *token = NULL;
    char **stringp = NULL;
    int len = 0, ret = 0;
    uint8_t bPortNum = 0;
    uint8_t fgIsFullSpeed = 0;
    uint32_t argv[3] = {0}, i = 0;
    uint32_t mdelay = 0;
    static const uint32_t max_argc = 3;
    

    // Retrive magic char from 
    //echo 0,1,100 > /proc/driver/usb/root_port
    inp_buf = kmalloc(count + 1, GFP_KERNEL);

    if (!inp_buf)
    {
        printk("[%s] inp_buf allocate failed !!\n", __func__);
        return 0;
    }
        
    if(copy_from_user(inp_buf, buffer, count))
            return -EFAULT;

    inp_buf[count] = '\0';
    stringp = &inp_buf;

    i = 0;

    do{
        token = strsep(stringp, ",;"); //separate the argument with ',' and ';   
        if(token)
        {
            len = strlen(token);
            argv[i] = Str2Hex(token, len);    
        }

        i++;
    }while(token != NULL && i < max_argc);

    bPortNum = (uint8_t)(argv[0]&0x000000FF);
    fgIsFullSpeed = (uint8_t)(argv[1]&0x000000FF);
    mdelay = argv[2];

    kfree(inp_buf);

    if(i < 2)
    {
        printk("Usage: echo [port_num],[is_full_speed],[mdelay] > /proc/driver/usb/root_port\n");
    }

    printk("Root Port[%d] reset to %s speed for %d ms\n", 
            bPortNum, fgIsFullSpeed==TRUE?"Full":"High", mdelay);
    
    ret = MGC_ResetRootPort(bPortNum, fgIsFullSpeed, mdelay);

    if(ret) 
    {
        printk("Reset command failed!\n");
    }

    printk("Reset success");    
    return count;
}

static int __init usb_init_procfs(void)
{
}

static void __exit usb_uninit_procfs(void)
{
}
#endif // #ifdef USB_READ_WRITE_TEST

#ifdef SIMULATE_USB_INSERT
static void usb_plug_out(MGC_LinuxCd * _pThis)
{
    MGC_LinuxCd *pThis = _pThis;
    //const void *pBase = pThis->pRegs;
    struct usb_hcd *hcd = musbstruct_to_hcd(pThis);

    printk("[USB] Device is disconnected\n");

    //pThis->fgIsWantFullSpeed = FALSE;
    pThis->fgIsWantFullSpeed = config_usb11() ? TRUE : FALSE;
    //2009.09.21
    printk("[USB] update hub status\n");
    
    pThis->dwVirtualHubPortStatus = (1 << USB_PORT_FEAT_POWER)
                                    | (1 << USB_PORT_FEAT_C_CONNECTION);            
    pThis->dwVirtualHubPortStatus &= ~(1 << USB_PORT_FEAT_CONNECTION);
    usb_hcd_poll_rh_status(hcd);

    pThis->bInsert = FALSE;

    mod_timer(&pThis->events_timer, jiffies
            + msecs_to_jiffies(
                    20));
    pThis->events = USB_EVENTS_UNLINK_INVALID_URB;  
    MGC_ServiceUSBIntr(pThis, MGC_M_INTR_DISCONNECT);
    /*
    MGC_PHY_Write32(pBase, 0x6C, (MGC_PHY_Read32(pBase,0x6C) &(~0x3F3F)) | 0x3C10); 
    MGC_PHY_Write32(pBase, 0x6C, (MGC_PHY_Read32(pBase,0x6C) &(~0x3F3F)) | 0x3E10); 

    mdelay(10);

    MGC_PHY_Write32(pBase, 0x6C, (MGC_PHY_Read32(pBase,0x6C) & (~0x3F3F)) |  0x3E2C); 
    MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, MGC_Read8(pBase,MGC_O_HDRC_DEVCTL)|MGC_M_DEVCTL_SESSION);
    */
}

static ssize_t simulate_insert_enable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    printk("simulate_insert_enable_show, simulate_insert_enable=%d\n", simulate_insert_enable);
    
    return sprintf(buf, "%d\n", simulate_insert_enable);
}

static ssize_t simulate_insert_enable_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
    int enable_simulate = 0;
    printk("simulate_insert_enable_store, set simulate_insert_enable=%s\n", buf);
    enable_simulate = simple_strtoul(buf, NULL, 10);
    
    if (enable_simulate == 1)
    {
        simulate_insert_enable = SIMULATE_INSERT_ENABLE;
    }
    else if (enable_simulate == 0)
    {
        simulate_insert_enable = SIMULATE_INSERT_DISABLE;
    }
    else
    {
        printk("simulate_insert_enable_store, error set value.\n");
    }
    
    return count;
}

static ssize_t simulate_insert_usb_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    printk("simulate_insert_usb_show, simulate_insert_usb=%08x\n", simulate_insert_usb);
    
    return sprintf(buf, "%d\n", simulate_insert_usb);
}

static ssize_t simulate_insert_usb_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
    printk("simulate_insert_usb_store, set simulate_insert_usb=%s\n", buf);

    simulate_insert_usb = simple_strtoul(buf, NULL, 10);

    if(simulate_insert_enable) {
        if(simulate_insert_usb == 0) {
            printk("[usb] Usb Plug Out Op\n");
            usb_plug_out(pThisSimulate);
        } else {
            printk("[usb] Usb plug In Op\n");
        }
    }
    
    return count;
}

#ifdef REPORT_USB_STATE

static ssize_t usb_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    	printk("usb_state_show, usb_state=%d\n",  (pThisSimulate ? pThisSimulate->bInsert : 0));
    
	return sprintf(buf, "%d\n", pThisSimulate->bInsert);   
}
static ssize_t usb_state_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
    printk("usb_state_store, set usb_state=%s\n", buf);
    return count;
}

#endif

static ssize_t usb_protocol_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    	printk("usb_protocol=%s\n", usb_protocol);
    
	return sprintf(buf, "%s\n",usb_protocol);
}

static ssize_t usb_protocol_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
 
}

static struct kobj_attribute simulate_insert_enable_attribute =
    __ATTR(simulate_insert_enable, 0660, simulate_insert_enable_show, simulate_insert_enable_store);

static struct kobj_attribute simulate_insert_usb_attribute =
    __ATTR(simulate_insert_usb, 0660, simulate_insert_usb_show, simulate_insert_usb_store);

static struct kobj_attribute usb_state_attribute =
    __ATTR(usb_state, 0660, usb_state_show, usb_state_store);

static struct kobj_attribute usb_protocol_attribute =
    __ATTR(protocol, 0660, usb_protocol_show, usb_protocol_store);

static struct attribute *attrs[] = {
    &simulate_insert_usb_attribute.attr,
    &simulate_insert_enable_attribute.attr,
    &usb_state_attribute.attr,
    &usb_protocol_attribute.attr,
    NULL,   /* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
    .attrs = attrs,
};


static int __init usb_init_sysfs(void)
{   
    int retval;

    //sddetect_kobj = kobject_create_and_add("kobject_sddetect", kernel_kobj);
    usbdetect_kobj = kobject_create_and_add("udisk1detect", NULL);
    if (!usbdetect_kobj)
        return -ENOMEM;

    /* Create the files associated with this kobject */
    retval = sysfs_create_group(usbdetect_kobj, &attr_group);
    if (retval)
        kobject_put(usbdetect_kobj);

    return 0;
}

static void __exit usb_uninit_sysfs(void)
{
    if(usbdetect_kobj)
        kobject_put(usbdetect_kobj);
}

#endif
/*-------------------------------------------------------------------------*/
static int __init MUC_init(void)
{
    int32_t retval = 0;
    uint32_t nCount;
    uint32_t i;
    struct platform_device *pPlatformDev;
        static u64 dummy_mask = DMA_BIT_MASK(32);
    printk("usb driver version is 24\n");
    if (usb_disabled ())
    {
        return -ENODEV;
    }

    retval = platform_driver_register (&MUC_hcd_driver);
    if (retval < 0)
    {
        return retval;
    }

    nCount = sizeof(MUC_pdev) / sizeof(struct platform_device);
    for (i = 0; i < nCount; i++)
    {
            pPlatformDev = &MUC_pdev[i];
            pPlatformDev->name = MUC_HcdName;
            pPlatformDev->id = i;        
            DBG(3, "pdev = 0x%08X, id = %d.\n", (uint32_t)pPlatformDev, i);
            //  This controller has DMA capability.
            //pPlatformDev->dev.dma_mask = (u64 *) (~(u64) 0);
            pPlatformDev->dev.dma_mask = &dummy_mask; /* FIXME: for improper usb code */
            pPlatformDev->dev.coherent_dma_mask = 0xffffffff;
            pPlatformDev->dev.release = MUC_hcd_release;
            
            if (MUC_usb_config_suspend())
            { 
                pPlatformDev->dev.type = &usb_host_type;
            }
            
            retval = platform_device_register(pPlatformDev);
            if (retval < 0)
            {
                platform_device_unregister(pPlatformDev);
                break;
            }

            #if(0)
            if (MUC_usb_config_suspend())
            {
                if(pPlatformDev->dev.type)
                {
                    if(pPlatformDev->dev.type->pm)
                {
                        pPlatformDev->dev.type->pm->suspend = usbsuspend;
                        pPlatformDev->dev.type->pm->resume = usbresume;
                }
                else
                {
                        pPlatformDev->dev.type->suspend = usbsuspend;
                        pPlatformDev->dev.type->resume = usbresume;
                }
            }
            }          
            #endif  
    }

    if (retval < 0)
    {
        platform_driver_unregister (&MUC_hcd_driver);
    }
//#if CONFIG_SONY_USB_POWER_CTRL    
//  usb_power_onoff(POWER_ON);
//#endif
//#ifdef SIMULATE_USB_INSERT
    usb_init_sysfs();
//#endif
#ifdef USB_READ_WRITE_TEST
    // init /proc/usb
    usb_init_procfs();
#endif

    return retval;
}

/*-------------------------------------------------------------------------*/
static void __exit MUC_cleanup(void)
{
    uint32_t nCount;
    uint32_t i;
    struct platform_device *pPlatformDev;
#ifdef CONFIG_USB_QUEUE 
    MGC_LinuxCd * pThis;
#endif

    nCount = sizeof(MUC_pdev) / sizeof(struct platform_device);
    for (i = 0; i < nCount; i++)
    {
        #if 0
        if(plocaltimer[i]->function)
        {
            del_timer_sync(plocaltimer[i]);            
        }
        #endif
        if(peventstimer[i]->function)
        {
            del_timer_sync(peventstimer[i]);            
        }
        pPlatformDev = &MUC_pdev[i];
        
#ifdef CONFIG_USB_QUEUE
        pThis = hcd_to_musbstruct(dev_get_drvdata(&pPlatformDev->dev));
        if(pThis->port_q_configed){
            printk("[usb]disable queue @ port %d\n",pThis->bPhyIndex);
            mtk_disable_q_all(pThis);
        }   
#endif        
        DBG(3, "pdev = 0x%08X, id = %d.\n", (uint32_t)pPlatformDev, i);        
        platform_device_unregister(pPlatformDev);
    }

    platform_driver_unregister (&MUC_hcd_driver);

#ifdef USB_READ_WRITE_TEST
    usb_uninit_procfs();
#endif

#ifdef SIMULATE_USB_INSERT
    usb_uninit_sysfs();
#endif

    MGC_hareware_down();

    return;
}



/*-------------------------------------------------------------------------*/
module_init(MUC_init);
module_exit(MUC_cleanup);
module_param(usb_protocol, charp, S_IRUGO);

MODULE_DESCRIPTION("ATC 83XX USB Host Controller Driver");
MODULE_LICENSE("GPL");

