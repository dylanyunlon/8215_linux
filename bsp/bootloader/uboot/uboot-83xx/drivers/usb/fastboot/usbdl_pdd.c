#include"usbfntypedef.h"

#include "pdd.h"
extern void printf (LPCSTR sz, ...);

#define KITLOutputDebugString  printf

static volatile VOID          *g_pUSBRegs = NULL;
static UFN_PDD_CONTEXT   g_PddContext;   
static UINT g_FIFOadd = USB_FIFO_START_ADDRESS;




#include "reg_timer.h"

#define ONE_MS_TICK    CFG_CLOCK_PER_TICKS


static AC83XX_TIMER_64B_REG *g_TimerVirt64b;


static UINT32 OALGetTickCount(VOID)
{

    UINT64 u4H_pre, u4H_next;
    UINT32 u4L;
    UINT64  CurrentCount = 0;

 
    do
    {

        u4H_pre  = (UINT64)g_TimerVirt64b->TIMER0_64B_HI;
       
        u4L      = g_TimerVirt64b->TIMER0_64B_LO;
        u4H_next = (UINT64)g_TimerVirt64b->TIMER0_64B_HI;

    }while(u4H_pre != u4H_next);
    
    CurrentCount = (u4H_next<<32)+u4L;

    return ((UINT32)(CurrentCount>>18));
    
}

static VOID OALTickInit(VOID)
{
    BOOL rc = FALSE;
    g_TimerVirt64b = (AC83XX_TIMER_64B_REG *) (BIM_BASE + REG_RW_64B_TIMER0_OFFSET); 
}

static DWORD    UfnPdd_InitEndpoint(VOID *pPddContext,DWORD endPoint,UFN_BUS_SPEED speed,USB_ENDPOINT_DESCRIPTOR *pEPDesc);

static BOOL HandleSetupPacket(USB_DEVICE_REQUEST *pUdr);
static VOID  Pdd_StartTrans(
               UFN_PDD_CONTEXT  *pPdd,
               DWORD            endPoint,
               STransfer        *pTransfer
               );

static STransfer g_EP0Transfer; // EP0
static STransfer g_EP1Transfer; // Bulk out
static STransfer g_EP2Transfer; // Bulk in


#define iCONF           18
#define CFGLEN          32

#define MANUFACTURER    "Mediatek"
#define PRODUCT         "MTK ADB Device"
#define SERIANUMBER     "90B7B7B9-2C3B-4da2-93F3-034802"//EB5F1A" 
#define INTRRFACE_STRING "ADB"



static  UCHAR gs_pucSupportedLanguage[] = 
{
    0x04,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09, 0x04          //  US English only..
};
static const UCHAR gs_Manufacturer[] =
{
   2*sizeof(MANUFACTURER) + 2,
   USB_STRING_DESCRIPTOR_TYPE,
   'M',0,'e',0,'d',0,'i',0,'a',0,'t',0,'e',0,'k',0,0,0
 };



static const UCHAR gs_Product[] =
{
   2*sizeof(PRODUCT) + 2,
   USB_STRING_DESCRIPTOR_TYPE,
   'M',0,'T',0,'K',0,' ',0,'A',0,'D',0,'B',0,' ',0,'D',0,'e',0,'v',0,'i',0,'c',0,'e',0,0,0
 };


static const UCHAR gs_SerialNumber[] =
{
   2*sizeof(SERIANUMBER) + 2,
   USB_STRING_DESCRIPTOR_TYPE,
   '9',0,'0',0,'B',0,'7',0,'B',0,'7',0,'B',0,'9',0,'-',0,'2',0,'C',0,'3',0,'B',0,'-',0,
   '4',0,'d',0,'a',0,'2',0,'-',0,'9',0,'3',0,'F',0,'3',0,'-',0,'0',0,'3',0,'4',0,
   '8',0,'0',0,'2',0, 0 ,0
   //'E',0,'B',0,'5',0,'F',0,'1',0,'A',0,0,0
 };

static const UCHAR gs_Interface[] =
{
   2*sizeof(INTRRFACE_STRING) + 2,
   USB_STRING_DESCRIPTOR_TYPE,
   'A',0,'D',0,'B',0,0,0
 };


static UCHAR *gs_pucUSBDescriptors;

static const UCHAR gs_pucUSBFSDescriptors[] =
{
    ////////////////////////////////////////////////////////////////////////////
    // Standard Device Descriptor
    //

/* 0  */    18,                         //  bLength = 18 bytes.
/* 1  */    USB_DEVICE_DESCRIPTOR_TYPE, //  bDescriptorType = DEVICE
/* 2  */    0x10, 0x01,                 //  bcdUSB          = 1.1
/* 4  */    0x00,                       //  bDeviceClass    = Communication Device Class
/* 5  */    0x00,                       //  bDeviceSubClass = Unused at this time.
/* 6  */    0x00,                       //  bDeviceProtocol = Unused at this time.
/* 7  */    USB_EP0_MAXP,               //  bMaxPacketSize0 = EP0 buffer size..
/* 8  */    0xB4, 0x0B,                 //  idVendor        = Microsoft Vendor ID.
/* 10 */    0x01, 0x0C,                 //  idProduct       = Microsoft generic RNDISMINI Product ID.
/* 12 */    0x01, 0x00,                 //  bcdDevice       = 0.1
/* 14 */    0x01,                       //  iManufacturer   = OEM should fill this..
/* 15 */    0x02,                       //  iProduct        = OEM should fill this..
/* 16 */    0x03,                       //  iSerialNumber   = OEM should fill this..
/* 17 */    0x01,                       //  bNumConfigs     = 1 


    ////////////////////////////////////////////////////////////////////////////
    //  RNDIS requires only one configuration as follows..
    //  And we have 1 interfaces (Communication Class if & Dataclass if).
    //

/* 18 */    9,                                  //  bLength         = 9 bytes.
/* 19 */    USB_CONFIGURATION_DESCRIPTOR_TYPE,  //  bDescriptorType = CONFIGURATION
/* 20 */    0x20, 0x00,                 //  wTotalLength    = From offset 18 to end <---
/* 22 */    0x01,                       //  bNumInterfaces  = 1 
/* 23 */    0x01,                       //  bConfValue      = 1
/* 24 */    0x00,                       //  iConfiguration  = unused.
/* 25 */    0x40,                       //  bmAttributes    = Self-Powered.
/* 26 */    0x00,                       //  MaxPower        = x2mA



        
    ////////////////////////////////////////////////////////////////////////////
    //  Communication Class INTERFACE descriptor.
    //  RNDIS specifies 2 endpoints, EP0 & notification element (interrupt)
    //

/* 27 */    9,                          //  bLength         = 9 bytes.
/* 28 */    USB_INTERFACE_DESCRIPTOR_TYPE, //  bDescriptorType = INTERFACE
/* 29 */    0x00,                       //  bInterfaceNo    = 0
/* 30 */    0x00,                       //  bAlternateSet   = 0
/* 31 */    0x02,                       //  bNumEndPoints   = 2
/* 32 */    0xFF,                       //  bInterfaceClass = Comm if class (RNDIS spec)
/* 33 */    0x00,                       //  bIfSubClass     = Comm if sub        (ditto)
/* 34 */    0x00,                       //  bIfProtocol     = Vendor specific    (ditto)
/* 35 */    0x00,                       //  iInterface      = unused.




    ////////////////////////////////////////////////////////////////////////////
    //  Endpoint descriptors for Data Class Interface
    //

/* 36 */    7,                  //  bLength         = 7 bytes.
/* 37 */    USB_ENDPOINT_DESCRIPTOR_TYPE, //  bDescriptorType = ENDPOINT [IN]
/* 38 */    0x82,               //  bEndpointAddr   = IN -- EP2
/* 39 */    0x02,               //  bmAttributes    = BULK
/* 40 */    USB_EP_BULK_MAXP_FS, 0x00,    //  wMaxPacketSize
/* 42 */    0,                  //  bInterval       = ignored for BULK.

/* 43 */    7,                  //  bLength         = 7 bytes.
/* 44 */    USB_ENDPOINT_DESCRIPTOR_TYPE, //  bDescriptorType = ENDPOINT [OUT]
/* 45 */    0x01,               //  bEndpointAddr   = OUT -- EP1
/* 46 */    0x02,               //  bmAttributes    = BULK
/* 47 */    USB_EP_BULK_MAXP_FS, 0x00,    //  wMaxPacketSize
/* 49 */    0                   //  bInterval       = ignored for BULK.

};  //  gs_pucUSBFSDescriptors[]


