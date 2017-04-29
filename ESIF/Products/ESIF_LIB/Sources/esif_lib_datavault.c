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

// Friend Classes
#define _DATACACHE_CLASS
#define _DATAVAULT_CLASS
#define _IOSTREAM_CLASS

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_cfgmgr.h"
#include "esif_ccb_sort.h"

#include "esif_lib_esifdata.h"
#include "esif_lib_iostream.h"
#include "esif_lib_datacache.h"
#include "esif_lib_datavault.h"
#include "esif_lib_datarepo.h"
#include "esif_lib_databank.h"

#include "esif_ws_algo.h"

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

// DataVault File and Data Signatures
#define ESIFDV_HEADER_SIGNATURE			0x1FE5	// [E5 1F] = DataVault Header Signature
#define ESIFDV_ITEM_KEYS_REV0_SIGNATURE	0xA0D8	// [D8 A0] = DV 2.0 Key-Value Pair Item Signature (Revision 0)
#define ESIFDV_ITEM_KEYS_REV1_SIGNATURE	0xA1D8	// [D8 A1] = Reserved for Future Expansion (Revision 1)

// Default Version Number for new DV files
#define ESIFDV_MAJOR_VERSION        2			// 1-99:    DV Header Major Version [2.1 can read 2.0-2.1 but not 3.0]
#define ESIFDV_MINOR_VERSION        0			// 0-99:    DV Header Minor Version	[2.1 can write 2.1 but not 2.0]
#define ESIFDV_REVISION             0			// 0-65535: DV Header Revision [2.1.x and read/write 2.1.x]
#define ESIFDV_SUPPORTED_VERSION_MIN    ESIFHDR_VERSION(1, 0, 0)
#define ESIFDV_SUPPORTED_VERSION_MAX    ESIFHDR_VERSION(2, 0, 65535)

#define ESIFDV_MAX_REVISION         0xFFFF
#define ESIFDV_MAX_HEADER			0xfffe
#define ESIFDV_MAX_PAYLOAD			0x7ffffffe
#define ESIFDV_MAX_KEYLEN			0x7ffffffe
#define ESIFDV_MAX_DATALEN			0x7ffffffe
#define ESIFDV_IO_BUFFER_SIZE		(4 * 1024)

// Define valid bit flags for each DV version so we can validate header and item flags
// v1  = Unknown bit flags will fail with ESIF_E_NOT_SUPPORTED (since Item Signature is unavailable)
// v2+ = Unknown bit flags are allowed if major version matches so they can be used by future minor versions or revisions
#define ESIFDV_VALID_HEADER_FLAGS_V1	(ESIF_SERVICE_CONFIG_PERSIST|ESIF_SERVICE_CONFIG_SCRAMBLE|ESIF_SERVICE_CONFIG_READONLY|ESIF_SERVICE_CONFIG_NOCACHE|ESIF_SERVICE_CONFIG_FILELINK|ESIF_SERVICE_CONFIG_STATIC)
#define ESIFDV_VALID_ITEM_FLAGS_V1		(ESIF_SERVICE_CONFIG_PERSIST|ESIF_SERVICE_CONFIG_SCRAMBLE|ESIF_SERVICE_CONFIG_READONLY|ESIF_SERVICE_CONFIG_NOCACHE|ESIF_SERVICE_CONFIG_FILELINK)

// Flags that are ignored when setting/getting values (all versions)
#define ESIFDV_IGNORED_HEADER_FLAGS		(ESIF_SERVICE_CONFIG_DELETE|ESIF_SERVICE_CONFIG_PERSIST|ESIF_SERVICE_CONFIG_SCRAMBLE|ESIF_SERVICE_CONFIG_FILELINK)
#define ESIFDV_IGNORED_ITEM_FLAGS		(ESIF_SERVICE_CONFIG_DELETE|ESIF_SERVICE_CONFIG_STATIC|ESIF_SERVICE_CONFIG_COMPRESSED)

#pragma pack(push, 1)

// DV File Header v0.0 = Common Header Fields for all DV Versions
typedef struct DataVaultHeaderV0_s {
	UInt16  signature;		// File Signature [E5 1F]
	UInt16  headersize;		// Header Size, including signature & headersize
	UInt32  version;		// File Format Version
} DataVaultHeaderV0, *DataVaultHeaderV0Ptr;

// DV File Header v1.0.0
typedef struct DataVaultHeaderV1_s {
	UInt16	signature;		// File Signature [E5 1F]
	UInt16  headersize;		// Header Size, including signature & headersize
	UInt32  version;		// File Format Version
	UInt32  flags;			// Global Payload Flags v1
} DataVaultHeaderV1, *DataVaultHeaderV1Ptr;

// DV File Header v2.0.0
typedef struct DataVaultHeaderV2_s {
	UInt16	signature;						// File Signature [E5 1F]
	UInt16  headersize;						// Header Size, including signature & headersize
	UInt32  version;						// File Format Version
	UInt32  flags;							// Global Payload Flags v2
	char	segmentid[ESIFDV_NAME_LEN];		// Optional DV SegmentID [Cache Name] (not null terminated)
	char	comment[ESIFDV_DESC_LEN];	    // Optional DV Comment (not null terminated)
	UInt8	payload_hash[SHA1_HASH_BYTES];	// SHA1 Hash of Payload
	UInt32	payload_size;					// Payload Size
	UInt32	payload_class;					// Payload Class (default=KEYS)
} DataVaultHeaderV2, *DataVaultHeaderV2Ptr;

// DV File Header Union (all versions)
// DataVaultHeader typedef in esif_lib_datarepo.h for clang compliance
union DataVaultHeader_u {
	DataVaultHeaderV0	common;	// Common Header Parameters for All DV Versions
	DataVaultHeaderV1	v1;		// DV Version 1.0.0
	DataVaultHeaderV2	v2;		// DV Version 2.0.x
};

#pragma pack(pop)

// DataVault Import Types
typedef enum import_type {
	ImportMerge,	// Set Key/Value pair only if Key does not exit; Do not persist
	ImportCopy,		// Always set Key/Value pair even if Key already exists; Persist
} ImportType;

static void DataVault_ctor(DataVaultPtr self);
static void DataVault_dtor(DataVaultPtr self);

static esif_error_t DataVault_ReadSegment(
	DataVaultPtr self,
	DataRepoPtr repo,
	DataVaultHeaderPtr header,
	ImportType importMode
	);

static esif_error_t DataVault_ReadPayload(
	DataVaultPtr self,
	IOStreamPtr stream,
	DataVaultHeaderPtr header,
	ImportType importMode
	);

static esif_error_t DataVault_ReadKeyValuePair(
	DataVaultPtr self,
	IOStreamPtr stream,
	DataVaultHeaderPtr header,
	ImportType importMode,
	esif_flags_t *flagsPtr,
	EsifDataPtr keyPtr,
	EsifDataPtr valuePtr
	);

static esif_error_t DataVault_RepoFlush(
	DataVaultPtr self,
	IOStreamPtr payload,
	Bool compressPayload
);

static esif_error_t DataVault_WriteBuffer(
	DataVaultPtr self,
	IOStreamPtr stream,
	void *bufPtr,
	size_t bytes
);

static esif_error_t DataVault_WriteKeyValuePair(
	DataVaultPtr self,
	IOStreamPtr stream,
	DataCacheEntryPtr keyPair
	);

static Bool DataVault_IsPrimary(
	DataVaultPtr self,
	DataRepoPtr repo,
	DataVaultHeaderPtr header
	);


/////////////////////////////////////////////////////////////////////////
// DataVault Class

// static methods
Bool DataVault_IsValidSignature(UInt16 signature)
{
	UInt16 headerSignature = ESIFDV_HEADER_SIGNATURE;
	return (memcmp(&signature, &headerSignature, sizeof(signature)) == 0);
}


