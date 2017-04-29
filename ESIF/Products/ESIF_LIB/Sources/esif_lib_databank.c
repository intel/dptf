/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

// Friend Classes
#define _DATABANK_CLASS
#define _DATACACHE_CLASS

#include "esif_lib_databank.h"
#include "esif_lib_datarepo.h"

#define DATABANK_GROWBY		5	// Number of items to grow DataBank.elements by at a time

// define this (and provide a valid static_dv.h) to link static_dv as a Static Data Vault
// #define STATIC_DV

// Static DataVaults
#ifdef STATIC_DV
# include "static_dv.h"	// Static DV converted to Byte array
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
	StringPtr		name;
	const BytePtr	buffer;
	size_t			buf_len;
	Bool			loaded;
}
g_StaticDataVaults[] = {
#ifdef STATIC_DV
	{"static", static_dv, sizeof(static_dv) },
#endif
	{0}
};

// Singleton DataBank (NameSpace) Manager
static DataBankPtr g_DBMgr = NULL;

// Default DataVault Namespace
char g_DBMgr_DefaultDV[ESIF_NAME_LEN] = "dptf";

static DataBankPtr DataBank_Create();
static void DataBank_Destroy(DataBankPtr self);

static DataVaultPtr DataBank_GetDataVault_Locked(
	StringPtr nameSpace,
	UInt32 *indexPtr
);

static esif_error_t DataBank_LoadStaticRepos(DataBankPtr self);
static esif_error_t DataBank_LoadFileRepos(DataBankPtr self);

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
		for (index = 0; self->elements != NULL && index < self->size; index++) {
			DataVault_Destroy(self->elements[index]);
			self->elements[index] = NULL;
		}
		esif_ccb_free(self->elements);
		self->elements = NULL;
		self->size = 0;
		esif_ccb_lock_uninit(&self->lock);
	}
	esif_ccb_free(self);
}

//////////////////////////////////////////////////////////
// deprecated non-singleton methods
// TODO: Remove these and all references and replace calls with singleton functions

DataBankPtr DataBank_GetMgr(void)
{
	return g_DBMgr;
}

//////////////////////////////////////////////////////////
// singleton methods

// Return NameSpace in the Configuration Manager or NULL if it doesn't exist
DataVaultPtr DataBank_GetDataVault(
	StringPtr nameSpace
	)
{
	DataBankPtr self = g_DBMgr;
	DataVaultPtr vaultPtr = NULL;

	if (NULL == nameSpace) {
		nameSpace = g_DBMgr_DefaultDV;
	}
	esif_ccb_read_lock(&self->lock);
	vaultPtr = DataBank_GetDataVault_Locked(nameSpace, NULL);
	esif_ccb_read_unlock(&self->lock);
	return vaultPtr;
}


// Return NameSpace in the Configuration Manager or NULL if it doesn't exist
static DataVaultPtr DataBank_GetDataVault_Locked(
	StringPtr nameSpace,
	UInt32 *indexPtr
	)
{
	DataBankPtr self = g_DBMgr;
	DataVaultPtr DB = NULL;
	UInt32 index;

	ESIF_ASSERT(nameSpace != NULL);

	for (index = 0; self->elements != NULL && index < self->size; index++) {
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
DataVaultPtr DataBank_OpenDataVault(
	StringPtr nameSpace
)
{
	DataBankPtr self = g_DBMgr;
	DataVaultPtr DB = NULL;
	DataVaultPtr *old_elements = NULL;
	UInt32 insertAt = 0;

	// Exit if NameSpace already exists
	esif_ccb_write_lock(&self->lock);
	DB = DataBank_GetDataVault_Locked(nameSpace, NULL);
	if (DB != NULL) {
		goto exit;
	}

	// Not Found. Create NameSpace
	DB = DataVault_Create(nameSpace);
	if (NULL == DB) {
		goto exit;
	}

	// Grow the elements array when it reaches a GROWBY boundary
	if (self->elements == NULL || (self->size % DATABANK_GROWBY) == 0) {
		old_elements = self->elements;
		self->elements = (DataVaultPtr *)esif_ccb_realloc(self->elements, (self->size + DATABANK_GROWBY) * sizeof(DataVaultPtr));
		if (NULL == self->elements) {
			self->elements = old_elements;
			DataVault_Destroy(DB);
			DB = NULL;
			goto exit;
		}
	}
	
	// Keep elements sorted alphabetically
	for (insertAt = 0; insertAt < self->size; insertAt++) {
		if (self->elements[insertAt] != NULL && esif_ccb_stricmp(DB->name, self->elements[insertAt]->name) < 0) {
			UInt32 moveTo = 0;
			for (moveTo = self->size; moveTo > insertAt; moveTo--) {
				self->elements[moveTo] = self->elements[moveTo - 1];
			}
			break;
		}
	}
	self->elements[insertAt] = DB;
	self->size++;

exit:
	esif_ccb_write_unlock(&self->lock);
	return DB;
}

// Open a DataVault and Import its Primary Repo, if it exists
esif_error_t DataBank_ImportDataVault(StringPtr nameSpace)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (nameSpace) {
		char filename[MAX_PATH] = { 0 };
		DataVaultPtr DB = DataBank_OpenDataVault(nameSpace);
		if (!DB) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			// Import DataVault Primary Repo (name.dv) if it exists
			esif_build_path(filename, sizeof(filename), ESIF_PATHTYPE_DV, DB->name, ESIFDV_FILEEXT);
			IOStream_SetFile(DB->stream, StoreReadWrite, filename, "rb");
			rc = DataVault_ImportStream(DB, DB->stream);
			if (rc == ESIF_E_NOT_FOUND) {
				rc = ESIF_OK;
			}
		}
	}
	return rc;
}

