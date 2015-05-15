/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
		esif_ccb_lock_init(&self->lock);
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
		esif_ccb_lock_uninit(&self->lock);
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
	eEsifError rc = ESIF_OK;
	DataVaultHeader header;
	struct esif_ccb_file dv_file    = {0};
	struct esif_ccb_file dv_filebak = {0};
	IOStreamPtr vault    = 0;
	IOStreamPtr vaultBak = 0;
	u32 idx;

	if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC | ESIF_SERVICE_CONFIG_READONLY)) {
		rc = ESIF_E_READONLY;
		goto exit;
	}

	vault = IOStream_Create();
	if (!vault) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	// If any rows contain NOCACHE PERSIST values, we need to make a copy the original DataVault while creating the new one
	esif_build_path(dv_file.filename, sizeof(dv_file.filename), ESIF_PATHTYPE_DV, self->name, ESIFDV_FILEEXT);

	for (idx = 0; idx < self->cache->size; idx++) {
		if (FLAGS_TESTALL(self->cache->elements[idx].flags, ESIF_SERVICE_CONFIG_NOCACHE | ESIF_SERVICE_CONFIG_PERSIST) &&
			self->cache->elements[idx].value.buf_len == 0) {
			struct stat filebak_stat = {0};
			esif_build_path(dv_filebak.filename, sizeof(dv_file.filename), ESIF_PATHTYPE_DV, self->name, ESIFDV_BAKFILEEXT);

			// Delete BAK file if it exists
			if (esif_ccb_stat(dv_filebak.filename, &filebak_stat) == 0 && esif_ccb_unlink(dv_filebak.filename) != EOK) {
				rc = ESIF_E_IO_DELETE_FAILED;
			}
			// Rename DV file to BAK file
			else if (esif_ccb_rename(dv_file.filename, dv_filebak.filename) != 0) {
				rc = ESIF_E_IO_OPEN_FAILED;
			}

			// Open BAK File
			if (rc != ESIF_OK) {
				esif_ccb_memset(dv_filebak.filename, 0, sizeof(dv_filebak.filename));
			}
			else {
				if ((vaultBak = IOStream_Create()) == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else if (IOStream_OpenFile(vaultBak, dv_filebak.filename, "rb") != 0) {
					rc = ESIF_E_IO_OPEN_FAILED;
				}
			}
			break;
		}
	}

	// Create DataVault, Overwrite if necessary
	if (rc == ESIF_OK && IOStream_SetFile(vault, self->stream->file.name, "wb") != EOK) {
		rc = ESIF_E_NO_MEMORY;
	}
	if (rc == ESIF_OK && IOStream_Open(vault) != EOK) {
		rc = ESIF_E_IO_OPEN_FAILED;
	}
	if (rc != ESIF_OK) {
		goto exit;
	}

	// Create File Header
	memset(&header, 0, sizeof(header));
	esif_ccb_memcpy(&header.signature, ESIFDV_SIGNATURE, sizeof(header.signature));
	header.headersize = sizeof(header);
	header.version    = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION, ESIFDV_MINOR_VERSION, ESIFDV_REVISION);
	header.flags = 0;	// TODO: get from self->flags

	// Write File Header
	if (IOStream_Seek(vault, 0, SEEK_SET) != EOK)
		rc = ESIF_E_IO_ERROR;
	if (rc == ESIF_OK && IOStream_Write(vault, &header, sizeof(header)) != sizeof(header))
		rc = ESIF_E_IO_ERROR;

	// Write All Persisted Rows from Sorted List to DataVault
	for (idx = 0; rc == ESIF_OK && idx < self->cache->size; idx++) {
		DataCacheEntryPtr keypair = &self->cache->elements[idx];
		if (keypair->flags & ESIF_SERVICE_CONFIG_PERSIST) {
			UInt8 *buffer     = 0;
			UInt32 buffer_len = 0;
			UInt32 byte = 0;

			// Write Key: <flags><len><value...>
			if (rc == ESIF_OK && IOStream_Write(vault, &keypair->flags, sizeof(keypair->flags)) != sizeof(keypair->flags))
				rc = ESIF_E_IO_ERROR;
			if (rc == ESIF_OK && IOStream_Write(vault, &keypair->key.data_len, sizeof(keypair->key.data_len)) != sizeof(keypair->key.data_len))
				rc = ESIF_E_IO_ERROR;
			if (rc == ESIF_OK && IOStream_Write(vault, keypair->key.buf_ptr, keypair->key.data_len) != keypair->key.data_len)
				rc = ESIF_E_IO_ERROR;

			// Write Value: <type><len><value...>
			if (rc == ESIF_OK && IOStream_Write(vault, &keypair->value.type, sizeof(keypair->value.type)) != sizeof(keypair->value.type))
				rc = ESIF_E_IO_ERROR;
			if (rc == ESIF_OK && IOStream_Write(vault, &keypair->value.data_len, sizeof(keypair->value.data_len)) != sizeof(keypair->value.data_len))
				rc = ESIF_E_IO_ERROR;

			if (rc != ESIF_OK)
				break;

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
						rc = ESIF_E_IO_ERROR;
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
				if (IOStream_Write(vault, buffer, buffer_len) != buffer_len)
					rc = ESIF_E_IO_ERROR;
				esif_ccb_free(buffer);
			}
			else {
				if (IOStream_Write(vault, keypair->value.buf_ptr, keypair->value.data_len) != keypair->value.data_len)
					rc = ESIF_E_IO_ERROR;
			}
		}
	}