// constructor
static void DataVault_ctor(DataVaultPtr self)
{
	if (self) {
		WIPEPTR(self);
		esif_ccb_lock_init(&self->lock);
		self->cache  = DataCache_Create();
		self->stream = IOStream_Create();
		self->version = ESIFHDR_VERSION(ESIFDV_MAJOR_VERSION, ESIFDV_MINOR_VERSION, ESIFDV_REVISION);
		self->dataclass = ESIFDV_PAYLOAD_CLASS_KEYS;
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

// set default DataVault comment operator based on Known DataVault namespaces
// TODO: Make data-driven and remove if Comments are added to DSPs for CONFIG action namespaces
// new operator
DataVaultPtr DataVault_Create(char* name)
{
	DataVaultPtr self = (DataVaultPtr)esif_ccb_malloc(sizeof(*self));
	if (self) {
		DataVault_ctor(self);
		esif_ccb_strcpy(self->name, (name ? name : ""), sizeof(self->name));
		esif_ccb_strlwr(self->name, sizeof(self->name));
		DataVault_SetDefaultComment(self);
	}
	return self;
}


// delete operator
void DataVault_Destroy(DataVaultPtr self)
{
	DataVault_dtor(self);
	esif_ccb_free(self);
}

void DataVault_SetDefaultComment(DataVaultPtr self)
{
	if (self && self->name[0] && self->comment[0] == 0) {
		static struct {
			StringPtr name;
			StringPtr comment;
		} knownDVs[] = {
			{ "dsp",		"Device Support Packages" },
			{ "dptf",		"DPTF Policy Configuration" },
			{ "override",	"Primitive Overrides" },
			{ "startup",	"Startup Configuration" },
			{ "gddv",		"OEM BIOS-Embedded DataVault" },
			{ NULL, NULL }
		};
		int j;

		// Set default DataVault Comment if it is a known namespace (match "name", "$name", and "$$name")
		for (j = 0; knownDVs[j].name && knownDVs[j].comment; j++) {
			StringPtr segmentid = (self->segmentid[0] ? self->segmentid : self->name);
			if ((esif_ccb_stricmp(knownDVs[j].name, segmentid) == 0) ||
				(esif_ccb_strncmp(segmentid, ESIFDV_TEMP_PREFIX, esif_ccb_strlen(ESIFDV_TEMP_PREFIX, MAX_PATH)) == 0 && esif_ccb_stricmp(knownDVs[j].name, segmentid + esif_ccb_strlen(ESIFDV_TEMP_PREFIX, MAX_PATH)) == 0) ||
				(esif_ccb_strncmp(segmentid, ESIFDV_EXPORT_PREFIX, esif_ccb_strlen(ESIFDV_EXPORT_PREFIX, MAX_PATH)) == 0 && esif_ccb_stricmp(knownDVs[j].name, segmentid + esif_ccb_strlen(ESIFDV_EXPORT_PREFIX, MAX_PATH)) == 0)) {
				esif_ccb_strncpy(self->comment, knownDVs[j].comment, sizeof(self->comment));
			}
		}
	}
}

// Flush a DataVault Repo to Disk using optional Payload
static esif_error_t DataVault_RepoFlush(
	DataVaultPtr self,
	IOStreamPtr payload,
	Bool compressPayload
)
{
	esif_error_t rc = ESIF_OK;
	DataCacheEntryPtr keyPair = NULL;
	UInt32 idx;
	IOStreamPtr tempStream = NULL;
	char tempName[MAX_PATH] = { 0 };
	UInt16 headerSignature = ESIFDV_HEADER_SIGNATURE;
	DataVaultHeader header = { 0 };
	size_t rewind_pos = 0;
	BytePtr buffer = NULL;
	Bool payload_compressed = ESIF_FALSE;

	// Cannot flush Static or ReadOnly Repos
	if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC|ESIF_SERVICE_CONFIG_READONLY) ||
		(self->stream != NULL && self->stream->base.type != StreamNull && self->stream->base.store == StoreStatic)) {
		rc = ESIF_E_READONLY;
		goto exit;
	}

	// Set Stream to File if this is the first persisted write for this DataVault
	if (self->stream == NULL || self->stream->type == StreamNull) {
		char filename[MAX_PATH] = { 0 };
		StringPtr fileExt = (payload ? ESIFDV_REPOEXT : ESIFDV_FILEEXT);
		esif_build_path(filename, sizeof(filename), ESIF_PATHTYPE_DV, self->name, fileExt);
		IOStream_SetFile(self->stream, StoreReadWrite, filename, "rb");
		
		// Always create new .dv files using the latest version
		if (ESIFHDR_GET_MAJOR(self->version) < ESIFDV_MAJOR_VERSION) {
			self->version = ESIFHDR_VERSION(ESIFDV_MAJOR_VERSION, ESIFDV_MINOR_VERSION, ESIFDV_REVISION);
		}
	}

	if (self->cache == NULL || self->stream == NULL || self->stream->type != StreamFile || self->stream->file.name == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_sha1_init(&self->digest);

	//
	// Build the Initial DV Header and Write it
	// Use all-zeros for DV 2.0 Payload Hash and Size and Update them after writing the Payload
	// An all-zeros Hash is invalid since SHA1 Hash for an empty payload is DA39A3EE5E6B4B0D3255BFEF95601890AFD80709
	// 
	UInt32 major_version = ESIFHDR_GET_MAJOR(self->version);
	StringPtr segmentid = (self->segmentid[0] ? self->segmentid : self->name);
	StringPtr comment = self->comment;

	if (major_version == 1) {
		esif_ccb_memcpy(&header.v1.signature, &headerSignature, sizeof(header.v1.signature));
		header.v1.headersize = (UInt16)sizeof(DataVaultHeaderV1);
		header.v1.version = self->version;
		header.v1.flags = self->flags;
	}
	else if (major_version == 2) {
		esif_ccb_memcpy(&header.v2.signature, &headerSignature, sizeof(header.v2.signature));
		header.v2.headersize = (UInt16)sizeof(DataVaultHeaderV2);
		header.v2.version = self->version;
		header.v2.flags = self->flags;
		esif_ccb_strmemcpy(header.v2.segmentid, sizeof(header.v2.segmentid), segmentid, esif_ccb_strlen(segmentid, sizeof(header.v2.segmentid)));
		esif_ccb_strmemcpy(header.v2.comment, sizeof(header.v2.comment), comment, esif_ccb_strlen(comment, sizeof(header.v2.comment)));
		esif_ccb_memset(header.v2.payload_hash, 0, sizeof(header.v2.payload_hash));
		header.v2.payload_size = 0;
		header.v2.payload_class = self->dataclass;
	}
	else {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	// Write all output to a temporary file so that DV is not corrupted in the event of I/O failure
	tempStream = IOStream_Create();
	if (tempStream == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	esif_ccb_sprintf(sizeof(tempName), tempName, "%s.tmp", self->stream->file.name);

	// Open stream and write the incomplete Header, saving the current file position for Rewind
	if (IOStream_OpenFile(tempStream, StoreReadWrite, tempName, "wb") != EOK) {
		rc = ESIF_E_IO_OPEN_FAILED;
		goto exit;
	}
	rewind_pos = IOStream_GetOffset(tempStream);

	if (IOStream_Write(tempStream, &header, (size_t)header.common.headersize) != (size_t)header.common.headersize) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	// Write Repo Stream if provided
	if (payload != NULL) {
		size_t bytes_read = 0;
		size_t bytes_written = 0;
		size_t buf_len = (4 * 1024);

		// Read in entire stream at once if compressing payload
		if (major_version < 2) {
			compressPayload = ESIF_FALSE;
		}
		if (compressPayload) {
			buf_len = IOStream_GetSize(payload);
			if (buf_len < 1) {
				rc = ESIF_E_IO_ERROR;
				goto exit;
			}
		}

		buffer = esif_ccb_malloc(buf_len);
		if (buffer == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		if (IOStream_Open(payload) != EOK) {
			rc = ESIF_E_IO_OPEN_FAILED;
			goto exit;
		}

		// Copy Payload Stream contents to DataVault and add to SHA1 Hash
		while ((rc == ESIF_OK) && ((bytes_read = IOStream_Read(payload, buffer, buf_len)) > 0)) {
			if (compressPayload) {
				EsifData payload_buffer = { ESIF_DATA_BLOB, buffer, (u32)buf_len, (u32)buf_len };
				if ((rc = EsifData_Compress(&payload_buffer)) == ESIF_OK) {
					buffer = payload_buffer.buf_ptr;
					buf_len = payload_buffer.buf_len;
					bytes_read = payload_buffer.data_len;
				}
			}

			// Set Payload Compression Flag if data is Compressed
			if (rc == ESIF_OK) {
				EsifData streamData = { ESIF_DATA_BLOB, buffer, (u32)buf_len, (u32)bytes_read };
				if (bytes_written == 0 && EsifData_IsCompressed(&streamData)) {
					payload_compressed = ESIF_TRUE;
				}
				rc = DataVault_WriteBuffer(self, tempStream, buffer, bytes_read);
				bytes_written += bytes_read;
			}
		}

		IOStream_Close(payload);
		if (rc != ESIF_OK) {
			goto exit;
		}
	}
	// Otherwise Write All Persisted Key/Value Pairs from Sorted Cache to DataVault and compute Hash
	else if (self->dataclass == ESIFDV_PAYLOAD_CLASS_KEYS) {
		for (idx = 0; rc == ESIF_OK && idx < self->cache->size; idx++) {

			keyPair = &self->cache->elements[idx];

			if (!FLAGS_TEST(keyPair->flags, ESIF_SERVICE_CONFIG_PERSIST)) {
				continue;
			}

			// Persisted Compressed KEYS Payloads are not supported
			if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_COMPRESSED)) {
				FLAGS_CLEAR(self->flags, ESIF_SERVICE_CONFIG_COMPRESSED);
			}

			rc = DataVault_WriteKeyValuePair(self, tempStream, keyPair);
			if (rc != ESIF_OK) {
				goto exit;
			}
		}
	}


	esif_sha1_finish(&self->digest);

	// Rewind and Update the DV 2.0 Header with computed Payload Hash and Size
	if (major_version == 2) {
		if (payload_compressed) {
			FLAGS_SET(header.v2.flags, ESIF_SERVICE_CONFIG_COMPRESSED);
		}
		else {
			FLAGS_CLEAR(header.v2.flags, ESIF_SERVICE_CONFIG_COMPRESSED);
		}
		esif_ccb_memcpy(header.v2.payload_hash, self->digest.hash, sizeof(self->digest.hash));
		header.v2.payload_size = (UInt32)(self->digest.digest_bits / 8);
		size_t current_pos = IOStream_GetOffset(tempStream);

		if ((IOStream_Seek(tempStream, rewind_pos, SEEK_SET) < 0) ||
			(IOStream_Write(tempStream, &header, (size_t)header.common.headersize) != (size_t)header.common.headersize) ||
			(IOStream_Seek(tempStream, current_pos, SEEK_SET) < 0)) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
	}

exit:
	IOStream_Destroy(tempStream);
	esif_ccb_free(buffer);

	// Replace existing File if successful
	if (rc == ESIF_OK) {
		if (esif_ccb_file_exists(self->stream->file.name) && esif_ccb_unlink(self->stream->file.name) != EOK) {
			IGNORE_RESULT(esif_ccb_unlink(tempName));
			rc = ESIF_E_IO_DELETE_FAILED;
		}
		if (rc == ESIF_OK) {
			IGNORE_RESULT(esif_ccb_rename(tempName, self->stream->file.name));
		}
	}
	else {
		IGNORE_RESULT(esif_ccb_unlink(tempName));
	}
	return rc;
}

// Write a buffer to the payload and add it to the SHA1 digest
static esif_error_t DataVault_WriteBuffer(
	DataVaultPtr self,
	IOStreamPtr stream,
	void *bufPtr,
	size_t bytes)
{
	if (IOStream_Write(stream, bufPtr, bytes) != bytes) {
		return ESIF_E_IO_ERROR;
	}
	esif_sha1_update(&self->digest, bufPtr, bytes);
	return ESIF_OK;
}

// Write a single Key/Value Pair to the DataVault Payload and add it to the SHA1 Digest
static esif_error_t DataVault_WriteKeyValuePair(
	DataVaultPtr self,
	IOStreamPtr stream,
	DataCacheEntryPtr keyPair
)
{
	esif_error_t rc = ESIF_OK;
	UInt8 *buffer = NULL;
	size_t buffer_len = 0;
	UInt32 byte = 0;
	size_t orgOffset = 0;
	size_t newOffset = 0;
	UInt16 itemSignature = ESIFDV_ITEM_KEYS_REV0_SIGNATURE;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(keyPair != NULL);
	ESIF_ASSERT(stream != NULL);

	// Write Item Signature: [D8 A0]
	UInt32 major_version = ESIFHDR_GET_MAJOR(self->version);
	if (major_version == 2) {
		if (IOStream_Write(stream, &itemSignature, sizeof(itemSignature)) != sizeof(itemSignature)) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		esif_sha1_update(&self->digest, &itemSignature, sizeof(itemSignature));
	}

	// Write Flags: <flags>
	if (IOStream_Write(stream, &keyPair->flags, sizeof(keyPair->flags)) != sizeof(keyPair->flags)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, &keyPair->flags, sizeof(keyPair->flags));

	// Write Key: <len><value...>
	if (IOStream_Write(stream, &keyPair->key.data_len, sizeof(keyPair->key.data_len)) != sizeof(keyPair->key.data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, &keyPair->key.data_len, sizeof(keyPair->key.data_len));

	if (IOStream_Write(stream, keyPair->key.buf_ptr, keyPair->key.data_len) != keyPair->key.data_len) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, keyPair->key.buf_ptr, keyPair->key.data_len);

	// Write Value: <type><len><value...>
	if (IOStream_Write(stream, &keyPair->value.type, sizeof(keyPair->value.type)) != sizeof(keyPair->value.type)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, &keyPair->value.type, sizeof(keyPair->value.type));

	if (IOStream_Write(stream, &keyPair->value.data_len, sizeof(keyPair->value.data_len)) != sizeof(keyPair->value.data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, &keyPair->value.data_len, sizeof(keyPair->value.data_len));

	// Read NOCACHE Entries from disk file
	if (FLAGS_TEST(keyPair->flags, ESIF_SERVICE_CONFIG_NOCACHE) && self->stream->type != StreamNull) {
		newOffset = IOStream_GetOffset(stream);

		// Read Block from disk
		if (keyPair->value.buf_len == 0) {
			orgOffset = (size_t)keyPair->value.buf_ptr;
			buffer = (UInt8*)esif_ccb_malloc(keyPair->value.data_len);
			buffer_len = (size_t)keyPair->value.data_len;
			if (!buffer) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}

			if (IOStream_LoadBlock(self->stream, buffer, buffer_len, orgOffset) != EOK) {
				rc = ESIF_E_IO_ERROR;
				goto exit;
			}
		}
		// Convert internal storage to NOCACHE
		else {
			buffer = (UInt8*)keyPair->value.buf_ptr;
			buffer_len = (size_t)keyPair->value.data_len;
			keyPair->value.buf_len = 0;// Set to 0 so we don't free twice
		}
		// Update pair with offset in new file
		keyPair->value.buf_ptr = (void*)newOffset;
	}

	// Scramble Data?
	if (FLAGS_TEST(keyPair->flags, ESIF_SERVICE_CONFIG_SCRAMBLE)) {
		if (!buffer) {
			buffer = (UInt8*)esif_ccb_malloc(keyPair->value.data_len);
			buffer_len = (size_t)keyPair->value.data_len;
			if (!buffer) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
		}
		for (byte = 0; byte < keyPair->value.data_len; byte++)
			buffer[byte] = ~((UInt8*)(keyPair->value.buf_ptr))[byte];
	}

	if (buffer) {
		if (IOStream_Write(stream, buffer, buffer_len) != buffer_len)
			rc = ESIF_E_IO_ERROR;
		esif_sha1_update(&self->digest, buffer, buffer_len);
	}
	else {
		if (IOStream_Write(stream, keyPair->value.buf_ptr, keyPair->value.data_len) != keyPair->value.data_len)
			rc = ESIF_E_IO_ERROR;
		esif_sha1_update(&self->digest, keyPair->value.buf_ptr, keyPair->value.data_len);
	}

exit:
	esif_ccb_free(buffer);
	return rc;
}

// True if the optional segmentid in the Segment header matches the DataVault name
static Bool DataVault_IsSegmentMatch(
	DataVaultPtr self,
	DataVaultHeaderPtr header
)
{
	Bool rc = ESIF_FALSE;

	if (self && header) {
		rc = ESIF_TRUE;
		if (header->common.version == 2 && header->v2.segmentid[0]) {
			if (esif_ccb_strnicmp(self->name, header->v2.segmentid, sizeof(header->v2.segmentid)) != 0) {
				rc = ESIF_FALSE;
			}
		}
	}
	return rc;
}

// Validate SHA1 Hash with the one in the Header
// This function is called twice: First to validate Hash before loading Payload, Second to log an error after loading Payload
static esif_error_t DataVault_ValidateHash(
	DataVaultPtr self,
	DataVaultHeaderPtr header
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && header) {
		rc = ESIF_OK;

		// NOTE: SHA1 Hash of empty payload (0 bytes) = DA39A3EE5E6B4B0D3255BFEF95601890AFD80709
		if (ESIFHDR_GET_MAJOR(header->common.version) == 2) {
			if ((memcmp(header->v2.payload_hash, self->digest.hash, sizeof(header->v2.payload_hash)) != 0) || (header->v2.payload_size != (UInt32)(self->digest.digest_bits / 8))) {
				ESIF_TRACE_ERROR("Invalid DV Payload Hash (%s)\n"
					"  Expected = 0x%08X%08X%08X%08X%08X (%d bytes)\n"
					"  Actual   = 0x%08X%08X%08X%08X%08X (%d bytes)\n",
					self->name,
					esif_ccb_htonl(((UInt32*)header->v2.payload_hash)[0]),
					esif_ccb_htonl(((UInt32*)header->v2.payload_hash)[1]),
					esif_ccb_htonl(((UInt32*)header->v2.payload_hash)[2]),
					esif_ccb_htonl(((UInt32*)header->v2.payload_hash)[3]),
					esif_ccb_htonl(((UInt32*)header->v2.payload_hash)[4]),
					header->v2.payload_size,
					esif_ccb_htonl(((UInt32*)self->digest.hash)[0]),
					esif_ccb_htonl(((UInt32*)self->digest.hash)[1]),
					esif_ccb_htonl(((UInt32*)self->digest.hash)[2]),
					esif_ccb_htonl(((UInt32*)self->digest.hash)[3]),
					esif_ccb_htonl(((UInt32*)self->digest.hash)[4]),
					(UInt32)(self->digest.digest_bits / 8)
				);
				rc = ESIF_E_IO_HASH_FAILED;
			}
		}
	}
	return rc;
}

// Validate Payload Class, Size, and Hash before loading into Cache
static esif_error_t DataVault_ValidatePayload(
	DataVaultPtr self,
	DataRepoPtr repo,
	DataVaultHeaderPtr header
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	
	if (self && repo && repo->stream && header) {
		UInt32 major_version = ESIFHDR_GET_MAJOR(header->common.version);
		UInt32 payload_class = ESIFDV_PAYLOAD_CLASS_KEYS;
		size_t payload_size = 0;
		size_t total_bytes = 0;

		// Only KEYS Payloads are supported for v1 Headers
		rc = ESIF_OK;
		if (major_version == 2) {
			// Bounds check
			if (header->v2.payload_size > ESIFDV_MAX_PAYLOAD) {
				rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			}
			else {
				payload_size = (size_t)header->v2.payload_size;
				payload_class = header->v2.payload_class;
			}
		}

		// Ignore (Read and Discard) Unsupported Payload Classes
		if (rc == ESIF_OK) {
			switch (payload_class) {
			case ESIFDV_PAYLOAD_CLASS_KEYS:	// DV 2.0 Key/Value Pair List [Revisions 0xA0D8-0xFFD8]
			case ESIFDV_PAYLOAD_CLASS_REPO:	// DV 2.0 Data Repository [Revision 0x1FE5 only]
				rc = ESIF_OK;
				break;

			case ESIFDV_PAYLOAD_CLASS_NULL: // Undefined Payload Class
			default:
				rc = ESIF_E_NOT_SUPPORTED;
				if (payload_size) {
					if (IOStream_Seek(repo->stream, payload_size, SEEK_CUR) == EOK) {
						rc = ESIF_E_ITERATION_DONE;
					}
				}
				break;
			}
		}

		// Verify SHA1 Hash before processing Payload
		if (rc == ESIF_OK && major_version == 2) {
			size_t offset = IOStream_GetOffset(repo->stream);
			size_t buffer_size = ESIFDV_IO_BUFFER_SIZE;
			size_t bytes_to_read = payload_size;
			size_t bytes_read = 0;
			unsigned char *buffer = esif_ccb_malloc(buffer_size);

			esif_sha1_init(&self->digest);

			if (buffer == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				// Read raw Payload data to compute SHA1 Hash
				while ((bytes_to_read > 0) && ((bytes_read = IOStream_Read(repo->stream, buffer, esif_ccb_min(bytes_to_read, buffer_size))) > 0)) {
					esif_sha1_update(&self->digest, buffer, bytes_read);
					bytes_to_read -= bytes_read;
					total_bytes += bytes_read;
				}
				esif_ccb_free(buffer);
				
				if (rc == ESIF_OK && bytes_to_read != 0) {
					rc = ESIF_E_IO_ERROR;
				}
			}
			esif_sha1_finish(&self->digest);

			// Verify SHA1 Hash and Payload Size
			if (rc == ESIF_OK) {
				rc = DataVault_ValidateHash(self, header);
			}

			// Rewind to start of Payload if Validated
			if (rc == ESIF_OK) {
				if (IOStream_Seek(repo->stream, offset, SEEK_SET) != EOK) {
					rc = ESIF_E_IO_ERROR;
				}
			}
		}
	}
	return rc;
}

// Load a DataRepo Segment into a DataVault using the optional header
static esif_error_t DataVault_ReadSegment(
	DataVaultPtr self,
	DataRepoPtr repo,
	DataVaultHeaderPtr header,
	ImportType importMode
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	DataVaultHeader segmentHeader = { 0 };
	esif_sha1_t currentDigest = { 0 };
	Bool restoreDigest = ESIF_TRUE;

	if (repo && repo->stream) {
		// Read Header from stream if none is provided
		if (header == NULL) {
			if ((rc = DataRepo_ReadHeader(repo, &segmentHeader)) != ESIF_OK) {
				return rc;
			}
			header = &segmentHeader;
		}

		// Set DataVault Header values if this is the Primary Stream
		Bool IsEmptyDigest = (memcmp(&self->digest, &currentDigest, sizeof(currentDigest)) == 0);
		UInt32 major_version = ESIFHDR_GET_MAJOR(header->common.version);
		if (major_version == 1) {
			if (IsEmptyDigest || importMode == ImportCopy) {
				self->version = header->common.version;
				self->flags = header->v1.flags;
				self->dataclass = ESIFDV_PAYLOAD_CLASS_KEYS;
				restoreDigest = ESIF_FALSE;
			}
		}
		else if (major_version == 2) {
			if (IsEmptyDigest || (DataVault_IsSegmentMatch(self, header) && importMode == ImportCopy)) {
				self->version = header->common.version;
				self->flags = header->v2.flags;
				esif_ccb_strmemcpy(self->segmentid, sizeof(self->segmentid), header->v2.segmentid, sizeof(header->v2.segmentid));
				esif_ccb_strmemcpy(self->comment, sizeof(self->comment), header->v2.comment, sizeof(header->v2.comment));
				self->dataclass = header->v2.payload_class;
				restoreDigest = ESIF_FALSE;
			}
		}
		else {
			return ESIF_E_NOT_SUPPORTED;
		}

		// Validate Payload Class and SHA1 Hash before loading into Cache
		esif_ccb_memcpy(&currentDigest, &self->digest, sizeof(currentDigest));
		rc = DataVault_ValidatePayload(self, repo, header);

		// Read and Process Payload
		if (rc == ESIF_OK) {
			rc = DataVault_ReadPayload(self, repo->stream, header, importMode);
		}
		
		if (rc == ESIF_E_ITERATION_DONE) {
			rc = ESIF_OK;
		}
		if (restoreDigest) {
			esif_ccb_memcpy(&self->digest, &currentDigest, sizeof(currentDigest));
		}
	}
	return rc;
}

// Import a (closed) IOStream into the given DataVault in ImportCopy mode, overwriting any existing keys
// This function assumes the stream has been assigned by the caller but not opened.
esif_error_t DataVault_ImportStream(
	DataVaultPtr self,
	IOStreamPtr stream
)
{
	esif_error_t rc = ESIF_OK;
	DataVaultHeader header = { 0 };
	DataRepo repo = { 0 };

	esif_ccb_write_lock(&self->lock);

	esif_ccb_strcpy(repo.name, self->name, sizeof(repo.name));
	repo.stream = (stream ? stream : self->stream);

	// Open the DataVault Repo
	if (IOStream_Open(repo.stream) != 0) {
		IOStream_dtor(self->stream);
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	// Copy only the first Segment from the Repo into this DataVault, ignoring Segment Name in Header
	rc = DataRepo_ReadHeader(&repo, &header);
	if (rc == ESIF_OK) {
		rc = DataVault_ReadSegment(self, &repo, &header, ImportCopy);

		// Mark DataVault Stream as Read-Only if this Repo has more than one segment
		if (rc == ESIF_OK) {
			if (DataRepo_ReadHeader(&repo, &header) == ESIF_OK && self->stream->base.store == StoreReadWrite) {
				self->stream->base.store = StoreReadOnly;
			}
		}
	}

exit:
	IOStream_Close(repo.stream);
	esif_ccb_write_unlock(&self->lock);
	return rc;
}

// Read and Process the Segment Payload
static esif_error_t DataVault_ReadPayload(
	DataVaultPtr self,
	IOStreamPtr stream,
	DataVaultHeaderPtr header,
	ImportType importMode
	)
{
	esif_error_t rc = ESIF_OK;
	UInt32 payload_class = ESIFDV_PAYLOAD_CLASS_KEYS;
	size_t payload_size = 0;
	esif_flags_t payload_flags = 0;
	EsifDataPtr payload = NULL;
	IOStreamPtr uncompressed_stream = NULL;

	if (NULL == self || NULL == header) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (ESIFHDR_GET_MAJOR(header->common.version) == 2) {
		payload_flags = (esif_flags_t) header->v2.flags;
		payload_size = (size_t) header->v2.payload_size;
		payload_class = header->v2.payload_class;
	}

	// Decompress Payload if it is compressed
	if (FLAGS_TEST(payload_flags, ESIF_SERVICE_CONFIG_COMPRESSED)) {
		BytePtr payload_buffer = esif_ccb_malloc(payload_size);

		if (payload_buffer == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if (IOStream_Read(stream, payload_buffer, payload_size) != payload_size) {
			rc = ESIF_E_IO_ERROR;
		}
		else {
			payload = EsifData_CreateAs(ESIF_DATA_BLOB, payload_buffer, (u32)payload_size, (u32)payload_size);

			if (payload == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				payload_buffer = NULL; // Now owned by payload

				if ((rc = EsifData_Decompress(payload)) == ESIF_OK) {
					uncompressed_stream = IOStream_Create();
					if (uncompressed_stream == NULL) {
						rc = ESIF_E_NO_MEMORY;
					}
					else {
						IOStream_SetMemory(
							uncompressed_stream,
							StoreStatic,
							payload->buf_ptr,
							payload->data_len);

						stream = uncompressed_stream;
						payload_size = payload->data_len;
					}
				}
			}
		}
		if (rc != ESIF_OK) {
			esif_ccb_free(payload_buffer);
			goto exit;
		}
	}

	switch (payload_class) {

	// KEYS = DataVault 2.0 Key/Value Pair List [Revisions 0xA0D8-0xFFD8]
	case ESIFDV_PAYLOAD_CLASS_KEYS:
	{
		size_t fileOffset = 0;
		size_t curFileOffset = 0;
		size_t pairSize = 0;
		esif_flags_t item_flags = 0;
		EsifData key = { ESIF_DATA_STRING };
		EsifData value = { ESIF_DATA_VOID };

		esif_sha1_init(&self->digest);

		// Read Key/Value Pair Payload into DataVault Cache
		while (rc == ESIF_OK) {
			EsifData_ctor(&key);
			EsifData_ctor(&value);

			// Set Item Flags that are banned for Compressed streams
			item_flags = 0;
			if (uncompressed_stream != NULL) {
				FLAGS_SET(item_flags, ESIF_SERVICE_CONFIG_NOCACHE);
			}

			rc = DataVault_ReadKeyValuePair(self, stream, header, importMode, &item_flags, &key, &value);
			if (rc != ESIF_OK) {
				if (ESIF_E_ITERATION_DONE == rc) {
					rc = ESIF_OK;
				}
				break;
			}

			// Validate that the data read is from within the region specified by the header
			curFileOffset = IOStream_GetOffset(stream);
			pairSize = curFileOffset - fileOffset;

			fileOffset = curFileOffset;

			// Add value (including allocated buf_ptr) to cache
			if (importMode == ImportMerge) {
				if (DataCache_GetValue(self->cache, (esif_string)key.buf_ptr) == NULL) {
					FLAGS_CLEAR(item_flags, ESIF_SERVICE_CONFIG_PERSIST | ESIF_SERVICE_CONFIG_NOCACHE | ESIF_SERVICE_CONFIG_SCRAMBLE);
					rc = DataCache_InsertValue(self->cache, (esif_string)key.buf_ptr, &value, item_flags);
				}
			}
			else if (importMode == ImportCopy) {
				if (DataCache_GetValue(self->cache, (esif_string)key.buf_ptr) != NULL) {
					rc = DataCache_DeleteValue(self->cache, (esif_string)key.buf_ptr);
				}
				if (rc == ESIF_OK) {
					rc = DataCache_InsertValue(self->cache, (esif_string)key.buf_ptr, &value, item_flags);
				}
			}
			EsifData_dtor(&key);
			EsifData_dtor(&value);
		}

		// Re-Validate SHA1 Hash after loading Payload
		// TODO: Cannot undo Cache changes if this fails unless DataVault Transaction support is added
		esif_sha1_finish(&self->digest);
		if (rc == ESIF_OK && !FLAGS_TEST(payload_flags, ESIF_SERVICE_CONFIG_COMPRESSED)) {
			rc = DataVault_ValidateHash(self, header);
		}

		EsifData_dtor(&key);
		EsifData_dtor(&value);
	}
	break;

	// REPO = Data Repository containing DV 2.0 Segment(s) [Revision 0x1FE5 only]
	case ESIFDV_PAYLOAD_CLASS_REPO:
	{
		// Create a Temporary Repo Object and Load it into DataCache(s)
		DataRepoPtr repo = DataRepo_CreateAs(StreamNull, StoreReadOnly, "##repo");
		if (repo && payload_size > 0) {
			BytePtr repo_buffer = NULL;

			// Use existing memory buffer if stream is a Memory Stream
			if (stream->base.type == StreamMemory) {
				if (stream->memory.offset + payload_size > stream->memory.buf_len) {
					rc = ESIF_E_IO_ERROR;
				}
				else {
					IOStream_SetMemory(
						repo->stream,
						StoreReadOnly,
						(stream->memory.buffer + stream->memory.offset),
						payload_size);

					IOStream_Seek(stream, payload_size, SEEK_CUR);
				}
			}
			// Load Payload into Memory if stream is a File Stream
			else {
				repo_buffer = esif_ccb_malloc(payload_size);
				if (repo_buffer == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else if (IOStream_Read(stream, repo_buffer, payload_size) != payload_size) {
					rc = ESIF_E_IO_ERROR;
				}
				else {
					IOStream_SetMemory(
						repo->stream,
						StoreReadOnly,
						repo_buffer,
						payload_size);
				}
			}

			if (rc == ESIF_OK) {
				rc = DataRepo_LoadSegments(repo);
			}
			esif_ccb_free(repo_buffer);
		}
		DataRepo_Destroy(repo);
	}
	break;

	// Unsupported Payload Class
	case ESIFDV_PAYLOAD_CLASS_NULL:
	default:
	{
		rc = ESIF_E_NOT_SUPPORTED;
		if (payload_size) {
			if (IOStream_Seek(stream, payload_size, SEEK_CUR) == EOK) {
				rc = ESIF_E_ITERATION_DONE;
			}
		}
	}
	break;
	}

exit:
	IOStream_Destroy(uncompressed_stream);
	EsifData_Destroy(payload);
	return rc;
}

// Reads a key/value pair from the current location in the open DataVault stream
static esif_error_t DataVault_ReadKeyValuePair(
	DataVaultPtr self,
	IOStreamPtr stream,
	DataVaultHeaderPtr header,
	ImportType importMode,
	esif_flags_t *flagsPtr,
	EsifDataPtr keyPtr,
	EsifDataPtr valuePtr
	)
{
	esif_error_t rc = ESIF_OK;
	size_t bytes = 0;
	size_t rewind_pos = 0;
	UInt16 headerSignature = ESIFDV_HEADER_SIGNATURE;
	esif_flags_t bannedFlags = 0;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(stream != NULL);
	ESIF_ASSERT(flagsPtr != NULL);
	ESIF_ASSERT(keyPtr != NULL);
	ESIF_ASSERT(valuePtr != NULL);

	bannedFlags = *flagsPtr;
	rewind_pos = IOStream_GetOffset(stream);

	// Read v2 Item Signature
	UInt32 major_version = ESIFHDR_GET_MAJOR(header->common.version);
	if (major_version == 2) {
		UInt16 itemSignature = ESIFDV_ITEM_KEYS_REV0_SIGNATURE;
		UInt16 thisSignature = 0;

		// Do not read past Payload Size, if defined
		size_t bytes_read = ((size_t)(self->digest.digest_bits / 8)) + ((size_t)(self->digest.blockbytes));
		if (!FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_COMPRESSED) && bytes_read >= header->v2.payload_size) {
			rc = ESIF_E_ITERATION_DONE;
			goto exit;
		}

		// Read v2 Item Signature
		if ((bytes = IOStream_Read(stream, &thisSignature, sizeof(thisSignature))) != sizeof(thisSignature)) {
			rc = (bytes == 0 ? ESIF_E_ITERATION_DONE : ESIF_E_IO_ERROR);
			goto exit;
		}

		// If Header Signature found, Rewind to Item Start so next DV can be read
		if (thisSignature == headerSignature) {
			IOStream_Seek(stream, rewind_pos, SEEK_SET);
			rc = ESIF_E_ITERATION_DONE;
			goto exit;
		}

		// Validate Item Signature
		if (thisSignature != itemSignature) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		esif_sha1_update(&self->digest, &thisSignature, sizeof(thisSignature));
	}

	// Read Item Flags
	if ((bytes = IOStream_Read(stream, flagsPtr, sizeof(*flagsPtr))) != sizeof(*flagsPtr)) {
		rc = (bytes == 0 ? ESIF_E_ITERATION_DONE : ESIF_E_IO_ERROR);
		goto exit;
	}

	// For v1, detect end-of-payload by detecting either a valid DV Header Signature [E5 1F] or unknown Item Flags
	// For v2, unknown Item Flags are allowed since they may be used by future minor versions or revisions
	if (major_version == 1) {
		if ((UInt16)(*flagsPtr) == headerSignature) {
			IOStream_Seek(stream, rewind_pos, SEEK_SET);
			rc = ESIF_E_ITERATION_DONE;
			goto exit;
		}
		// Fail v1 DV Read if any unknkown Item Flags are detected, since this most likely indicates corrupted/invalid data
		if (!FLAGS_TESTVALID(*flagsPtr, ESIFDV_VALID_ITEM_FLAGS_V1)) {
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}
	}
	FLAGS_CLEAR(*flagsPtr, ESIFDV_IGNORED_ITEM_FLAGS);
	FLAGS_CLEAR(*flagsPtr, bannedFlags);
	esif_sha1_update(&self->digest, flagsPtr, sizeof(*flagsPtr));

	// Read key length
	keyPtr->type = ESIF_DATA_STRING;
	if (IOStream_Read(stream, &keyPtr->data_len, sizeof(keyPtr->data_len)) < sizeof(keyPtr->data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, &keyPtr->data_len, sizeof(keyPtr->data_len));

	// Read Key after bounds check
	if (keyPtr->data_len > ESIFDV_MAX_KEYLEN || keyPtr->data_len <= 1) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	// Use Memory Pointers for Static DataVaults, otherwise allocate memory
	if ((IOStream_GetType(stream) == StreamMemory) && FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC)) {
		keyPtr->buf_len = 0;
		keyPtr->buf_ptr = IOStream_GetMemoryBuffer(stream) + IOStream_GetOffset(stream);
		if (IOStream_Seek(stream, keyPtr->data_len, SEEK_CUR) != EOK) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		FLAGS_CLEAR(*flagsPtr, ESIF_SERVICE_CONFIG_NOCACHE); // ignore for Static DataVaults
	}
	else {
		keyPtr->buf_len = esif_ccb_max(1, keyPtr->data_len);
		keyPtr->buf_ptr = esif_ccb_malloc(keyPtr->buf_len);
		if (!keyPtr->buf_ptr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		if (IOStream_Read(stream, keyPtr->buf_ptr, keyPtr->data_len) != keyPtr->data_len) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
	}

	// Sanity Check - Verify Key is Null Terminated
	if (keyPtr->buf_ptr == NULL || keyPtr->data_len < 1 || ((UInt8 *)keyPtr->buf_ptr)[keyPtr->data_len - 1] != 0) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}
	esif_sha1_update(&self->digest, keyPtr->buf_ptr, keyPtr->data_len);

	// Read Value Data Type
	if (IOStream_Read(stream, &valuePtr->type, sizeof(valuePtr->type)) != sizeof(valuePtr->type)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, &valuePtr->type, sizeof(valuePtr->type));

	// Read Value Length
	if (IOStream_Read(stream, &valuePtr->data_len, sizeof(valuePtr->data_len)) != sizeof(valuePtr->data_len)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_sha1_update(&self->digest, &valuePtr->data_len, sizeof(valuePtr->data_len));

	// Read Value after bounds check
	if (valuePtr->data_len > ESIFDV_MAX_DATALEN) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	// If NOCACHE mode, use buf_ptr to store the file offset of the data and skip the file
	if (FLAGS_TEST(*flagsPtr, ESIF_SERVICE_CONFIG_NOCACHE) && self->stream && self->stream->type != StreamNull && importMode == ImportCopy) {
		size_t offset = IOStream_GetOffset(stream);
		size_t buffer_size = ESIFDV_IO_BUFFER_SIZE;
		size_t bytes_to_read = valuePtr->data_len;
		size_t bytes_read = 0;
		unsigned char *buffer = esif_ccb_malloc(buffer_size);

		// Read NOCACHE data and discard
		if (buffer == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		while ((bytes_to_read > 0) && ((bytes_read = IOStream_Read(stream, buffer, esif_ccb_min(bytes_to_read, buffer_size))) > 0)) {
			esif_sha1_update(&self->digest, buffer, bytes_read);
			bytes_to_read -= bytes_read;
		}
		esif_ccb_free(buffer);

		valuePtr->buf_ptr = (void*)offset; // For non-cached...we save the offset in the file as the buffer pointer.
		valuePtr->buf_len = 0;	// buf_len == 0 so we don't release buffer as not allocated; data_len = original length
	} 
	else {
		// Use static pointer for static data vaults (unless scrambled), otherwise make a dynamic copy
		if ((IOStream_GetType(stream) == StreamMemory) && FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC) && !FLAGS_TEST(*flagsPtr, ESIF_SERVICE_CONFIG_SCRAMBLE)) {
			valuePtr->buf_len = 0;	// static
			valuePtr->buf_ptr = IOStream_GetMemoryBuffer(stream) + IOStream_GetOffset(stream);
			if (valuePtr->buf_ptr == NULL || IOStream_Seek(stream, valuePtr->data_len, SEEK_CUR) != EOK) {
				rc = ESIF_E_IO_ERROR;
				goto exit;
			}
			esif_sha1_update(&self->digest, valuePtr->buf_ptr, valuePtr->data_len);
		} 
		else {
			valuePtr->buf_len = esif_ccb_max(1, valuePtr->data_len); // dynamic
			valuePtr->buf_ptr = esif_ccb_malloc(valuePtr->buf_len);
			if (valuePtr->buf_ptr == NULL) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
			else {
				if (IOStream_Read(stream, valuePtr->buf_ptr, valuePtr->data_len) != valuePtr->data_len) {
					rc = ESIF_E_IO_ERROR;
					goto exit;
				}
				esif_sha1_update(&self->digest, valuePtr->buf_ptr, valuePtr->data_len);
			}
		}
			
		//  Unscramble Data?
		if (FLAGS_TEST(*flagsPtr, ESIF_SERVICE_CONFIG_SCRAMBLE)) {
			UInt32 byte;
			for (byte = 0; byte < valuePtr->data_len; byte++)
				((UInt8*)(valuePtr->buf_ptr))[byte] = ~((UInt8*)(valuePtr->buf_ptr))[byte];
		}
	}
exit:
	return rc;
}

// Load an External File into a specified memory buffer
static esif_error_t ReadFileIntoBuffer(
	esif_string filename,
	UInt32 offset,
	void **buffer,
	UInt32 *buf_size
	)
{
	esif_error_t rc    = ESIF_OK;
	IOStreamPtr file = IOStream_Create();
	BytePtr bufPtr = NULL;
	size_t size = 0;
	size_t bytesRead = 0;

	if (IOStream_SetFile(file, StoreReadOnly, filename, "rb") != EOK) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	size = IOStream_GetSize(file);

	bufPtr = (BytePtr)esif_ccb_malloc(size + offset + 1);
	if (NULL == bufPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	if (IOStream_Open(file) != 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	bytesRead = IOStream_ReadAt(file, bufPtr + offset, size, 0);
	if (bytesRead != size) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	bufPtr[bytesRead + offset] = 0;

	*buffer = bufPtr;
	*buf_size = (UInt32)(size + offset);
exit:
	IOStream_Destroy(file);
	if (rc != ESIF_OK) {
		esif_ccb_free(bufPtr);
	}
	return rc;
}


// Retrieve a single value from a DataVault
static esif_error_t DataVault_GetValue(
	DataVaultPtr self,
	esif_string key,
	EsifDataPtr value,
	esif_flags_t *flagsPtr
	)
{
	esif_error_t rc = ESIF_E_NOT_FOUND;
	DataCacheEntryPtr keypair = NULL;

	if (!self)
		return ESIF_E_PARAMETER_IS_NULL;

	esif_ccb_read_lock(&self->lock);

	if (flagsPtr)
		*flagsPtr = 0;

	// Return "keyname1|keyname2|..." if key contains "*" or "?"
	if (esif_ccb_strpbrk(key, "*?") != NULL) {
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, esif_ccb_strdup(self->name), ESIFAUTOLEN, ESIFAUTOLEN);
		EsifDataPtr path	  = EsifData_CreateAs(ESIF_DATA_STRING, key, 0, ESIFAUTOLEN);
		EsifConfigFindContext context = NULL;
		esif_string keylist = NULL;
		u32 data_len = 0;
		
		// Verify valid Data Type and Data Buffer size
		if (value->type != ESIF_DATA_STRING && value->type != ESIF_DATA_AUTO) {
			rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		}
		
		if (rc == ESIF_E_NOT_FOUND && nameSpace != NULL && path != NULL && (rc = EsifConfigFindFirst(nameSpace, path, NULL, &context)) == ESIF_OK) {
			do {
				data_len += (u32)path->data_len;
				esif_string newlist = esif_ccb_realloc(keylist, data_len);
				if (newlist == NULL) {
					EsifData_Set(path, ESIF_DATA_STRING, "", 0, ESIFAUTOLEN);
					rc = ESIF_E_NO_MEMORY;
					break;
				}
				keylist = newlist;
				esif_ccb_sprintf_concat(data_len, keylist, "%s%s", (*keylist ? "|" : ""), (char *)path->buf_ptr);
				EsifData_Set(path, ESIF_DATA_STRING, key, 0, ESIFAUTOLEN);
			} while ((rc = EsifConfigFindNext(nameSpace, path, NULL, &context)) == ESIF_OK);
		
			EsifConfigFindClose(&context);
			if (rc == ESIF_E_ITERATION_DONE) {
				rc = ESIF_OK;
			}
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(path);
		if (!keylist || rc != ESIF_OK) {
			esif_ccb_free(keylist);
			goto exit;
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
		goto exit;
	}

	keypair = DataCache_GetValue(self->cache, key);
	
	if (NULL != keypair) {
		UInt32 data_len = keypair->value.data_len;
		void *buf_ptr   = keypair->value.buf_ptr;
		UInt32 buf_len  = 0;
		Bool buf_alloc = ESIF_FALSE;

		// File Redirect?
		if (FLAGS_TEST(keypair->flags, ESIF_SERVICE_CONFIG_FILELINK)) {
			if (ReadFileIntoBuffer((esif_string)buf_ptr, 0, &buf_ptr, &data_len) != ESIF_OK) {
				value->data_len = 0;
				if (value->type == ESIF_DATA_AUTO) {
					value->type = keypair->value.type;
				}
				if (value->buf_len == ESIF_DATA_ALLOCATE) {
					value->buf_len = 0;
					value->buf_ptr = 0;
				}
				rc = ESIF_OK;	// Return OK and a blank buffer if file not found/error
				goto exit;
			}
			// Include Null Terminator if result is STRING
			if (value->buf_len == ESIF_DATA_ALLOCATE && (value->type == ESIF_DATA_STRING || (value->type == ESIF_DATA_AUTO && keypair->value.type == ESIF_DATA_STRING))) {
				data_len++;
			}
			buf_len = data_len;
			buf_alloc = ESIF_TRUE;
		}

		// Match Found. Verify Data Type matches unless AUTO
		if (value->type != keypair->value.type && value->type != ESIF_DATA_AUTO) {
			rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		}
		// Verify Data Buffer is large enough unless Auto-Allocate
		else if (value->buf_len < data_len && value->buf_len != ESIF_DATA_ALLOCATE) {
			value->data_len = data_len;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		// Return pointer to static contents if this is a static vault
		else if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC) &&
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
					if (buf_alloc) {
						esif_ccb_free(buf_ptr);
					}
					rc = ESIF_E_NO_MEMORY;
					goto exit;
				}
			}

			// Read from file if NOCACHE option
			if (FLAGS_TEST(keypair->flags, ESIF_SERVICE_CONFIG_NOCACHE) && keypair->value.buf_len == 0 && self->stream && self->stream->type != StreamNull) {
				size_t offset = (size_t)keypair->value.buf_ptr;
				if (IOStream_LoadBlock(self->stream, (esif_string)value->buf_ptr, data_len, offset) != EOK) {
					data_len = 0;
					if (buf_alloc) {
						esif_ccb_free(buf_ptr);
					}
					rc = ESIF_E_NOT_FOUND;
					goto exit;
				}
				// Unscramble Data?
				if (FLAGS_TEST(keypair->flags, ESIF_SERVICE_CONFIG_SCRAMBLE)) {
					UInt32 byte;
					for (byte = 0; byte < data_len; byte++)
						((UInt8*)(value->buf_ptr))[byte] = ~((UInt8*)(value->buf_ptr))[byte];
				}
			}
			else {
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
		if (buf_alloc) {
			esif_ccb_free(buf_ptr);
		}
	}

exit:
	esif_ccb_read_unlock(&self->lock);
	return rc;
}

/* Lock DataVault and Flush a Specified Payload Stream to disk  */
esif_error_t DataVault_SetPayload(
	DataVaultPtr self,
	UInt32 payload_class,
	IOStreamPtr payload,
	Bool compressPayload
)
{
	esif_error_t rc = ESIF_OK;
	
	if (!self || !payload)
		return ESIF_E_PARAMETER_IS_NULL;

	esif_ccb_write_lock(&self->lock);

	if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_STATIC | ESIF_SERVICE_CONFIG_READONLY)) {
		rc = ESIF_E_READONLY;
		goto exit;
	}

	// Cannot Flush a Payload Stream to Disk if its currently a KEYS Payload and the Cache is not empty
	if (self->dataclass == ESIFDV_PAYLOAD_CLASS_KEYS && self->cache != NULL && self->cache->size > 0) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	// Create Repo File and copy the Payload Stream to it
	self->dataclass = payload_class;
	rc = DataVault_RepoFlush(self, payload, compressPayload);

exit:
	esif_ccb_write_unlock(&self->lock);
	return rc;
}

/* Lock DataVault and Set Key/Value Pair, Flushing to Disk if necessary */
esif_error_t DataVault_SetValue(
	DataVaultPtr self,
	esif_string key,
	EsifDataPtr value,
	esif_flags_t flags
	)
{
	esif_error_t rc = ESIF_OK;
	DataCacheEntryPtr keypair;
	DataCachePtr nocacheClone = NULL;

	if (!self)
		return ESIF_E_PARAMETER_IS_NULL;

	esif_ccb_write_lock(&self->lock);

	if (FLAGS_TEST(self->flags, ESIF_SERVICE_CONFIG_READONLY)) {
		rc = ESIF_E_READONLY;
		goto exit;
	}

	// Make a clone of NOCACHE offsets so they can be restored in the event of failure
	nocacheClone = DataCache_CloneOffsets(self->cache);
	if (NULL == nocacheClone) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	// Flush when NULL Key
	if (key == NULL) {
		goto exit;
	}

	// Ignore value for DELETEs
	if (FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_DELETE)) {
		value = NULL;
	}

	// AUTO data types or AUTO_ALLOCATE are not allowed for SET
	if (value && (value->type == ESIF_DATA_AUTO || value->buf_len == ESIF_DATA_ALLOCATE)) {
		rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		goto exit;
	}

	// Reject SETs for static-sized data types whose buffer is too small
	if (value) {
		u32 len = (u32) esif_data_type_sizeof(value->type);
		if (len > 0 && value->buf_len < len) {
			value->data_len = len;
			rc = ESIF_E_NEED_LARGER_BUFFER;
			goto exit;
		}
	}

	// Delete DataVault Key(s)?
	if (esif_ccb_strpbrk(key, "*?") != NULL) {
		if (FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_DELETE)) {
			UInt32 item = 0;
			while (item < self->cache->size) {
				if (esif_ccb_strmatch((esif_string)self->cache->elements[item].key.buf_ptr, key)) {
					FLAGS_SET(flags, FLAGS_TEST(self->cache->elements[item].flags, ESIF_SERVICE_CONFIG_PERSIST));
					if (DataCache_DeleteValue(self->cache, (esif_string)self->cache->elements[item].key.buf_ptr) == ESIF_OK) {
						continue;
					}
				}
				item++;
			}
			goto exit;
		}
		rc = ESIF_E_NOT_SUPPORTED; // Keys may not contain "*" or "?"
		goto exit;
	}

	// Read data from input file if value begins with "<<"
	if (value && value->buf_ptr && esif_ccb_strncmp((char*)value->buf_ptr, "<<", 2) == 0) {
		void *buffer  = 0;
		UInt32 buflen = 0;
		UInt32 offset = 0;
		StringPtr filename = (StringPtr)value->buf_ptr + 2;
		union esif_data_variant header = { 0 };

		// "<<<filename" = load file and prepend with esif_variant header
		if (*filename == '<') {
			filename++;
			header.type = value->type;
			offset = (UInt32)sizeof(header);
		}

		if ((rc = ReadFileIntoBuffer(filename, offset, &buffer, &buflen)) != ESIF_OK) {
			goto exit;
		}

		// Fill in optional esif_variant header
		if (offset && buflen > offset) {
			header.integer.value = ((u64)buflen) - offset;
			esif_ccb_memcpy(buffer, &header, sizeof(header));
		}

		if (value->buf_len) {
			esif_ccb_free(value->buf_ptr);
		}
		value->buf_ptr = buffer;
		if (value->type == ESIF_DATA_STRING) {
			buflen++;	// Include Null Terminator
		}
		value->buf_len = value->data_len = buflen;
	}

	// Get the Data Row or create it if it does not exist
	keypair = DataCache_GetValue(self->cache, key);

	if (keypair) {	// Match Found
		esif_flags_t key_flags = keypair->flags;
		// READONLY?
		if (FLAGS_TEST(keypair->flags, ESIF_SERVICE_CONFIG_READONLY)) {
			rc = ESIF_E_READONLY;
		}
		// DELETE?
		else if (FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_DELETE)) {
			FLAGS_SET(flags, keypair->flags);
			DataCache_DeleteValue(self->cache, key);
		} 
		// UPDATE
		else if (value && value->buf_ptr) {
			if (keypair->value.buf_len > 0 && value->data_len != keypair->value.buf_len) {
				void *new_buf = NULL;
				u32 new_buf_len = 0;
				
				// Grow or shrink buffer if it was allocated, otherwise ask for a larger buffer
				new_buf_len = esif_ccb_max(1, value->data_len);
				new_buf= (void *)esif_ccb_realloc(keypair->value.buf_ptr, new_buf_len);

				if (new_buf == NULL) {
					rc = ESIF_E_NEED_LARGER_BUFFER;
					goto exit;
				}
				else {
					keypair->value.buf_len = new_buf_len;
					keypair->value.buf_ptr = new_buf;
				}
			} 

			// Replace the File Offset stored in buf_ptr with a copy of the data for updated NOCACHE values
			if (FLAGS_TEST(keypair->flags, ESIF_SERVICE_CONFIG_NOCACHE) && keypair->value.buf_len == 0) {
				keypair->value.buf_len = esif_ccb_max(1, value->data_len);
				keypair->value.buf_ptr = esif_ccb_malloc(value->buf_len);
			}
			keypair->flags = flags;
			keypair->value.type     = value->type;
			keypair->value.data_len = value->data_len;
			FLAGS_SET(flags, FLAGS_TEST(key_flags, ESIF_SERVICE_CONFIG_PERSIST)); // Flush if PERSIST -> NOPERSIST

			esif_ccb_memcpy(keypair->value.buf_ptr, value->buf_ptr, value->data_len);
			rc = ESIF_OK;
		}
	}
	else if (value && value->buf_ptr && !FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_DELETE)) {
		EsifDataPtr valueClonePtr = NULL;

		//
		// The data passed in may be in a buffer owned elsewhere, so clone the data
		//
		valueClonePtr = EsifData_Clone(value);
		if (NULL == valueClonePtr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		DataCache_InsertValue(self->cache, key, valueClonePtr, flags);

		EsifData_Destroy(valueClonePtr);
	}

exit:
	// If Persisted, Flush to disk
	if (rc == ESIF_OK && FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_PERSIST)) {
		if (nocacheClone) {
			rc = DataVault_RepoFlush(self, NULL, ESIF_FALSE);

			// Restore NOCACHE Offsets on Failure
			if (rc != ESIF_OK) {
				DataCache_RestoreOffsets(self->cache, nocacheClone);
			}
		}
	}
	DataCache_Destroy(nocacheClone);

	esif_ccb_write_unlock(&self->lock);
	return rc;
}

