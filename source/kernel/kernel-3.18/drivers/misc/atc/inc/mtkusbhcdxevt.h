#ifndef _MTKUSBHCDXEVT_H_
#define _MTKUSBHCDXEVT_H_

#include<windows.h>

/* don't change these words */
#define USBHCD_USBROOTMONITOR                    TEXT("SYSTEM/UsbHcdMonitor")
#define USB_PHYSICSPORT_PLUGIN                    TEXT("UsbPhysicsPortPlugIn")

#define USBHCD_PORT0_CONNECT                     ((DWORD)1 << 0)
#define USBHCD_PORT0_DISCONNECT                  ((DWORD)0 << 0)

#define USBHCD_PORT1_CONNECT                     ((DWORD)1 << 1)
#define USBHCD_PORT1_DISCONNECT                  ((DWORD)0 << 1)

#define USBHCD_DISCONNECT_SINCE_BIBBLE           ((DWORD)1 << 16)

#define USBHCD_PORT0_ROOT                        ((DWORD)1 << 31)
#define USBHCD_PORT1_ROOT                        ((DWORD)1 << 30)


#define USBHCD_BIBBLE(dwRootHub)                 ((USBHCD_DISCONNECT_SINCE_BIBBLE) << ((dwRootHub)&0x01))
#define USBHCDPORT0_CONNECTED(dwEvtData)         (((dwEvtData)&(~(USBHCD_PORT0_CONNECT))) | USBHCD_PORT0_ROOT | USBHCD_PORT0_CONNECT)
#define USBHCDPORT0_DISCONNECTED(dwEvtData)      (((dwEvtData)&(~(USBHCD_PORT0_CONNECT | USBHCD_BIBBLE(0) | USBHCD_PORT0_ROOT))) | USBHCD_PORT0_DISCONNECT)
#define USBHCDPORT0_DIS_SINCEBIBBLE(dwEvtData)   (((dwEvtData)&(~(USBHCD_PORT0_CONNECT | USBHCD_PORT0_ROOT))) | USBHCD_BIBBLE(0))

#define USBHCDPORT1_CONNECTED(dwEvtData)         (((dwEvtData)&(~(USBHCD_PORT1_CONNECT))) | USBHCD_PORT1_ROOT | USBHCD_PORT1_CONNECT)
#define USBHCDPORT1_DISCONNECTED(dwEvtData)      (((dwEvtData)&(~(USBHCD_PORT1_CONNECT | USBHCD_BIBBLE(1) | USBHCD_PORT1_ROOT))) | USBHCD_PORT1_DISCONNECT)
#define USBHCDPORT1_DIS_SINCEBIBBLE(dwEvtData)   (((dwEvtData)&(~(USBHCD_PORT1_CONNECT | USBHCD_PORT1_ROOT))) | USBHCD_BIBBLE(1))

#define IS_USBHCDPORT0_CONNECTED(dwEvtData)      (!!(((dwEvtData)&(USBHCD_PORT0_ROOT | USBHCD_PORT0_CONNECT))   == (USBHCD_PORT0_ROOT | USBHCD_PORT0_CONNECT)))
#define IS_USBHCDPORT0_DISCONNECTED(dwEvtData)   (!!(((dwEvtData)&( USBHCD_PORT0_CONNECT)) == ( USBHCD_PORT0_DISCONNECT)))
#define IS_USBHCDPORT0_HADBIBBLE(dwEvtData)      (!!(((dwEvtData)&(USBHCD_BIBBLE(0))) == (USBHCD_BIBBLE(0))))

#define IS_USBHCDPORT1_CONNECTED(dwEvtData)      (!!(((dwEvtData)&(USBHCD_PORT1_ROOT | USBHCD_PORT1_CONNECT))   == (USBHCD_PORT1_ROOT | USBHCD_PORT1_CONNECT)))
#define IS_USBHCDPORT1_DISCONNECTED(dwEvtData)   (!!(((dwEvtData)&( USBHCD_PORT1_CONNECT)) == ( USBHCD_PORT1_DISCONNECT)))
#define IS_USBHCDPORT1_HADBIBBLE(dwEvtData)      (!!(((dwEvtData)&(USBHCD_BIBBLE(1))) == (USBHCD_BIBBLE(1))))

#endif /* _MTKUSBHCDXEVT_H_ */