exit:
	// Rollback on Error
	if (rc != ESIF_OK) {
		IOStream_Destroy(vaultBak);
		IOStream_Destroy(vault);
		if (dv_filebak.filename[0]) {
			IGNORE_RESULT(esif_ccb_unlink(dv_file.filename));
			IGNORE_RESULT(esif_ccb_rename(dv_filebak.filename, dv_file.filename));
		}
	}
	else {
		// Remove BAK file and Commit
		if (vaultBak) {
			IOStream_Close(vaultBak);
			IGNORE_RESULT(esif_ccb_unlink(dv_filebak.filename));
			IOStream_Destroy(vaultBak);
		}
		if (vault) {
			IOStream_Close(vault);
			IOStream_Destroy(vault);
		}
	}
	return rc;
}


// Read DataVault from Disk
eEsifError DataVault_ReadVault (DataVaultPtr self)
{
	eEsifError rc = ESIF_OK;
	int vrc = EPERM;
	DataVaultHeader header = {0};
	UInt16 header_toread = sizeof(header.signature) + sizeof(header.headersize);
	UInt32 min_version = 0;
	UInt32 max_version = 0;
	esif_flags_t item_flags = 0;
	IOStreamPtr vault = self->stream;
	EsifData key = {ESIF_DATA_STRING};
	EsifData value = {ESIF_DATA_VOID};
	size_t bytes = 0;

	if ((vault = self->stream) == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Open File or Memory Block
	if (rc == ESIF_OK && ((vrc = IOStream_Open(vault)) != 0)) {
		if (vrc == ENOENT) {
			rc = ESIF_E_NOT_FOUND;
		}
		else {
			rc = ESIF_E_IO_OPEN_FAILED;
		}
		goto exit;
	}
	
	// Read File Header
	if (IOStream_Read(vault, &header, header_toread) != header_toread) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	header_toread = esif_ccb_max(header_toread, esif_ccb_min(header.headersize, sizeof(header))) - header_toread;
	if (rc == ESIF_OK && IOStream_Read(vault, ((UInt8*)&header) + sizeof(header.signature) + sizeof(header.headersize), header_toread) != header_toread) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	// The DataVault's Major.Minor Version must be <= the App's Major.Minor Version and at least Major.0
	min_version = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION, 0, 0);
	max_version = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION, ESIFDV_MINOR_VERSION, ESIFDV_MAX_REVISION);
	if (memcmp(header.signature, ESIFDV_SIGNATURE, sizeof(header.signature)) != 0 || header.version > max_version || header.version < min_version) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}
	if (header.headersize > sizeof(header) && (IOStream_Seek(vault, header.headersize - sizeof(header), SEEK_CUR) != EOK)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	self->flags = header.flags;

	// Read Data and add to DataVault
	while (rc == ESIF_OK) {
		EsifData_ctor(&key);
		EsifData_ctor(&value);
		
		// Read Flags
		if ((bytes = IOStream_Read(vault, &item_flags, sizeof(item_flags))) != sizeof(item_flags)) {
			if (bytes != 0) {
				rc = ESIF_E_IO_ERROR;
			}
			break;
		}

		// Read Key
		key.type = ESIF_DATA_STRING;
		if (IOStream_Read(vault, &key.data_len, sizeof(key.data_len)) < sizeof(key.data_len)) {
			rc = ESIF_E_IO_ERROR;
		}
		if (rc == ESIF_OK && key.data_len > MAX_DV_DATALEN) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}
		if (rc != ESIF_OK) {
			break;
		}

		// Use Memory Pointers for Static DataVaults, otherwise allocate memory
		if ((IOStream_GetType(vault) == StreamMemory) && (self->flags & ESIF_SERVICE_CONFIG_STATIC)) {
			key.buf_len = 0;
			key.buf_ptr = IOStream_GetMemoryBuffer(vault) + IOStream_GetOffset(vault);
			if (IOStream_Seek(vault, key.data_len, SEEK_CUR) != EOK) {
				rc = ESIF_E_IO_ERROR;
			}
			else {
				item_flags &= ~ESIF_SERVICE_CONFIG_NOCACHE;	// ignore for Static DataVaults
			}
		}
		else {
			key.buf_len = esif_ccb_max(1, key.data_len);
			key.buf_ptr = esif_ccb_malloc(key.buf_len);
			if (!key.buf_ptr) {
				rc = ESIF_E_NO_MEMORY;
				break;
			}
			if (IOStream_Read(vault, key.buf_ptr, key.data_len) != key.data_len) {
				rc = ESIF_E_IO_ERROR;
			}
			else if (key.data_len) {
				((esif_string)(key.buf_ptr))[key.data_len - 1] = 0;
			}
		}

		// Read Value
		if (rc == ESIF_OK && IOStream_Read(vault, &value.type, sizeof(value.type)) != sizeof(value.type)) {
			rc = ESIF_E_IO_ERROR;
		}
		if (rc == ESIF_OK && IOStream_Read(vault, &value.data_len, sizeof(value.data_len)) != sizeof(value.data_len)) {
			rc = ESIF_E_IO_ERROR;
		}
		if (rc == ESIF_OK && value.data_len > MAX_DV_DATALEN) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}
		if (rc != ESIF_OK) {
			break;
		}

		// If NOCACHE mode, use buf_ptr to store the file offset of the data and skip the file
		if (item_flags & ESIF_SERVICE_CONFIG_NOCACHE) {
			size_t offset = IOStream_GetOffset(vault);
			if (IOStream_Seek(vault, value.data_len, SEEK_CUR) != EOK) {
				rc = ESIF_E_IO_ERROR;
			}
			else {
				value.buf_ptr = (void*)offset;
				value.buf_len = 0;	// data_len = original length
			}
		} 
		else {
			// Use static pointer for static data vaults (unless encrypted), otherwise make a dynamic copy
			if ((IOStream_GetType(vault) == StreamMemory) && (self->flags & ESIF_SERVICE_CONFIG_STATIC) && !(item_flags & ESIF_SERVICE_CONFIG_ENCRYPT)) {
				value.buf_len = 0;	// static
				value.buf_ptr = IOStream_GetMemoryBuffer(vault) + IOStream_GetOffset(vault);
				if (value.buf_ptr == NULL || IOStream_Seek(vault, value.data_len, SEEK_CUR) != EOK) {
					rc = ESIF_E_IO_ERROR;
				}
			} 
			else {
				value.buf_len = esif_ccb_max(1, value.data_len); // dynamic
				value.buf_ptr = esif_ccb_malloc(value.buf_len);
				if (value.buf_ptr == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else if (IOStream_Read(vault, value.buf_ptr, value.data_len) != value.data_len) {
					rc = ESIF_E_IO_ERROR;
				}
			}
			
			// Decrypt Encrypted Data?
			if (rc == ESIF_OK && (item_flags & ESIF_SERVICE_CONFIG_ENCRYPT)) {
				UInt32 byte;
				for (byte = 0; byte < value.data_len; byte++)
					((UInt8*)(value.buf_ptr))[byte] = ~((UInt8*)(value.buf_ptr))[byte];
			}
		}
		
		// Add value (including allocated buf_ptr) to cache
		if (rc == ESIF_OK) {
			DataCache_SetValue(self->cache, (esif_string)key.buf_ptr, value, item_flags);
			value.buf_ptr = 0;
			item_flags    = 0;
		}
		EsifData_dtor(&key);
		EsifData_dtor(&value);
	}