static const UCHAR gs_pucUSBHSDescriptors[] =
{
    ////////////////////////////////////////////////////////////////////////////
    // Standard Device Descriptor
    //

/* 0  */    18,                         //  bLength = 18 bytes.
/* 1  */    USB_DEVICE_DESCRIPTOR_TYPE, //  bDescriptorType = DEVICE
/* 2  */    0x00, 0x02,                 //  bcdUSB          = 2.0
/* 4  */    0x00,                       //  bDeviceClass    = Communication Device Class
/* 5  */    0x00,                       //  bDeviceSubClass = Unused at this time.
/* 6  */    0x00,                       //  bDeviceProtocol = Unused at this time.
/* 7  */    USB_EP0_MAXP,                 //  bMaxPacketSize0 = EP0 buffer size..
        /* 8  */    0xB4, 0x0B,                 //  idVendor        = Microsoft Vendor ID.
        /* 10 */    0x01, 0x0C,                 //  idProduct       = Microsoft generic RNDISMINI Product ID.
/* 12 */    0x01, 0x00,                 //  bcdDevice       = 0.1
/* 14 */    0x01,                       //  iManufacturer   = OEM should fill this..
/* 15 */    0x02,                       //  iProduct        = OEM should fill this..
/* 16 */    0x03,                       //  iSerialNumber   = OEM should fill this..
/* 17 */    0x01,                       //  bNumConfigs     = 1 


    ////////////////////////////////////////////////////////////////////////////
    //  RNDIS requires only one configuration as follows..
    //  And we have 2 interfaces (Communication Class if & Dataclass if).
    //

/* 18 */    9,                                  //  bLength         = 9 bytes.
/* 19 */    USB_CONFIGURATION_DESCRIPTOR_TYPE,  //  bDescriptorType = CONFIGURATION
/* 20 */    0x20, 0x00,                 //  wTotalLength    = From offset 18 to end <---
/* 22 */    0x01,                       //  bNumInterfaces  = 1 
/* 23 */    0x01,                       //  bConfValue      = 1
/* 24 */    0x00,                       //  iConfiguration  = unused.
/* 25 */    0x40,                       //  bmAttributes    = Self-Powered.
/* 26 */    0x00,                       //  MaxPower        = x2mA



        
    ////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////
        //  Communication Class INTERFACE descriptor.
        //  RNDIS specifies 2 endpoints, EP0
        
    
    /* 27 */    9,                          //  bLength         = 9 bytes.
    /* 28 */    USB_INTERFACE_DESCRIPTOR_TYPE, //  bDescriptorType = INTERFACE
    /* 29 */    0x00,                       //  bInterfaceNo    = 0
    /* 30 */    0x00,                       //  bAlternateSet   = 0
    /* 31 */    0x02,                       //  bNumEndPoints   = 2
    /* 32 */    0xFF,                       //  bInterfaceClass = Comm if class (RNDIS spec)
    /* 33 */    0x42,                      //  bIfSubClass     = Comm if sub        (ditto)
    /* 34 */    0x03,                     //  bIfProtocol     = Vendor specific    (ditto)
    /* 35 */    0x07,                       //  iInterface      = unused.



    ////////////////////////////////////////////////////////////////////////////
    //  Endpoint descriptors for Data Class Interface
    //

/* 36 */    7,                  //  bLength         = 7 bytes.
/* 37 */    USB_ENDPOINT_DESCRIPTOR_TYPE, //  bDescriptorType = ENDPOINT [IN]
/* 38 */    0x82,               //  bEndpointAddr   = IN -- EP2
/* 39 */    0x02,               //  bmAttributes    = BULK
/* 40 */    (USB_EP_BULK_MAXP_HS&0xFF), (USB_EP_BULK_MAXP_HS>>8),    //  wMaxPacketSize
/* 42 */    0,                  //  bInterval       = ignored for BULK.

/* 43 */    7,                  //  bLength         = 7 bytes.
/* 44 */    USB_ENDPOINT_DESCRIPTOR_TYPE, //  bDescriptorType = ENDPOINT [OUT]
/* 45 */    0x01,               //  bEndpointAddr   = OUT -- EP1
/* 46 */    0x02,               //  bmAttributes    = BULK
/* 47 */    (USB_EP_BULK_MAXP_HS&0xFF), (USB_EP_BULK_MAXP_HS>>8),    //  wMaxPacketSize
/* 49 */    0                   //  bInterval       = ignored for BULK.

};  //  gs_pucUSBHSDescriptors[]




static DWORD Pdd_Log2(DWORD value)
{
    DWORD rc = 0;

    if(value == 3072/8)
    {
        return 0xF;
    }

    while ( 0 != value )
    {
        value >>= 1;
        rc++;
    }
    rc--;
    return rc;
}

static VOID UsbDelay(DWORD millisec){
    DWORD start_tick, curr_tick;
    start_tick = OALGetTickCount();
    do {
        curr_tick = OALGetTickCount();
        if (curr_tick < start_tick) {
            start_tick = curr_tick;  //overflow
        }
    }
    while ((curr_tick - start_tick) <= millisec);
}

static DWORD   UfnPdd_SendControlStatusHandshake(
                                  VOID *pPddContext,
                                  DWORD endPoint
                                  )
{
    UFN_PDD_CONTEXT     *pPdd = (UFN_PDD_CONTEXT*)pPddContext;  
    DWORD               epxCSR = 0;
    DWORD               dwWriteToEp0CSR = 0;


    if(pPdd->fSendDataEnd)
    {   

        MGC_SelectEnd(g_pUSBRegs,0);

        epxCSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0);
        dwWriteToEp0CSR = (epxCSR & ~(USB_EP0_STALL_BIT));
        dwWriteToEp0CSR |= (MGC_M_CSR0_P_DATAEND | MGC_M_CSR0_P_SVDRXPKTRDY);
        
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0,dwWriteToEp0CSR);

        pPdd->fSendDataEnd = FALSE;   

    }

    return ERROR_SUCCESS;
}


static DWORD  UfnPdd_StallEndpoint(
                     VOID *pPddContext,
                     DWORD endPoint
                     )
{
    DWORD               rc = ERROR_SUCCESS;
    UFN_PDD_CONTEXT     *pPdd = (UFN_PDD_CONTEXT*)pPddContext;
    DWORD               dwWriteToCSR = 0;


    MGC_SelectEnd(g_pUSBRegs,endPoint);
    if (endPoint == 0)
    {
        // Must Clear both Send and Sent Stall
        dwWriteToCSR |= (MGC_M_CSR0_P_DATAEND |
            MGC_M_CSR0_P_SVDRXPKTRDY |
            MGC_M_CSR0_P_SENDSTALL);
        MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0,dwWriteToCSR);
        pPdd->fSendDataEnd = FALSE;
        pPdd->Ep0State = EP0_STATE_IDLE;
    }
    else if (pPdd->ep[endPoint].fdirRx)
    { // OUT Endpoint
        WORD CSR;
        CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,endPoint);
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,endPoint, (CSR | MGC_M_RXCSR_P_SENDSTALL));
    }
    else
    {   // INT Endpoint
        // Must Clear both Send and Sent Stall

        WORD CSR;
        CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,endPoint);
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,endPoint, (CSR | MGC_M_TXCSR_P_SENDSTALL));
    }



    return ERROR_SUCCESS;
}



static WORD   UfnPdd_ClearEndpointStall(
                          VOID *pPddContext,
                          DWORD endPoint
                          )
{
    DWORD               rc = ERROR_SUCCESS;
    UFN_PDD_CONTEXT     *pPdd = (UFN_PDD_CONTEXT*)pPddContext;
    WORD CSR;


    MGC_SelectEnd(g_pUSBRegs,endPoint);

    if (endPoint == 0)
    {
        // Must Clear both Send and Sent Stall
        CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0);
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0,(CSR & (~USB_EP0_STALL_BIT)));

    }
    else if (pPdd->ep[endPoint].fdirRx)  // OUT Endpoint
    {

        CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,endPoint);
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,endPoint,(CSR & (~USB_RX_STALL_BIT)));



    }
    else                                 // IN Endpoint
    {
        DWORD IntrE;
        CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,endPoint);
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,endPoint,(CSR & (~USB_TX_STALL_BIT)));

        IntrE = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRTXE);
        MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTXE,IntrE |(( 0x01 << endPoint)));

    }


    return rc;
}



static UFN_BUS_SPEED Pdd_GetSpeed(UFN_PDD_CONTEXT* pPdd)
{
    UCHAR  power = MGC_Read8(g_pUSBRegs, MGC_O_HDRC_POWER); 

    if ((power&MGC_M_POWER_HSMODE) != 0)
    {

         
        return BS_HIGH_SPEED;
    }
    else
    {
         
        return BS_FULL_SPEED;   
    }
}


static void Pdd_CompleteTransfer(
                     UFN_PDD_CONTEXT* pPdd,
                     DWORD endPoint,
                     DWORD dwStatus
                     )
{


    if(0 == endPoint)
    {
           UfnPdd_SendControlStatusHandshake(pPdd,0);
       
      
    }

    return;

}


