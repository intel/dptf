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
#include "esif_uf_cfgmgr.h"

#define _DATABANK_CLASS
#define _DATACACHE_CLASS
#define _DATAVAULT_CLASS
#define _IOSTREAM_CLASS

#include "esif_lib_datacache.h"
#include "esif_lib_esifdata.h"
#include "esif_lib_iostream.h"
#include "esif_lib_databank.h"

#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Limits
#define MAX_DV_DATALEN  0x7fffffff

/*
   // TODO: Delayed Writes
   DataVault_BeginTran
   DataVault_RollbackTran
   DataVault_CommitTran
   DataVault_Flush
 */

#define ESIFDV_VERSION(major, minor, revision)  ((UInt32)((((major) & 0xFF) << 24) | (((minor) & 0xFF) << 16) | ((revision) & 0xFFFF)))

// ESIFDV File Header
struct DataVaultHeader_s {
	UInt8   signature[2];	// File Signature
	UInt16  headersize;		// Header Size, including signature & headersize
	UInt32  version;		// File Version
	UInt32  flags;			// Global Flags
	// TODO: AllocationMap, PageSize, etc
};

typedef struct DataVaultHeader_s DataVaultHeader, *DataVaultHeaderPtr, *DataVaultHeaderPtrLocation;

/////////////////////////////////////////////////////////////////////////
// DataVault Class

// constructor
void DataVault_ctor (DataVaultPtr self)
{
	if (self) {
		WIPEPTR(self);
		self->cache  = DataCache_Create();
		self->stream = IOStream_Create();
	}
}


// destructor
void DataVault_dtor (DataVaultPtr self)
{
	if (self) {
		DataCache_Destroy(self->cache);
		IOStream_Destroy(self->stream);
		WIPEPTR(self);
	}
}


// new operator
DataVaultPtr DataVault_Create ()
{
	DataVaultPtr self = (DataVaultPtr)esif_ccb_malloc(sizeof(*self));
	DataVault_ctor(self);
	return self;
}


// delete operator
void DataVault_Destroy (DataVaultPtr self)
{
	DataVault_dtor(self);
	esif_ccb_free(self);
}