// Is this Repo the Primary Stream for the given DataVault?
static Bool DataVault_IsPrimary(
	DataVaultPtr self,
	DataRepoPtr repo,
	DataVaultHeaderPtr header
	)
{
	Bool result = ESIF_FALSE;

	/* A DataVault's Primary Stream is loaded using ImportCopy (overwrite) and updates to
	 * its persisted values may be written to it, unless the stream is marked StoreStatic
	 * (i.e., static in-memory DVs) or StoreReadOnly (multi-segment .dv files).
	 *
	 * This Repo is the Primary Stream for this DV if all of these conditions are met:
	 * 1. No Primary Stream has been assigned, or the Primary Stream is a Memory Stream and this Repo is a File Stream
	 * 2. The Name in the Segment Header matches the DataVault name
	 * 3. The Name of the Repo matches the DataVault name ('name' or 'name.dv')
	 * 4. The Repo Name file has a .dv extension (or none)
	 */
	if (self && repo && repo->stream && repo->stream->type != StreamMemory &&
		(self->stream == NULL || self->stream->type == StreamNull ||
		(self->stream->type == StreamMemory && repo->stream->type == StreamFile))) {

		char reponame[sizeof(self->name)] = { 0 };
		DataRepo_GetName(repo, reponame, sizeof(reponame));

		if (DataVault_IsSegmentMatch(self, header) && esif_ccb_stricmp(self->name, reponame) == 0) {
			StringPtr ext = esif_ccb_strrchr(repo->name, '.');
			if (ext == NULL || esif_ccb_stricmp(ext, ESIFDV_FILEEXT) == 0) {
				result = ESIF_TRUE;
			}
		}
	}
	return result;
}

