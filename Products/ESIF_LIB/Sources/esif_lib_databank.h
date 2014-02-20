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

#ifndef _DATABANK_H
#define _DATABANK_H

#include "esif_uf.h"
#include "esif_lib_datavault.h"

///////////////////////////////////////////////////
// DataBank Class

#define ESIF_MAX_NAME_SPACES        10

// ESIFDV File Definitions
#define ESIFDV_FILEEXT              ".dv"				// DataVault File Extension
#define ESIFDV_BAKFILEEXT           ".dvk"				// DataVault File Extension for Backup
#define ESIFDV_LOGFILEEXT           ".lg"				// DataVault Log File Extension
#define ESIFDV_SIGNATURE            "\xE5\x1F"			// "ESIF" Signature = 0xE51F
#define ESIFDV_MAJOR_VERSION        1
#define ESIFDV_MINOR_VERSION        0
#define ESIFDV_REVISION             0
#define ESIFDV_MAX_REVISION         0xFFFF

struct DataBank_s;
typedef struct DataBank_s DataBank, *DataBankPtr, **DataBankPtrLocation;

#ifdef _DATABANK_CLASS
struct DataBank_s {
	UInt32     size;
	DataVault  elements[ESIF_MAX_NAME_SPACES];
	esif_ccb_lock_t lock;
};

#endif

extern DataBankPtr g_DataBankMgr;		// Global Instance, Dynamically Allocated
extern char g_DataVaultDir[MAX_PATH];	// Global Folder which contains all DataVaults
extern char g_DataVaultDefault[ESIF_NAME_LEN]; // Global Default DataVault Namespace
extern char *g_DataVaultStartScript;    // Optional Startup Script, if none specified in Default DataVault or cmd/start script

// object management
DataBankPtr DataBank_Create ();
void DataBank_Destroy (DataBankPtr self);

// methods
DataVaultPtr DataBank_GetNameSpace (DataBankPtr self, StringPtr nameSpace);
DataVaultPtr DataBank_OpenNameSpace (DataBankPtr self, esif_string nameSpace);
void DataBank_CloseNameSpace (DataBankPtr self, esif_string nameSpace);
int DataBank_KeyExists (DataBankPtr self, StringPtr nameSpace, StringPtr keyName);
eEsifError DataBank_LoadDataVaults (DataBankPtr self);

// Backwards compatibility
eEsifError EsifCfgMgrInit (void);
void EsifCfgMgrExit (void);

#endif /* _ESIF_UF_CFGMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

