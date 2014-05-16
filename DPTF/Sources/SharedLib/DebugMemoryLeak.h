/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#pragma once

//
// To enable memory leak debugging for Windows:
//
// 1) Include DebugMemoryLeak.h at the top of Dptf.h in the shared lib.
//
// 2) Add this code to the DptfManger constructor:
//#ifdef _DEBUG
//#ifdef ESIF_ATTR_OS_WINDOWS
//    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
//#endif
//#endif
//
// 3) In rapidxml.hpp (Misc\ThirdParty\RapidXML), put the following at the beginning of the file after the include guard:
//    #undef new
//
// 4) Compile in debug mode.  This is debug only.
//
// Then start debugging in Visual Studio.  When the DPTF application is stopped (within ESIF)
// any memory leaks found will be written to the output windows within Visual Studio.
//
// More information can be found here:  http://msdn.microsoft.com/en-us/library/e5ewb1h3(v=vs.80).aspx
//

#ifdef _DEBUG
#ifdef ESIF_ATTR_OS_WINDOWS

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

#endif
#endif