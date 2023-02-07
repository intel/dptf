/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_LIB_H
#define _ESIF_LIB_H

#include "esif_ccb.h"
#include "esif_ccb_memory.h"
#include "esif_sdk_data.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>

// Aliases
typedef unsigned char Byte, *BytePtr, **BytePtrLocation;

#define STATIC_CAST(type, value)    ((type)(value))

#ifdef ESIF_ATTR_DEBUG
# define    SAFETY(stmt)        stmt
# define    WIPEPTR(p)          esif_ccb_memset(p, 0, sizeof(*p))
# define    WIPEARRAY(a, n)     esif_ccb_memset(a, 0, sizeof(*a) * n)
#else
# define    SAFETY(stmt)
# define    WIPEPTR(p)          esif_ccb_memset(p, 0, sizeof(*p))
# define    WIPEARRAY(a, n)     esif_ccb_memset(a, 0, sizeof(*a) * n)
#endif

// Access Array Items without class definition (by using ClassName_Sizeof() function
#define ARRAYITEM(class, array, idx)    (class##Ptr)((unsigned char*)(array) + ((class##_Sizeof()) * (idx)))

// Bit Flags Macros
#define FLAGS_SET(flags, mask)			((flags) |= (mask))				// Set flags for all bits in mask
#define FLAGS_CLEAR(flags, mask)		((flags) &= ~(mask))			// Clear flags for all bits in mask
#define FLAGS_TEST(flags, mask)			((flags) & (mask))				// Test flags for any bits in mask 
#define FLAGS_TESTALL(flags, mask)		(((flags) & (mask)) == (mask))	// Test flags for all bits in mask
#define FLAGS_TESTVALID(flags, mask)	(((flags) & (mask)) == (flags))	// Test flags contains only valid bits in mask

// Limits
#define MAXAUTOLEN      65536		// Max length of strings when creating with ESIFAUTOLEN

#define IGNORE_RESULT(expr)		do { if (expr) {} } while ESIF_CONSTEXPR(ESIF_ALWAYSFALSE)

#endif