/*
 * DataRepo Class Friend Members implemented here to keep DataVaultHeader local to this file only
 */

// Read the next DV header from an open Repo
esif_error_t DataRepo_ReadHeader(
	DataRepoPtr self,
	DataVaultHeaderPtr header
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Read Common Header
	if (self && self->stream && header) {
		rc = ESIF_OK;
		esif_ccb_memset(header, 0, sizeof(DataVaultHeader));
		size_t current_pos = IOStream_GetOffset(self->stream);
		size_t bytes_read = IOStream_Read(self->stream, header, sizeof(DataVaultHeaderV0));

		if (bytes_read != sizeof(DataVaultHeaderV0)) {
			rc = (bytes_read == 0 ? ESIF_E_ITERATION_DONE : ESIF_E_IO_ERROR);
		}
		if (rc == ESIF_OK) {
			IOStream_Seek(self->stream, current_pos, SEEK_SET);
		}
	}

	// Verify Header Signature, Version, and Size
	if (rc == ESIF_OK && !DataVault_IsValidSignature(header->common.signature)) {
		rc = ESIF_E_NOT_SUPPORTED;
	}
	if (rc == ESIF_OK && (header->common.version < ESIFDV_SUPPORTED_VERSION_MIN || header->common.version > ESIFDV_SUPPORTED_VERSION_MAX)) {
		rc = ESIF_E_NOT_SUPPORTED;
	}
	if (rc == ESIF_OK && header->common.headersize > ESIFDV_MAX_HEADER) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	if (rc == ESIF_OK) {
		// v1 Header must be an exact size match
		if (ESIFHDR_GET_MAJOR(header->common.version) == 1 && header->common.headersize != sizeof(DataVaultHeaderV1)) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		// v2 Header may be larger in future minor versions (but extra data will be lost if the DV is written)
		if (ESIFHDR_GET_MAJOR(header->common.version) == 2) {
			if (header->common.headersize < sizeof(DataVaultHeaderV2)) {
				rc = ESIF_E_NOT_SUPPORTED;
			}
			// Validate v2 Payload Size
			else if (header->v2.payload_size > ESIFDV_MAX_PAYLOAD) {
				rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			}
		}
	}

	// Read the Entire Header
	if (rc == ESIF_OK) {
		if (IOStream_Read(self->stream, header, esif_ccb_min(header->common.headersize, sizeof(DataVaultHeader))) != esif_ccb_min(header->common.headersize, sizeof(DataVaultHeader))) {
			rc = ESIF_E_IO_ERROR;
		}

		// Bounds check
		if (header->common.headersize > ESIFDV_MAX_HEADER) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}

		// Skip any extra Header bytes (if reading a newer minor revision). Note that this data will be lost if the DV is rewritten
		if (rc == ESIF_OK && header->common.headersize > sizeof(DataVaultHeader)) {
			if (IOStream_Seek(self->stream, (header->common.headersize - sizeof(DataVaultHeader)), SEEK_CUR) != EOK) {
				rc = ESIF_E_IO_ERROR;
			}
		}

		// v1  = Unknown Header Flags are invalid, as a sanity check
		// v2+ = Unknown Header Flags are allowed to be used by future minor versions or revisions
		if (rc == ESIF_OK && ESIFHDR_GET_MAJOR(header->common.version) == 1) {
			if (!FLAGS_TESTVALID(header->v1.flags, ESIFDV_VALID_HEADER_FLAGS_V1)) {
				rc = ESIF_E_NOT_SUPPORTED;
			}
			else {
				FLAGS_CLEAR(header->v1.flags, ESIFDV_IGNORED_HEADER_FLAGS);
			}
		}
		else if (ESIFHDR_GET_MAJOR(header->common.version) == 2) {
			FLAGS_CLEAR(header->v2.flags, ESIFDV_IGNORED_HEADER_FLAGS);
		}
	}
	return rc;
}

