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

// Global DataVault Folder
char g_DataVaultDir[MAX_PATH];

// Global Default DataVault Namespace
char g_DataVaultDefault[ESIF_NAME_LEN] = "dptf";

// Optional Startup Script, if none specified in Default DataVault or cmd/start script
char *g_DataVaultStartScript = NULL;

DataBankPtr DataBank_Create ();
void DataBank_Destroy (DataBankPtr self);

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// DataBank class
///////////////////////////////////////////////////////////////////////////////////////////

DataBankPtr DataBank_Create ()
{
	DataBankPtr self = (DataBankPtr)esif_ccb_malloc(sizeof(*self));
	if (self) {
		esif_ccb_lock_init(&self->lock);
	}
	return self;
}


void DataBank_Destroy (DataBankPtr self)
{
	UInt32 idx;
	if (self) {
		for (idx = 0; idx < self->size; idx++)
			DataVault_dtor(&self->elements[idx]);
		self->size = 0;
		esif_ccb_lock_uninit(&self->lock);
	}
	esif_ccb_free(self);
}


// Return NameSpace in the Configuration Manager or NULL if it doesn't exist
DataVaultPtr DataBank_GetNameSpace (
	DataBankPtr self,
	StringPtr nameSpace
	)
{
	DataVaultPtr result = NULL;
	UInt32 ns;

	if (NULL == nameSpace) {
		nameSpace = g_DataVaultDefault;
	}
	if (NULL != nameSpace) {
		esif_ccb_read_lock(&self->lock);
		for (ns = 0; ns < self->size; ns++) {
			if (esif_ccb_stricmp(nameSpace, self->elements[ns].name) == 0) {
				result = &self->elements[ns];
				break;
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}


DataVaultPtr DataBank_OpenNameSpace (
	DataBankPtr self,
	esif_string nameSpace
	)
{
	DataVaultPtr DB = NULL;
	UInt32 ns;

	// Exit if NameSpace already exists
	// TODO: Change this to a linked list or array of pointers so each DataVaultPtr is static
	esif_ccb_read_lock(&self->lock);
	for (ns = 0; ns < self->size; ns++) {
		if (esif_ccb_stricmp(nameSpace, self->elements[ns].name) == 0) {
			DB = &self->elements[ns];
			break;
		}
	}
	esif_ccb_read_unlock(&self->lock);
	if (DB != NULL || ns >= ESIF_MAX_NAME_SPACES) {
		return DB;
	}

	// Not Found. Create NameSpace
	esif_ccb_write_lock(&self->lock);
	DB = &self->elements[self->size++];
	DataVault_ctor(DB);
	esif_ccb_strcpy(DB->name, nameSpace, ESIF_NAME_LEN);
	esif_ccb_strlwr(DB->name, sizeof(DB->name));
	esif_ccb_write_unlock(&self->lock);
	return DB;
}


void DataBank_CloseNameSpace (
	DataBankPtr self,
	esif_string nameSpace
	)
{
	UInt32 ns;

	esif_ccb_write_lock(&self->lock);

	// Find Existing NameSpace
	for (ns = 0; ns < self->size; ns++) {
		if (esif_ccb_stricmp(nameSpace, self->elements[ns].name) == 0) {
			DataVault_dtor(&self->elements[ns]);

			// Move Array Items down one and wipe the final item
			for ( ; ns + 1 < self->size; ns++)
				esif_ccb_memcpy(&self->elements[ns], &self->elements[ns + 1], sizeof(self->elements[ns]));
			if (ns < ESIF_MAX_NAME_SPACES) {
				WIPEPTR(&self->elements[ns]);
			}
			self->size--;
		}
	}
	esif_ccb_write_unlock(&self->lock);
}


// Does specified Key exist in the given nameSpace?
int DataBank_KeyExists (
	DataBankPtr self,
	StringPtr nameSpace, // NULL == Default Namespace
	StringPtr keyName
	)
{
	DataVaultPtr DB = NULL;
	int result = ESIF_FALSE;

	esif_ccb_read_lock(&self->lock);
	DB = DataBank_GetNameSpace(self, nameSpace);
	if (NULL != DB && DataCache_GetValue(DB->cache, keyName) != NULL) {
		result = ESIF_TRUE;
	}
	esif_ccb_read_unlock(&self->lock);
	return result;
}


// Automatically Load all Static DataVaults and *.dv files in the current folder into the DataBank
eEsifError DataBank_LoadDataVaults (DataBankPtr self)
{
	eEsifError rc = ESIF_OK;
	esif_ccb_file_find_handle find_handle = INVALID_HANDLE_VALUE;
	char file_path[MAX_PATH] = {0};
	char file_pattern[MAX_PATH] = {0};
	struct esif_ccb_file *ffd_ptr;
	UInt32 idx;

	ASSERT(self);

	// Import all Static DataVaults into ReadOnly DataVaults
	for (idx = 0; g_StaticDataVaults[idx].name; idx++) {
		DataVaultPtr DB = DataBank_OpenNameSpace(self, g_StaticDataVaults[idx].name);
		if (DB) {
			IOStream_SetMemory(DB->stream, g_StaticDataVaults[idx].buffer, g_StaticDataVaults[idx].buf_len);
			DB->flags |= (ESIF_SERVICE_CONFIG_READONLY | ESIF_SERVICE_CONFIG_NOCACHE);
			DataVault_ReadVault(DB);
		}
	}

	// Create DataVault Directory if it doesn't exit
	esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_DV, NULL, NULL);
	esif_ccb_makepath(file_path);

	// Import all matching *.dv files into ReadWrite DataVaults
	esif_ccb_sprintf(MAX_PATH, file_pattern, "*%s", ESIFDV_FILEEXT);
	ffd_ptr   = (struct esif_ccb_file*)esif_ccb_malloc(sizeof(*ffd_ptr));
	if (NULL == ffd_ptr) {
		rc = ESIF_E_NO_MEMORY;
	}

	if (rc == ESIF_OK) {
		find_handle = esif_ccb_file_enum_first(file_path, file_pattern, ffd_ptr);
	}
	if (INVALID_HANDLE_VALUE != find_handle) {
		do {
			struct esif_ccb_file dv_file = {0};
			DataVaultPtr DB = 0;

			// Read DataVault File, unless it's already been loaded as a Static DataVault
			if (esif_ccb_strlen(ffd_ptr->filename, MAX_PATH) > sizeof(ESIFDV_FILEEXT)) {
				ffd_ptr->filename[esif_ccb_strlen(ffd_ptr->filename, MAX_PATH) - (sizeof(ESIFDV_FILEEXT) - 1)] = 0;	// Truncate ".dv" extension
				if (DataBank_GetNameSpace(self, ffd_ptr->filename) == NULL) {
					DB = DataBank_OpenNameSpace(self, ffd_ptr->filename);
					if (DB) {
						esif_build_path(dv_file.filename, sizeof(dv_file.filename), ESIF_PATHTYPE_DV, DB->name, ESIFDV_FILEEXT);
						IOStream_SetFile(DB->stream, dv_file.filename, "rb");
						DataVault_ReadVault(DB);
					}
				}
			}
		} while (esif_ccb_file_enum_next(find_handle, file_pattern, ffd_ptr));
		esif_ccb_file_enum_close(find_handle);
	}
	esif_ccb_free(ffd_ptr);
	return rc;
}


/*****************************************************************************/
// Backwards compatibility

eEsifError EsifConfigInit (esif_string name)
{
	eEsifError rc = ESIF_OK;
	if (DataBank_OpenNameSpace(g_DataBankMgr, name) == 0) {
		rc = ESIF_E_UNSPECIFIED;
	}
	return rc;
}


void EsifConfigExit (esif_string name)
{
	DataBank_CloseNameSpace(g_DataBankMgr, name);
}


eEsifError EsifCfgMgrInit ()
{
	if (!g_DataBankMgr) {
		g_DataBankMgr = DataBank_Create();
		if (g_DataBankMgr) {
			return DataBank_LoadDataVaults(g_DataBankMgr);
		}
	}
	return ESIF_E_NO_MEMORY;
}


void EsifCfgMgrExit ()
{
	if (g_DataBankMgr) {
		DataBank_Destroy(g_DataBankMgr);
		g_DataBankMgr = 0;
	}
}