exit:
	EsifData_dtor(&key);
	EsifData_dtor(&value);
	if (vault) {
		IOStream_Close(vault);
	}
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

	// Display all Matching Keys in DataCache if path contains "*" or "?"
	if (esif_ccb_strpbrk((esif_string)path->buf_ptr, "*?") != NULL) {
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, esif_ccb_strdup(self->name), ESIFAUTOLEN, ESIFAUTOLEN);
		EsifDataPtr key       = EsifData_CreateAs(ESIF_DATA_STRING, path->buf_ptr, 0, ESIFAUTOLEN);
		EsifDataPtr value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		EsifConfigFindContext context = NULL;
		int item = 0;

		if (nameSpace != NULL && key != NULL && value != NULL && (rc = EsifConfigFindFirst(nameSpace, key, value, &context)) == ESIF_OK) {
			do {
				StringPtr valuestr = EsifData_ToString(value);
				item++;
				CMD_DEBUG("\n"
					"item    = %d\n"
					"type    = %s\n"
					"length  = %d\n"
					"key     = \"%s\"\n"
					"value   = %s%s%s\n",
					item,
					esif_data_type_str(value->type),
					value->data_len,
					(char*)key->buf_ptr,
					(value->type == ESIF_DATA_STRING ? "\"" : ""), (valuestr ? valuestr : ""), (value->type == ESIF_DATA_STRING ? "\"" : "")
					);

#ifdef ESIF_ATTR_DEBUG
				keypair = DataCache_GetValue(self->cache, (esif_string)key->buf_ptr);
				if (keypair) {
					CMD_DEBUG(
					"flags   = 0x%08X\n"
					"buf_ptr = %p %s\n"
					"buf_len = %d\n"
					"data_len= %d\n",
					keypair->flags,
					keypair->value.buf_ptr,
					((keypair->flags & ESIF_SERVICE_CONFIG_FILELINK) ? (esif_string)keypair->value.buf_ptr : ""),
					keypair->value.buf_len,
					keypair->value.data_len
					);
				}
#endif
				esif_ccb_free(valuestr);
				EsifData_Set(key, ESIF_DATA_STRING, path->buf_ptr, 0, ESIFAUTOLEN);
				EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
			} while ((rc = EsifConfigFindNext(nameSpace, key, value, &context)) == ESIF_OK);
			if (rc == ESIF_E_ITERATION_DONE) {
				rc = ESIF_OK;
			}
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		EsifData_Destroy(value);
		return rc;
	}

	// Write to Log before retrieval if AUTO
	if (value->type == ESIF_DATA_AUTO || value->buf_len == ESIF_DATA_ALLOCATE) {
		DataVault_WriteLog(self, "AUTO", (esif_string)(self->name), path, 0, value);
	}

	keypair = DataCache_GetValue(self->cache, (esif_string)path->buf_ptr);
	
	if (NULL != keypair) {
		UInt32 data_len = keypair->value.data_len;
		void *buf_ptr   = keypair->value.buf_ptr;
		UInt32 buf_len  = 0;

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
			// Include Null Terminator if result is STRING
			if (value->buf_len == ESIF_DATA_ALLOCATE && (value->type == ESIF_DATA_STRING || (value->type == ESIF_DATA_AUTO && keypair->value.type == ESIF_DATA_STRING))) {
				data_len++;
			}
			buf_len = data_len;
		}

		// Match Found. Verify Data Type matches unless AUTO
		if (value->type != keypair->value.type && value->type != ESIF_DATA_AUTO) {
			rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;	// TODO: ESIF_E_INVALID_DATA_TYPE
		}
		// Verify Data Buffer is large enough unless Auto-Allocate
		else if (value->buf_len < data_len && value->buf_len != ESIF_DATA_ALLOCATE) {
			value->data_len = data_len;
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
		} 
		else {
			// Set Data Type and Auto-Allocate Buffer?
			if (value->type == ESIF_DATA_AUTO) {
				value->type = keypair->value.type;
			}
			if (ESIF_DATA_ALLOCATE == value->buf_len) {
				value->buf_len = esif_ccb_max(1, data_len);
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

		// Destroy Dynamically copied data, such as FILELINK contents
		if (buf_len) {
			esif_ccb_free(buf_ptr);
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

	// AUTO data types or AUTO_ALLOCATE are not allowed for SET
	if (value && (value->type == ESIF_DATA_AUTO || value->buf_len == ESIF_DATA_ALLOCATE)) {
		return ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
	}

	// Reject SETs for static-sized data types whose buffer is too small
	if (value) {
		u32 len = (u32) esif_data_type_sizeof(value->type);
		if (len > 0 && value->buf_len < len) {
			value->data_len = len;

			return ESIF_E_NEED_LARGER_BUFFER;
		}
	}

	// Write to Log
	DataVault_WriteLog(self, (flags & ESIF_SERVICE_CONFIG_DELETE ? "DELETE" : "SET"), self->name, path, flags, value);

	// Delete DataVault Key(s)?
	if (esif_ccb_strpbrk((esif_string)path->buf_ptr, "*?") != NULL) {
		if (flags & ESIF_SERVICE_CONFIG_DELETE) {
			// Delete Entire DataVault?
			if (esif_ccb_strcmp((esif_string)path->buf_ptr, "*") == 0) {
				DataCache_Destroy(self->cache);
				if ((self->cache = DataCache_Create()) == NULL) {
					return ESIF_E_NO_MEMORY;
				}
				return DataVault_WriteVault(self);
			}
			// Delete matching keys only
			else {
				UInt32 item = 0;
				while (item < self->cache->size) {
					if (esif_ccb_strmatch((esif_string)self->cache->elements[item].key.buf_ptr, (esif_string)path->buf_ptr)) {
						flags |= FLAGS_TEST(self->cache->elements[item].flags, ESIF_SERVICE_CONFIG_PERSIST);
						if (DataCache_Delete(self->cache, (esif_string)self->cache->elements[item].key.buf_ptr) == ESIF_OK) {
							continue;
						}
					}
					item++;
				}
				goto exit;
			}
		}
		return ESIF_E_NOT_SUPPORTED; // Keys may not contain "*" or "?"
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
			if (keypair->value.buf_len && value->data_len != keypair->value.buf_len) {
				void *new_buf = NULL;
				u32 new_buf_len = 0;
				
				// Grow or shrink buffer if it was allocated, otherwise ask for a larger buffer
				if (keypair->value.buf_len > 0) {
					new_buf_len = esif_ccb_max(1, value->data_len);
					new_buf= (void *)esif_ccb_realloc(keypair->value.buf_ptr, new_buf_len);
				}

				if (new_buf == NULL) {
					return ESIF_E_NEED_LARGER_BUFFER;
				}
				else {
					keypair->value.buf_len = new_buf_len;
					keypair->value.buf_ptr = new_buf;
				}
			} 

			// Replace the File Offset stored in buf_ptr with a copy of the data for updated NOCACHE values
			if (keypair->flags & ESIF_SERVICE_CONFIG_NOCACHE && keypair->value.buf_len == 0) {
				keypair->value.buf_len = esif_ccb_max(1, value->data_len);
				keypair->value.buf_ptr = esif_ccb_malloc(value->buf_len);
			}
			keypair->flags = flags;
			keypair->value.type     = value->type;
			keypair->value.data_len = value->data_len;
			
			esif_ccb_memcpy(keypair->value.buf_ptr, value->buf_ptr, value->data_len);
			rc = ESIF_OK;
		}
	} else if (value && value->buf_ptr && !(flags & ESIF_SERVICE_CONFIG_DELETE)) {
		// Copy Key/Value Pair to new Data Row
		EsifData newkey   = {ESIF_DATA_STRING, esif_ccb_strdup((esif_string)path->buf_ptr), path->data_len, path->data_len};
		EsifData newvalue = {value->type, esif_ccb_malloc(esif_ccb_max(1, esif_ccb_max(value->buf_len, value->data_len))), esif_ccb_max(1, value->data_len), value->data_len};
		esif_ccb_memcpy(newvalue.buf_ptr, value->buf_ptr, value->data_len);
		DataCache_SetValue(self->cache, (esif_string)newkey.buf_ptr, newvalue, flags);
		esif_ccb_free(newkey.buf_ptr);
	}

exit:
	// If Persisted, Flush to disk
	if (rc == ESIF_OK && FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_PERSIST)) {
		rc = DataVault_WriteVault(self);
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
	eEsifError rc = ESIF_OK;
	DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, (StringPtr)(nameSpace->buf_ptr));
	if (DB) {
		esif_ccb_read_lock(&DB->lock);
		rc = DataVault_GetValue(DB, path, value);
		esif_ccb_read_unlock(&DB->lock);
	}
	else {
		rc = ESIF_E_NOT_FOUND;
	}
	return rc;
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
		esif_ccb_write_lock(&DB->lock);
		rc = DataVault_SetValue(DB, path, value, flags);
		esif_ccb_write_unlock(&DB->lock);
	}
	return rc;
}

