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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_DATAVAULT

#include "esif_uf.h"		/* Upper Framework */

// Friend Classes
#define _DATABANK_CLASS
#define _DATACACHE_CLASS

#include "esif_lib_databank.h"
#include "esif_lib_datarepo.h"

#define DATABANK_GROWBY		5		// Number of items to grow DataBank.elements by at a time
#define DATABANK_DEFAULT_DV	"dptf"	// Default Namespace

// define this (and provide a valid static_dv.h) to link static_dv as a Static Data Vault
// #define STATIC_DV

// Static DataVaults
#ifdef STATIC_DV
# include "static_dv.h"	// Static DV converted to Byte array
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

// Optional NULL-terminated array of Data Repositories to load on startup
char **g_autorepos = NULL;

// Singleton DataBank (NameSpace) Manager
static DataBankPtr g_DBMgr = NULL;

static DataBankPtr DataBank_Create();
static void DataBank_Destroy(DataBankPtr self);

static DataVaultPtr DataBank_GetDataVault_Locked(
	StringPtr nameSpace,
	UInt32 *indexPtr
);

static esif_error_t DataBank_RollbackRepos(DataBankPtr self);
static esif_error_t DataBank_LoadStaticRepos(DataBankPtr self);
static esif_error_t DataBank_LoadFileRepos(DataBankPtr self);
static esif_error_t DataBank_LoadSpecifiedRepos(DataBankPtr self);

///////////////////////////////////////////////////////////////////////////////////////////
// DataBank class
///////////////////////////////////////////////////////////////////////////////////////////

static DataBankPtr DataBank_Create()
{
	DataBankPtr self = (DataBankPtr)esif_ccb_malloc(sizeof(*self));
	if (self) {
		esif_ccb_lock_init(&self->lock);
		esif_ccb_strcpy(self->defaultDV, DATABANK_DEFAULT_DV, sizeof(self->defaultDV));
	}
	return self;
}