static DWORD UfnPdd_SetAddress(VOID *pPddContext, UCHAR address)
{
    UFN_PDD_CONTEXT *pPdd = (UFN_PDD_CONTEXT*)pPddContext;



    pPdd->fSetAddress = TRUE;
    pPdd->HWAddress   = address;  
    g_EP0Transfer.pvBuffer;
    g_EP0Transfer.dwFlags = 0;
    g_EP0Transfer.cbBuffer = 0;
    g_EP0Transfer.cbTransferred = 0;
    g_EP0Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
    g_EP0Transfer.pvPddData = NULL;
    g_EP0Transfer.pvPddTransferInfo = NULL;

    Pdd_StartTrans(&g_PddContext,0,&g_EP0Transfer);

    return ERROR_SUCCESS;
}

static DWORD  UfnPdd_Start( VOID *pPddContext )
{
    UFN_PDD_CONTEXT *pPdd = (UFN_PDD_CONTEXT*)pPddContext;
    DWORD dwRet,u4Reg;


    // set necessary EP INT, SUSPENDM enable and High-speed disable
    // set necessary EP INT, SUSPENDM enable and High-speed disable

    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_INTRUSBE,USBD_COMM_IRQ_MASK);
    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_POWER,
        MGC_Read8(g_pUSBRegs, MGC_O_HDRC_POWER) | MGC_M_POWER_HSENAB);
    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_POWER,
        MGC_Read8(g_pUSBRegs, MGC_O_HDRC_POWER) | MGC_M_POWER_SOFTCONN);
    u4Reg = MGC_Read32(g_pUSBRegs, MGC_O_INTRLEVEL1EN);
    u4Reg |= 0x0f; 
    MGC_Write32(g_pUSBRegs, MGC_O_INTRLEVEL1EN, u4Reg);
    KITLOutputDebugString("Setting level1En to 0x%x\n",u4Reg);
    dwRet = ERROR_SUCCESS;

    return dwRet;
}

static VOID  Pdd_ResetEP(UFN_PDD_CONTEXT* pPdd,DWORD dwEndPoint)
{   
    UsbFnEp     *pEP = &pPdd->ep[dwEndPoint];

    // Clear all IN and OUT bits associated with endpoint.
    MGC_SelectEnd(g_pUSBRegs,dwEndPoint);

    if(dwEndPoint == 0 )
    {
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0,0); 
        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0,MGC_M_CSR0_P_FLUSHFIFO); 


        MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0, 
            (MGC_M_CSR0_P_SVDRXPKTRDY | MGC_M_CSR0_P_SVDSETUPEND));
        pPdd->Ep0State = EP0_STATE_IDLE;
    }
    else if(dwEndPoint < ENDPOINT_COUNT)
    {
        if(pPdd->ep[dwEndPoint].fdirRx)  
        {

            /* flush twice in case of double packet buffering */
            MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,dwEndPoint, 
                (MGC_M_RXCSR_CLRDATATOG | MGC_M_RXCSR_FLUSHFIFO));    

            MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,dwEndPoint, 
                (MGC_M_RXCSR_CLRDATATOG | MGC_M_RXCSR_FLUSHFIFO));


            //clear and disable RX interrupt
            
            MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRX, (0x01ul << dwEndPoint));
            if(pEP->pTransfer)
            {
                WORD IntrE;
                IntrE = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRRXE);
                MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRXE,IntrE | ( 0x01 << dwEndPoint));
            }
                


        }
        else         //IN  -- Tx
        {

            /* twice in case of double packet buffering */
            MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,dwEndPoint, 
                (MGC_M_TXCSR_CLRDATATOG | MGC_M_TXCSR_FLUSHFIFO));    

            MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,dwEndPoint, 
                (MGC_M_TXCSR_CLRDATATOG | MGC_M_TXCSR_FLUSHFIFO));


            //disable Tx interrupt
            //Tx Intr Read Clear 
            MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTX, (0x01ul << dwEndPoint)); 
            
            if(pEP->pTransfer)
            {
                WORD IntrE;
                IntrE = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRTXE);
                MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTXE,IntrE | ( 0x01 << dwEndPoint));
            }
        }
    }   


}

static VOID Pdd_ResetDevice(UFN_PDD_CONTEXT* pPdd)
{
    UINT epx;
    WORD wEpStatus;
    UINT32 u4Reg;
    // Reset Fifo address counter
    g_FIFOadd = USB_FIFO_START_ADDRESS;

    // Disable endpoint interrupts - write Zeros to Disable
    MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTXE,0x01);
    wEpStatus = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRTX);
    MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTX,wEpStatus);

    MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRXE,0);
    //clear all ep interrupt
    wEpStatus = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRRX);
    MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRX,wEpStatus);


    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_INTRUSBE,0);
    wEpStatus = MGC_Read8(g_pUSBRegs, MGC_O_HDRC_INTRUSB);
    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_INTRUSB,(BYTE)wEpStatus);

    //set necessary EP INT, SUSPENDM enable and High-speed disable
    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_INTRUSBE,USBD_COMM_IRQ_MASK);

    //Initialize EP0    
    MGC_SelectEnd(g_pUSBRegs,EP0_INDEX);
    MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_CSR0,0, MGC_M_CSR0_P_FLUSHFIFO);  
    MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTXE,1);




    for( epx = 1; epx < ENDPOINT_COUNT; epx++ )
    {
        Pdd_ResetEP(pPdd, epx);
    }


    u4Reg = MGC_Read32(g_pUSBRegs, MGC_O_INTRLEVEL1EN);
    u4Reg |= 0x0f; 
    MGC_Write32(g_pUSBRegs, MGC_O_INTRLEVEL1EN, u4Reg);
    
}


static void Pdd_ReadFIFO(DWORD dwEndPoint,DWORD dwToTransferSize, BYTE* pbBuffer)
{
    DWORD dwCount = dwToTransferSize;
    DWORD bFifoOffset = MGC_FIFO_OFFSET(dwEndPoint);
    DWORD cbTmpRead;
    DWORD *pdwBuffer,cbRead;


    // read unaligned data
    cbTmpRead = min(dwCount, ((DWORD)pbBuffer&0x3));

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 0);
    for(cbRead = 0; cbRead < cbTmpRead; cbRead++)
    {
        *pbBuffer++ = *((volatile BYTE *)(0xF000E000 + MUSB_COREBASE + bFifoOffset));
    }

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 2);
    // read aligned data
    pdwBuffer = (PDWORD)pbBuffer;
    for(; (cbRead+4) <= dwCount; cbRead+=4)
    {
        *pdwBuffer++ = *((volatile DWORD *)(0xF000E000 + MUSB_COREBASE + bFifoOffset));
    }

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 0);
    // read last unaligned data
    pbBuffer = (PBYTE)pdwBuffer;
    for(; cbRead < dwCount; cbRead++)
    {                                                             
        *pbBuffer++ = *((volatile BYTE *)(0xF000E000 + MUSB_COREBASE + bFifoOffset));
    }  

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 2);



    return;
}

static void Pdd_WriteFIFO(DWORD dwEndPoint,DWORD dwToTransferSize, BYTE* pbBuffer)
{


    DWORD dwCount = dwToTransferSize;
    DWORD bFifoOffset = MGC_FIFO_OFFSET(dwEndPoint);
    DWORD cbTmpWrite;
    DWORD *pdwBuffer,cbWrite;


    // read unaligned data
    cbTmpWrite = min(dwCount, ((DWORD)pbBuffer&0x3));

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 0);
    for(cbWrite = 0; cbWrite < cbTmpWrite; cbWrite++)
    {
        *((volatile BYTE *)(0xF000E000 + MUSB_COREBASE + bFifoOffset)) = *pbBuffer++;
    }

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 2);
    // read aligned data
    pdwBuffer = (PDWORD)pbBuffer;
    for(; (cbWrite+4) <= dwCount; cbWrite+=4)
    {
        *((volatile DWORD *)(0xF000E000 + MUSB_COREBASE + bFifoOffset)) =  *pdwBuffer++ ;
    }

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 0);
    // read last unaligned data
    pbBuffer = (PBYTE)pdwBuffer;
    for(; cbWrite < dwCount; cbWrite++)
    {                                                             
        *((volatile BYTE *)(0xF000E000 + MUSB_COREBASE + bFifoOffset)) = *pbBuffer++;
    }  

    MGC_FIFO_CNT(g_pUSBRegs, M_REG_FIFOBYTECNT, 2);

    return;
}


