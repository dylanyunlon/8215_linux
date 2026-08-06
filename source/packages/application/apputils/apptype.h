/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef APPTYPE
#define APPTYPE

#include <stddef.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) {if (a) {delete a; a = NULL;}}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) {if (a) {delete [] a; a = NULL;}}
#endif


#define MAIN_APPLICATION_SOCKET_ADDR "/tmp/mainapplicationSocketAddr"

class LaunchPacket
{
public:
    int size;
    int cmd;
    char arg[256];
};

#endif // APPTYPE
