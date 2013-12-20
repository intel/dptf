/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#ifndef _ESIFDATA_H
#define _ESIFDATA_H

#include "esif_lib.h"

#define ESIFSTATIC      0			// Use for buf_len to indicate static (non-dynamic) data
#define ESIFAUTOLEN     0x80000000	// Use for buf_len and data_len to auto-compute size using strlen()
#define ISNULL(a, b)     ((a) ? (a) : (b))


typedef enum esif_data_type EsifDataType;

// object management
void EsifData_ctor (EsifDataPtr self);	// constructor
void EsifData_dtor (EsifDataPtr self);	// destructor
EsifDataPtr EsifData_Create ();					// new operator [dynamic, caller-owned]
void EsifData_Destroy (EsifDataPtr self);	// delete operator
size_t EsifData_Sizeof ();					// sizeof() operator

// additional constructors
EsifDataPtr EsifData_CreateAs (EsifDataType type, void *buf_ptr, u32 buf_len, u32 data_len);// new operator [dynamic, caller-owned]

// array[] management
EsifDataPtr EsifData_CreateArray (int size);					// new[] operator [dynamic, caller-owned]
void EsifData_DestroyArray (EsifDataPtr self, int size);// delete[] operator
EsifDataPtr EsifData_GetArray (EsifDataPtr self, int item);		// [] operator

// methods
void EsifData_Set (EsifDataPtr self, EsifDataType type, void *buf_ptr, u32 buf_len, u32 data_len);
char*EsifData_AsString (EsifDataPtr self);	// Convert to string  [cast, non-dynamic, no conversion]
Byte*EsifData_AsPointer (EsifDataPtr self);	// Convert to pointer [cast, non-dynamic, no conversion]
Int32 EsifData_AsInt32 (EsifDataPtr self);		// Convert to Int32   [cast, non-dynamic, no conversion]
UInt32 EsifData_AsUInt32 (EsifDataPtr self);	// Convert to UInt32  [cast, non-dynamic, no conversion]

char*EsifData_ToString (EsifDataPtr self);	// Convert to string  [dynamic, caller-owned]
eEsifError EsifData_FromString (EsifDataPtr self, char *str, EsifDataType type);// Convert from null-terminated string [dynamic, self-owned]

#endif
