/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

struct DataBank_s;
typedef struct DataBank_s DataBank, *DataBankPtr, **DataBankPtrLocation;

#ifdef _DATABANK_CLASS
struct DataBank_s {
	esif_ccb_lock_t		lock;		// Read/Write Lock
	UInt32				size;		// Number of DataVaults in elements array
	DataVaultPtr		*elements;	// Array of DataVaults, sorted by name	
	char				defaultDV[ESIF_NAME_LEN];	// Default Namespace
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

// singleton methods
DataBankPtr DataBank_GetMgr(void);
DataVaultPtr DataBank_GetDataVault(StringPtr nameSpace);
DataVaultPtr DataBank_OpenDataVault(StringPtr nameSpace);
esif_error_t DataBank_ImportDataVault(StringPtr nameSpace);
void DataBank_CloseDataVault(StringPtr nameSpace);
Bool DataBank_KeyExists(StringPtr nameSpace, StringPtr keyName);
esif_data_type_t DataBank_KeyType(StringPtr nameSpace, StringPtr keyName);
void DataBank_SetDefault(const StringPtr nameSpace);
const StringPtr DataBank_GetDefault();

// Legacy Public API Functions
esif_error_t EsifCfgMgrInit(void);
void EsifCfgMgrExit(void);

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_UF_CFGMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