static WORD  Pdd_HandleRx(
             UFN_PDD_CONTEXT    *pPdd,
             DWORD              endPoint,
             PBOOL              pbCompleted,
             PDWORD             pdwStatus
             )
{
    UsbFnEp *pEP = &pPdd->ep[endPoint];
    STransfer *pTransfer = pEP->pTransfer;
    BOOL     bcomplete = FALSE;
    DWORD    FifoCount; //bytes in FIFO

    //byte read this time,left to read,remain this time
    DWORD    cbBuffer, cbToRead;    
    WORD     wWriteToCSR = 0;   
    UCHAR    *pBuffer;

    DWORD    dwStatus = ERROR_GEN_FAILURE;


    if (pTransfer == NULL)
    {
        goto cleanUp;
    }

    pBuffer = (UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
    cbBuffer = pTransfer->cbBuffer - pTransfer->cbTransferred;

    // Select EP   
    MGC_SelectEnd(g_pUSBRegs,endPoint);
    if(endPoint == 0)
    {
        FifoCount = MGC_ReadCsr8(g_pUSBRegs, MGC_O_HDRC_COUNT0, 0);
    }
    else
    {
        FifoCount = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_RXCOUNT, endPoint);
    }

    // Read data
    // if(FifoCount > cbBuffer);what should do?
    cbToRead = min(FifoCount, cbBuffer);

    if(FifoCount > cbBuffer)
    {
         
    }
    Pdd_ReadFIFO(endPoint,cbToRead, pBuffer);  

    pTransfer->cbTransferred += cbToRead;
if(pTransfer->cbBuffer > 64)
{
    if ((pTransfer->cbTransferred == pTransfer->cbBuffer) ||
        (cbToRead < pEP->maxPacketSize))
    {
        dwStatus = UFN_NO_ERROR;
        bcomplete = TRUE;
    }
}
else
{

    dwStatus = UFN_NO_ERROR;
    bcomplete = TRUE;
}

    if(endPoint == 0)
    {
        wWriteToCSR |= MGC_M_CSR0_P_SVDRXPKTRDY;
        if(bcomplete)
        {
            wWriteToCSR |= MGC_M_CSR0_P_DATAEND;
            pPdd->Ep0State = EP0_STATE_IDLE;
        }
    }
cleanUp:
    

    *pbCompleted = bcomplete;
    *pdwStatus = dwStatus;

    return wWriteToCSR;
}


static WORD  Pdd_HandleTx(
             UFN_PDD_CONTEXT *pPdd,
             DWORD endPoint,
             PBOOL              pbCompleted,
             PDWORD             pdwStatus
             )
{
    UsbFnEp *pEP = &pPdd->ep[endPoint];
    STransfer *pTransfer = pEP->pTransfer;
    BOOL  bcomplete = FALSE;
    //Data left, actual write,want write
    DWORD cbBuffer, cbToWrite;
    WORD  wEpCsrToWrite = 0;
    PBYTE pBuffer;

    DWORD dwStatus = ERROR_GEN_FAILURE;

    

    // When transfer is NULL it is handshake ACK
    if (pTransfer == NULL)
    {
        goto cleanUp;
    }
    MGC_SelectEnd(g_pUSBRegs,endPoint);


    pBuffer = (PBYTE)pTransfer->pvBuffer + pTransfer->cbTransferred;
    if(NULL == pBuffer){
        goto cleanUp;
    }

    cbBuffer = pTransfer->cbBuffer - pTransfer->cbTransferred;

    // How many bytes we can send just now?
    cbToWrite = min(cbBuffer, pEP->maxPacketSize);

    // Write data to FIFO
    if(endPoint == 0)
    {   
        Pdd_WriteFIFO(0,cbToWrite,pBuffer);

        // updata the bytes Transferred
        pTransfer->cbTransferred += cbToWrite;
        if((pTransfer->cbTransferred == pTransfer->cbBuffer) &&
            (pTransfer->pvPddData == 0))
        {
            dwStatus = UFN_NO_ERROR;;
            pPdd->Ep0State = EP0_STATE_IDLE;
            wEpCsrToWrite |= MGC_M_CSR0_P_DATAEND;
            bcomplete = TRUE;
        }

        if((cbToWrite > 0) || (pTransfer->cbBuffer == 0))
        {
            // We transfered some data and it maybe end
            wEpCsrToWrite |= MGC_M_CSR0_TXPKTRDY;
        }
        else if(pTransfer->pvPddData)
        {
            // need send zero-length data
            wEpCsrToWrite |= MGC_M_CSR0_TXPKTRDY;
            pTransfer->pvPddData = 0;
        }
    }
    else   //EP n(1~5)
    {
        //Enable interrupt before write to FIFO. This ensure
        //any interrupt generated because data is transmitted.

        WORD IntrE;

        IntrE = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRTXE);
        MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTXE,IntrE | ( 0x01 << endPoint));


        Pdd_WriteFIFO(endPoint,cbToWrite,pBuffer);

        wEpCsrToWrite = MGC_M_TXCSR_TXPKTRDY;   

        // updata the bytes Transferred               
        pTransfer->cbTransferred += cbToWrite;              
        if((pTransfer->cbTransferred == pTransfer->cbBuffer) ||
            (cbToWrite == 0))
        {                   
            dwStatus = UFN_NO_ERROR;
            bcomplete = TRUE;
        }
    }

cleanUp:
    

    *pbCompleted = bcomplete;
    *pdwStatus = dwStatus;

    return wEpCsrToWrite;
}