// Test whether a pattern matches a given path (key) [case-insensitive, only "*" and "?" supported]
int EsifConfigKeyMatch(
	EsifDataPtr pattern,
	EsifDataPtr path
	)
{
	int result = ESIF_TRUE;

	// Do pattern match for ESIF_DATA_STRING only. ESIF_DATA_AUTO and all other types always match
	if (pattern->type == ESIF_DATA_STRING) {
		result = esif_ccb_strmatch(path->buf_ptr, pattern->buf_ptr);
	}
	return result;
}

// Find Next path in nameSpace
eEsifError EsifConfigFindNext(
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value,
	EsifConfigFindContextPtr context
	)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	DataVaultPtr DB = 0;

	ESIF_ASSERT(nameSpace && path && context);
	DB = DataBank_GetNameSpace(g_DataBankMgr, (StringPtr)(nameSpace->buf_ptr));
	if (DB) {
		UInt32 item = 0;

		esif_ccb_read_lock(&DB->lock);

		for (item = 0; item < DB->cache->size; item++) {
			if (*context == NULL || esif_ccb_stricmp(DB->cache->elements[item].key.buf_ptr, *context) > 0) {
				break;
			}
		}

		// Find next matching key
		while (item < DB->cache->size && EsifConfigKeyMatch(path, &DB->cache->elements[item].key) != ESIF_TRUE) {
			item++;
		}

		// Return matching item, if any
		if (item < DB->cache->size) {
			esif_ccb_free(*context);
			*context = esif_ccb_strdup(DB->cache->elements[item].key.buf_ptr);
			
			if (path->buf_len && path->buf_len < DB->cache->elements[item].key.data_len) {
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			else {
				EsifData_Set(path, 
							 ESIF_DATA_STRING,
							 esif_ccb_strdup((char*)DB->cache->elements[item].key.buf_ptr),
							 DB->cache->elements[item].key.data_len,
							 DB->cache->elements[item].key.data_len);

				// If no value buffer for result, just return next matching key
				if (NULL == value)
					rc = ESIF_OK;
				else
					rc = EsifConfigGet(nameSpace, path, value);
			}
		}
		else if (*context != NULL) {
			rc = ESIF_E_ITERATION_DONE;
			esif_ccb_free(*context);
			*context = NULL;
		}
		esif_ccb_read_unlock(&DB->lock);
	}
	return rc;
}


