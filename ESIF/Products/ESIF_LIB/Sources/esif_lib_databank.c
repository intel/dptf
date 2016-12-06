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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_DATAVAULT

#include "esif_uf.h"		/* Upper Framework */

#define _DATABANK_CLASS
#define _DATACACHE_CLASS
#define _DATAVAULT_CLASS
#define _IOSTREAM_CLASS

#include "esif_lib_datacache.h"
#include "esif_lib_iostream.h"
#include "esif_lib_esifdata.h"
#include "esif_lib_databank.h"

// define this (and provide a valid dsp_dv.h) to link "_dsp.dv" as a static datavault)
// #define STATIC_DV_DSP

// Static DataVaults
#ifdef STATIC_DV_DSP
# include "dsp_dv.h"		// DSP Configuration Data Vault
# include "esif_dsp.h"		// DSP Namespace Declaration
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

static struct {
	StringPtr  name;
	BytePtr    buffer;
	size_t     buf_len;
}

g_StaticDataVaults[] = {
#ifdef STATIC_DV_DSP
	{ESIF_DSP_NAMESPACE, dsp_dv, sizeof(dsp_dv)},
#endif
	{0, 0, 0}
};

// Global DataBank (NameSpace) Manager
DataBankPtr g_DataBankMgr = 0;

// Global Default DataVault Namespace
char g_DataVaultDefault[ESIF_NAME_LEN] = "dptf";

// Optional Startup Script, if none specified in Default DataVault or cmd/start script
char *g_DataVaultStartScript = NULL;

static DataBankPtr DataBank_Create ();
static void DataBank_Destroy (DataBankPtr self);

static DataVaultPtr DataBank_GetNameSpace_Locked (
	DataBankPtr self,
	StringPtr nameSpace,
	UInt32 *indexPtr
	);

static eEsifError DataBank_LoadStaticDataVaults(DataBankPtr self);
static eEsifError DataBank_LoadFileDataVaults(DataBankPtr self);

///////////////////////////////////////////////////////////////////////////////////////////
// DataBank class
///////////////////////////////////////////////////////////////////////////////////////////

static DataBankPtr DataBank_Create()
{
	DataBankPtr self = (DataBankPtr)esif_ccb_malloc(sizeof(*self));
	if (self) {
		esif_ccb_lock_init(&self->lock);
	}
	return self;
}


static void DataBank_Destroy(DataBankPtr self)
{
	UInt32 index;
	if (self) {
		for (index = 0; index < self->size; index++) {
			DataVault_Destroy(self->elements[index]);
			self->elements[index] = NULL;
		}
		self->size = 0;
		esif_ccb_lock_uninit(&self->lock);
	}
	esif_ccb_free(self);
}


// Return NameSpace in the Configuration Manager or NULL if it doesn't exist
DataVaultPtr DataBank_GetNameSpace(
	DataBankPtr self,
	StringPtr nameSpace
	)
{
	DataVaultPtr vaultPtr = NULL;

	if (NULL == nameSpace) {
		nameSpace = g_DataVaultDefault;
	}
	esif_ccb_read_lock(&self->lock);
	vaultPtr = DataBank_GetNameSpace_Locked(self, nameSpace, NULL);
	esif_ccb_read_unlock(&self->lock);
	return vaultPtr;
}


// Return NameSpace in the Configuration Manager or NULL if it doesn't exist
static DataVaultPtr DataBank_GetNameSpace_Locked(
	DataBankPtr self,
	StringPtr nameSpace,
	UInt32 *indexPtr
	)
{
	DataVaultPtr DB = NULL;
	UInt32 index;

	ESIF_ASSERT(nameSpace != NULL);

	for (index = 0; index < self->size; index++) {
		if (NULL == self->elements[index]) {
			continue;
		}
		if (esif_ccb_stricmp(nameSpace, self->elements[index]->name) == 0) {
			DB = self->elements[index];
			break;
		}
	}
	if (indexPtr != NULL) {
		*indexPtr = index;
	}

	return DB;
}


// Gets a current namespace or creates a new one if not found
DataVaultPtr DataBank_OpenNameSpace(
	DataBankPtr self,
	esif_string nameSpace
	)
{
	DataVaultPtr DB = NULL;

	// Exit if NameSpace already exists
	esif_ccb_write_lock(&self->lock);
	DB = DataBank_GetNameSpace_Locked(self, nameSpace, NULL);
	if (DB != NULL) {
		goto exit;
	}

	if (self->size >= (sizeof(self->elements) / sizeof(*self->elements))) {
		DB = NULL;
		goto exit;
	}

	// Not Found. Create NameSpace
	DB = DataVault_Create(nameSpace);
	if (NULL == DB) {
		goto exit;
	}
	self->elements[self->size] = DB;
	self->size++;
exit:
	esif_ccb_write_unlock(&self->lock);
	return DB;
}


void DataBank_CloseNameSpace(
	DataBankPtr self,
	esif_string nameSpace
	)
{
	UInt32 index;
	DataVaultPtr DB = NULL;

	esif_ccb_write_lock(&self->lock);

	DB = DataBank_GetNameSpace_Locked(self, nameSpace, &index);
	if (NULL == DB) {
		goto exit;
	}
	DataVault_Destroy(DB);
	self->elements[index] = NULL;

	// Move Array Items down one
	for ( ; index + 1 < self->size; index++) {
		self->elements[index] = self->elements[index + 1];
		self->elements[index + 1] = NULL;
	}
	self->size--;
exit:
	esif_ccb_write_unlock(&self->lock);
}


