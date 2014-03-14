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

#ifndef _DATAVAULT_H
#define _DATAVAULT_H

#include "esif_uf.h"
#include "esif_lib_datacache.h"
#include "esif_lib_iostream.h"

/* Sorted List - TODO: Move to own header */
// ESIF Database Data Row to store in DataCache and File

struct DataVault_s;
typedef struct DataVault_s DataVault, *DataVaultPtr, **DataVaultPtrLocation;

// #ifdef _DATAVAULT_CLASS
struct DataVault_s {
	esif_ccb_lock_t lock;
	char			name[ESIF_NAME_LEN];
	esif_flags_t	flags;
	DataCachePtr	cache;
	IOStreamPtr		stream;
};

// #endif

// object management
void DataVault_ctor (DataVaultPtr self);
void DataVault_dtor (DataVaultPtr self);
DataVaultPtr DataVault_Create ();
void DataVault_Destroy (DataVaultPtr self);

eEsifError DataVault_ReadVault (DataVaultPtr self);
eEsifError DataVault_WriteVault (DataVaultPtr self);

#endif	// _ESIF_UF_CONFIG_