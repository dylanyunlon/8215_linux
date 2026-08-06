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

#ifndef __DRV_IF_H
#define __DRV_IF_H


//
// Interface definition
//

typedef enum
{
    IID_IRegisterComponent,   ///< Interface ID of IRegisterComponent defined in "drv_if_rc.h".
    IID_ICpsa,                           ///< Interface ID of ICpsa defined in "drv_if_cpsa.h".
    IID_IFilter,                          ///< Interface ID of IFilter defined in "drv_if_ftr.h".
    IID_IPBBUF,                         ///< The interface is in drv_if_pbbuf.h
    IID_ISyncCtrl,                      ///< The interface is in drv_if_syncctrl.h
    IID_ISyncCtrlUser,              ///< The interface is in drv_if_syncctrl.h
    IID_IVdp,                             ///< Interface defined in drv_if_vdp.h
    IID_IVDec,                           ///< Interface defined in drv_if_vdec.h
    IID_IADec,                           ///< Interface defined in drv_if_adec.h
    IID_IPMX,                         ///< Interface defined in drv_if_pmx.h
    IID_IEse,                         ///< Interface defined in drv_if_ese.h
    IID_IAudin,                         ///< The interface is in drv_if_audin.h
    IID_IIPodin,                         ///< The interface is in drv_if_audin.h
    IID_IEADev2Ifcon,                 ///< Interface defined in drv_if_eadev.h
    IID_IEADev2Aud                    ///< Interface defined in drv_if_eadev.h
} IM_IID;


#endif // __DRV_IM_H