// Does specified Key exist in the given nameSpace?
int DataBank_KeyExists(
	DataBankPtr self,
	StringPtr nameSpace, // NULL == Default Namespace
	StringPtr keyName
	)
{
	DataVaultPtr DB = NULL;
	int result = ESIF_FALSE;

	if (NULL == self) {
		goto exit;
	}

	esif_ccb_read_lock(&self->lock);
	DB = DataBank_GetNameSpace(self, nameSpace);
	if (NULL != DB && DataCache_GetValue(DB->cache, keyName) != NULL) {
		result = ESIF_TRUE;
	}
	esif_ccb_read_unlock(&self->lock);
exit:
	return result;
}


// Automatically Load all Static DataVaults and *.dv files in the current folder into the DataBank
static eEsifError DataBank_LoadDataVaults(DataBankPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self);

	if (ESIF_FALSE == self->staticVaultsLoaded) {
		rc = DataBank_LoadStaticDataVaults(self);
		self->staticVaultsLoaded = ESIF_TRUE;
	}
	if (ESIF_OK == rc) {
		rc = DataBank_LoadFileDataVaults(self);
	}
	return rc;
}


// Load all Static DataVaults into the DataBank
static eEsifError DataBank_LoadStaticDataVaults(DataBankPtr self)
{
	eEsifError rc = ESIF_OK;
	UInt32 index;

	ESIF_ASSERT(self);

	// Import all Static DataVaults into ReadOnly DataVaults
	for (index = 0; g_StaticDataVaults[index].name; index++) {
		DataVaultPtr DB = DataBank_OpenNameSpace(self, g_StaticDataVaults[index].name);
		if (DB) {
			IOStream_SetMemory(DB->stream, g_StaticDataVaults[index].buffer, g_StaticDataVaults[index].buf_len);
			DB->flags |= (ESIF_SERVICE_CONFIG_READONLY | ESIF_SERVICE_CONFIG_NOCACHE | ESIF_SERVICE_CONFIG_STATIC);
			if (DataVault_ReadVault(DB) != ESIF_OK) {
				DataBank_CloseNameSpace(self, g_StaticDataVaults[index].name);
			}
		}
	}

	return rc;
}


// Automatically Load all *.dv files in the current folder into the DataBank
static eEsifError DataBank_LoadFileDataVaults(DataBankPtr self)
{
	eEsifError rc = ESIF_OK;
	esif_ccb_file_enum_t find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
	char file_path[MAX_PATH] = {0};
	char file_pattern[MAX_PATH] = {0};
	struct esif_ccb_file *ffd_ptr;
	size_t name_len;

	ESIF_ASSERT(self);

	// Create DataVault Directory if it doesn't exit
	esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_DV, NULL, NULL);
	esif_ccb_makepath(file_path);

	// Import all matching *.dv files into ReadWrite DataVaults
	esif_ccb_sprintf(MAX_PATH, file_pattern, "*%s", ESIFDV_FILEEXT);
	ffd_ptr   = (struct esif_ccb_file*)esif_ccb_malloc(sizeof(*ffd_ptr));
	if (NULL == ffd_ptr) {
		rc = ESIF_E_NO_MEMORY;
	}

	// Search the DV directory and load all DV's
	if (rc == ESIF_OK) {
		find_handle = esif_ccb_file_enum_first(file_path, file_pattern, ffd_ptr);
	}
	if (ESIF_INVALID_FILE_ENUM_HANDLE != find_handle) {
		do {
			struct esif_ccb_file dv_file = {0};
			DataVaultPtr DB = 0;

			// Check for init pause
			if (g_stopEsifUfInit != ESIF_FALSE) {
				esif_ccb_file_enum_close(find_handle);
				ESIF_TRACE_API_INFO("Pausing DV loading\n");
				rc = ESIF_I_INIT_PAUSED;
				goto exit;
			}

			name_len = esif_ccb_strlen(ffd_ptr->filename, MAX_PATH);
			if ((name_len < MAX_PATH) && (name_len >= sizeof(ESIFDV_FILEEXT))) {
				ffd_ptr->filename[name_len - (sizeof(ESIFDV_FILEEXT) - 1)] = 0;	// Truncate ".dv" extension

				ESIF_TRACE_API_INFO("Loading DV %s\n", ffd_ptr->filename);

				// Read DataVault File, unless it's already been loaded as a Static DataVault
				if (DataBank_GetNameSpace(self, ffd_ptr->filename) == NULL) {
					DB = DataBank_OpenNameSpace(self, ffd_ptr->filename);
					if (DB) {
						esif_build_path(dv_file.filename, sizeof(dv_file.filename), ESIF_PATHTYPE_DV, DB->name, ESIFDV_FILEEXT);
						IOStream_SetFile(DB->stream, dv_file.filename, "rb");
						if (DataVault_ReadVault(DB) != ESIF_OK) {
							DataBank_CloseNameSpace(self, ffd_ptr->filename);
						}
					}
				}
				ESIF_TRACE_API_INFO("Done loading DV %s\n", ffd_ptr->filename);
			}
		} while (esif_ccb_file_enum_next(find_handle, file_pattern, ffd_ptr));
		esif_ccb_file_enum_close(find_handle);
	}
exit:
	esif_ccb_free(ffd_ptr);
	return rc;
}


eEsifError EsifCfgMgrInit()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	if (!g_DataBankMgr) {
		g_DataBankMgr = DataBank_Create();
		if (!g_DataBankMgr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
	}
	rc = DataBank_LoadDataVaults(g_DataBankMgr);
exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifCfgMgrExit()
{
	ESIF_TRACE_ENTRY_INFO();

	if (g_DataBankMgr) {
		DataBank_Destroy(g_DataBankMgr);
		g_DataBankMgr = 0;
	}
	ESIF_TRACE_EXIT_INFO();
}


