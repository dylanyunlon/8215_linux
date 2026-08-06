//---------------------------------------------------------------------------
// Chip definition
//---------------------------------------------------------------------------

#include"ac83xx_usb_regs.h"

#define KITL_USB_PORT0_BASE                 0xF000E000
#define KITL_USB_PORT1_BASE                 0xF003C000
#ifndef KITL_CURRENT_USBPORT_BASE
#define KITL_CURRENT_USBPORT_BASE           KITL_USB_PORT0_BASE
#endif

#define USB_EP_COUNT        MUSB_C_NUM_EPS
#define CONFIG_USB_GADGET_MUSB_HDRC     1

/*
* USB directions
*
* This bit flag is used in endpoint descriptors' bEndpointAddress field.
* It's also one of three fields in control requests bRequestType.
*/
#define USB_DIR_OUT			0		/* to device */
#define USB_DIR_IN			0x80		/* to host */

/*
* USB types, the second of three bRequestType fields
*/
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
* USB recipients, the third of three bRequestType fields
*/
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03
/* From Wireless USB 1.0 */
#define USB_RECIP_PORT			0x04
#define USB_RECIP_RPIPE		0x05

/*
* Standard requests, for the bRequest field of a SETUP packet.
*
* These are qualified by the bRequestType field, so that for example
* TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
* by a GET_STATUS request.
*/
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

#define USB_REQ_SET_ENCRYPTION		0x0D	/* Wireless USB */
#define USB_REQ_GET_ENCRYPTION		0x0E
#define USB_REQ_RPIPE_ABORT		0x0E
#define USB_REQ_SET_HANDSHAKE		0x0F
#define USB_REQ_RPIPE_RESET		0x0F
#define USB_REQ_GET_HANDSHAKE		0x10
#define USB_REQ_SET_CONNECTION		0x11
#define USB_REQ_SET_SECURITY_DATA	0x12
#define USB_REQ_GET_SECURITY_DATA	0x13
#define USB_REQ_SET_WUSB_DATA		0x14
#define USB_REQ_LOOPBACK_DATA_WRITE	0x15
#define USB_REQ_LOOPBACK_DATA_READ	0x16
#define USB_REQ_SET_INTERFACE_DS	0x17

/* The Link Power Mangement (LPM) ECN defines USB_REQ_TEST_AND_SET command,
* used by hubs to put ports into a new L1 suspend state, except that it
* forgot to define its number ...
*/

/*
* USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
* are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
* are at most sixteen features of each type.)  Hubs may also support a
* new USB_REQ_TEST_AND_SET_FEATURE to put ports into L1 suspend.
*/
#define USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define USB_DEVICE_BATTERY		2	/* (wireless) */
#define USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE		3	/* (wireless)*/
#define USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

#define USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */

#define MUSB_TXCSR_P_WZC_BITS	\
	(MGC_M_TXCSR_P_INCOMPTX | MGC_M_TXCSR_P_SENTSTALL \
	| MGC_M_TXCSR_P_UNDERRUN | MGC_M_TXCSR_FIFONOTEMPTY)

/* RXCSR bits to avoid zeroing (write zero clears, write 1 ignored) */
#define MUSB_RXCSR_P_WZC_BITS	\
	(MGC_M_RXCSR_P_SENTSTALL | MGC_M_RXCSR_P_OVERRUN \
	| MGC_M_RXCSR_RXPKTRDY)


extern void KITLOutputDebugString (LPCSTR sz, ...);

//#include <ceddkex.h>
//------------------------------------------------------------------------------
//
//  Define:  USBD IRQ MASK
//
//  This is composite interrupt mask used in driver.
//
//#define USBD_IRQ_MASKTX     \
//(USBD_INTRTXE_EP1TXE | USBD_INTRTXE_EP2TXE | USBD_INTRTXE_EP3TXE )
//#define USBD_IRQ_MASKRX     \
//(USBD_INTRRXE_EP1RXE | USBD_INTRRXE_EP2RXE | USBD_INTRRXE_EP3RXE)

//#define USBD_COMM_IRQ_MASK   \
//    (USBD_INTRUSBE_SUSPEND_E | USBD_INTRUSBE_RESUME_E | \
//USBD_INTRUSBE_RESET_BAB_E |USBD_INTRUSBE_DISCON_E)