static void  Pdd_EpnHandle(
              UFN_PDD_CONTEXT* pPdd,
              DWORD endPoint
              )
{
    UsbFnEp *pEP = &pPdd->ep[endPoint];
    WORD wEpXCSR = 0, wWriteToEpXCSR = 0;
    USB_DEVICE_REQUEST udr;
    DWORD dwStatus = 0;


    MGC_SelectEnd(g_pUSBRegs,endPoint);


    if(pEP->fdirRx)
        wEpXCSR = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_RXCSR, endPoint);
    else
        wEpXCSR = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_TXCSR, endPoint);

    if(pEP->fdirRx)  //OUT Transfer
    {
        wWriteToEpXCSR = (wEpXCSR & (~MGC_M_RXCSR_CLRDATATOG));

        // fake usb request - clear stall
        if(wEpXCSR & MGC_M_RXCSR_P_SENTSTALL)
        {
            WORD IntrE;
            udr.bmRequestType = USB_REQUEST_FOR_ENDPOINT;
            udr.bRequest = USB_REQUEST_CLEAR_FEATURE;
            udr.wValue = USB_FEATURE_ENDPOINT_STALL;
            udr.wIndex = (USHORT) endPoint;
            udr.wLength = 0;


            IntrE = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRRXE);
            MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRXE,IntrE & (~( 0x01 << endPoint)));



            wWriteToEpXCSR &= ~(MGC_M_RXCSR_P_SENTSTALL | MGC_M_RXCSR_P_SENDSTALL);
        }

        if(wEpXCSR & MGC_M_RXCSR_RXPKTRDY) //Read FIFO not empty
        {
            wWriteToEpXCSR |= Pdd_HandleRx(pPdd, endPoint, &pEP->bTransferComplete, &dwStatus);
            //if Fifo read not complete?how to do?
            wWriteToEpXCSR &= ~(MGC_M_RXCSR_RXPKTRDY);

        }
        MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_RXCSR,endPoint,wWriteToEpXCSR);

    }
    else    //IN Transfer
    {
        wWriteToEpXCSR = (wEpXCSR & ~MGC_M_TXCSR_CLRDATATOG);

        if(wEpXCSR & MGC_M_TXCSR_P_SENTSTALL)
        {


            WORD IntrE;
            udr.bmRequestType = USB_REQUEST_FOR_ENDPOINT;
            udr.bRequest = USB_REQUEST_CLEAR_FEATURE;
            udr.wValue = USB_FEATURE_ENDPOINT_STALL;
            udr.wIndex = USB_ENDPOINT_DIRECTION_MASK | (BYTE) endPoint;
            udr.wLength = 0;
            //disable ep intr
            IntrE = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRTXE);
            MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTXE,IntrE & (~( 0x01 << endPoint)));

            //the host send a stall packet,so,the last data transfer have not yet complete
            pEP->bTransferComplete = FALSE; 

        

            wWriteToEpXCSR &= ~USB_TX_STALL_BIT;

        }

        if((!pEP->bTransferComplete) && !(wEpXCSR & MGC_M_TXCSR_TXPKTRDY)) //transfer the next packet
        {
            wWriteToEpXCSR |= Pdd_HandleTx(pPdd, endPoint, &pEP->bTransferComplete, &dwStatus);
        }


        MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_TXCSR,endPoint,wWriteToEpXCSR);



    } 


    if(pEP->bTransferComplete)
    {
    
        Pdd_CompleteTransfer(pPdd, endPoint, dwStatus);
    }



}
static void Pdd_Ep0Handle(UFN_PDD_CONTEXT* pPdd)
{
    UsbFnEp *pEP = &pPdd->ep[0];
    STransfer *pTransfer = pEP->pTransfer;
    WORD  wEp0CsrToWrite = 0;
    DWORD cbRxCount, dwStatus;
    BOOL fSendUdr = FALSE;
    BOOL bHandleTX = FALSE;
    BYTE * pbUdr;


    MGC_SelectEnd(g_pUSBRegs,0);
    wEp0CsrToWrite = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0);



    if(wEp0CsrToWrite & MGC_M_CSR0_P_SENTSTALL)
    {
        DWORD CSR;
        //Clear Sent Stall
        pPdd->Ep0State = EP0_STATE_IDLE;
        CSR =  MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0);
        CSR &= (~USB_EP0_STALL_BIT);
        MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0,0,CSR);
    }

    if(wEp0CsrToWrite & MGC_M_CSR0_P_SETUPEND)
    {
        DWORD CSR;
        pPdd->Ep0State = EP0_STATE_IDLE;
        CSR =  MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0);
        MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0,0,(CSR | MGC_M_CSR0_P_SVDSETUPEND) );
        if(pTransfer)
        {
            pPdd->fSendDataEnd = FALSE;
            Pdd_CompleteTransfer(pPdd, 0, UFN_NO_ERROR);
        }
    }

    wEp0CsrToWrite = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0);
    cbRxCount = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_COUNT0, 0);

    //Handle SETUP Packet
    if(pPdd->Ep0State == EP0_STATE_IDLE)
    {
        if(pPdd->fSetAddress)
        {
            
            MGC_Write8(g_pUSBRegs, MGC_O_HDRC_FADDR,pPdd->HWAddress );
            pPdd->fSetAddress = FALSE;
            
        }

        if(wEp0CsrToWrite & MGC_M_CSR0_RXPKTRDY)
        {
            if (cbRxCount == sizeof(pPdd->udr))
            {
                pbUdr = (BYTE *)&pPdd->udr;
                //we know that PC request is 8 bytes.
                Pdd_ReadFIFO(0,8, pbUdr);

                if(pPdd->udr.wLength > 0)
                {
                    if(pPdd->udr.bmRequestType & USB_ENDPOINT_DIRECTION_MASK)
                    {
                        pPdd->Ep0State = EP0_STATE_IN_DATA_PHASE;
                        
                        
                    }
                    else
                    {
                        
                        
                        pPdd->Ep0State = EP0_STATE_OUT_DATA_PHASE;
                    }
                    pPdd->fSendDataEnd = FALSE;
                    

                }
                else if(pPdd->udr.wLength == 0) // Determine if this is a 0 length Data Packet
                {
                    pPdd->fSendDataEnd = TRUE;
                    
                    pPdd->Ep0State = EP0_STATE_IDLE;
                }
                
                    fSendUdr = TRUE;
            }
            else
            {
                

                // Ideally this should not hapen. This is a recovery mechanism if
                // we get out of sync somehow.
                wEp0CsrToWrite |= (MGC_M_CSR0_P_SENDSTALL | MGC_M_CSR0_P_SVDRXPKTRDY |
                    MGC_M_CSR0_P_DATAEND);
            }           
        }
        //wEp0CsrToWrite |= MGC_M_CSR0_P_SVDRXPKTRDY;
    }
    else if(pPdd->Ep0State == EP0_STATE_IN_DATA_PHASE)
    {
        if((wEp0CsrToWrite & MGC_M_CSR0_RXPKTRDY) && (cbRxCount == 0))
        {
            bHandleTX = TRUE;
        }
        // issue next
        else if((wEp0CsrToWrite & MGC_M_CSR0_TXPKTRDY) == 0)
        {
            
            bHandleTX = TRUE;
        }
        if(bHandleTX)
        {
            wEp0CsrToWrite |= Pdd_HandleTx(pPdd, 0, &pEP->bTransferComplete, &dwStatus);
        }
    }
    else if(pPdd->Ep0State == EP0_STATE_OUT_DATA_PHASE)
    {
        if((wEp0CsrToWrite & MGC_M_CSR0_RXPKTRDY))
        {
            wEp0CsrToWrite |= MGC_M_CSR0_P_SVDRXPKTRDY;
            if(cbRxCount > 0)
            {
                
                wEp0CsrToWrite |= Pdd_HandleRx(pPdd, 0, &pEP->bTransferComplete, &dwStatus);
            }
            else
                pPdd->Ep0State = EP0_STATE_IDLE;
        }
    }

    MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0,wEp0CsrToWrite);

    //notify MDD that We receive a setup packet
    if(fSendUdr)
    {
        

        HandleSetupPacket(&pPdd->udr);

    }else if(pEP->bTransferComplete)//notify MDD that data phase transfer complete
    {
        pEP->bTransferComplete = FALSE;
        Pdd_CompleteTransfer(pPdd, 0, dwStatus);
    }
}
static VOID  Pdd_StartTrans(
               UFN_PDD_CONTEXT  *pPdd,
               DWORD            endPoint,
               STransfer        *pTransfer
               )
{
    UsbFnEp     *pEP = &pPdd->ep[endPoint];
    WORD        wEpXCSR = 0, bEpXWriteCSR = 0;
    DWORD       dwTkStart= 0, dwTkEnd= 0, dwStatus;

    pEP->pTransfer = pTransfer;
    pEP->bTransferComplete = FALSE;
    //Select EP

    MGC_SelectEnd(g_pUSBRegs,endPoint);

    //if(pTransfer->dwFlags == USB_IN_TRANSFER)   //IN Transfer
    if(pTransfer->dwFlags == 0x80)
    {
        
        if (endPoint == 0)
        {
                 
            if((pTransfer->cbBuffer < pPdd->udr.wLength)&&
                (pTransfer->cbBuffer != 0) &&
                ((pTransfer->cbBuffer % pPdd->ep[endPoint].maxPacketSize) == 0))
            {
                
                pTransfer->pvPddData = (PVOID)1;
            }

            wEpXCSR = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0);
            bEpXWriteCSR = (wEpXCSR & ~(USB_EP0_STALL_BIT));
            if(wEpXCSR & MGC_M_CSR0_RXPKTRDY)
            {
                //Clear previous RXPKETRDY bit for next EP0's read interrupt
                bEpXWriteCSR |= MGC_M_CSR0_P_SVDRXPKTRDY;

                MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0,bEpXWriteCSR);

                bEpXWriteCSR &= ~MGC_M_CSR0_P_SVDRXPKTRDY;
            }
            bEpXWriteCSR |= Pdd_HandleTx(pPdd, 0, &pEP->bTransferComplete, &dwStatus);
            MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0,bEpXWriteCSR);


        }
        else   //EPn
        {  
            BOOL timeout = FALSE;
            wEpXCSR = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_TXCSR, endPoint);
            bEpXWriteCSR = (wEpXCSR & ~USB_TX_STALL_BIT);

            dwTkStart = OALGetTickCount();

            while(MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_TXCSR, endPoint) & MGC_M_TXCSR_FIFONOTEMPTY)
            {
                dwTkEnd = OALGetTickCount();

                if((dwTkEnd - dwTkStart) > 1000)
                {
                    timeout = TRUE;
                    break;
                }
            }
            if (timeout == FALSE) 
            {
                bEpXWriteCSR |= Pdd_HandleTx(pPdd, endPoint, &pEP->bTransferComplete, &dwStatus);
                MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_TXCSR, endPoint,bEpXWriteCSR);

            }
            else
            {

                KITLOutputDebugString("Wait EP[%d] FIFO Empty Time-out\r\n",endPoint);
            }

        }

    }
    else    //OUT Transfer
    {
        if(endPoint == 0)
        {
            wEpXCSR = MGC_ReadCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0); 

            bEpXWriteCSR = (wEpXCSR & ~(USB_EP0_STALL_BIT));

            if(wEpXCSR & MGC_M_CSR0_RXPKTRDY)
            {
                //Clear previous RXPKETRDY bit for next EP0's read interrupt
                bEpXWriteCSR |= MGC_M_CSR0_P_SVDRXPKTRDY;
            }
            MGC_WriteCsr16(g_pUSBRegs, MGC_O_HDRC_CSR0, 0,bEpXWriteCSR);
        }
        else
        {
            //Enable EP Interrupt,only here


            WORD IntrE;
            IntrE = MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRRXE);
            MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRXE,IntrE | ( 0x01 << (endPoint)));


        }
    }
    
}





