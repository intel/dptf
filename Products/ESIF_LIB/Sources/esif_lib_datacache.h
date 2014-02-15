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
#ifndef _DATACACHE_H
#define _DATACACHE_H

#include "esif.h"

#include "esif_lib.h"
#include "esif_lib_esifdata.h"

///////////////////////////////////////////////////////
// DataCacheEntry Class

struct DataCacheEntry_s;
typedef struct DataCacheEntry_s DataCacheEntry, *DataCacheEntryPtr, **DataCacheEntryPtrLocation;

#ifdef _DATACACHE_CLASS
struct DataCacheEntry_s {
	esif_flags_t  flags;		// Flags for this Row (i.e., Persist, Encrypted, etc)
	EsifData      key;			// Key for this Row
	EsifData      value;		// Value for this Row
};

#endif	// _DATACACHE_CLASS

///////////////////////////////////////////////////////
// DataCache Class

struct DataCache_s;
typedef struct DataCache_s DataCache, *DataCachePtr, **DataCachePtrLocation;

#ifdef _DATACACHE_CLASS
struct DataCache_s {
	UInt32  size;
	DataCacheEntryPtr elements;
};

#endif	// _DATACACHE_CLASS

// object management
DataCachePtr DataCache_Create ();
void DataCache_Destroy (DataCachePtr self);

// members
DataCacheEntryPtr DataCache_GetValue (DataCachePtr self, esif_string key);
void DataCache_SetValue (DataCachePtr self, esif_string key, EsifData value, esif_flags_t flags);
void DataCache_Delete (DataCachePtr self, esif_string key);
UInt32 DataCache_GetCount (DataCachePtr self);
#endif