#define USBD_IRQ_MASKTX     0x07E
#define USBD_IRQ_MASKRX     0x07E
#define USBD_COMM_IRQ_MASK   \
	( MGC_M_INTR_SUSPEND | MGC_M_INTR_RESUME | MGC_M_INTR_BABBLE | \
	MGC_M_INTR_CONNECT | MGC_M_INTR_DISCONNECT /*| MGC_M_INTR_SESSREQ | \
MGC_M_INTR_VBUSERROR  | MGC_M_INTR_SOF  */)


//------------------------------------------------------------------------------
// Global Variables
//

#define USB_FIFO_START_ADDRESS  64
#define EP0_INDEX               0
#define EP_0_PACKET_SIZE        0x40        //64 byte
#define ISO_PACKET_SIZE         0x400       //1024 byte
#define EPX_PACKET_SIZE8        0x08
#define EPX_PACKET_SIZE16       0x10
#define EPX_PACKET_SIZE64       0x40
#define EPX_PACKET_SIZE512      0x200

#define USB_EP0_STAT_STALL      (MGC_M_CSR0_P_SENDSTALL  | MGC_M_CSR0_P_SENTSTALL)
#define USB_RX_STAT_STALL       (MGC_M_RXCSR_P_SENDSTALL | MGC_M_RXCSR_P_SENTSTALL)
#define USB_TX_STAT_STALL       (MGC_M_TXCSR_P_SENDSTALL | MGC_M_TXCSR_P_SENTSTALL)

#define ENDPOINT_COUNT        MUSB_C_NUM_EPS
#define MTK_SIG    'mtk' // MTK signature
#define USB_EP0_MAXP			64			/* control pipe, the same when HS or FS */
#define USB_EP_BULK_MAXP_HS	512  	/* maximum packet size for high-speed bulk endpoints */
#define USB_EP_BULK_MAXP_FS	64  	/* maximum packet size for full-speed bulk endpoints */
#define USB_EP_INTR_MAXP_HS	512  	/* maximum packet size for high-speed intr endpoints */
#define USB_EP_INTR_MAXP_FS	64  	/* maximum packet size for full-speed intr endpoints */

//------------------------------------------------------------------------------
//  EP Stall Bit.
//
#define USB_EP0_STALL_BIT      (MGC_M_CSR0_P_SENTSTALL | MGC_M_CSR0_P_SENDSTALL)
#define USB_RX_STALL_BIT       (MGC_M_RXCSR_P_SENTSTALL | MGC_M_RXCSR_P_SENDSTALL)
#define USB_TX_STALL_BIT       (MGC_M_TXCSR_P_SENTSTALL | MGC_M_TXCSR_P_SENDSTALL)



// USB_DEVICE_REQUEST.bmRequestType bits for control Pipes
#define     USB_REQUEST_DEVICE_TO_HOST      0x80
#define     USB_REQUEST_HOST_TO_DEVICE      0x00
#define     USB_REQUEST_STANDARD            0x00
#define     USB_REQUEST_CLASS               0x20
#define     USB_REQUEST_VENDOR              0x40
#define     USB_REQUEST_RESERVED            0x60
#define     USB_REQUEST_FOR_DEVICE          0x00
#define     USB_REQUEST_FOR_INTERFACE       0x01
#define     USB_REQUEST_FOR_ENDPOINT        0x02
#define     USB_REQUEST_FOR_OTHER           0x03

// These are the correct values based on the USB 1.0
// specification

#define USB_REQUEST_GET_STATUS                    0x00
#define USB_REQUEST_CLEAR_FEATURE                 0x01

#define USB_REQUEST_SET_FEATURE                   0x03

#define USB_REQUEST_SET_ADDRESS                   0x05
#define USB_REQUEST_GET_DESCRIPTOR                0x06
#define USB_REQUEST_SET_DESCRIPTOR                0x07
#define USB_REQUEST_GET_CONFIGURATION             0x08
#define USB_REQUEST_SET_CONFIGURATION             0x09
#define USB_REQUEST_GET_INTERFACE                 0x0A
#define USB_REQUEST_SET_INTERFACE                 0x0B
#define USB_REQUEST_SYNC_FRAME                    0x0C

//
// USB defined Feature selectors
//

#define USB_FEATURE_ENDPOINT_STALL          0x0000
#define USB_FEATURE_REMOTE_WAKEUP           0x0001
#define USB_FEATURE_POWER_D0                0x0002
#define USB_FEATURE_POWER_D1                0x0003
#define USB_FEATURE_POWER_D2                0x0004
#define USB_FEATURE_POWER_D3                0x0005


#define USB_ENDPOINT_DIRECTION_MASK               0x80