// Load all DataVault Segments from a DataRepo into the DataBank
esif_error_t DataRepo_LoadSegments(DataRepoPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	DataVaultHeader header = { 0 };
	DataVaultPtr DV = NULL;
	char PrimaryDV[sizeof(DV->name)] = { 0 };
	int segments = 0;

	if (self && self->stream) {
		if (IOStream_Open(self->stream) != EOK) {
			rc = ESIF_E_IO_OPEN_FAILED;
			goto exit;
		}

		// Default DataVault (Cache) Name = Repo Name (without file path and extension)
		char reponame[sizeof(self->name)] = { 0 };
		char segmentid[sizeof(self->name)] = { 0 };
		DataRepo_GetName(self, reponame, sizeof(reponame));
		esif_ccb_strcpy(segmentid, reponame, sizeof(segmentid));

		// Keep Reading from Repo Stream until EOF or no more 
		while ((rc = DataRepo_ReadHeader(self, &header)) == ESIF_OK) {
			UInt32 major_version = ESIFHDR_GET_MAJOR(header.common.version);

			// Use previous segmentid if not defined in current header
			if (major_version == 1) {
				if (self->stream->type == StreamMemory && self->stream->base.store == StoreStatic) {
					FLAGS_SET(header.v1.flags, ESIF_SERVICE_CONFIG_STATIC);
				}
			}
			else if (major_version == 2) {
				if (self->stream->type == StreamMemory && self->stream->base.store == StoreStatic) {
					FLAGS_SET(header.v2.flags, ESIF_SERVICE_CONFIG_STATIC);
				}
				if (header.v2.segmentid[0]) {
					esif_ccb_strmemcpy(segmentid, sizeof(segmentid), header.v2.segmentid, sizeof(header.v2.segmentid));
				}
			}
			else {
				rc = ESIF_E_NOT_SUPPORTED;
				break;
			}

			// Open or Create a Named DataVault Cache and load the next Repo Segment into it
			if ((DV = DataBank_OpenDataVault(segmentid)) != NULL) {
				ImportType importMode = ImportMerge;
				esif_ccb_write_lock(&DV->lock);

				// Assign Repo Stream to the Primary DV if the Repo, DV, and Segment names all match
				// The Primary DV for DataVault 'name' is always 'name.dv'
				IOStreamPtr currentStream = DV->stream;
				if (DataVault_IsPrimary(DV, self, &header)) {
					importMode = ImportCopy;
					DV->stream = self->stream;
					DataRepo_GetName(self, PrimaryDV, sizeof(PrimaryDV));
				}
				rc = DataVault_ReadSegment(DV, self, &header, importMode);
				DV->stream = currentStream;

				esif_ccb_write_unlock(&DV->lock);

				if (rc != ESIF_OK && rc != ESIF_E_ITERATION_DONE) {
					break;
				}
			}
			segments++;
		}
		IOStream_Close(self->stream);

		if (rc == ESIF_E_ITERATION_DONE) {
			rc = ESIF_OK;
		}

		// Transfer ownership of this Repo stream to the Primary DV, if one was found
		if (PrimaryDV[0] && ((DV = DataBank_GetDataVault(PrimaryDV)) != NULL)) {
			esif_ccb_write_lock(&DV->lock);
			IOStream_Destroy(DV->stream);
			DV->stream = self->stream;
			self->stream = NULL;

			// Mark Primary Stream as read-only if it is a multi-segment Repo
			if (segments > 1 && DV->stream->base.store == StoreReadWrite) {
				DV->stream->base.store = StoreReadOnly;
			}
			esif_ccb_write_unlock(&DV->lock);
		}
	}

exit:
	return rc;
}

