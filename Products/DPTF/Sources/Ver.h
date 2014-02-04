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

#ifndef _VER_H_
#define _VER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*** Global Declarations
**************************/

// Identifies if this is a Developer Build.
#define DEV_BUILD        1

/* Version and definitions */
#define VER_MAJOR        8
#define VER_MAJOR_STR   "8"
#define VER_MINOR        0
#define VER_MINOR_STR   "0"
#define VER_HOTFIX       0
#define VER_HOTFIX_STR  "0"
#define VER_BUILD       9999
#define VER_BUILD_STR   "9999"

//#define VER_BUILD_TIMESTAMP "20130101 00:00:00"

#define VER_SEPARATOR_STR  "."

/* Special Build String */
#if (DEV_BUILD)
#define VER_SPECIAL_BUILD "(Dev Build) "
#else
#define VER_SPECIAL_BUILD
#endif

/* Flags set based on build type */
#ifdef DBG
#define VER_DEBUG_TAG   " (DBG)"
#else
#define VER_DEBUG_TAG   
#endif

/* Combined file version information  */
#define VERSION_STR VER_SPECIAL_BUILD VER_MAJOR_STR  VER_SEPARATOR_STR VER_MINOR_STR VER_SEPARATOR_STR VER_HOTFIX_STR VER_SEPARATOR_STR VER_BUILD_STR VER_DEBUG_TAG

#define VERSION_STRA VER_SPECIAL_BUILD, VER_MAJOR_STR, VER_SEPARATOR_STR, VER_MINOR_STR, VER_SEPARATOR_STR, VER_HOTFIX_STR, VER_SEPARATOR_STR, VER_BUILD_STR, VER_DEBUG_TAG

/* Used in some areas of RC files */
#define VERSION_RC VERSION_STR "\0"

/* Defines a common Copyright String used in the tools and OROM headers */
#define COPYRIGHT_STR  "Copyright(C) 2003-2013 Intel Corporation.  All Rights Reserved."

/* Defines a common Copyright String used in the Windows UI */
#define COPYRIGHT_UI_STR  "Copyright(C) 2003-2013 Intel Corporation \0"

/* Defines a common Copyright String used in the Windows Driver */
#define COPYRIGHT_DRV_STR "Copyright(C) 2003-2013 Intel Corporation \0"

// NOTE:  VER_VERSION is used for displaying version in RC file
#define VER_VERSION  VER_MAJOR, VER_MINOR, VER_HOTFIX, VER_BUILD

//Version Header Field Definitions
//Structure version definitions
#define VTR_MAJOR_VER  2
#define VTR_MINOR_VER  1

#ifdef __cplusplus
}
#endif

/*** Global Functions
 **************************/

#endif // _VER_H_