// Write DataVault to Disk
eEsifError DataVault_WriteVault (DataVaultPtr self)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	DataVaultHeader header;
	struct esif_ccb_file dv_file    = {0};
	struct esif_ccb_file dv_filebak = {0};
	IOStreamPtr vault    = 0;
	IOStreamPtr vaultBak = 0;
	u32 idx;

	if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC | ESIF_SERVICE_CONFIG_READONLY)) {
		return ESIF_E_READONLY;
	}

	// TODO: Locking
	vault = IOStream_Create();
	if (!vault) {
		return ESIF_E_NO_MEMORY;
	}

	// If any rows contain NOCACHE PERSIST values, we need to make a copy the original DataVault while creating the new one
	esif_build_path(dv_file.filename, sizeof(dv_file.filename), ESIF_PATHTYPE_DV, self->name, ESIFDV_FILEEXT);

	for (idx = 0; idx < self->cache->size; idx++)
		if (FLAGS_TESTALL(self->cache->elements[idx].flags, ESIF_SERVICE_CONFIG_NOCACHE | ESIF_SERVICE_CONFIG_PERSIST) &&
			self->cache->elements[idx].value.buf_len == 0) {
			struct stat filebak_stat = {0};
			esif_build_path(dv_filebak.filename, sizeof(dv_file.filename), ESIF_PATHTYPE_DV, self->name, ESIFDV_BAKFILEEXT);

			// Delete BAK file if it exists
			if (esif_ccb_stat(dv_filebak.filename, &filebak_stat) == 0) {
				esif_ccb_unlink(dv_filebak.filename);
			}
			if (esif_ccb_rename(dv_file.filename, dv_filebak.filename) == 0) {
				if ((vaultBak = IOStream_Create()) == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				if (!vaultBak || IOStream_OpenFile(vaultBak, dv_filebak.filename, "rb") != 0) {
					IOStream_Destroy(vault);
					IOStream_Destroy(vaultBak);
					return rc;
				}
			}
			break;
		}

	// Create DataVault, Overwrite if necessary
	IOStream_SetFile(vault, self->stream->file.name, "wb");
	if (IOStream_Open(vault) != 0) {
		if (vaultBak) {
			IOStream_Destroy(vaultBak);
			esif_ccb_unlink(dv_filebak.filename);
		}
		IOStream_Destroy(vault);
		return rc;
	}

	// Create File Header
	memset(&header, 0, sizeof(header));
	esif_ccb_memcpy(&header.signature, ESIFDV_SIGNATURE, sizeof(header.signature));
	header.headersize = sizeof(header);
	header.version    = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION, ESIFDV_MINOR_VERSION, ESIFDV_REVISION);
	header.flags = 0;	// TODO: get from self->flags

	// Write File Header
	IOStream_Seek(vault, 0, SEEK_SET);
	IOStream_Write(vault, &header, sizeof(header));
	rc = ESIF_OK;

	// Write All Persisted Rows from Sorted List to DataVault
	for (idx = 0; idx < self->cache->size; idx++) {
		DataCacheEntryPtr keypair = &self->cache->elements[idx];
		if (keypair->flags & ESIF_SERVICE_CONFIG_PERSIST) {
			UInt8 *buffer     = 0;
			UInt32 buffer_len = 0;
			UInt32 byte = 0;

			IOStream_Write(vault, &keypair->flags, sizeof(keypair->flags));
			IOStream_Write(vault, &keypair->key.data_len, sizeof(keypair->key.data_len));
			IOStream_Write(vault, keypair->key.buf_ptr, keypair->key.data_len);

			IOStream_Write(vault, &keypair->value.type, sizeof(keypair->value.type));
			IOStream_Write(vault, &keypair->value.data_len, sizeof(keypair->value.data_len));

			// Read NOCACHE Entries from Backup file
			if (keypair->flags & ESIF_SERVICE_CONFIG_NOCACHE) {
				size_t offset = IOStream_GetOffset(vault);

				// Read Block from BAK file
				if (keypair->value.buf_len == 0) {
					size_t bakoffset = (size_t)keypair->value.buf_ptr;
					buffer     = (UInt8*)esif_ccb_malloc(keypair->value.data_len);
					buffer_len = keypair->value.data_len;
					if (!buffer) {
						rc = ESIF_E_NO_MEMORY;
						break;
					}
					if (IOStream_Seek(vaultBak, bakoffset, SEEK_SET) != 0 || IOStream_Read(vaultBak, buffer, buffer_len) != buffer_len) {
						esif_ccb_free(buffer);
						rc = ESIF_E_UNSPECIFIED;// TODO: ESIF_E_IOERROR;
						break;
					}
					keypair->value.buf_ptr = (void*)offset;
				}
				// Convert internal storage to NOCACHE
				else {
					buffer     = (UInt8*)keypair->value.buf_ptr;
					buffer_len = keypair->value.data_len;
					keypair->value.buf_ptr = (void*)offset;
					keypair->value.buf_len = 0;
				}
			}

			// Encrypt Data?
			if (keypair->flags & ESIF_SERVICE_CONFIG_ENCRYPT) {
				if (!buffer) {
					buffer     = (UInt8*)esif_ccb_malloc(keypair->value.data_len);
					buffer_len = keypair->value.data_len;
					if (!buffer) {
						rc = ESIF_E_NO_MEMORY;
						break;
					}
				}
				for (byte = 0; byte < keypair->value.data_len; byte++)
					buffer[byte] = ~((UInt8*)(keypair->value.buf_ptr))[byte];
			}

			if (buffer) {
				IOStream_Write(vault, buffer, buffer_len);
				esif_ccb_free(buffer);
			} else {
				IOStream_Write(vault, keypair->value.buf_ptr, keypair->value.data_len);
			}
		}
	}

	// Rollback on Error
	if (rc != ESIF_OK) {
		IOStream_Destroy(vaultBak);
		IOStream_Destroy(vault);
		esif_ccb_unlink(dv_file.filename);
		IGNORE_RESULT(esif_ccb_rename(dv_filebak.filename, dv_file.filename));
		return rc;
	}
	// Remove BAK file and Commit
	if (vaultBak) {
		IOStream_Close(vaultBak);
		esif_ccb_unlink(dv_filebak.filename);
		IOStream_Destroy(vaultBak);
	}
	IOStream_Close(vault);
	IOStream_Destroy(vault);
	return rc;
}


