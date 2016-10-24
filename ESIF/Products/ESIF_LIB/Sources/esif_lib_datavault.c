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

#define _DATABANK_CLASS
#define _DATACACHE_CLASS
#define _DATAVAULT_CLASS
#define _IOSTREAM_CLASS

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_cfgmgr.h"
#include "esif_ccb_sort.h"

#include "esif_lib_datavault.h"
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


static void DataVault_ctor(DataVaultPtr self);
static void DataVault_dtor(DataVaultPtr self);

static eEsifError DataVault_ReadBlock(
	DataVaultPtr self,
	void *bufPtr,
	UInt32 bytes,
	size_t offset
	);

static eEsifError DataVault_ReadHeader(
	DataVaultPtr self,
	DataVaultHeaderPtr headerPtr
	);

static eEsifError DataVault_ReadPayload(
	DataVaultPtr self
	);
static eEsifError DataVault_GetFromSource(
	DataVaultPtr self,
	void *bufPtr,
	UInt32 bytes,
	size_t offset
	);

static eEsifError DataVault_ReadNextKeyValuePair(
	DataVaultPtr self,
	esif_flags_t *flagsPtr,
	EsifDataPtr keyPtr,
	EsifDataPtr valuePtr
	);

static eEsifError DataVault_WriteKeyValuePair(
	DataVaultPtr self,
	DataCacheEntryPtr pairPtr,
	IOStreamPtr destStreamPtr
	);

/////////////////////////////////////////////////////////////////////////
// DataVault Class

// constructor
static void DataVault_ctor(DataVaultPtr self)
{
	if (self) {
		WIPEPTR(self);
		esif_ccb_lock_init(&self->lock);
		self->cache  = DataCache_Create();
		self->stream = IOStream_Create();
	}
}


// destructor
static void DataVault_dtor(DataVaultPtr self)
{
	if (self) {
		DataCache_Destroy(self->cache);
		IOStream_Destroy(self->stream);
		esif_ccb_lock_uninit(&self->lock);
		WIPEPTR(self);
	}
}


// new operator
DataVaultPtr DataVault_Create(char* name)
{
	DataVaultPtr self = (DataVaultPtr)esif_ccb_malloc(sizeof(*self));
	DataVault_ctor(self);

	esif_ccb_strcpy(self->name, name, sizeof(self->name));
	esif_ccb_strlwr(self->name, sizeof(self->name));

	return self;
}


// delete operator
void DataVault_Destroy(DataVaultPtr self)
{
	DataVault_dtor(self);
	esif_ccb_free(self);
}