// Close and Destroy a the specified DataVault NameSpace
void DataBank_CloseDataVault(
	StringPtr nameSpace
	)
{
	DataBankPtr self = g_DBMgr;
	UInt32 index;
	DataVaultPtr DB = NULL;

	esif_ccb_write_lock(&self->lock);

	DB = DataBank_GetDataVault_Locked(nameSpace, &index);
	if (NULL == DB || NULL == self->elements) {
		goto exit;
	}
	DataVault_Destroy(DB);
	self->elements[index] = NULL;

	// Move Array Items down one
	for ( ; index + 1 < self->size; index++) {
		self->elements[index] = self->elements[index + 1];
		self->elements[index + 1] = NULL;
	}

	// Shrink elements Array when it reaches a GROWBY boundary
	if ((self->size % DATABANK_GROWBY) == 1) {
		DataVaultPtr *new_elements = NULL;
		if (self->size == 1) {
			esif_ccb_free(self->elements);
			new_elements = NULL;
		}
		else {
			new_elements = (DataVaultPtr *)esif_ccb_realloc(self->elements, (self->size - 1) * sizeof(DataVaultPtr));
			if (new_elements == NULL) {
				goto exit;
			}
		}
		self->elements = new_elements;
	}
	self->size--;

exit:
	esif_ccb_write_unlock(&self->lock);
}


// Does specified Key exist in the given nameSpace?
Bool DataBank_KeyExists(
	StringPtr nameSpace, // NULL == Default Namespace
	StringPtr keyName
)
{
	DataBankPtr self = g_DBMgr;
	DataVaultPtr DB = NULL;
	Bool result = ESIF_FALSE;

	if (NULL == self) {
		goto exit;
	}

	esif_ccb_read_lock(&self->lock);
	DB = DataBank_GetDataVault(nameSpace);
	if (NULL != DB && DataCache_GetValue(DB->cache, keyName) != NULL) {
		result = ESIF_TRUE;
	}
	esif_ccb_read_unlock(&self->lock);
exit:
	return result;
}

// Get the Data Type of a DataVault Key
esif_data_type_t DataBank_KeyType(
	StringPtr nameSpace, // NULL == Default Namespace
	StringPtr keyName
)
{
	DataBankPtr self = g_DBMgr;
	DataCacheEntryPtr value = NULL;
	DataVaultPtr DB = NULL;
	esif_data_type_t result = (esif_data_type_t)0;

	if (NULL == self) {
		goto exit;
	}

	esif_ccb_read_lock(&self->lock);
	DB = DataBank_GetDataVault_Locked(nameSpace, NULL);
	if (NULL != DB && ((value = DataCache_GetValue(DB->cache, keyName)) != NULL)) {
		result = value->value.type;
	}
	esif_ccb_read_unlock(&self->lock);
exit:
	return result;
}

// Set Default DataVault
void DataBank_SetDefault(
	const StringPtr nameSpace
)
{
	if (nameSpace) {
		esif_ccb_strcpy(g_DBMgr_DefaultDV, nameSpace, sizeof(g_DBMgr_DefaultDV));
	}
}

// Set Default DataVault
const StringPtr DataBank_GetDefault()
{
	return g_DBMgr_DefaultDV;
}

// Automatically Load all Static DataVaults and *.dv files in the current folder into the DataBank
static esif_error_t DataBank_LoadDataVaults()
{
	DataBankPtr self = g_DBMgr;
	esif_error_t rc = ESIF_OK;

	ESIF_ASSERT(self);

	rc = DataBank_LoadStaticRepos(self);
	if (ESIF_OK == rc) {
		rc = DataBank_LoadFileRepos(self);
	}
	return rc;
}