// Read DataVault from Disk
eEsifError DataVault_ReadVault (DataVaultPtr self)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;	// TODO
	int vrc = EPERM;
	DataVaultHeader header;
	UInt32 min_version;
	UInt32 max_version;
	esif_flags_t item_flags;
	IOStreamPtr vault = self->stream;

	// TODO: Locking

	// Open File or Memory Block
	if ((vrc = IOStream_Open(vault)) != 0) {
		if (vrc == ENOENT) {
			rc = ESIF_E_NOT_FOUND;
		}
		return rc;
	}

	// Read and Verify File Header
	// The DataVault's Major.Minor Version must be <= the App's Major.Minor Version and at least Major.0
	memset(&header, 0, sizeof(header));
	IOStream_Read(vault, &header, 4);	// E51F#### where #### is header size, including this
	IOStream_Read(vault, ((UInt8*)&header) + 4, esif_ccb_min(header.headersize, sizeof(header)) - 4);
	min_version = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION, 0, 0);
	max_version = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION, ESIFDV_MINOR_VERSION, ESIFDV_MAX_REVISION);
	if (memcmp(header.signature, ESIFDV_SIGNATURE, sizeof(header.signature)) != 0 || header.version > max_version || header.version < min_version) {
		IOStream_Close(vault);
		return ESIF_E_NOT_SUPPORTED;
	}
	if (header.headersize > sizeof(header)) {
		IOStream_Seek(vault, header.headersize - sizeof(header), SEEK_CUR);
	}
	self->flags = header.flags;
	if (IOStream_GetType(vault) == StreamMemory) {
		self->flags |= ESIF_SERVICE_CONFIG_STATIC;
	}
	rc = ESIF_OK;

	// Read Data and add to DataVault
	while (IOStream_Read(vault, &item_flags, sizeof(item_flags)) == sizeof(item_flags)) {
		EsifData key;
		EsifData value;

		// Read Key
		EsifData_ctor(&key);
		EsifData_ctor(&value);
		key.type = ESIF_DATA_STRING;
		IOStream_Read(vault, &key.data_len, sizeof(key.data_len));
		if (key.data_len > MAX_DV_DATALEN) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			break;
		}
		switch (IOStream_GetType(vault)) {
		case StreamMemory:
			key.buf_len = 0;
			key.buf_ptr = IOStream_GetMemoryBuffer(vault) + IOStream_GetOffset(vault);
			IOStream_Seek(vault, key.data_len, SEEK_CUR);
			item_flags &= ~ESIF_SERVICE_CONFIG_NOCACHE;	// ignore for Memory Vaults
			break;

		case StreamFile:
		default:
			key.buf_len = key.data_len;
			key.buf_ptr = esif_ccb_malloc(key.buf_len);
			if (!key.buf_ptr) {
				rc = ESIF_E_NO_MEMORY;
				break;
			}
			IOStream_Read(vault, key.buf_ptr, key.data_len);
			((esif_string)(key.buf_ptr))[key.data_len - 1] = 0;
			break;
		}
		if (rc != ESIF_OK) {
			EsifData_dtor(&key);
			break;
		}

		// Read Value
		IOStream_Read(vault, &value.type, sizeof(value.type));
		IOStream_Read(vault, &value.data_len, sizeof(value.data_len));
		if (value.data_len > MAX_DV_DATALEN) {
			EsifData_dtor(&key);
			break;
		}

		// If NOCACHE mode, use buf_ptr to store the file offset of the data and skip the file
		if (item_flags & ESIF_SERVICE_CONFIG_NOCACHE) {
			size_t offset = IOStream_GetOffset(vault);
			IOStream_Seek(vault, value.data_len, SEEK_CUR);
			value.buf_ptr = (void*)offset;
			value.buf_len = 0;	// data_len = original length
		} else {
			// Use static pointer for static data vaults (unless encrypted), otherwise make a dynamic copy
			if (IOStream_GetType(vault) == StreamMemory && !(item_flags & ESIF_SERVICE_CONFIG_ENCRYPT)) {
				value.buf_len = 0;	// static
				value.buf_ptr = IOStream_GetMemoryBuffer(vault) + IOStream_GetOffset(vault);
				IOStream_Seek(vault, value.data_len, SEEK_CUR);
			} else {
				value.buf_len = value.data_len;	// dynamic
				value.buf_ptr = esif_ccb_malloc(value.buf_len);
				if (!value.buf_ptr) {
					EsifData_dtor(&key);
					rc = ESIF_E_NO_MEMORY;
					break;
				}
				IOStream_Read(vault, value.buf_ptr, value.data_len);
			}
			// Decrypt Encrypted Data?
			if (item_flags & ESIF_SERVICE_CONFIG_ENCRYPT) {
				UInt32 byte;
				for (byte = 0; byte < value.data_len; byte++)
					((UInt8*)(value.buf_ptr))[byte] = ~((UInt8*)(value.buf_ptr))[byte];
			}
		}
		// Add value (including allocated buf_ptr) to cache
		DataCache_SetValue(self->cache, (esif_string)key.buf_ptr, value, item_flags);
		value.buf_ptr = 0;
		item_flags    = 0;
		EsifData_dtor(&key);
		EsifData_dtor(&value);
	}
	IOStream_Close(vault);
	return rc;
}


// Read a Section of a DataVault from Disk
static eEsifError DataVault_ReadBlock (
	DataVaultPtr self,
	void *buf_ptr,
	UInt32 buf_len,
	size_t offset
	)
{
	eEsifError rc     = ESIF_E_NOT_FOUND;	// TODO
	IOStreamPtr vault = self->stream;

	// Open DB File or Memory Block
	if (IOStream_Open(vault) != 0) {
		return rc;
	}

	// Seek and Read Buffer
	if (IOStream_Seek(vault, (long)offset, SEEK_SET) == 0 && IOStream_Read(vault, buf_ptr, buf_len) == buf_len) {
		rc = ESIF_OK;
	}

	IOStream_Close(vault);
	return rc;
}