// Find First path in nameSpace
eEsifError EsifConfigFindFirst(
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value,
	EsifConfigFindContextPtr context
	)
{
	ESIF_ASSERT(nameSpace && path && context);
	*context = NULL;
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

/* Copy/Merge Keys from one NameSpace to Another */
eEsifError EsifConfigCopy(
	EsifDataPtr nameSpaceFrom,	// Source DV
	EsifDataPtr nameSpaceTo,	// Target DV
	EsifDataPtr keyspecs,		// Tab-separated Keyspec List (wildcards OK)
	esif_flags_t flags,			// Item Flags
	Bool replaceKeys)			// TRUE=COPY Keys (Replace if exists), FALSE=MERGE Keys (Do Not Replace)
{
	eEsifError rc = ESIF_OK;
	EsifConfigFindContext context = NULL;
	EsifDataPtr data_key = NULL;
	EsifDataPtr data_value = NULL;
	esif_string keylist = NULL;
	esif_string keyspec = NULL;
	esif_string keyspec_context = NULL;

	ESIF_ASSERT(nameSpaceFrom && nameSpaceTo && keyspecs && nameSpaceFrom->buf_ptr && nameSpaceTo->buf_ptr && keyspecs->buf_ptr);

	// Parse Key List (optionally Tab-separated)
	keylist = esif_ccb_strdup((esif_string)keyspecs->buf_ptr);
	if (keylist == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	// Enumerate Each Matching keyspec
	keyspec = esif_ccb_strtok(keylist, "\t", &keyspec_context);
	while (rc == ESIF_OK && keyspec != NULL) {
		data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		if ((rc = EsifConfigFindFirst(nameSpaceFrom, data_key, NULL, &context)) == ESIF_OK) {
			do {
				// copy  = always replace existing key in target if it already exists
				// merge = never replace existing key in target if it already exists
				if (replaceKeys == ESIF_TRUE || DataBank_KeyExists(g_DataBankMgr, (esif_string)nameSpaceTo->buf_ptr, (esif_string)data_key->buf_ptr) == ESIF_FALSE) {

					data_value = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
					if (data_value == NULL) {
						rc = ESIF_E_NO_MEMORY;
						break;
					}
					rc = EsifConfigGet(nameSpaceFrom, data_key, data_value);
					if (rc != ESIF_OK) {
						break;
					}
					rc = EsifConfigSet(nameSpaceTo, data_key, flags, data_value);
					if (rc != ESIF_OK) {
						break;
					}

					ESIF_TRACE_DEBUG("DV %s: @%s => @%s [%s] {%s, %u bytes}\n",
						(replaceKeys ? "Copy" : "Merge"),
						(esif_string)nameSpaceFrom->buf_ptr,
						(esif_string)nameSpaceTo->buf_ptr,
						(esif_string)data_key->buf_ptr,
						esif_data_type_str(data_value->type),
						data_value->data_len);

					EsifData_Destroy(data_value);
					data_value = NULL;
				}

				// Reset Key for next search
				EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			} while ((rc == ESIF_OK) && ((rc = EsifConfigFindNext(nameSpaceFrom, data_key, NULL, &context)) == ESIF_OK));

			EsifData_Destroy(data_key);
			data_key = NULL;
		}
		if (rc == ESIF_E_ITERATION_DONE || rc == ESIF_E_NOT_FOUND) {
			rc = ESIF_OK;
		}
		keyspec = esif_ccb_strtok(NULL, "\t", &keyspec_context);
	}

exit:
	EsifData_Destroy(data_key);
	EsifData_Destroy(data_value);
	esif_ccb_free(keylist);
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