static DWORD  UfnPdd_RegisterDevice(VOID *pPddContext)
{   
    UFN_PDD_CONTEXT *pPdd = (UFN_PDD_CONTEXT *)pPddContext;
    USB_DEVICE_DESCRIPTOR *pDeviceDesc;
    UFN_CONFIGURATION *pConfig;
    UFN_ENDPOINT *pEpConfig;
    UINT i, ep, numEp;
    UFN_BUS_SPEED speed;
    static USB_DEVICE_DESCRIPTOR fullSpeedDeviceDesc;
    static UFN_CONFIGURATION fullSpeedConfig;
    static UFN_ENDPOINT fullSpeedEndPoints[2];
    static UFN_INTERFACE fullSpeedInterface;

    static USB_DEVICE_DESCRIPTOR highSpeedDeviceDesc;
    static UFN_CONFIGURATION highSpeedConfig;
    static UFN_ENDPOINT highSpeedEndPoints[2];
    static UFN_INTERFACE highSpeedInterface;
    

    fullSpeedConfig.pInterfaces = &fullSpeedInterface;
    fullSpeedConfig.Descriptor.bNumInterfaces = 1;
    fullSpeedInterface.pEndpoints = fullSpeedEndPoints;
    fullSpeedInterface.Descriptor.bNumEndpoints = 2;
    
    fullSpeedEndPoints[0].Descriptor.bEndpointAddress = 0x01; // EP1, OUT (BULK)
    fullSpeedEndPoints[0].Descriptor.wMaxPacketSize = USB_EP_BULK_MAXP_FS;
    fullSpeedEndPoints[0].Descriptor.bmAttributes = USB_ENDPOINT_TYPE_BULK;
    fullSpeedEndPoints[1].Descriptor.bEndpointAddress = 0x82; // EP2, IN (BULK)
    fullSpeedEndPoints[1].Descriptor.wMaxPacketSize = USB_EP_BULK_MAXP_FS;
    fullSpeedEndPoints[1].Descriptor.bmAttributes = USB_ENDPOINT_TYPE_BULK;
    fullSpeedDeviceDesc.bMaxPacketSize0 = USB_EP0_MAXP;
    
    highSpeedConfig.pInterfaces = &highSpeedInterface;
    highSpeedConfig.Descriptor.bNumInterfaces = 1;
    highSpeedInterface.pEndpoints = highSpeedEndPoints;
    highSpeedInterface.Descriptor.bNumEndpoints = 2;
    highSpeedEndPoints[0].Descriptor.bEndpointAddress = 0x01; // EP1, OUT (BULK)
    highSpeedEndPoints[0].Descriptor.wMaxPacketSize = USB_EP_BULK_MAXP_HS;
    highSpeedEndPoints[0].Descriptor.bmAttributes = USB_ENDPOINT_TYPE_BULK;
    highSpeedEndPoints[1].Descriptor.bEndpointAddress = 0x82; // EP2, IN (BULK)
    highSpeedEndPoints[1].Descriptor.wMaxPacketSize = USB_EP_BULK_MAXP_HS;
    highSpeedEndPoints[1].Descriptor.bmAttributes = USB_ENDPOINT_TYPE_BULK;
    highSpeedDeviceDesc.bMaxPacketSize0 = USB_EP0_MAXP;

    

    g_FIFOadd = USB_FIFO_START_ADDRESS;

    speed = Pdd_GetSpeed(pPdd);

    if (speed == BS_HIGH_SPEED)
    {
        
        pDeviceDesc = (USB_DEVICE_DESCRIPTOR *)&highSpeedDeviceDesc;
        pConfig = (UFN_CONFIGURATION *)&highSpeedConfig;
        pEpConfig = (UFN_ENDPOINT *)&highSpeedEndPoints;
    }
    else
    {
        
        pDeviceDesc = (USB_DEVICE_DESCRIPTOR *)&fullSpeedDeviceDesc;
        pConfig = (UFN_CONFIGURATION *)&fullSpeedConfig;    
        pEpConfig = (UFN_ENDPOINT *)&fullSpeedEndPoints;
    }

    numEp = pConfig->pInterfaces->Descriptor.bNumEndpoints;

    for(i = 0; i < numEp; i++)
    {
        ep = pEpConfig[i].Descriptor.bEndpointAddress & 0x0F;
        UfnPdd_InitEndpoint(pPdd, ep, speed, &(pEpConfig[i].Descriptor));
    }

    return ERROR_SUCCESS;
}

static void  Pdd_USBEventHandle( UFN_PDD_CONTEXT *pPdd, UINT16 UsbIntrSrc)
{
    UINT16   state;

    state = UsbIntrSrc;

    // Disconnection
    if ((state & MGC_M_INTR_DISCONNECT) != 0)
    {
    
        return;
    }

    // Reset, means Connect
    if ((state & MGC_M_INTR_BABBLE) != 0)
    {
        UfnPdd_RegisterDevice(pPdd);
        Pdd_ResetDevice(pPdd);
        
        KITLOutputDebugString("USB Reset\n");
        return;
    }

    // Suspend
    if ((state & MGC_M_INTR_SUSPEND) != 0)
    {
         
    }

    //resume
    if ((state & MGC_M_INTR_RESUME) != 0)
    {
     

    }
}



static DWORD    UfnPdd_InitEndpoint(VOID *pPddContext,DWORD endPoint,UFN_BUS_SPEED speed,USB_ENDPOINT_DESCRIPTOR *pEPDesc)
{
    UFN_PDD_CONTEXT  *pPdd = (UFN_PDD_CONTEXT*)pPddContext;
    BYTE      bEndpointAddress = 0;
    DWORD     ep, dTxRxSZ;
    BOOL      fModeOut = FALSE;
    UINT16    wMaxPacketSize = 0;
    BYTE      bTransferType;


    wMaxPacketSize =
        pEPDesc->wMaxPacketSize & USB_ENDPOINT_MAX_PACKET_SIZE_MASK;

    // If the target is endpoint 0, then only allow the function driver
    // to register a notification function.
    if (endPoint == 0)
    {
        pPdd->ep[endPoint].maxPacketSize = wMaxPacketSize;

        //reset selected endpoint
        Pdd_ResetEP(pPdd, endPoint);
    }
    else if (endPoint < ENDPOINT_COUNT)
    {
        // Get Endpoint number
        ep = pEPDesc->bEndpointAddress & 0x0F;

        // Setup Direction (from host side looking)
        bEndpointAddress = pEPDesc->bEndpointAddress;
        fModeOut = USB_ENDPOINT_DIRECTION_OUT(bEndpointAddress);

        if (fModeOut)
            pPdd->ep[ep].fdirRx = TRUE;
        else
            pPdd->ep[ep].fdirRx = FALSE;

        pPdd->ep[ep].maxPacketSize = wMaxPacketSize;

        //reset selected endpoint
        Pdd_ResetEP(pPdd, endPoint);

        // Set Transfer Type
        bTransferType = pEPDesc->bmAttributes & USB_ENDPOINT_TYPE_MASK;
        // DEBUGCHK(bTransferType != USB_ENDPOINT_TYPE_CONTROL);

        //Select EP

        MGC_SelectEnd(g_pUSBRegs,ep);

        dTxRxSZ = Pdd_Log2(wMaxPacketSize >> 3);

        if(pPdd->ep[ep].fdirRx) //OUT Transfer
        {

            MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_RXMAXP, endPoint, wMaxPacketSize);
            MGC_Write8(g_pUSBRegs, MGC_O_HDRC_RXFIFOSZ,  dTxRxSZ);
            MGC_Write16(g_pUSBRegs,MGC_O_HDRC_RXFIFOADD, g_FIFOadd >> 3);
  
        }
        else
        {
            MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_TXMAXP, endPoint, wMaxPacketSize);
            MGC_Write8(g_pUSBRegs, MGC_O_HDRC_TXFIFOSZ,  dTxRxSZ);
            MGC_Write16(g_pUSBRegs,MGC_O_HDRC_TXFIFOADD, g_FIFOadd >> 3);
        }

        g_FIFOadd += wMaxPacketSize;

        switch(bTransferType)
        {
        case USB_ENDPOINT_TYPE_ISOCHRONOUS:

            // Set the ISO bit
            if(pPdd->ep[ep].fdirRx)          //OUT Transfer
            {
                WORD CSR;
                CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,ep);
                MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,ep,(CSR | MGC_M_RXCSR_P_ISO));
            }
            else                             //IN Transfer
            {
                WORD CSR;
                CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,ep);
                MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,ep,(CSR | MGC_M_TXCSR_ISO));
            }
            break;

        case USB_ENDPOINT_TYPE_BULK:
        case USB_ENDPOINT_TYPE_INTERRUPT:
        default:

            // Clear ISO bit - Set type to Bulk or INTERRUPT
            if(pPdd->ep[ep].fdirRx)         //OUT Transfer
            {
                WORD CSR;
                CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,ep);
                MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_RXCSR,ep,(CSR & (~MGC_M_RXCSR_P_ISO)));
            }
            else                            //IN Transfer
            {
                WORD CSR;
                CSR = MGC_ReadCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,ep);
                MGC_WriteCsr16(g_pUSBRegs,MGC_O_HDRC_TXCSR,ep,(CSR & (~MGC_M_TXCSR_ISO)));
            }
        }



        UfnPdd_ClearEndpointStall(pPdd, ep);
    }
    else
    {
        
    }
    return ERROR_SUCCESS;
}



