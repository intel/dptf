/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

// Limits
#define MAX_DV_DATALEN  0x7fffffff

// ESIFDV File Definitions
#define ESIFDV_FILEEXT              ".dv"				// DataVault File Extension
#define ESIFDV_BAKFILEEXT           ".dvk"				// DataVault File Extension for Backup
#define ESIFDV_LOGFILEEXT           ".lg"				// DataVault Log File Extension
#define ESIFDV_SIGNATURE            "\xE5\x1F"			// "ESIF" Signature = 0xE51F

#define ESIFDV_MAJOR_VERSION_MIN    1
#define ESIFDV_MAJOR_VERSION        1
#define ESIFDV_MINOR_VERSION        0
#define ESIFDV_REVISION             0
#define ESIFDV_MAX_REVISION         0xFFFF

#define ESIFDV_VERSION(major, minor, revision)  ((UInt32)((((major) & 0xFF) << 24) | (((minor) & 0xFF) << 16) | ((revision) & 0xFFFF)))

#define ESIFDV_DATA_REVISION_DEF	0
#define ESIFDV_DATA_DESC_LEN		32

#pragma pack(push, 1)

typedef struct DataVault_s {
	esif_ccb_lock_t			lock;
	char					name[ESIF_NAME_LEN];
	esif_flags_t			flags;
	UInt32					version;		// Data Vault format version
	char					description[ESIFDV_DATA_DESC_LEN];
	IOStreamPtr				stream;
	DataCachePtr			cache;
} DataVault, *DataVaultPtr;


// ESIFDV File Header v1.0.0
typedef struct DataVaultHeader_s {
	UInt8   signature[2];	// File Signature
	UInt16  headersize;		// Header Size, including signature & headersize
	UInt32  version;		// File Version
	UInt32  flags;			// Global Flags
} DataVaultHeader, *DataVaultHeaderPtr;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

// object management
DataVaultPtr DataVault_Create(char *name);
void DataVault_Destroy(DataVaultPtr self);

eEsifError DataVault_ReadVault(DataVaultPtr self);
eEsifError DataVault_WriteVault(DataVaultPtr self);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_CONFIG_