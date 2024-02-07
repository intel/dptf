/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
// 1) Uncomment out the below section for DEBUG_MEMORY_LEAKS to enable memory leak debugging
// 2) Add intentional memory leaks using the function createMemoryLeak() as needed to make sure your
//    environment is set up correctly to catch real leaks
// 3) Compile in debug mode
// 4) Start debugging in Visual Studio.  When the DPTF application is stopped (within ESIF)
// any memory leaks found will be written to the output window within Visual Studio.  Or just run
// DPTF normally (within ESIF) and capture output with SysInternal's DebugView utility.
//
// More information can be found here:  http://msdn.microsoft.com/en-us/library/e5ewb1h3(v=vs.80).aspx
//

// Uncomment below to enable memory leak detection
//#ifndef DEBUG_MEMORY_LEAKS
//	#define DEBUG_MEMORY_LEAKS
//#endif


	#define DEBUG_MEMORY_LEAK_INIT()
	#define REPORT_MEMORY_LEAK_INFO_TO_LOG()
	#include <string>
	#define CREATE_MEMORY_LEAK(name)
	#include <esif_ccb_library.h>
	inline void unloadLibrary(esif_lib_t lib)
	{
		esif_ccb_library_unload(lib);
	}
