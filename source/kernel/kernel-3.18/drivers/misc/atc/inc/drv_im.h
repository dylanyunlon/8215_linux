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

#ifndef __DRV_IM_H
#define __DRV_IM_H


#include "drv_if.h"

//
// IM typedef and functions
//

#define S_IM_OK             (0)
#define E_IM_OUT_OF_MEMORY  (-1)
#define E_IM_ALREADY_EXIST  (-2)
#define E_IM_NOT_FOUND      (-3)
#define E_IM_OS_FAIL        (-4)

/// The function pointer to get a new handle.
/// \return This function returns the object handle.
typedef void *(*PNEW_HANDLE_CALLBACK)(
    UINT16 eType,                   ///< [in] The component type.
    UINT16 u2Id,                    ///< [in] The component ID.
    IM_IID eIID                     ///< [in] The interface ID.
    );

/// The function pointer to destory a handle.
typedef void (*PDELETE_HANDLE_CALLBACK)(
    UINT16 eType,                   ///< [in] The component type.
    UINT16 u2Id,                    ///< [in] The component ID.
    IM_IID eIID,                    ///< [in] The interface ID.
    void *pvTag                     ///< [in] The object handle.
    );


/// Register interface to IM
/// \return This function returns negative value if failed.
INT32 i4ImRegisterInterface(
    UINT16 eType,                   ///< [in] The component type.
    IM_IID eIID,                    ///< [in] The interface ID.
    void *pvInterface,              ///< [in] The interface object.
    PNEW_HANDLE_CALLBACK pNew,      ///< [in] The callback function to get a new handle.
    PDELETE_HANDLE_CALLBACK pDelete ///< [in] The callback function to destory a handle.
    );

/// Register interface to IM
/// \return This function returns negative value if failed.
INT32 i4ImRegisterInterface2(
    UINT16 eType,                   ///< [in] The component type.
    UINT16 u2Id,                    ///< [in] The component ID.
    IM_IID eIID,                    ///< [in] The interface ID.
    void *pvInterface,              ///< [in] The interface object.
    void *pvTag                     ///< [in] The object handle.
    );

/// Unregister interface to IM
/// \return This function returns negative value if failed.
INT32 i4ImUnregisterInterface(
    UINT16 eType,                   ///< [in] The component type.
    IM_IID eIID                     ///< [in] The interface ID.
    );

/// Unregister interface to IM
/// \return This function returns negative value if failed.
INT32 i4ImUnregisterInterface2(
    UINT16 eType,                   ///< [in] The component type.
    UINT16 u2Id,                    ///< [in] The component ID.
    IM_IID eIID                     ///< [in] The interface ID.
    );

/// Query interface
/// \return This function returns negative value if failed.
INT32 i4ImQueryInterface(
    UINT16 eType,                   ///< [in] The component type.
    UINT16 u2Id,                    ///< [in] The component ID.
    IM_IID eIID,                    ///< [in] The interface ID.
    void **ppvInterface,            ///< [out] The interface object.
    void **ppvTag                   ///< [out] The object handle.
    );

/// Release interface
/// \return This function returns negative value if failed.
INT32 i4ImReleaseInterface(
    UINT16 eType,                   ///< [in] The component type.
    UINT16 u2Id,                    ///< [in] The component ID.
    IM_IID eIID,                    ///< [in] The interface ID.
    void *pvTag                     ///< [in] The object handle.
    );


#endif // __DRV_IM_H