// test direction bit in the bEndpointAddress field of
// an endpoint descriptor.
#define USB_ENDPOINT_DIRECTION_OUT(addr)          (!((addr) & USB_ENDPOINT_DIRECTION_MASK))
#define USB_ENDPOINT_DIRECTION_IN(addr)           ((addr) & USB_ENDPOINT_DIRECTION_MASK)

typedef enum _UFN_BUS_SPEED {
    BS_UNKNOWN_SPEED = 0,
    BS_FULL_SPEED = (1 << 0),
    BS_HIGH_SPEED = (1 << 1),
} UFN_BUS_SPEED, *PUFN_BUS_SPEED;

#define USB_ENDPOINT_TYPE_MASK                    0x03

#define USB_ENDPOINT_TYPE_CONTROL                 0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS             0x01
#define USB_ENDPOINT_TYPE_BULK                    0x02
#define USB_ENDPOINT_TYPE_INTERRUPT               0x03
#define USB_ENDPOINT_MAX_PACKET_SIZE_MASK                       0x07FF

// UFN Transfer Errors
#define     UFN_NO_ERROR                        0x00000000
#define     UFN_DEVICE_NOT_RESPONDING_ERROR     0x00000005
#define     UFN_CANCELED_ERROR                  0x00000101
#define     UFN_NOT_COMPLETE_ERROR              0x00000103
#define     UFN_CLIENT_BUFFER_ERROR             0x00000104
typedef struct _USB_DEVICE_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    USHORT bcdUSB;
    UCHAR bDeviceClass;
    UCHAR bDeviceSubClass;
    UCHAR bDeviceProtocol;
    UCHAR bMaxPacketSize0;

    USHORT idVendor;
    USHORT idProduct;
    USHORT bcdDevice;
    UCHAR iManufacturer;
    UCHAR iProduct;
    UCHAR iSerialNumber;
    UCHAR bNumConfigurations;
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;


 struct _USB_ENDPOINT_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bEndpointAddress;
    UCHAR bmAttributes;
    USHORT wMaxPacketSize;
    UCHAR bInterval;
}__attribute__ ((packed));

typedef struct _USB_ENDPOINT_DESCRIPTOR  USB_ENDPOINT_DESCRIPTOR;
typedef struct _USB_ENDPOINT_DESCRIPTOR*  PUSB_ENDPOINT_DESCRIPTOR;

struct _USB_CONFIGURATION_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    USHORT wTotalLength;
    UCHAR bNumInterfaces;
    UCHAR bConfigurationValue;
    UCHAR iConfiguration;
    UCHAR bmAttributes;
    UCHAR MaxPower;
} __attribute__ ((packed));

typedef struct _USB_CONFIGURATION_DESCRIPTOR USB_CONFIGURATION_DESCRIPTOR;

typedef struct _USB_CONFIGURATION_DESCRIPTOR *PUSB_CONFIGURATION_DESCRIPTOR;


typedef struct _USB_INTERFACE_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bInterfaceNumber;
    UCHAR bAlternateSetting;
    UCHAR bNumEndpoints;
    UCHAR bInterfaceClass;
    UCHAR bInterfaceSubClass;
    UCHAR bInterfaceProtocol;
    UCHAR iInterface;
} USB_INTERFACE_DESCRIPTOR, *PUSB_INTERFACE_DESCRIPTOR;

typedef struct _UFN_ENDPOINT {
    DWORD                           dwCount;

    USB_ENDPOINT_DESCRIPTOR         Descriptor;
    PVOID                           pvExtended;
    DWORD                           cbExtended;
} UFN_ENDPOINT, *PUFN_ENDPOINT;
typedef UFN_ENDPOINT const * PCUFN_ENDPOINT;


typedef struct _UFN_INTERFACE {
    DWORD                           dwCount;

    USB_INTERFACE_DESCRIPTOR        Descriptor;
    PVOID                           pvExtended;
    DWORD                           cbExtended;
    PUFN_ENDPOINT                   pEndpoints;
} UFN_INTERFACE, *PUFN_INTERFACE;
typedef UFN_INTERFACE const * PCUFN_INTERFACE;


typedef struct _UFN_CONFIGURATION {
    DWORD                           dwCount;

    USB_CONFIGURATION_DESCRIPTOR    Descriptor;
    PVOID                           pvExtended;
    DWORD                           cbExtended;
    PUFN_INTERFACE                  pInterfaces;
} UFN_CONFIGURATION, *PUFN_CONFIGURATION;
typedef UFN_CONFIGURATION const * PCUFN_CONFIGURATION;