// Write to Transaction Log
// NOTE: This version is just a dumb text log, not a recoverable transaction log
//
static void DataVault_WriteLog (
	DataVaultPtr self,
	esif_string action,
	esif_string nameSpace,
	EsifDataPtr path,
	esif_flags_t flags,
	EsifDataPtr value
	)
{
#ifndef ESIF_CONFIG_LOG
	UNREFERENCED_PARAMETER(self);
	UNREFERENCED_PARAMETER(action);
	UNREFERENCED_PARAMETER(nameSpace);
	UNREFERENCED_PARAMETER(path);
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(value);
#else
	struct esif_ccb_file log_file = {0};
	FILE *filePtr = NULL;
	time_t now;
	char theTime[30];

	UNREFERENCED_PARAMETER(self);

	if (!flags) {
		return;
	}
	
	esif_build_path(log_file.filename, sizeof(log_file.filename), ESIF_PATHTYPE_DV, nameSpace, ESIFDV_LOGFILEEXT);
	esif_ccb_fopen(&filePtr, log_file.filename, "ab");
	if (NULL == filePtr) {
		return;
	}

	time(&now);
	esif_ccb_ctime(theTime, sizeof(theTime), &now);
	esif_ccb_fprintf(filePtr, "%.24s: %-6s %s flags=%d",
					 theTime,
					 action,
					 (EsifString)path->buf_ptr,
					 flags);
	if (NULL != value) {
		UInt32 ch;
		esif_ccb_fprintf(filePtr, " type=%d buf_len=%d data_len=%d buf_ptr=%s", value->type, value->buf_len, value->data_len, (value->buf_ptr ? "0x" : "NULL"));
		for (ch = 0; NULL != value->buf_ptr && ch < value->data_len; ch++)
			esif_ccb_fprintf(filePtr, "%02X", (int)((UInt8*)(value->buf_ptr))[ch]);
	}
	esif_ccb_fprintf(filePtr, "\r\n");
	fclose(filePtr);
#endif
}


// Load an External File into a specified memory buffer
static eEsifError DataVault_ReadFile (
	DataVaultPtr self,
	esif_string filename,
	void * *buffer,
	UInt32 *buf_size
	)
{
	eEsifError rc    = ESIF_E_UNSPECIFIED;
	IOStreamPtr file = IOStream_Create();

	UNREFERENCED_PARAMETER(self);

	if (file && IOStream_OpenMemoryCloneFile(file, filename) == 0) {
		*buffer   = file->memory.buffer;
		*buf_size = (UInt32)file->memory.buf_len;
		file->memory.buffer  = 0;
		file->memory.buf_len = 0;
		rc = ESIF_OK;
	}
	IOStream_Destroy(file);
	return rc;
}


#ifdef ESIF_ATTR_OS_WINDOWS
static struct {
	HKEY  hkey;
	char  *name;
}

HKeyList[] = {
	{HKEY_LOCAL_MACHINE,  "HKLM"                        },
	{HKEY_LOCAL_MACHINE,  "HKEY_LOCAL_MACHINE"          },
	{HKEY_CLASSES_ROOT,   "HKCR"                        },
	{HKEY_CLASSES_ROOT,   "HKEY_CLASSES_ROOT"           },
	{HKEY_USERS,          "HKU"                         },
	{HKEY_USERS,          "HKEY_USERS"                  },
	{HKEY_CURRENT_USER,   "HKCU"                        },
	{HKEY_CURRENT_USER,   "HKEY_CURRENT_USER"           },
	{HKEY_CURRENT_CONFIG, "HKCC"                        },
	{HKEY_CURRENT_CONFIG, "HKEY_CURRENT_CONFIG"         },
	{                  0,                              0}
};

// Read a Registry Key Value into a specified memory buffer
static eEsifError DataVault_ReadRegistry (
	DataVaultPtr self,
	esif_string keyname,
	void * *buffer,
	UInt32 *buf_size
	)
{
	eEsifError rc   = ESIF_E_UNSPECIFIED;
	HKEY hKey       = 0;
	DWORD dwFlags   = RRF_RT_ANY;
	DWORD dwError   = 0;
	DWORD dwSize    = 0;
	char *regkey    = 0;
	char *valuename = 0;
	u32 j;

	UNREFERENCED_PARAMETER(self);

	// Format: HKLM\Subkey\Path\ValueName
	for (j = 0; HKeyList[j].name; j++) {
		size_t namelen = esif_ccb_strlen(HKeyList[j].name, 30);
		if (esif_ccb_strnicmp(HKeyList[j].name, keyname, namelen) == 0 && keyname[namelen] == '\\') {
			hKey     = HKeyList[j].hkey;
			keyname += esif_ccb_strlen(HKeyList[j].name, 30) + 1;
			break;
		}
	}
	if (!hKey) {
		return rc;
	}

	regkey = esif_ccb_strdup(keyname);
	if ((valuename = strrchr(regkey, '\\')) == 0) {
		esif_ccb_free(regkey);
		return rc;
	}
	*valuename++ = 0;

	// Get Data Size from Registry and copy into buffer if found
	if ((dwError = RegGetValueA(hKey, regkey, valuename, dwFlags, NULL, 0, &dwSize)) == 0) {
		void *reg_buf = esif_ccb_malloc(dwSize);
		dwError = RegGetValueA(hKey, regkey, valuename, dwFlags, NULL, reg_buf, &dwSize);
		if (dwError == 0) {
			*buffer   = reg_buf;
			*buf_size = dwSize;
			rc = ESIF_OK;
		} else {
			esif_ccb_free(reg_buf);
		}
	} else {
		rc = ESIF_E_NOT_FOUND;
	}
	esif_ccb_free(regkey);
	return rc;
}