/*
 * Public API used for Backwards Compatibility
 */

// backwards compatibility
esif_error_t EsifConfigGetItem(
	EsifDataPtr nameSpace,
	EsifDataPtr key,
	EsifDataPtr value,
	esif_flags_t *flagsPtr
	)
{
	esif_error_t rc = ESIF_OK;
	DataVaultPtr DB = DataBank_GetDataVault((StringPtr)(nameSpace->buf_ptr));
	if (DB) {
		rc = DataVault_GetValue(DB, (esif_string)key->buf_ptr, value, flagsPtr);
	}
	else {
		rc = ESIF_E_NOT_FOUND;
	}
	return rc;
}

// backwards compatibility
esif_error_t EsifConfigGet(
	EsifDataPtr nameSpace,
	EsifDataPtr key,
	EsifDataPtr value
	)
{
	return EsifConfigGetItem(nameSpace, key, value, NULL);
}

// backwards compatibility
esif_error_t EsifConfigSet(
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	esif_flags_t flags,
	EsifDataPtr value
	)
{
	esif_error_t rc = ESIF_OK;

	// Get the NameSpace or create it if it does not exist
	DataVaultPtr DB = DataBank_GetDataVault((StringPtr)(nameSpace->buf_ptr));
	if (!DB) {
		rc = DataBank_ImportDataVault((StringPtr)(nameSpace->buf_ptr));
		DB = DataBank_GetDataVault((StringPtr)(nameSpace->buf_ptr));
	}
	if (rc == ESIF_OK) {
		StringPtr key = (path ? (StringPtr)path->buf_ptr : NULL);
		flags = (path ? flags : ESIF_SERVICE_CONFIG_PERSIST);
		rc = DataVault_SetValue(DB, key, value, flags);
	}
	return rc;
}

