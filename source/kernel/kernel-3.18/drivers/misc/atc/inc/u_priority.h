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


#ifndef U_PRIORITY_H
#define U_PRIORITY_H

/*
* ===========
*  GUIDELINE
* ===========
*
* 1. This header file is used by each layer to define thread priority.
*
* 2. A thread priority is defined by three parts, namely [class], [layer], and [offset].
*
* 3. You can add a new class or layer or modify the value if necessary.
*
* 4. Do not assume what the value is, because it may be changed when porting to other OS.
*
* 5. [offset] should be in the range of [-5, 5]. The larger [offset] value, the lower priority.
*
* 5. Define your thread priority like this:
*
*    #define XXX_THREAD_PRIORITY PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_DRIVER, 0)
*
* 6. You can use an assertion macro to check the priority relationship between two threads.
*
*/


//
// priority class definition
//

#define PRIORITY_CLASS_REALTIME         50    ///< for hard real-time
#define PRIORITY_CLASS_HIGH             100   ///< for streaming
#define PRIORITY_CLASS_NORMAL           150   ///< for normal application
#define PRIORITY_CLASS_IDLE             200   ///< for background

//
// priority layer definition
//

#define PRIORITY_LAYER_TIME_CRITICAL    0
#define PRIORITY_LAYER_DRIVER           10
#define PRIORITY_LAYER_MIDDLEWARE       20
#define PRIORITY_LAYER_UI               30

//
// priority macro definition
//

#define PRIORITY(CLASS, LAYER, OFFSET)  ((CLASS) + (LAYER) + (OFFSET))


#endif // U_PRIORITY_H