// Write a Registry Key Value from an EsifData value
static eEsifError DataVault_WriteRegistry (
	DataVaultPtr self,
	esif_string keyname,
	EsifDataPtr value
	)
{
	eEsifError rc   = ESIF_E_UNSPECIFIED;
	HKEY hKey       = 0;
	DWORD dwFlags   = RRF_RT_ANY;
	DWORD dwError   = 0;
	DWORD dwSize    = 0;
	DWORD dwType    = REG_NONE;
	char *regkey    = 0;
	char *valuename = 0;
	u32 j;

	UNREFERENCED_PARAMETER(self);

	// Keyname Format: "HKLM\Subkey\Path\ValueName"
	for (j = 0; HKeyList[j].name; j++) {
		size_t namelen = esif_ccb_strlen(HKeyList[j].name, 30);
		if (esif_ccb_strnicmp(HKeyList[j].name, keyname, namelen) == 0 && keyname[namelen] == '\\') {
			hKey     = HKeyList[j].hkey;
			keyname += esif_ccb_strlen(HKeyList[j].name, 30) + 1;
			break;
		}
	}
	if (!hKey) {
		return rc;
	}

	regkey = esif_ccb_strdup(keyname);
	if ((valuename = strrchr(regkey, '\\')) == 0) {
		esif_ccb_free(regkey);
		return rc;
	}
	*valuename++ = 0;

	// Get Data Type from Registry if value already exists, otherwise deduce it from ESIF Data Type
	if ((dwError = RegGetValueA(hKey, regkey, valuename, dwFlags, &dwType, 0, 0)) != 0) {
		switch (value->type) {
		case ESIF_DATA_INT32:
		case ESIF_DATA_UINT32:
		case ESIF_DATA_TEMPERATURE:
			dwType = REG_DWORD;
			dwSize = value->data_len;
			break;

		case ESIF_DATA_INT64:
		case ESIF_DATA_UINT64:
		case ESIF_DATA_FREQUENCY:
			dwType = REG_QWORD;
			dwSize = value->data_len;
			break;

		case ESIF_DATA_STRING:
			dwType = REG_SZ;
			dwSize = value->data_len;
			break;

		case ESIF_DATA_BINARY:
		case ESIF_DATA_BLOB:
			dwType = REG_BINARY;
			dwSize = value->data_len;
			break;

		default:
			rc = ESIF_E_NOT_SUPPORTED;
			break;
		}
	}
	// Write Registry Value
	if (dwType != REG_NONE) {
		dwError = RegSetKeyValueA(hKey, regkey, valuename, dwType, value->buf_ptr, value->data_len);
		if (dwError == 0) {
			rc = ESIF_OK;
		}
	}
	esif_ccb_free(regkey);
	return rc;
}


#endif

// Retrieve a single value from a DataVault
eEsifError DataVault_GetValue (
	DataVaultPtr self,
	EsifDataPtr path,
	EsifDataPtr value
	)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	DataCacheEntryPtr keypair = NULL;

	if (!self)
		return ESIF_E_PARAMETER_IS_NULL;

	// TODO: Locking

	// Debug: Dump Entire Contents of DataVault if path="*"
	if (strcmp((esif_string)path->buf_ptr, "*") == 0) {
		UInt32 context = 0;
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, esif_ccb_strdup(self->name), ESIFAUTOLEN, ESIFAUTOLEN);
		EsifDataPtr key       = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		EsifDataPtr value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

#ifdef _DEBUG
		// Dump DataCache if Debug Mode
		UInt32 i;
		for (i = 0; i < self->cache->size; i++) {
			DataCacheEntryPtr keypair = &self->cache->elements[i];
			CMD_DEBUG("\n"
					  "id=%d\n"
					  "key=%s\n"
					  "flags=%08X\n"
					  "value.type = %s (%d)\n"
					  "value.buf_ptr = %p %s\n"
					  "value.buf_len = %d\n"
					  "value.data_len= %d\n",
					  i,
					  (char*)keypair->key.buf_ptr,
					  keypair->flags,
					  esif_data_type_str(keypair->value.type), keypair->value.type,
					  keypair->value.buf_ptr,
					  ((keypair->flags & (ESIF_SERVICE_CONFIG_FILELINK | ESIF_SERVICE_CONFIG_REGLINK)) ? (esif_string)keypair->value.buf_ptr : ""),
					  keypair->value.buf_len,
					  keypair->value.data_len
					  );
		}
		if (i > 0) {
			CMD_DEBUG("*****************\n");
		}