#define USB_DEVICE_DESCRIPTOR_TYPE                0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE         0x02
#define USB_STRING_DESCRIPTOR_TYPE                0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE             0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE              0x05
#define USB_POWER_DESCRIPTOR_TYPE                 0x08






typedef enum {
    EP0_STATE_IDLE = 0,
    EP0_STATE_IN_DATA_PHASE,
    EP0_STATE_OUT_DATA_PHASE
}EP0_STATE;

// Transfer structure passed to the PDD from the MDD in IssueTransfer.
typedef struct _STransfer {
    DWORD               dwFlags;
    PVOID               pvBuffer;
    DWORD               dwBufferPhysicalAddress;
    DWORD               cbBuffer;
    DWORD               cbTransferred;
    DWORD               dwUsbError; // Possible values are in usbfntypes.h

    PVOID               pvPddData; // PDD can do whatever it likes with this
    PVOID               pvPddTransferInfo; // Specific to PDD from client
} STransfer, *PSTransfer;

typedef struct _USB_DEVICE_REQUEST {
    UCHAR   bmRequestType;
    UCHAR   bRequest;
    USHORT  wValue;
    USHORT  wIndex;
    USHORT  wLength;
} USB_DEVICE_REQUEST, *PUSB_DEVICE_REQUEST;





typedef struct {
    DWORD        dwEndpointNumber;
    UINT16       maxPacketSize;
    BOOL         fdirRx;
	BOOL         bTransferComplete;
    STransfer    *pTransfer;
} UsbFnEp;

typedef struct {
    DWORD                   dwSig;
    UsbFnEp                 ep[ENDPOINT_COUNT];
    EP0_STATE               Ep0State;
    USB_DEVICE_REQUEST      udr;
    BOOL                    fSendDataEnd;   
    BOOL                    fSetAddress;   
    UCHAR                   HWAddress;      
}UFN_PDD_CONTEXT;




#define USE_1BYTE_TERMINATING_PACKETS

//------------------------------------------------------------------------------
#define USB_EP0_MAXP			64			/* control pipe, the same when HS or FS */
#define USB_EP_BULK_MAXP_HS	512  	/* maximum packet size for high-speed bulk endpoints */
#define USB_EP_BULK_MAXP_FS	64  	/* maximum packet size for full-speed bulk endpoints */

enum TRANSFER_STATE {
    TS_IDLE=0,
    TS_RECEIVING_MESSAGE,
    TS_SENDING_MESSAGE,
    TS_RECEIVING_PACKET,
    TS_SENDING_PACKET,
    TS_SENT_PACKET_END
};

enum DEVICE_STATE {
    DS_DETACHED = 0,
    DS_ATTACHED,
    DS_POWERED,
    DS_DEFAULT,
    DS_ADDRESSED,
    DS_CONFIGURED,
    DS_SUSPENDED,
};

typedef struct {
    UCHAR ucbLength;
    UCHAR udbDescriptorType;
    TCHAR ptcbString[12 + 1];
} USB_SERIAL_NUMBER;

typedef struct _USB_STRING{
    UCHAR   ucbLength;
    UCHAR   ucbDescriptorType;
    TCHAR    ptcbString[41];
}__attribute__ ((packed));

typedef struct _USB_STRING  USB_STRING;

// Check dwFlags
#define TRANSFER_IS_IN(pTransfer)   (pTransfer->dwFlags & USB_REQUEST_DEVICE_TO_HOST)
#define TRANSFER_IS_OUT(pTransfer)  (!TRANSFER_IS_IN(pTransfer))

typedef struct {
    DWORD Notification;
    DWORD dwReserved;
} INTERRUPT_DATA, *PINTERRUPT_DATA;

#define MAX_INCOMING_BUFFER         8192
#define EP0_MAX_RECEIVE_BUFFER      1024

typedef enum {
    EP0Setup = 0,
    EP0Out,
    EP0In
} EP0_DIR;

typedef struct _EP0_REQUEST EP0_REQUEST, *PEP0_REQUEST;
struct _EP0_REQUEST
{
    EP0_DIR eDir;
    UCHAR *pucData;
    DWORD dwExpectedSize;
    DWORD dwActualSize;
    VOID (*pfnNotification)(EP0_REQUEST *pRequest, PVOID pvUser);
    PVOID pvUser;

    DWORD dwProcessed;
    BOOL fCompleted;
};

typedef BOOL (*PFN_GETDESCRIPTOR)(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest);
typedef BOOL (*PFN_DOVENDORCOMMAND)(USB_DEVICE_REQUEST *pUdr, EP0_REQUEST *pRequest);







