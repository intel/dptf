/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#pragma once

#include "esif_uf.h"
#include "esif_lib_datacache.h"
#include "esif_lib_iostream.h"
#include "esif_sdk_sha.h"

/*
 * Data Vault 2.0 Repository Overview:
 *
 * A "DataVault" is an Internal ESIF object that contains a Name and a Cache of
 * Key/Value Pairs, some of which may be persisted to disk after a restart, and
 * the contents of which may be populated from multiple sources, including Files,
 * GDDV BIOS object, Memory blocks (static and dynamic), Loadable Apps such as DPTF,
 * and ESIF shell commands. Keys flagged as "Persisted" in a DataVault cache are always
 * written to a file named "dvname.dv", where dvname is the name of the DataVault.
 *
 * A "DataRepo" or Repository is a Container (File, Memory, or GDDV object) that can
 * contain multiple Data Segments, each of which consists of a Header and a Payload.
 * The Header contains the DataVault name that the Segment is associated with, along
 * with other metadata, such as a Description, Payload Size, Class, and SHA256 Hash.
 * The Payload can contain different classes of data, such a list of Key/Value Pairs
 * [KEYS] or another Repo [REPO], and potentially other class types in the future.
 *
 * There are several types of Repo container formats that are supported:
 *
 *    1. Singleton Repo: Contains a single Header and Payload of Key/Value Pairs,
 *       Indicated by a Payload Class of KEYS.
 *
 *    2. Multi-Segment Repo: Contains two or more Header/Payload Data Segments
 *       concatenated together. Each Segment contains the DataVault name it will be
 *       loaded into, along with a Description, Size, Class, and SHA256 Hash.
 *
 *    3. Embedded Repo: Contains a Header/Payload where the Payload Class is REPO,
 *       which indicates that the Payload is another Repo (Singleton or Multi-Segment),
 *       and the SHA256 hash is for the entire Payload.
 *
 * Notes:
 *    A. DV 1.0 format Repos are supported for backwards compatibility (Read/Write)
 *       but may ONLY be Singleton Repos containing a Key/Value Pairs Payload.
 *    B. DV 2.0 Payloads may be compressed, but Headers are not compressed,
 *       unless they are part of the Payload in an Embedded Repo.
 *    C. Unlike DV 1.0, there is no longer a one-to-one relationship between
 *       a DataVault and its dvname.dv file, even for DV 1.0 Repos.
 *    D. The "Primary Repo" of a DataVault is the file stream that Persisted Keys
 *       are written to when they are added, deleted, or updated, 
 *       and is always named dvname.dv.
 *    E. Repos can use any file extension, but by convention we load the
 *       following types of files on startup:
 *         .dv  files are Singleton Repos and usually a DataVault's Primary Repo
 *         .dvx files are Singleton, Multi-Segment, or Embedded Repos.
 *       Repos with other extensions may be loaded manually in the ESIF shell.
 *    F. Primary Repos (dvname.dv) are Singleton Repos loaded in COPY mode,
 *       which means the Keys are loaded into the DataVault as Persisted values
 *       and will replace any keys that may already exist in the DataVault Cache.
 *    G. Multi-Segment or Embedded Repos (name.dvx or GDDV) are loaded in MERGE mode,
 *       which means the Keys are loaded as NON-Persisted values and will NOT
 *       replace any keys that already exist in the DataVault Cache.
 *    H. Multiple Singleton and Multi-Segment Repos may be combined together
 *       merely by concatenating their .dv and .dvx files together.
 *    I. On startup, all Repos are loaded alphabetically (All .dv files then .dvx files)
 *       but the data in a Primary Repo effectively takes precedence over all other
 *       Repos, including the optional GDDV object in BIOS.
 *    J. Any type of Repo may be loaded into the optional GDDV object in BIOS,
 *       but for DV 2.0, it will usually be an Embedded Repo so that any Data Segment(s)
 *       contained in the Repo can be compressed and the SHA256 hash will be computed
 *       for all segment(s) in the Payload.
 */