#endif
		// Iterate the Contents of the Data Cache
		if ((rc = EsifConfigFindFirst(nameSpace, key, value, &context)) == ESIF_OK) {
			do {
				StringPtr valuestr = EsifData_ToString(value);
				CMD_DEBUG("\n"
						  "EsifFind%s(\"%s\",%d):\n"
						  "  path = %s\n"
						  "  value= %s%s%s\n"
						  "  type = %s, len=%d\n",
						  (context <= 1 ? "First" : "Next"), (char*)nameSpace->buf_ptr, context - 1,
						  (char*)key->buf_ptr,
						  (value->type == ESIF_DATA_STRING ? "\"" : ""), (valuestr ? valuestr : ""), (value->type == ESIF_DATA_STRING ? "\"" : ""),
						  esif_data_type_str(value->type), value->data_len
						  );
				esif_ccb_free(valuestr);
				EsifData_Set(key, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
				EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
			} while ((rc = EsifConfigFindNext(nameSpace, key, value, &context)) == ESIF_OK);
			if (rc == ESIF_E_NOT_FOUND) {
				rc = ESIF_OK;
			}
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		EsifData_Destroy(value);
		return ESIF_OK;
	}

	// Write to Log before retrieval if AUTO
	if (value->type == ESIF_DATA_AUTO || value->buf_len == ESIF_DATA_ALLOCATE) {
		DataVault_WriteLog(self, "AUTO", (esif_string)(self->name), path, 0, value);
	}

	keypair = DataCache_GetValue(self->cache, (esif_string)path->buf_ptr);
	
	if (NULL != keypair) {
		UInt32 data_len = keypair->value.data_len;
		void *buf_ptr   = keypair->value.buf_ptr;

		// File Redirect?
		if (keypair->flags & ESIF_SERVICE_CONFIG_FILELINK) {
			if (DataVault_ReadFile(self, (esif_string)buf_ptr, &buf_ptr, &data_len) != ESIF_OK) {
				value->data_len = 0;
				if (value->type == ESIF_DATA_AUTO) {
					value->type = keypair->value.type;
				}
				if (value->buf_len == ESIF_DATA_ALLOCATE) {
					value->buf_len = 0;
					value->buf_ptr = 0;
				}
				return ESIF_OK;	// return OK and a blank buffer if file not found/error
			}
		}

#ifdef ESIF_ATTR_OS_WINDOWS
		// Registry Redirect?
		else if (keypair->flags & ESIF_SERVICE_CONFIG_REGLINK) {
			if (DataVault_ReadRegistry(self, (esif_string)buf_ptr, &buf_ptr, &data_len) != ESIF_OK) {
				value->data_len = 0;
				if (value->type == ESIF_DATA_AUTO) {
					value->type = keypair->value.type;
				}
				if (value->buf_len == ESIF_DATA_ALLOCATE) {
					value->buf_len = 0;
					value->buf_ptr = 0;
				}
				return ESIF_OK;	// return OK and a blank buffer if Registry value not found/error
			}
		}
#endif
		// Match Found. Verify Data Type matches unless AUTO
		if (value->type != keypair->value.type && value->type != ESIF_DATA_AUTO) {
			rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;	// TODO: ESIF_E_INVALID_DATA_TYPE
		}
		// Verify Data Buffer is large enough unless Auto-Allocate
		else if (value->buf_len < data_len && value->buf_len != ESIF_DATA_ALLOCATE) {
			value->buf_len = data_len;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		// Return pointer to static contents if this is a static vault
		else if ((self->flags & ESIF_SERVICE_CONFIG_STATIC) &&
				 (value->type == ESIF_DATA_AUTO) &&
				 (value->buf_len == ESIF_DATA_ALLOCATE)) {
			value->type     = keypair->value.type;
			value->data_len = data_len;
			value->buf_len  = 0;	// Caller MUST NOT Free!
			value->buf_ptr  = buf_ptr;
			rc = ESIF_OK;
		} else {
			// Set Data Type and Auto-Allocate Buffer?
			if (value->type == ESIF_DATA_AUTO) {
				value->type = keypair->value.type;
			}
			if (ESIF_DATA_ALLOCATE == value->buf_len) {
				value->buf_len = data_len;
				value->buf_ptr = esif_ccb_malloc(value->buf_len);
				if (!value->buf_ptr) {
					return ESIF_E_NO_MEMORY;
				}
			}

			// Read from file if NOCACHE option
			if ((keypair->flags & ESIF_SERVICE_CONFIG_NOCACHE) && keypair->value.buf_len == 0) {
				size_t offset = (size_t)keypair->value.buf_ptr;
				if (DataVault_ReadBlock(self, (esif_string)value->buf_ptr, data_len, offset) != ESIF_OK) {
					data_len = 0;
					return ESIF_E_NOT_FOUND;
				}
				// Decrypt?
				if (keypair->flags & ESIF_SERVICE_CONFIG_ENCRYPT) {
					UInt32 byte;
					for (byte = 0; byte < data_len; byte++)
						((UInt8*)(value->buf_ptr))[byte] = ~((UInt8*)(value->buf_ptr))[byte];
				}
			} else {
				esif_ccb_memcpy(value->buf_ptr, buf_ptr, data_len);
			}
			value->data_len = data_len;
			rc = ESIF_OK;
		}
	}
	// Write to Log
	DataVault_WriteLog(self, "GET", (esif_string)self->name, path, 0, value);
	return rc;
}


/* Set */
eEsifError DataVault_SetValue (
	DataVaultPtr self,
	EsifDataPtr path,
	EsifDataPtr value,
	esif_flags_t flags
	)
{
	eEsifError rc = ESIF_OK;
	DataCacheEntryPtr keypair;

	if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC | ESIF_SERVICE_CONFIG_READONLY)) {
		return ESIF_E_READONLY;
	}

	// Ignore value for DELETEs
	if (FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_DELETE)) {
		value = NULL;
	}

	// TODO: Locking

	// AUTO data types or AUTO_ALLOCATE are not allowed for SET
	if (value && (value->type == ESIF_DATA_AUTO || value->buf_len == ESIF_DATA_ALLOCATE)) {
		return ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
	}

	// Write to Log
	DataVault_WriteLog(self, (flags & ESIF_SERVICE_CONFIG_DELETE ? "DELETE" : "SET"), self->name, path, flags, value);

	// Delete entire DataVault?
	if (strcmp((esif_string)path->buf_ptr, "*") == 0) {
		if (flags & ESIF_SERVICE_CONFIG_DELETE) {
			DataCache_Destroy(self->cache);
			if ((self->cache = DataCache_Create()) == NULL) {
				return ESIF_E_NO_MEMORY;
			}
			DataVault_WriteVault(self);
			return ESIF_OK;
		}
		return ESIF_E_NOT_SUPPORTED;// "*" is an invalid key value
	}

	// Read data from File
	// TODO: Change Parser Logic and Syntax instead
	if (value && value->buf_ptr && esif_ccb_strncmp((char*)value->buf_ptr, "<<", 2) == 0) {
		void *buffer  = 0;
		UInt32 buflen = 0;
		if (DataVault_ReadFile(self, (char*)value->buf_ptr + 2, &buffer, &buflen) == ESIF_OK) {
			if (value->buf_len) {
				esif_ccb_free(value->buf_ptr);
			}
			value->buf_ptr = buffer;
			if (value->type == ESIF_DATA_STRING) {
				buflen++;	// Include Null Terminator
			}
			value->buf_len = value->data_len = buflen;
		} else {
			return ESIF_E_UNSPECIFIED;	// TODO: File Not Found
		}
	}

	// Get the Data Row or create it if it does not exist
	keypair = DataCache_GetValue(self->cache, (esif_string)path->buf_ptr);
	if (keypair) {	// Match Found
		// READONLY?
		if (keypair->flags & ESIF_SERVICE_CONFIG_READONLY) {
			rc = ESIF_E_READONLY;
		}
		// DELETE?
		else if (flags & ESIF_SERVICE_CONFIG_DELETE) {
			flags |= keypair->flags;
			DataCache_Delete(self->cache, (esif_string)path->buf_ptr);
		} else if (value && value->buf_ptr) {
			// UPDATE
#ifdef ESIF_ATTR_OS_WINDOWS
			// Update Registry Value for REGKEY values if REGLINK is *NOT* specified in SET command
			if ((keypair->flags & ESIF_SERVICE_CONFIG_REGLINK) && !(flags & ESIF_SERVICE_CONFIG_REGLINK)) {
				keypair->value.type = value->type;
				rc = DataVault_WriteRegistry(self, (esif_string)keypair->value.buf_ptr, value);
			} else	// ...
#endif
			if (keypair->value.buf_len && value->data_len > keypair->value.buf_len) {
				void *new_buf = NULL;
				
				// Autogrow buffer if it was allocated, otherwise ask for a larger buffer
				if (keypair->value.buf_len > 0) {
					new_buf= (void *)esif_ccb_realloc(keypair->value.buf_ptr, value->data_len);
				}

				if (new_buf == NULL) {
					return ESIF_E_NEED_LARGER_BUFFER;
				}
				else {
					keypair->value.buf_len = value->data_len;
					keypair->value.buf_ptr = new_buf;
				}
			} 
			keypair->flags = flags;
			keypair->value.type     = value->type;
			keypair->value.data_len = value->data_len;

			// Replace the File Offset stored in buf_ptr with a copy of the data for updated NOCACHE values
			if (keypair->flags & ESIF_SERVICE_CONFIG_NOCACHE && keypair->value.buf_len == 0) {
				keypair->value.buf_ptr = esif_ccb_malloc(value->data_len);
				keypair->value.buf_len = value->data_len;
			}
			esif_ccb_memcpy(keypair->value.buf_ptr, value->buf_ptr, value->data_len);
			rc = ESIF_OK;
		}
	} else if (value && value->buf_ptr && !(flags & ESIF_SERVICE_CONFIG_DELETE)) {
		// Copy Key/Value Pair to new Data Row
		EsifData newkey   = {ESIF_DATA_STRING, esif_ccb_strdup((esif_string)path->buf_ptr), path->data_len, path->data_len};
		EsifData newvalue = {value->type, esif_ccb_malloc(esif_ccb_max(value->buf_len, value->data_len)), value->data_len, value->data_len};
		esif_ccb_memcpy(newvalue.buf_ptr, value->buf_ptr, value->data_len);
		DataCache_SetValue(self->cache, (esif_string)newkey.buf_ptr, newvalue, flags);
		esif_ccb_free(newkey.buf_ptr);
	}

	// If Persisted, Flush to disk
	if (rc == ESIF_OK && FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_PERSIST)) {
		DataVault_WriteVault(self);
	}
	return rc;
}