//------------------------------------------------------------------------------
//
//  Function: Pdd_PhyPowerOnOff
//
//  Power on/off USB PHY and Controller
//
static void  Pdd_PhyPowerOnOff(BOOL bPwrOn)
{
    volatile BYTE *pBase = g_pUSBRegs;
    

    if(bPwrOn)
    {
        unsigned int u4Reg = 0;
        KITLOutputDebugString("AC83XX Pdd_PhyPowerOnOff\r\n");

        //USB PLL
        u4Reg  = *(volatile DWORD *)(0xF0000284);
        u4Reg &=   0xFFFFFFFE; 
        *(volatile DWORD *)(0xF0000284) =u4Reg;

        // USB PLL swtich On
        u4Reg  = *(volatile DWORD *)(0xF00000A0);
        u4Reg  |= (0x1 << 13);
        *(volatile DWORD *)(0xF00000A0) =u4Reg;
        
        //Reset USB
        u4Reg = MGC_PHY_Read32((pBase+0x0001800),0x68);
        u4Reg |=   0x00004000; 
        MGC_PHY_Write32((pBase+0x0001800),0x68,u4Reg);


        u4Reg = MGC_PHY_Read32((pBase+0x0001800),0x68);
        u4Reg &=  ~0x00004000; 
        MGC_PHY_Write32((pBase+0x0001800),0x68,u4Reg);


        u4Reg = MGC_PHY_Read32((pBase+0x0001800),0x6C);
        u4Reg &= ~0x3f3f;
        u4Reg |=  0x3e10;
        MGC_PHY_Write32((pBase+0x0001800), 0x6C, u4Reg); 
        UsbDelay(10);

        
        u4Reg = MGC_PHY_Read32((pBase+0x0001800),0x6C);
        u4Reg &= ~0x3f3f;
        u4Reg |=  0x3e2e;
        MGC_PHY_Write32((pBase+0x0001800), 0x6C, u4Reg); 

        

        // USB Eye-pattern
        u4Reg  = *(volatile DWORD *)(0xF000F810);
        u4Reg &= 0x00070000; 
        u4Reg |= 0x00050000; 
        *(volatile DWORD *)(0xF000F810) = u4Reg;
        
        //suspendom control
        u4Reg = MGC_PHY_Read32((pBase+0x0001800),0x68);
        u4Reg &=  ~0x00040000; 
        MGC_PHY_Write32((pBase+0x0001800),0x68,u4Reg);





        u4Reg  = MGC_Read8(pBase,0x74);
        u4Reg |=  0x03;
        MGC_Write8(pBase, 0x74, (BYTE)u4Reg);

        u4Reg &= ~0x03;
        MGC_Write8(pBase, 0x74, (BYTE)u4Reg);


        UsbDelay(10);

      u4Reg = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);
     u4Reg = u4Reg|0X1;
      MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, (BYTE)u4Reg);

    



    }

}

static void  Pdd_IST(VOID *pPddContext)
{
    UFN_PDD_CONTEXT *pPdd = (UFN_PDD_CONTEXT *)pPddContext;
    DWORD    ep;  //code
    UINT16   CommUsbIntSrc, EpTxIntSrc, EpRxIntSrc;


    while(1)
    {
        BOOL fInterrupt = FALSE;

    
        // Get interrupt source
        //Read this reg will clear the content, so need to save for using
        CommUsbIntSrc   = MGC_Read8(g_pUSBRegs,MGC_O_HDRC_INTRUSB);
        EpTxIntSrc  = MGC_Read16(g_pUSBRegs,MGC_O_HDRC_INTRTX);
        EpRxIntSrc  = MGC_Read16(g_pUSBRegs,MGC_O_HDRC_INTRRX);
        //   if(!(CommUsbIntSrc||EpTxIntSrc||EpRxIntSrc)) break;
        if (CommUsbIntSrc){
            MGC_Write8(g_pUSBRegs, MGC_O_HDRC_INTRUSB, CommUsbIntSrc);
            CommUsbIntSrc &= MGC_Read8(g_pUSBRegs, MGC_O_HDRC_INTRUSBE);
        }

        if (EpTxIntSrc){

            MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTX, EpTxIntSrc);
            EpTxIntSrc &= MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRTXE);
        }

        if (EpRxIntSrc){
            MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRX, EpRxIntSrc);
            EpRxIntSrc &= MGC_Read16(g_pUSBRegs, MGC_O_HDRC_INTRRXE);
        }
        if (CommUsbIntSrc == (MGC_M_INTR_CONNECT | MGC_M_INTR_DISCONNECT))
        {
            KITLOutputDebugString("Connect/Disconnet interrupt = 0x%H.\r\n", CommUsbIntSrc);
            // treat it as disconnect interrupt only.
            CommUsbIntSrc &= ~MGC_M_INTR_CONNECT;
        }


        //Interrupt came from device state
        if((CommUsbIntSrc & ~MGC_M_INTR_SOF) != 0)
        {
            CommUsbIntSrc &= ~MGC_M_INTR_SOF;
            
            // Handle device state change
            Pdd_USBEventHandle(pPdd, CommUsbIntSrc);
            fInterrupt = TRUE;            
        }


        // EP0 interrupt
        if ((EpTxIntSrc & 0x1) != 0)
        {   

        
            Pdd_Ep0Handle(pPdd);

            fInterrupt = TRUE; 
        }        
        // EPn TX interrupt
        if ((EpTxIntSrc & USBD_IRQ_MASKTX) != 0)
        {
            // Get EP number
            for(ep = 1; ep < ENDPOINT_COUNT; ep++)       //skip EP0
            {
                if(EpTxIntSrc  & (0x01 << ep))
                {

                    Pdd_EpnHandle(pPdd, ep);
                    fInterrupt = TRUE; 
                }
            }
            EpTxIntSrc = 0;
        }


        // EPn RX interrupt
        if ((EpRxIntSrc & USBD_IRQ_MASKRX) != 0)
        {
            // Get EP number
            for(ep = 1; ep < ENDPOINT_COUNT; ep++)       //skip EP0
            {
                if(EpRxIntSrc & (0x01 << ep))
                {
                UsbFnEp *pEP;


                    Pdd_EpnHandle(pPdd, ep);

                    pEP = &pPdd->ep[ep];
                    if(pEP->bTransferComplete)
                        fInterrupt = FALSE; 
                    else
                        fInterrupt = TRUE; 
                }
            }
            EpRxIntSrc = 0;
        }

        if( !fInterrupt )
        {
            break;
        }
    }

    return ;
}

static BOOL PDD_GetDescriptor(USB_DEVICE_REQUEST *pUdr)
{
    UCHAR *pucData;
    WORD wLength;
    WORD wType = pUdr->wValue;
    BOOL fRet = TRUE;

    switch (HIBYTE(wType)) {
        case USB_DEVICE_DESCRIPTOR_TYPE:
            if (BS_HIGH_SPEED == Pdd_GetSpeed(&g_PddContext))
            {
                    gs_pucUSBDescriptors = (UCHAR *)gs_pucUSBHSDescriptors;
                    KITLOutputDebugString("usb high speed\r\n" );
                   
            }
            else
            {
                    gs_pucUSBDescriptors = (UCHAR *)gs_pucUSBFSDescriptors;
                    KITLOutputDebugString("usb full speed\r\n" );
                    
            }                           
            pucData = (UCHAR *)gs_pucUSBDescriptors;
            wLength = gs_pucUSBDescriptors[0];
            
            KITLOutputDebugString("USB GET Device Descriptor\r\n" );
            break;

        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            pucData = (UCHAR *)&gs_pucUSBDescriptors[iCONF];
            wLength = CFGLEN;
            
            KITLOutputDebugString("USB GET CONFIGURATIONs Descriptor\r\n" );
            break;

        case USB_STRING_DESCRIPTOR_TYPE:
            switch (LOBYTE(wType)) {
                case 0x00:
                    pucData = (UCHAR *)gs_pucSupportedLanguage;
                    wLength = gs_pucSupportedLanguage[0];
                   
            KITLOutputDebugString("USB GET Language string Descriptor\r\n" );
                    break;

                case 0x01:
                    pucData = gs_Manufacturer;
                    wLength = 2*sizeof(MANUFACTURER) + 2;
            KITLOutputDebugString("USB GET Manufacturer string Descriptor wLength \r\n" );
                    break;

                case 0x02:
                    pucData =  gs_Product;
                   wLength = 2*sizeof(PRODUCT) + 2;
            KITLOutputDebugString("USB GET Product string Descriptor wLength\r\n");
                    break;

                case 0x03:
                wLength = 2*sizeof(SERIANUMBER) + 2;
                 pucData = gs_SerialNumber;
                    break;
            case 0x07:
                pucData = gs_Interface;
                wLength = 2*sizeof(INTRRFACE_STRING) + 2;
                KITLOutputDebugString("USB GET interface string Descriptor wLength \r\n");
                    break;

                default:
                    
                    fRet = FALSE;
                    break;
            }
            
            break;

        default:
          
            fRet = FALSE;
            break;
    }

    if (fRet) {
        
        g_EP0Transfer.pvBuffer = pucData;
        g_EP0Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
        g_EP0Transfer.cbBuffer = min(wLength, pUdr->wLength);
        g_EP0Transfer.cbTransferred = 0;
        g_EP0Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
        g_EP0Transfer.pvPddData = NULL;
        g_EP0Transfer.pvPddTransferInfo = NULL;

        Pdd_StartTrans(&g_PddContext,0,&g_EP0Transfer);
    }
    return fRet;
}
static BYTE g_status[2]= {0x01,0x00};

