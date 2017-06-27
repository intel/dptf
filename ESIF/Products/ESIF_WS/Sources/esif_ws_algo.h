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

#ifndef ESIF_WS_ALGO_H
#define ESIF_WS_ALGO_H

#include "esif_ccb.h"

/*********************************************************************************
* Secure Hash Algorithm Public Interface (SHA-1, SHA-256)
*********************************************************************************/

/* SHA-1 (160 bit output) */
#define SHA1_HASH_BYTES		20	// Size of resulting SHA1 binary hash
#define SHA1_BLOCK_BYTES	64	// Size of data blocks used in SHA1 digest computation
#define SHA1_STRING_BYTES	((SHA1_HASH_BYTES * 2) + 1)			// Size of SHA256 string plus NUL
#define SHA1_DIGESTS		(SHA1_HASH_BYTES / sizeof(UInt32))	// Number of SHA1 32-bit Digest words

typedef struct esif_sha1_s {
	UInt8	hash[SHA1_HASH_BYTES];		// SHA1 produces a 20-byte binary hash
	UInt8	block[SHA1_BLOCK_BYTES];	// SHA1 is computed in 64-byte binary blocks
	UInt16	blockbytes;					// Number of bytes in current partially-filled block
	UInt32  digest_values[SHA1_DIGESTS];// SHA1 digest values array used during computation
	UInt64  digest_bits;				// Total bits used to compute SHA1 digest
} esif_sha1_t;

void esif_sha1_init(esif_sha1_t *self);
void esif_sha1_update(esif_sha1_t *self, const void *source, size_t bytes);
void esif_sha1_finish(esif_sha1_t *self);

/* SHA-256 (SHA-2 with 256 bit output) */
#define SHA256_HASH_BYTES		32	// Size of resulting SHA256 binary hash
#define SHA256_BLOCK_BYTES		64	// Size of data blocks used in SHA256 digest computation
#define SHA256_STRING_BYTES		((SHA256_HASH_BYTES * 2) + 1)			// Size of SHA256 string plus NUL
#define SHA256_DIGESTS			(SHA256_HASH_BYTES / sizeof(UInt32))	// Number of SHA256 32-bit Digest words

typedef struct esif_sha256_s {
	UInt8	hash[SHA256_HASH_BYTES];		// SHA256 produces a 32-byte binary hash
	UInt8	block[SHA256_BLOCK_BYTES];		// SHA256 is computed in 64-byte binary blocks
	UInt16	blockbytes;						// Number of bytes in current partially-filled block
	UInt32  digest_values[SHA256_DIGESTS];	// SHA256 digest values array used during computation
	UInt64  digest_bits;					// Total bits used to compute SHA256 digest
} esif_sha256_t;

void esif_sha256_init(esif_sha256_t *self);
void esif_sha256_update(esif_sha256_t *self, const void *source, size_t bytes);
void esif_sha256_finish(esif_sha256_t *self);

/* Convert SHA1 or SHA2 Hashes to Null-Terminated String */
const char *esif_sha_tostring(UInt8 hash_bytes[], size_t hash_len, char *buffer, size_t buf_len);
const char *esif_sha1_tostring(esif_sha1_t *self, char *buffer, size_t buf_len);
const char *esif_sha256_tostring(esif_sha256_t *self, char *buffer, size_t buf_len);

/* Base-64 Encode a string */
char *esif_base64_encode(char *dest, size_t dest_bytes, const void *source, size_t src_bytes);

#endif /* ESIF_WS_ALGO_H */