// DV Global Definitions
#define ESIFDV_FILEEXT              ".dv"		// DataVault File Extension [DV name = name.dv]
#define ESIFDV_REPOEXT              ".dvx"		// Data Repo Extension [repo.dvx]
#define ESIFDV_TEMPEXT              ".tmp"		// Temp Repo File Extension [name.dv.tmp or repo.dvx.tmp]
#define ESIFDV_ROLLBACKEXT          ".temp"		// Rollback File Extension [name.dv.temp or repo.dvx.temp]
#define ESIFDV_TEMP_PREFIX          "$$"		// Temp DV Name Prefix [i.e., $$name.dv]
#define ESIFDV_EXPORT_PREFIX        "$"			// Exported Repository DV Name Prefix [i.e., $name.dv]
#define ESIFDV_NAME_LEN				32			// Max DataVault Name (Cache Name) Length (not including NUL)
#define ESIFDV_DESC_LEN				64			// Max DataVault Description Length (not including NUL)

// Supported Data Vault Payload Classes
#define ESIFDV_PAYLOAD_CLASS_NULL	0			// Undefined Payload Class
#define ESIFDV_PAYLOAD_CLASS_KEYS	'SYEK'		// "KEYS" = DataVault 2.0 Key/Value Pair List
#define ESIFDV_PAYLOAD_CLASS_REPO	'OPER'		// "REPO" = Data Repository (1 or more DataVaults)

// DataVault = Named Cache of Key/Value pairs (both persisted and nonpersisted data)
// DataVaults are internal objects associated with a shared Named Cache and are multi-threaded.
// DataVaults can be populated from Data Repositories (DataRepo Segments), Primitives, and Shell Commands
typedef struct DataVault_s {
	esif_ccb_lock_t			lock;							// Thread Lock
	atomic_t				refCount;						// Reference Count
	UInt32					version;						// DataVault Version
	esif_flags_t			flags;							// Global DataVault Bit Flags
	char					name[ESIFDV_NAME_LEN+1];		// DataVault Name
	char					comment[ESIFDV_DESC_LEN+1];	    // Optional Description or Comment
	char					segmentid[ESIFDV_NAME_LEN+1];	// Optional SegmentID (Cache Name) in Repo Header, if different than DV name
	DataCachePtr			cache;							// Key/Value Pair Cache (Persisted and Non-persisted)
	IOStreamPtr				stream;							// Primary Stream (Cached Key/Values) ["name.dv"]
	UInt32					dataclass;						// Payload Data Class (KEYS, REPO, ...)
	esif_sha256_t			digest;							// SHA-256 Hash used to verify Payload
} DataVault, *DataVaultPtr;

#ifdef __cplusplus
extern "C" {
#endif

// static methods
Bool DataVault_IsValidSignature(UInt16 signature);
esif_error_t DataVault_TranslatePath(const esif_string filename, esif_string fullpath, size_t fullpath_len);

// object management
DataVaultPtr DataVault_Create(char *name);
void DataVault_Destroy(DataVaultPtr self);
void DataVault_GetRef(DataVaultPtr self);
void DataVault_PutRef(DataVaultPtr self);

// class methods
UInt32 DataVault_GetKeyCount(DataVaultPtr self);
Bool DataVault_KeyExists(DataVaultPtr self, StringPtr keyName, esif_data_type_t *typePtr, esif_flags_t *flagsPtr);
esif_error_t DataVault_SetValue(DataVaultPtr self, esif_string key, EsifDataPtr value, esif_flags_t flags);
esif_error_t DataVault_SetPayload(DataVaultPtr self, UInt32 payload_class, IOStreamPtr payload, Bool compressPayload);
void DataVault_SetDefaultComment(DataVaultPtr self);
esif_error_t DataVault_ImportStream(DataVaultPtr self);

#ifdef __cplusplus
}
#endif