static void DataBank_Destroy(DataBankPtr self)
{
	UInt32 index;
	if (self) {
		for (index = 0; self->elements != NULL && index < self->size; index++) {
			DataVault_PutRef(self->elements[index]);
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

	esif_ccb_read_lock(&self->lock);
	if (NULL == nameSpace) {
		nameSpace = self->defaultDV;
	}
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
			DataVault_GetRef(DB);
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
	DataVault_GetRef(DB);

	// Grow the elements array when it reaches a GROWBY boundary
	if (self->elements == NULL || (self->size % DATABANK_GROWBY) == 0) {
		old_elements = self->elements;
		self->elements = (DataVaultPtr *)esif_ccb_realloc(self->elements, (self->size + DATABANK_GROWBY) * sizeof(DataVaultPtr));
		if (NULL == self->elements) {
			self->elements = old_elements;
			DataVault_PutRef(DB);
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
	DataVault_GetRef(DB);

exit:
	esif_ccb_write_unlock(&self->lock);
	return DB;
}

// Open a DataVault and Import its Primary Repo, if it exists
esif_error_t DataBank_ImportDataVault(StringPtr nameSpace)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (nameSpace) {
		DataVaultPtr DB = DataBank_OpenDataVault(nameSpace);
		if (!DB) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			rc = DataVault_ImportStream(DB);
			if (rc == ESIF_E_NOT_FOUND) {
				rc = ESIF_OK;
			}
			if (rc != ESIF_OK) {
				IOStream_dtor(DB->stream);
			}
		}
		DataVault_PutRef(DB);
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
	if (NULL == DB) {
		goto exit;
	}
	DataVault_PutRef(DB);
	DB = self->elements[index];
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
	DataVault_PutRef(DB);
}


// Does specified Key exist in the given nameSpace?
Bool DataBank_KeyExists(
	StringPtr nameSpace, // NULL == Default Namespace
	StringPtr keyName
)
{
	DataBankPtr self = g_DBMgr;
	Bool result = ESIF_FALSE;

	if (self) {
		DataVaultPtr DB = DataBank_GetDataVault(nameSpace);
		if (DB) {
			result = DataVault_KeyExists(DB, keyName, NULL, NULL);
		}
		DataVault_PutRef(DB);
	}
	return result;
}

// Get the Data Type of a DataVault Key
esif_data_type_t DataBank_KeyType(
	StringPtr nameSpace, // NULL == Default Namespace
	StringPtr keyName
)
{
	DataBankPtr self = g_DBMgr;
	esif_data_type_t result = (esif_data_type_t)0;

	if (self) {
		esif_data_type_t dataType = result;
		DataVaultPtr DB = DataBank_GetDataVault(nameSpace);
		if (DataVault_KeyExists(DB, keyName, &dataType, NULL)) {
			result = dataType;
		}
		DataVault_PutRef(DB);
	}
	return result;
}

// Set Default DataVault
void DataBank_SetDefault(
	const StringPtr nameSpace
)
{
	DataBankPtr self = g_DBMgr;
	if (self && nameSpace) {
		esif_ccb_write_lock(&self->lock);
		esif_ccb_strcpy(self->defaultDV, nameSpace, sizeof(self->defaultDV));
		esif_ccb_write_unlock(&self->lock);
	}
}

// Set Default DataVault
const StringPtr DataBank_GetDefault()
{
	DataBankPtr self = g_DBMgr;
	StringPtr defaultDV = DATABANK_DEFAULT_DV;
	if (self) {
		esif_ccb_read_lock(&self->lock);
		defaultDV = self->defaultDV;
		esif_ccb_read_unlock(&self->lock);
	}
	return defaultDV;
}

// Automatically Load all Static DataVaults and *.dv files in the current folder into the DataBank
static esif_error_t DataBank_LoadDataVaults()
{
	DataBankPtr self = g_DBMgr;
	esif_error_t rc = ESIF_OK;

	ESIF_ASSERT(self);

	rc = DataBank_RollbackRepos(self);
	if (rc == ESIF_OK) {
		rc = DataBank_LoadStaticRepos(self);
	}
	if (ESIF_OK == rc) {
		rc = DataBank_LoadSpecifiedRepos(self);
	}
	if (ESIF_OK == rc) {
		rc = DataBank_LoadFileRepos(self);
	}
	return rc;
}

// Rollback all Uncommitted *.tmp files in the current folder
static esif_error_t DataBank_RollbackRepos(DataBankPtr self)
{
	esif_error_t rc = ESIF_OK;
	esif_ccb_file_enum_t find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
	char *extensions[] = {
		ESIFDV_FILEEXT ESIFDV_TEMPEXT,
		ESIFDV_FILEEXT ESIFDV_ROLLBACKEXT,
		ESIFDV_REPOEXT ESIFDV_TEMPEXT,
		ESIFDV_REPOEXT ESIFDV_ROLLBACKEXT };
	char file_path[MAX_PATH] = { 0 };
	char file_pattern[MAX_PATH] = { 0 };
	int extid = 0;
	int err = 0;

	UNREFERENCED_PARAMETER(self);

	// Create DataVault Directory if it doesn't exit
	esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_DV, NULL, NULL);
	if ((err = esif_ccb_makepath(file_path)) != 0) {
		rc = ESIF_E_API_ERROR;
		goto exit;
	}

	// Find all matching *.dv.tmp and *.dvx.tmp files and Rollback
	for (extid = 0; extid < sizeof(extensions) / sizeof(extensions[0]); extid++) {
		struct esif_ccb_file ffd = { 0 };

		esif_ccb_sprintf(MAX_PATH, file_pattern, "*%s", extensions[extid]);
		find_handle = esif_ccb_file_enum_first(file_path, file_pattern, &ffd);

		if (ESIF_INVALID_FILE_ENUM_HANDLE != find_handle) {
			do {
				char temp_pathname[MAX_PATH] = { 0 };
				char repo_pathname[MAX_PATH] = { 0 };
				char *ext = NULL;
				char *tempext = NULL;

				// Check for init pause
				if (g_stopEsifUfInit != ESIF_FALSE) {
					esif_ccb_file_enum_close(find_handle);
					ESIF_TRACE_API_INFO("Pausing DV loading\n");
					rc = ESIF_I_INIT_PAUSED;
					goto exit;
				}

				esif_build_path(temp_pathname, sizeof(temp_pathname), ESIF_PATHTYPE_DV, ffd.filename, NULL);
				esif_ccb_strcpy(repo_pathname, temp_pathname, sizeof(repo_pathname));
				
				if (((ext = esif_ccb_strrchr(repo_pathname, '.')) != NULL) && ((tempext = esif_ccb_strrchr(ffd.filename, '.')) != NULL)) {
					*ext = '\0';

					// Recover all .temp files if .dv/.dvx file does not exist
					if (esif_ccb_stricmp(tempext, ESIFDV_ROLLBACKEXT) == 0 && !esif_ccb_file_exists(repo_pathname)) {
						err = esif_ccb_rename(temp_pathname, repo_pathname);
						ESIF_TRACE_API_INFO("Recover Temp REPO %s (result=%d)\n", ffd.filename, err);
					}
					// Rollback all .tmp files (or .temp where .dv/.dvx exists)
					else {
						err = esif_ccb_unlink(temp_pathname);
						ESIF_TRACE_API_INFO("Rollback Temp REPO %s (result=%d)\n", ffd.filename, err);
					}
				}
			} while (esif_ccb_file_enum_next(find_handle, file_pattern, &ffd));
			esif_ccb_file_enum_close(find_handle);
		}
	}

exit:
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
	int err = 0;
	
	UNREFERENCED_PARAMETER(self);

	// Create DataVault Directory if it doesn't exit
	esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_DV, NULL, NULL);
	if ((err = esif_ccb_makepath(file_path)) != 0) {
		rc = ESIF_E_API_ERROR;
		goto exit;
	}

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

// Automatically Load all *.dv and *.dvx files in the current folder into the DataBank
static esif_error_t DataBank_LoadSpecifiedRepos(DataBankPtr self)
{
	esif_error_t rc = ESIF_OK;
	int j = 0;

	UNREFERENCED_PARAMETER(self);

	// Manually load all Repos in the optional Repo List
	for (j = 0; g_autorepos != NULL && g_autorepos[j] != NULL; j++) {

		// Check for init pause
		if (g_stopEsifUfInit != ESIF_FALSE) {
			ESIF_TRACE_API_INFO("Pausing DV loading\n");
			rc = ESIF_I_INIT_PAUSED;
			goto exit;
		}

		// Create a Repo Object and Load it into DataVault Cache(s)
		DataRepoPtr repo = DataRepo_CreateAs(
			StreamFile,
			StoreReadOnly,
			g_autorepos[j]);

		if (repo) {
			ESIF_TRACE_API_INFO("Loading File REPO %s\n", g_autorepos[j]);
			rc = DataRepo_LoadSegments(repo);

			if (rc != ESIF_OK) {
				ESIF_TRACE_ERROR("REPO Load Failed [%s (%d)]: %s\n",
					esif_rc_str(rc),
					rc,
					g_autorepos[j]);
				CMD_OUT("ERROR: Unable to load REPO: %s\n", g_autorepos[j]);
				rc = ESIF_OK;
			}
			DataRepo_Destroy(repo);
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
