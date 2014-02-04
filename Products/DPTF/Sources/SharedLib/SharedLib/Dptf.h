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

#ifdef __KLOCWORK__
#define final
#define override
#endif

#include "DptfExport.h"
#include "BasicTypes.h"
#include "DptfExceptions.h"
#include "Frequency.h"
#include "Guid.h"
#include "Percentage.h"
#include "Power.h"
#include "Temperature.h"
#include "Constants.h"
#include <bitset>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>
#include <string>
#include <sstream>

#define DELETE_MEMORY_TC(cd) try{if (cd != nullptr) {delete cd; cd = nullptr;}} catch (...) {cd = nullptr;}
#define DELETE_MEMORY(cd) if (cd != nullptr) {delete cd; cd = nullptr;}

//
// FIXME:  these pragma's are temporary while stubbing the header files and source files.
//         and should be removed once we fill in the code.
//
#ifdef ESIF_ATTR_OS_WINDOWS
#pragma warning(disable:4100)                                       // unreferenced formal parameter
#pragma warning(disable:4702)                                       // unreachable code
#pragma warning(disable:4251)                                       // needs to have dll-interface to be used by clients of class
#endif