// Write DataVault to Disk
eEsifError DataVault_WriteVault(DataVaultPtr self)
{
	eEsifError rc = ESIF_OK;
	DataCacheEntryPtr pairPtr = NULL;
	IOStreamPtr diskStreamPtr = NULL;
	IOStreamPtr memStreamPtr = NULL;
	BytePtr memStreamBuffer = NULL;
	size_t memStreamBufSize = 0;
	UInt32 idx;
	DataCachePtr cacheClonePtr = NULL;

	if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC | ESIF_SERVICE_CONFIG_READONLY)) {
		rc = ESIF_E_READONLY;
		goto exit;
	}

	if (self->stream->file.name == NULL || self->cache == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	//
	// As the cache data can be modified while writing the data vault (due to
	// non-cached values), we create a clone of the current cache so that we can
	// fall back to it in case of a failure.
	//
	cacheClonePtr = DataCache_Clone(self->cache);
	if ((NULL == cacheClonePtr) && (self->cache != NULL)) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	//
	// Use a memory stream for creation before writing the final output to a file
	//
	memStreamPtr = IOStream_Create();
	if (!memStreamPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	
	if (IOStream_SetMemory(memStreamPtr, NULL, 0) != EOK) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	//
	// If any rows contain NOCACHE PERSIST values, we need to read them from the
	// original file before we overwrite it
	//
	diskStreamPtr = IOStream_Create();
	if (diskStreamPtr == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	if (IOStream_Seek(memStreamPtr, sizeof(DataVaultHeader), SEEK_CUR) != EOK) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	//
	// Write the key/value pairs to the memory stream for each data category
	// Fill in the data header for each category after the key/value pairs are written
	// Note: Only write persisted data
	//

	// Write All Persisted Rows from Sorted List to DataVault
	for (idx = 0; rc == ESIF_OK && idx < self->cache->size; idx++) {

		pairPtr = &self->cache->elements[idx];

		if (!(pairPtr->flags & ESIF_SERVICE_CONFIG_PERSIST)) {
			continue;
		}

		rc = DataVault_WriteKeyValuePair(self, pairPtr, memStreamPtr);
		if (rc != ESIF_OK) {
			goto exit;
		}
	}

	//
	// Now go back and complete the headers
	// Fill in and write the main header
	//

	if (IOStream_Open(memStreamPtr) != EOK) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	DataVaultHeader header = { 0 };
	esif_ccb_memcpy(&header.signature, ESIFDV_SIGNATURE, sizeof(header.signature));
	header.headersize = sizeof(header);
	header.version = ESIFDV_VERSION(1, 0, ESIFDV_REVISION);
	header.flags = self->flags;

	if (IOStream_Write(memStreamPtr, &header, sizeof(header)) != sizeof(header)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	memStreamBuffer = IOStream_GetMemoryBuffer(memStreamPtr);
	memStreamBufSize = IOStream_GetSize(memStreamPtr);

	//
	// Now write the memory stream to the disk
	//
	if (rc == ESIF_OK && IOStream_OpenFile(diskStreamPtr, self->stream->file.name, "wb") != EOK) {
		rc = ESIF_E_IO_OPEN_FAILED;
		goto exit;
	}

	if (rc == ESIF_OK && (IOStream_Write(diskStreamPtr, memStreamBuffer, memStreamBufSize) != memStreamBufSize)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
exit:
	IOStream_Destroy(diskStreamPtr);
	IOStream_Destroy(memStreamPtr);

	if (rc != ESIF_OK && cacheClonePtr != NULL) {
		DataCache_Destroy(self->cache);
		self->cache = cacheClonePtr;
	} else {
		DataCache_Destroy(cacheClonePtr);
	}

	return rc;
}


static eEsifError DataVault_WriteKeyValuePair(
	DataVaultPtr self,
	DataCacheEntryPtr pairPtr,
	IOStreamPtr destStreamPtr
	)
{
	eEsifError rc = ESIF_OK;
	IOStreamPtr diskStreamPtr = NULL;
	UInt8 *buffer = NULL;
	UInt32 buffer_len = 0;
	UInt32 byte = 0;
	size_t orgOffset = 0;
	size_t newOffset = 0;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(pairPtr != NULL);
	ESIF_ASSERT(destStreamPtr != NULL);

	// Expected to be open
	diskStreamPtr = self->stream;

	// Write Flags: <flags>
	if (IOStream_Write(destStreamPtr, &pairPtr->flags, sizeof(pairPtr->flags)) != sizeof(pairPtr->flags)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	// Write Key: <len><value...>
	if (IOStream_Write(destStreamPtr, &pairPtr->key.data_len, sizeof(pairPtr->key.data_len)) != sizeof(pairPtr->key.data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	if (IOStream_Write(destStreamPtr, pairPtr->key.buf_ptr, pairPtr->key.data_len) != pairPtr->key.data_len) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	// Write Value: <type><len><value...>
	if (IOStream_Write(destStreamPtr, &pairPtr->value.type, sizeof(pairPtr->value.type)) != sizeof(pairPtr->value.type)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	if (IOStream_Write(destStreamPtr, &pairPtr->value.data_len, sizeof(pairPtr->value.data_len)) != sizeof(pairPtr->value.data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	// Read NOCACHE Entries from disk file
	if (pairPtr->flags & ESIF_SERVICE_CONFIG_NOCACHE) {
		newOffset = IOStream_GetOffset(destStreamPtr);

		// Read Block from disk
		if (pairPtr->value.buf_len == 0) {
			orgOffset = (size_t)pairPtr->value.buf_ptr;
			buffer = (UInt8*)esif_ccb_malloc(pairPtr->value.data_len);
			buffer_len = pairPtr->value.data_len;
			if (!buffer) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}

			rc = DataVault_GetFromSource(self, buffer, buffer_len, orgOffset);
			if (rc != ESIF_OK) {
				goto exit;
			}
		}
		// Convert internal storage to NOCACHE
		else {
			buffer = (UInt8*)pairPtr->value.buf_ptr;
			buffer_len = pairPtr->value.data_len;
			pairPtr->value.buf_len = 0;// Set to 0 so we don't free twice
		}
		// Update pair with offset in new file
		pairPtr->value.buf_ptr = (void*)newOffset;
	}

	// Scramble Data?
	if (pairPtr->flags & ESIF_SERVICE_CONFIG_SCRAMBLE) {
		if (!buffer) {
			buffer = (UInt8*)esif_ccb_malloc(pairPtr->value.data_len);
			buffer_len = pairPtr->value.data_len;
			if (!buffer) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
		}
		for (byte = 0; byte < pairPtr->value.data_len; byte++)
			buffer[byte] = ~((UInt8*)(pairPtr->value.buf_ptr))[byte];
	}

	if (buffer) {
		if (IOStream_Write(destStreamPtr, buffer, buffer_len) != buffer_len)
			rc = ESIF_E_IO_ERROR;
	}
	else {
		if (IOStream_Write(destStreamPtr, pairPtr->value.buf_ptr, pairPtr->value.data_len) != pairPtr->value.data_len)
			rc = ESIF_E_IO_ERROR;
	}

exit:
	esif_ccb_free(buffer);
	return rc;
}


// Read DataVault from Disk
eEsifError DataVault_ReadVault(DataVaultPtr self)
{
	eEsifError rc = ESIF_OK;
	IOStreamPtr memStreamPtr = NULL;
	IOStreamPtr orgStreamPtr = NULL;
	DataVaultHeader header = {0};
	UInt32 hdrSize = 0;
	UInt32 min_version = 0;
	UInt32 max_version = 0;

	orgStreamPtr = self->stream;

	// Read the file into a memory stream for faster accesses (hash, size, etc.)
	rc = IOStream_CloneAsMemoryStream(self->stream, &memStreamPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	self->stream = memStreamPtr;

	// Read in the header
	rc = DataVault_ReadHeader(self, &header);
	if (rc != ESIF_OK) {
		goto exit;
	}

	hdrSize = header.headersize;
	if (hdrSize < sizeof(header)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	// Verify signature
	if (memcmp(header.signature, ESIFDV_SIGNATURE, sizeof(header.signature)) != 0) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	// The DataVault's Major.Minor Version must be <= the App's Major.Minor Version and at least Major.1
	min_version = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION_MIN, 0, 0);
	max_version = ESIFDV_VERSION(ESIFDV_MAJOR_VERSION, ESIFDV_MINOR_VERSION, ESIFDV_MAX_REVISION);
	if ((header.version > max_version) || (header.version < min_version)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	// Save the header flags and version
	self->flags = header.flags;
	self->version = header.version;

	rc = DataVault_ReadPayload(self);

exit:
	IOStream_Close(self->stream);
	self->stream = orgStreamPtr; // Restore the original stream
	//esif_ccb_free(dataHdrsPtr);
	IOStream_Destroy(memStreamPtr);
	return rc;
}


// Read the data vault header
static eEsifError DataVault_ReadHeader(
	DataVaultPtr self,
	DataVaultHeaderPtr headerPtr
	)
{
	eEsifError rc = ESIF_OK;
	size_t bytesRead = 0;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(headerPtr != NULL);

	if (IOStream_Open(self->stream) != 0) {
		goto exit;
	}

	bytesRead = IOStream_ReadAt(self->stream, headerPtr, sizeof(*headerPtr), 0);
	if (bytesRead !=  sizeof(*headerPtr)) {
		rc = ESIF_E_IO_ERROR;
	}
exit:
	IOStream_Close(self->stream);
	return rc;
}

static eEsifError DataVault_ReadPayload(DataVaultPtr self)
{
	eEsifError rc = ESIF_OK;
	IOStreamPtr streamPtr = NULL;
	int vrc = EPERM;
	size_t fileOffset = 0;
	size_t curFileOffset = 0;
	size_t pairSize = 0;
	esif_flags_t item_flags = 0;
	EsifData key = { ESIF_DATA_STRING };
	EsifData value = { ESIF_DATA_VOID };

	ESIF_ASSERT(self != NULL);

	streamPtr = self->stream;

	// Open File or Memory Block
	if ((vrc = IOStream_Open(streamPtr)) != 0) {
		if (vrc == ENOENT) {
			rc = ESIF_E_NOT_FOUND;
		}
		else {
			rc = ESIF_E_IO_OPEN_FAILED;
		}
		goto exit;
	}

	// Move the file pointer to start of data
	fileOffset = sizeof(DataVaultHeader);
	if (IOStream_Seek(streamPtr, fileOffset, SEEK_SET) != EOK) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	// Read Data and add to DataVault
	while (rc == ESIF_OK) {
		EsifData_ctor(&key);
		EsifData_ctor(&value);

		item_flags = 0;

		rc = DataVault_ReadNextKeyValuePair(self, &item_flags, &key, &value);
		if (rc != ESIF_OK) {
			if (ESIF_E_ITERATION_DONE == rc) {
				rc = ESIF_OK;
			}
			break;
		}

		// Validate that the data read is from within the region specified by the header
		curFileOffset = IOStream_GetOffset(streamPtr);
		pairSize = curFileOffset - fileOffset;

		fileOffset = curFileOffset;

		// Add value (including allocated buf_ptr) to cache
		DataCache_InsertValue(self->cache, (esif_string)key.buf_ptr, &value, item_flags);

		EsifData_dtor(&key);
		EsifData_dtor(&value);
	}
exit:
	IOStream_Close(streamPtr);
	EsifData_dtor(&key);
	EsifData_dtor(&value);
	return rc;
}

// Reads a key/value pair from the current location in the open DataVault stream
static eEsifError DataVault_ReadNextKeyValuePair(
	DataVaultPtr self,
	esif_flags_t *flagsPtr,
	EsifDataPtr keyPtr,
	EsifDataPtr valuePtr
	)
{
	eEsifError rc = ESIF_OK;
	IOStreamPtr vault = NULL;
	size_t bytes = 0;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(flagsPtr != NULL);
	ESIF_ASSERT(keyPtr != NULL);
	ESIF_ASSERT(valuePtr != NULL);

	vault = self->stream;

	// Read Flags
	if ((bytes = IOStream_Read(vault, flagsPtr, sizeof(*flagsPtr))) != sizeof(*flagsPtr)) {
		// Check if we are done
		if (bytes != 0) {
			rc = ESIF_E_IO_ERROR;
		}
		rc = ESIF_E_ITERATION_DONE;
		goto exit;
	}

	// Read key length
	keyPtr->type = ESIF_DATA_STRING;
	if (IOStream_Read(vault, &keyPtr->data_len, sizeof(keyPtr->data_len)) < sizeof(keyPtr->data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	if (keyPtr->data_len > MAX_DV_DATALEN) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	// Use Memory Pointers for Static DataVaults, otherwise allocate memory
	if ((IOStream_GetType(vault) == StreamMemory) && (self->flags & ESIF_SERVICE_CONFIG_STATIC)) {
		keyPtr->buf_len = 0;
		keyPtr->buf_ptr = IOStream_GetMemoryBuffer(vault) + IOStream_GetOffset(vault);
		if (IOStream_Seek(vault, keyPtr->data_len, SEEK_CUR) != EOK) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		*flagsPtr &= ~ESIF_SERVICE_CONFIG_NOCACHE;	// ignore for Static DataVaults
	}
	else {
		keyPtr->buf_len = esif_ccb_max(1, keyPtr->data_len);
		keyPtr->buf_ptr = esif_ccb_malloc(keyPtr->buf_len);
		if (!keyPtr->buf_ptr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		if (IOStream_Read(vault, keyPtr->buf_ptr, keyPtr->data_len) != keyPtr->data_len) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		else if (keyPtr->data_len) {
			((esif_string)(keyPtr->buf_ptr))[keyPtr->data_len - 1] = 0;
		}
	}

	// Read Value
	if (IOStream_Read(vault, &valuePtr->type, sizeof(valuePtr->type)) != sizeof(valuePtr->type)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	if (IOStream_Read(vault, &valuePtr->data_len, sizeof(valuePtr->data_len)) != sizeof(valuePtr->data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	if  (valuePtr->data_len > MAX_DV_DATALEN) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	// If NOCACHE mode, use buf_ptr to store the file offset of the data and skip the file
	if (*flagsPtr & ESIF_SERVICE_CONFIG_NOCACHE) {
		size_t offset = IOStream_GetOffset(vault);
		if (IOStream_Seek(vault, valuePtr->data_len, SEEK_CUR) != EOK) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		valuePtr->buf_ptr = (void*)offset; // For non-cached...we save the offset in the file as the buffer pointer. Really???
		valuePtr->buf_len = 0;	// buf_len == 0 so we don't release buffer as not allocated; data_len = original length
	} 
	else {
		// Use static pointer for static data vaults (unless scrambled), otherwise make a dynamic copy
		if ((IOStream_GetType(vault) == StreamMemory) && (self->flags & ESIF_SERVICE_CONFIG_STATIC) && !(*flagsPtr & ESIF_SERVICE_CONFIG_SCRAMBLE)) {
			valuePtr->buf_len = 0;	// static
			valuePtr->buf_ptr = IOStream_GetMemoryBuffer(vault) + IOStream_GetOffset(vault);
			if (valuePtr->buf_ptr == NULL || IOStream_Seek(vault, valuePtr->data_len, SEEK_CUR) != EOK) {
				rc = ESIF_E_IO_ERROR;
				goto exit;
			}
		} 
		else {
			valuePtr->buf_len = esif_ccb_max(1, valuePtr->data_len); // dynamic
			valuePtr->buf_ptr = esif_ccb_malloc(valuePtr->buf_len);
			if (valuePtr->buf_ptr == NULL) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
			else if (IOStream_Read(vault, valuePtr->buf_ptr, valuePtr->data_len) != valuePtr->data_len) {
				rc = ESIF_E_IO_ERROR;
				goto exit;
			}
		}
			
		//  Unscramble Data?
		if (*flagsPtr & ESIF_SERVICE_CONFIG_SCRAMBLE) {
			UInt32 byte;
			for (byte = 0; byte < valuePtr->data_len; byte++)
				((UInt8*)(valuePtr->buf_ptr))[byte] = ~((UInt8*)(valuePtr->buf_ptr))[byte];
		}
	}
exit:
	return rc;
}


// Read a Section of a DataVault from Disk
static eEsifError DataVault_GetFromSource(
	DataVaultPtr self,
	void *bufPtr,
	UInt32 bytes,
	size_t offset
	)
{
	eEsifError rc = ESIF_OK;
	size_t bytesRead = 0;

	ESIF_ASSERT(self != NULL);

	if (IOStream_Open(self->stream) != 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	bytesRead = IOStream_ReadAt(self->stream, bufPtr, bytes, offset);
	if (bytesRead != bytes) {
		rc = ESIF_E_IO_ERROR;
	}

exit:
	IOStream_Close(self->stream);
	return rc;
}


//
// Write to Transaction Log - Used for debugging only
// NOTE: This version is just a dumb text log, not a recoverable transaction log
//
static void DataVault_WriteLog(
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
	filePtr = esif_ccb_fopen(log_file.filename, "ab", NULL);
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
static eEsifError ReadFileIntoBuffer(
	esif_string filename,
	void **buffer,
	UInt32 *buf_size
	)
{
	eEsifError rc    = ESIF_OK;
	IOStreamPtr file = IOStream_Create();
	BytePtr bufPtr = NULL;
	size_t size = 0;
	size_t bytesRead = 0;

	if (IOStream_SetFile(file, filename, "rb") != EOK) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	size = IOStream_GetSize(file);

	bufPtr = (BytePtr) esif_ccb_malloc(size + 1);
	if (NULL == bufPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	if (IOStream_Open(file) != 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	bytesRead = IOStream_ReadAt(file, bufPtr, size, 0);
	if (bytesRead != size) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	bufPtr[size] = 0;

	*buffer = bufPtr;
	*buf_size = (UInt32)size;
exit:
	IOStream_Destroy(file);
	if (rc != ESIF_OK) {
		esif_ccb_free(bufPtr);
	}
	return rc;
}


// Retrieve a single value from a DataVault
eEsifError DataVault_GetValue(
	DataVaultPtr self,
	EsifDataPtr path,
	EsifDataPtr value,
	esif_flags_t *flagsPtr
	)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	DataCacheEntryPtr keypair = NULL;

	if (!self)
		return ESIF_E_PARAMETER_IS_NULL;

	if (flagsPtr)
		*flagsPtr = 0;

	// Return "keyname1|keyname2|..." if path contains "*" or "?"
	if (esif_ccb_strpbrk((esif_string)path->buf_ptr, "*?") != NULL) {
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, esif_ccb_strdup(self->name), ESIFAUTOLEN, ESIFAUTOLEN);
		EsifDataPtr key		  = EsifData_CreateAs(ESIF_DATA_STRING, path->buf_ptr, 0, ESIFAUTOLEN);
		EsifConfigFindContext context = NULL;
		esif_string keylist = NULL;
		u32 data_len = 0;
		
		// Verify valid Data Type and Data Buffer size
		if (value->type != ESIF_DATA_STRING && value->type != ESIF_DATA_AUTO) {
			rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		}
		
		if (rc == ESIF_E_NOT_FOUND && nameSpace != NULL && key != NULL && (rc = EsifConfigFindFirst(nameSpace, key, NULL, &context)) == ESIF_OK) {
			do {
				data_len += (u32)key->data_len;
				esif_string newlist = esif_ccb_realloc(keylist, data_len);
				if (newlist == NULL) {
					EsifData_Set(key, ESIF_DATA_STRING, "", 0, ESIFAUTOLEN);
					rc = ESIF_E_NO_MEMORY;
					break;
				}
				keylist = newlist;
				esif_ccb_sprintf_concat(data_len, keylist, "%s%s", (*keylist ? "|" : ""), (char *)key->buf_ptr);
				EsifData_Set(key, ESIF_DATA_STRING, path->buf_ptr, 0, ESIFAUTOLEN);
			} while ((rc = EsifConfigFindNext(nameSpace, key, NULL, &context)) == ESIF_OK);
		
			EsifConfigFindClose(&context);
			if (rc == ESIF_E_ITERATION_DONE) {
				rc = ESIF_OK;
			}
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		if (!keylist || rc != ESIF_OK) {
			esif_ccb_free(keylist);
			return rc;
		}

		// Return keylist value and data type
		if (value->type == ESIF_DATA_AUTO) {
			value->type = ESIF_DATA_STRING;
		}
		if (value->buf_len == ESIF_DATA_ALLOCATE) {
			esif_ccb_free(value->buf_ptr);
			value->buf_ptr = esif_ccb_strdup(keylist);
			value->buf_len = data_len;
			value->data_len = data_len;
		}
		else if (value->buf_len < data_len) {
			rc = ESIF_E_NEED_LARGER_BUFFER;
			value->data_len = data_len;
		}
		else if (value->buf_ptr) {
			esif_ccb_strcpy(value->buf_ptr, keylist, value->buf_len);
			value->data_len = data_len;
		}
		esif_ccb_free(keylist);
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
			if (ReadFileIntoBuffer((esif_string)buf_ptr, &buf_ptr, &data_len) != ESIF_OK) {
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
				if (DataVault_GetFromSource(self, (esif_string)value->buf_ptr, data_len, offset) != ESIF_OK) {
					data_len = 0;
					return ESIF_E_NOT_FOUND;
				}
				// Unscramble Data?
				if (keypair->flags & ESIF_SERVICE_CONFIG_SCRAMBLE) {
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

		// Return flags
		if (rc == ESIF_OK) {
			if (flagsPtr != NULL)
				*flagsPtr = keypair->flags;
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
eEsifError DataVault_SetValue(
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
			UInt32 item = 0;
			while (item < self->cache->size) {
				if (esif_ccb_strmatch((esif_string)self->cache->elements[item].key.buf_ptr, (esif_string)path->buf_ptr)) {
					flags |= FLAGS_TEST(self->cache->elements[item].flags, ESIF_SERVICE_CONFIG_PERSIST);
					if (DataCache_DeleteValue(self->cache, (esif_string)self->cache->elements[item].key.buf_ptr) == ESIF_OK) {
						continue;
					}
				}
				item++;
			}
			goto exit;
		}
		return ESIF_E_NOT_SUPPORTED; // Keys may not contain "*" or "?"
	}

	// Read data from File
	// TODO: Change Parser Logic and Syntax instead
	if (value && value->buf_ptr && esif_ccb_strncmp((char*)value->buf_ptr, "<<", 2) == 0) {
		void *buffer  = 0;
		UInt32 buflen = 0;
		if (ReadFileIntoBuffer((char*)value->buf_ptr + 2, &buffer, &buflen) == ESIF_OK) {
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
			DataCache_DeleteValue(self->cache, (esif_string)path->buf_ptr);
		} else if (value && value->buf_ptr) {
			// UPDATE
			if (keypair->value.buf_len > 0 && value->data_len != keypair->value.buf_len) {
				void *new_buf = NULL;
				u32 new_buf_len = 0;
				
				// Grow or shrink buffer if it was allocated, otherwise ask for a larger buffer
				new_buf_len = esif_ccb_max(1, value->data_len);
				new_buf= (void *)esif_ccb_realloc(keypair->value.buf_ptr, new_buf_len);

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
		EsifDataPtr valueClonePtr = NULL;

		//
		// The data passed in may be in a buffer owned elsewhere, so clone the data
		//
		valueClonePtr = EsifData_Clone(value);
		if (NULL == valueClonePtr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		DataCache_InsertValue(self->cache, (esif_string)path->buf_ptr, valueClonePtr, flags);

		EsifData_Destroy(valueClonePtr);
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
eEsifError EsifConfigGetItem(
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value,
	esif_flags_t *flagsPtr
	)
{
	eEsifError rc = ESIF_OK;
	DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, (StringPtr)(nameSpace->buf_ptr));
	if (DB) {
		esif_ccb_read_lock(&DB->lock);
		rc = DataVault_GetValue(DB, path, value, flagsPtr);
		esif_ccb_read_unlock(&DB->lock);
	}
	else {
		rc = ESIF_E_NOT_FOUND;
	}
	return rc;
}

// backwards compatibility
eEsifError EsifConfigGet(
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value
	)
{
	return EsifConfigGetItem(nameSpace, path, value, NULL);
}

// backwards compatibility
eEsifError EsifConfigSet(
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

// Delete a DataVault Key
eEsifError EsifConfigDelete(
	EsifDataPtr nameSpace,
	EsifDataPtr path
)
{
	EsifData null_value = { 0 };
	return EsifConfigSet(nameSpace, path, ESIF_SERVICE_CONFIG_DELETE, &null_value);
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
		}
		esif_ccb_read_unlock(&DB->lock);
	}

	if (rc != ESIF_OK) {
		EsifConfigFindClose(context);
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

// Close an nameSpace Iterator
void EsifConfigFindClose(
	EsifConfigFindContextPtr context
	)
{
	if (context) {
		esif_ccb_free(*context);
		*context = NULL;
	}
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
		{"SCRAMBLE",	ESIF_SERVICE_CONFIG_SCRAMBLE },
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
	Bool replaceKeys,			// TRUE=COPY Keys (Replace if exists), FALSE=MERGE Keys (Do Not Replace)
	UInt32 *keycount)			// Optional pointer to variable to hold Key Count copied/merged
{
	eEsifError rc = ESIF_OK;
	EsifConfigFindContext context = NULL;
	EsifDataPtr data_key = NULL;
	EsifDataPtr data_value = NULL;
	esif_string keylist = NULL;
	esif_string keyspec = NULL;
	esif_string keyspec_context = NULL;
	char **keyset = NULL;
	size_t keyset_count = 0;
	UInt32 exported = 0;
	esif_context_t qsort_ctx = 0;
	size_t key = 0;

	ESIF_ASSERT(nameSpaceFrom && nameSpaceTo && keyspecs && nameSpaceFrom->buf_ptr && nameSpaceTo->buf_ptr && keyspecs->buf_ptr);

	// Parse Key List (optionally Tab-separated)
	keylist = esif_ccb_strdup((esif_string)keyspecs->buf_ptr);
	if (keylist == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	// Create sorted keyset with exclude keyspecs ("!keyspec") listed first
	keyspec = esif_ccb_strtok(keylist, "\t", &keyspec_context);
	while (keyspec != NULL) {
		char **new_keyset = (char **)esif_ccb_realloc(keyset, sizeof(char *) * (keyset_count + 1));
		if (new_keyset == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		keyset = new_keyset;
		keyset[keyset_count++] = keyspec;
		keyspec = esif_ccb_strtok(NULL, "\t", &keyspec_context);
	}
	esif_ccb_qsort(keyset, keyset_count, sizeof(char *), esif_ccb_qsort_stricmp, qsort_ctx);

	// Enumerate Each Matching keyspec
	for (key = 0; (rc == ESIF_OK && key < keyset_count); key++) {

		// Skip excludes for now so we can compare to each maching keyspec later
		if (keyset[key][0] == '!') {
			continue;
		}

		EsifData_Destroy(data_key);
		data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyset[key], 0, ESIFAUTOLEN);
		if (data_key == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		if ((rc = EsifConfigFindFirst(nameSpaceFrom, data_key, NULL, &context)) == ESIF_OK) {
			do {
				// Skip if matching key matches any exclude keyspecs
				Bool skip_key = ESIF_FALSE;
				size_t ex = 0;
				for (ex = 0; (ex < key && keyset[ex][0] == '!'); ex++) {
					if (esif_ccb_strmatch((esif_string)data_key->buf_ptr, &keyset[ex][1])) {
						skip_key = ESIF_TRUE;
						break;
					}
				}

				// copy  = always replace existing key in target if it already exists
				// merge = never replace existing key in target if it already exists
				if ((skip_key == ESIF_FALSE) &&
					(replaceKeys == ESIF_TRUE || DataBank_KeyExists(g_DataBankMgr, (esif_string)nameSpaceTo->buf_ptr, (esif_string)data_key->buf_ptr) == ESIF_FALSE)) {

					EsifData_Destroy(data_value);
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

					exported++;
				}

				// Reset Key for next search
				EsifData_Set(data_key, ESIF_DATA_STRING, keyset[key], 0, ESIFAUTOLEN);
			} while ((rc = EsifConfigFindNext(nameSpaceFrom, data_key, NULL, &context)) == ESIF_OK);

			EsifConfigFindClose(&context);
		}
		if (rc == ESIF_E_ITERATION_DONE || rc == ESIF_E_NOT_FOUND) {
			rc = ESIF_OK;
		}
	}

exit:
	if (rc == ESIF_OK && keycount != NULL) {
		*keycount = exported;
	}
	EsifData_Destroy(data_key);
	EsifData_Destroy(data_value);
	esif_ccb_free(keylist);
	esif_ccb_free(keyset);
	return rc;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