static BOOL HandleSetupPacket(USB_DEVICE_REQUEST *pUdr)
{
  

   

    switch(pUdr->bRequest) {
        case USB_REQUEST_GET_STATUS:

            
            KITLOutputDebugString("USB GET status\r\n" );
            g_EP0Transfer.pvBuffer = g_status;
            g_EP0Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
            g_EP0Transfer.cbBuffer = 2;//min(2, pUdr->wLength);
            g_EP0Transfer.cbTransferred = 0;
            g_EP0Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
            g_EP0Transfer.pvPddData = NULL;
            g_EP0Transfer.pvPddTransferInfo = NULL;
            
            Pdd_StartTrans(&g_PddContext,0,&g_EP0Transfer);
            break;

        case USB_REQUEST_CLEAR_FEATURE:
           
           

        case USB_REQUEST_SET_FEATURE:

            break;

        case USB_REQUEST_SET_ADDRESS:
            UfnPdd_SetAddress(&g_PddContext,(BYTE)pUdr->wValue);
            break;

        case USB_REQUEST_GET_DESCRIPTOR:
            PDD_GetDescriptor(pUdr);
            break;

        case USB_REQUEST_SET_DESCRIPTOR:
            
            break;

        case USB_REQUEST_GET_CONFIGURATION:
            
            break;

        case USB_REQUEST_SET_CONFIGURATION: 
            KITLOutputDebugString("USB set configuration status\r\n" );
            g_EP0Transfer.pvBuffer = g_status;
            g_EP0Transfer.dwFlags = USB_REQUEST_DEVICE_TO_HOST;
            g_EP0Transfer.cbBuffer = 0;//min(2, pUdr->wLength);
            g_EP0Transfer.cbTransferred = 0;
            g_EP0Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
            g_EP0Transfer.pvPddData = NULL;
            g_EP0Transfer.pvPddTransferInfo = NULL;
            
            Pdd_StartTrans(&g_PddContext,0,&g_EP0Transfer);
            break;

        case USB_REQUEST_GET_INTERFACE:
            
            break;

        case USB_REQUEST_SET_INTERFACE:
        
            break;

        case USB_REQUEST_SYNC_FRAME:
            break;

        default:
            break;
            
    }
    return TRUE;
}

void USBInit()
{
    UFN_PDD_CONTEXT     *pPdd;
    UINT    epx;
    BYTE                devctl;

    DWORD dwRet = ERROR_SUCCESS;


    pPdd = &g_PddContext;

    // Pdd SIG
    pPdd->dwSig = MTK_SIG;

    // Address
    pPdd->fSetAddress = FALSE;
    pPdd->HWAddress = 0;


    pPdd->ep[0].bTransferComplete = FALSE;
    pPdd->ep[1].bTransferComplete = FALSE;
    pPdd->ep[2].bTransferComplete = FALSE;
    pPdd->ep[3].bTransferComplete = FALSE;



    //Initial each ep[n] contant
    for(epx = 0; epx < ENDPOINT_COUNT; epx++) 
        memset(&pPdd->ep[epx], 0, sizeof(UsbFnEp));

    pPdd->ep[0].maxPacketSize = USB_EP_BULK_MAXP_FS;
    pPdd->ep[1].maxPacketSize = USB_EP_BULK_MAXP_FS;
    pPdd->ep[2].maxPacketSize = USB_EP_BULK_MAXP_FS;
    pPdd->ep[3].maxPacketSize = USB_EP_BULK_MAXP_FS;

    pPdd->ep[1].fdirRx =TRUE; 
    pPdd->ep[2].fdirRx =FALSE;    
    pPdd->ep[2].fdirRx =FALSE;

    pPdd->ep[0].dwEndpointNumber = 0;    
    pPdd->ep[1].dwEndpointNumber = 1;    
    pPdd->ep[2].dwEndpointNumber = 2;    
    pPdd->ep[3].dwEndpointNumber = 3;    

    OALTickInit();



    g_pUSBRegs = (volatile VOID *)KITL_CURRENT_USBPORT_BASE ;     
    Pdd_PhyPowerOnOff(TRUE);
    
    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_POWER, MGC_M_POWER_ISOUPDATE |MGC_M_POWER_SOFTCONN| MGC_M_POWER_HSENAB);  
    devctl = MGC_Read8(g_pUSBRegs, MGC_O_HDRC_DEVCTL);
    devctl &= ~MGC_M_DEVCTL_SESSION;

    if ((devctl & 0x18 ) == 0x18 )
        KITLOutputDebugString("AC83XX usb module is in device mode!!!\r\n");

    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_DEVCTL, devctl);


    // make it in disconnect status
    MGC_Write8(g_pUSBRegs, MGC_O_HDRC_POWER,MGC_Read8(g_pUSBRegs, MGC_O_HDRC_POWER) & (~MGC_M_POWER_SOFTCONN));   

    //disable all ep interrupt
    MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRTXE,0);
    MGC_Write16(g_pUSBRegs, MGC_O_HDRC_INTRRXE,0);

    UfnPdd_RegisterDevice(&g_PddContext);
    UfnPdd_Start(&g_PddContext);


    
    return;
}

BOOL  USBFNReadData(LPVOID lpBuffer, DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead)
{

   if(lpBuffer == NULL || nNumberOfBytesToRead <0 )
    {
      *lpNumberOfBytesRead=0;
      return FALSE;
    }
 
   // g_EP1Transfer.dwCallerPermissions;
   g_EP1Transfer.pvBuffer = lpBuffer;
   // g_EP1Transfer.dwBufferPhysicalAddress; // not used
   g_EP1Transfer.cbBuffer = nNumberOfBytesToRead;
   g_EP1Transfer.cbTransferred = 0;
   g_EP1Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
   g_EP1Transfer.pvPddData = NULL;
   g_EP1Transfer.pvPddTransferInfo = NULL;
   g_EP1Transfer.dwFlags = 0;
   Pdd_StartTrans(&g_PddContext,1, &g_EP1Transfer );


   do{
    
       Pdd_IST(&g_PddContext);

       
    }while(!g_PddContext.ep[1].bTransferComplete);

   *lpNumberOfBytesRead = g_EP1Transfer.cbTransferred;

   return TRUE;
     
}



BOOL  USBFNWriteData(LPVOID lpBuffer,  DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{

  
  if(lpBuffer == NULL || nNumberOfBytesToWrite <0 )
   {
     *lpNumberOfBytesWritten=0;
     return FALSE;
   }
  
 
   g_EP2Transfer.dwFlags = 0x80;
  g_EP2Transfer.pvBuffer = lpBuffer;
  // g_EP1Transfer.dwBufferPhysicalAddress; // not used
  g_EP2Transfer.cbBuffer = nNumberOfBytesToWrite;
  g_EP2Transfer.cbTransferred = 0;
  g_EP2Transfer.dwUsbError = UFN_NOT_COMPLETE_ERROR; // Possible values are in usbfntypes.h
  g_EP2Transfer.pvPddData = NULL;
  g_EP2Transfer.pvPddTransferInfo = NULL;

  Pdd_StartTrans(&g_PddContext,2, &g_EP2Transfer );
   
   do{
    
       Pdd_IST(&g_PddContext);

       
    }while(!g_PddContext.ep[2].bTransferComplete);
  
  *lpNumberOfBytesWritten = g_EP2Transfer.cbTransferred;
   return TRUE;

}

