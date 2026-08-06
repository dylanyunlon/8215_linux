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

#ifndef __DRV_IF_RC_H
#define __DRV_IF_RC_H


#include "drv_if.h"


/// This interface represents IID_IRegisterComponent interface.
/// This interface is used for register component.
typedef struct _IRegisterComponent
{
    /// Register component.
    /// \return This function returns negative value if failed.
    INT32 (*pi4RegisterComponent)(
        void *pvTag,    ///< [in] The object handle.
        UINT16 eType,   ///< [in] The component type.
        UINT16 u2Id     ///< [in] The component ID.
        );

    /// Unregister component.
    /// \return This function returns negative value if failed.
    INT32 (*pi4UnregisterComponent)(
        void *pvTag,    ///< [in] The object handle.
        UINT16 eType,   ///< [in] The component type.
        UINT16 u2Id     ///< [in] The component ID.
        );
} IRegisterComponent;


#endif // __DRV_IM_RC_H