/*****************************************************************************/

// backwards compatibility
eEsifError EsifConfigGet (
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value
	)
{
	DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, (StringPtr)(nameSpace->buf_ptr));
	if (DB) {
		return DataVault_GetValue(DB, path, value);
	} else {
		return ESIF_E_NOT_FOUND;
	}
}


// backwards compatibility
eEsifError EsifConfigSet (
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	esif_flags_t flags,
	EsifDataPtr value
	)
{
	eEsifError rc = ESIF_OK;

	// Get the NameSpace or create it if it does not exist
	DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, (StringPtr)(nameSpace->buf_ptr));
	if (!DB) {
		struct esif_ccb_file dv_file = {0};
		DB = DataBank_OpenNameSpace(g_DataBankMgr, (StringPtr)nameSpace->buf_ptr);
		if (!DB) {
			return ESIF_E_NOT_FOUND;
		}

		esif_build_path(dv_file.filename, sizeof(dv_file.filename), ESIF_PATHTYPE_DV, DB->name, ESIFDV_FILEEXT);
		IOStream_SetFile(DB->stream, dv_file.filename, "rb");
		rc = DataVault_ReadVault(DB);
		if (rc == ESIF_E_NOT_FOUND) {
			rc = ESIF_OK;
		}
	}
	if (rc == ESIF_OK) {
		rc = DataVault_SetValue(DB, path, value, flags);
	}
	return rc;
}