// Load all Static DataRepos into the DataBank
static esif_error_t DataBank_LoadStaticRepos(DataBankPtr self)
{
	esif_error_t rc = ESIF_OK;
	UInt32 index;

	UNREFERENCED_PARAMETER(self);

	// Import all Static DataVaults into ReadOnly DataVaults
	for (index = 0; g_StaticDataVaults[index].name; index++) {
		if (g_StaticDataVaults[index].loaded == ESIF_FALSE) {

			// Create a Repo Object and Load it into DataCache(s)
			DataRepoPtr repo = DataRepo_CreateAs(StreamNull, StoreStatic, g_StaticDataVaults[index].name);
			if (repo) {
				ESIF_TRACE_API_INFO("Loading Static REPO %s\n", g_StaticDataVaults[index].name);
				IOStream_SetMemory(
					repo->stream, 
					StoreStatic, 
					g_StaticDataVaults[index].buffer, 
					g_StaticDataVaults[index].buf_len);

				rc = DataRepo_LoadSegments(repo);

				if (rc != ESIF_OK) {
					ESIF_TRACE_ERROR("REPO Load Failed [%s (%d)]: %s\n",
						esif_rc_str(rc),
						rc,
						g_StaticDataVaults[index].name);
					rc = ESIF_OK;
				}
				DataRepo_Destroy(repo);
				g_StaticDataVaults[index].loaded = ESIF_TRUE;
			}
		}
	}
	return rc;
}


// Automatically Load all *.dv and *.dvx files in the current folder into the DataBank
static esif_error_t DataBank_LoadFileRepos(DataBankPtr self)
{
	esif_error_t rc = ESIF_OK;
	esif_ccb_file_enum_t find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
	char *extensions[] = { ESIFDV_FILEEXT, ESIFDV_REPOEXT };
	char file_path[MAX_PATH] = {0};
	char file_pattern[MAX_PATH] = {0};
	int extid = 0;
	
	UNREFERENCED_PARAMETER(self);

	// Create DataVault Directory if it doesn't exit
	esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_DV, NULL, NULL);
	esif_ccb_makepath(file_path);

	// Import all matching *.dv and *.dvx files into ReadWrite DataVaults
	for (extid = 0; extid < sizeof(extensions) / sizeof(extensions[0]); extid++) {
		struct esif_ccb_file ffd = { 0 };

		// Search the DV directory and load all DV's
		esif_ccb_sprintf(MAX_PATH, file_pattern, "*%s", extensions[extid]);
		find_handle = esif_ccb_file_enum_first(file_path, file_pattern, &ffd);

		if (ESIF_INVALID_FILE_ENUM_HANDLE != find_handle) {
			do {
				// Check for init pause
				if (g_stopEsifUfInit != ESIF_FALSE) {
					esif_ccb_file_enum_close(find_handle);
					ESIF_TRACE_API_INFO("Pausing DV loading\n");
					rc = ESIF_I_INIT_PAUSED;
					goto exit;
				}

				// Create a Repo Object and Load it into DataVault Cache(s)
				DataRepoPtr repo = DataRepo_CreateAs(
					StreamFile,
					StoreReadWrite,
					ffd.filename);

				if (repo) {
					ESIF_TRACE_API_INFO("Loading File REPO %s\n", ffd.filename);
					rc = DataRepo_LoadSegments(repo);

					if (rc != ESIF_OK) {
						ESIF_TRACE_ERROR("REPO Load Failed [%s (%d)]: %s\n",
							esif_rc_str(rc),
							rc,
							ffd.filename);
						rc = ESIF_OK;
					}
					DataRepo_Destroy(repo);
				}

			} while (esif_ccb_file_enum_next(find_handle, file_pattern, &ffd));
			esif_ccb_file_enum_close(find_handle);
		}
	}
exit:
	return rc;
}

esif_error_t EsifCfgMgrInit()
{
	esif_error_t rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	if (!g_DBMgr) {
		g_DBMgr = DataBank_Create();
		if (!g_DBMgr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
	}
	rc = DataBank_LoadDataVaults();
exit:
	if (rc != ESIF_OK) {
		DataBank_Destroy(g_DBMgr);
		g_DBMgr = 0;
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifCfgMgrExit()
{
	ESIF_TRACE_ENTRY_INFO();

	if (g_DBMgr) {
		DataBank_Destroy(g_DBMgr);
		g_DBMgr = 0;
	}
	ESIF_TRACE_EXIT_INFO();
}