// Delete a DataVault Key
esif_error_t EsifConfigDelete(
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
esif_error_t EsifConfigFindNext(
	EsifDataPtr nameSpace,
	EsifDataPtr path,
	EsifDataPtr value,
	EsifConfigFindContextPtr context
	)
{
	esif_error_t rc = ESIF_E_NOT_FOUND;
	DataVaultPtr DB = 0;

	ESIF_ASSERT(nameSpace && path && context);
	DB = DataBank_GetDataVault((StringPtr)(nameSpace->buf_ptr));
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
esif_error_t EsifConfigFindFirst(
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
			FLAGS_SET(bitmask, optionList[j].option);
			break;
		}
		// ~NAME = Unset option
		if (optionList[j].name[0] == '~' && esif_ccb_stricmp(optname, optionList[j].name + 1) == 0) {
			FLAGS_CLEAR(bitmask, optionList[j].option);
			break;
		}
	}
	if (!optionList[j].name) {
		//CMD_OUT("Error: Invalid Option: %s\n", optname);
	}
	return bitmask;
}

/* Copy/Merge Keys from one NameSpace to Another */
esif_error_t EsifConfigCopy(
	EsifDataPtr nameSpaceFrom,	// Source DV
	EsifDataPtr nameSpaceTo,	// Target DV
	EsifDataPtr keyspecs,		// Tab-separated Keyspec List (wildcards OK)
	esif_flags_t flags,			// Item Flags
	Bool replaceKeys,			// TRUE=COPY Keys (Replace if exists), FALSE=MERGE Keys (Do Not Replace)
	UInt32 *keycount)			// Optional pointer to variable to hold Key Count copied/merged
{
	esif_error_t rc = ESIF_OK;
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
					(replaceKeys == ESIF_TRUE || DataBank_KeyExists((esif_string)nameSpaceTo->buf_ptr, (esif_string)data_key->buf_ptr) == ESIF_FALSE)) {

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