// Find Next path in nameSpace
eEsifError EsifConfigFindNext (
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value,
	UInt32 *context
	)
{
	eEsifError rc   = ESIF_E_NOT_FOUND;
	DataVaultPtr DB = 0;
	ASSERT(nameSpace && path && value && context);
	DB = DataBank_GetNameSpace(g_DataBankMgr, (StringPtr)(nameSpace->buf_ptr));
	if (DB) {
		UInt32 item = *context;
		if (item >= DB->cache->size) {
			return rc;
		}
		(*context)++;

		if (path->buf_len && path->buf_len < DB->cache->elements[item].value.data_len) {
			return ESIF_E_NEED_LARGER_BUFFER;
		}
		EsifData_Set(path, ESIF_DATA_STRING, esif_ccb_strdup(
						 (char*)DB->cache->elements[item].key.buf_ptr), DB->cache->elements[item].key.data_len, DB->cache->elements[item].key.data_len);
		return EsifConfigGet(nameSpace, path, value);
	}
	return rc;
}


// Find First path in nameSpace
eEsifError EsifConfigFindFirst (
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value,
	UInt32 *context
	)
{
	ASSERT(nameSpace && path && value && context);
	*context = 0;
	return EsifConfigFindNext(nameSpace, path, value, context);
}

// Update flags bitmask used by EsifConfigSet
esif_flags_t EsifConfigFlags_Set(esif_flags_t bitmask, esif_string optname)
{
	// List of option names and codes. TODO: Keep this list sorted alphabetically and do a binary search
	static struct OptionList_s {
		StringPtr  name;
		UInt32     option;
	}
	optionList[] = {
		{"PERSIST",		ESIF_SERVICE_CONFIG_PERSIST },
		{"ENCRYPT",		ESIF_SERVICE_CONFIG_ENCRYPT },
		{"READONLY",	ESIF_SERVICE_CONFIG_READONLY},
		{"NOCACHE",		ESIF_SERVICE_CONFIG_NOCACHE },
		{"FILELINK",	ESIF_SERVICE_CONFIG_FILELINK},
#ifdef ESIF_ATTR_OS_WINDOWS
		{"REGLINK",		ESIF_SERVICE_CONFIG_REGLINK },
#endif
		{"DELETE",		ESIF_SERVICE_CONFIG_DELETE  },
		{"STATIC",		ESIF_SERVICE_CONFIG_STATIC  },	// DataVault-Level Option

		{"~NOPERSIST",	ESIF_SERVICE_CONFIG_PERSIST },	// Unset option
		{         0,                            0}
	};
	int j;

	for (j = 0; optionList[j].name; j++) {
		// NAME = Set option
		if (esif_ccb_stricmp(optname, optionList[j].name) == 0) {
			bitmask |= optionList[j].option;
			break;
		}
		// ~NAME = Unset option
		if (optionList[j].name[0] == '~' && esif_ccb_stricmp(optname, optionList[j].name+1) == 0) {
			bitmask &= ~optionList[j].option;
			break;
		}
	}
	if (!optionList[j].name) {
		//CMD_OUT("Error: Invalid Option: %s\n", optname);
	}
	return bitmask;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

