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

#ifndef ESIF_WS_ALGO_H
#define ESIF_WS_ALGO_H

#include "esif_ccb.h"

/*
*******************************************************************************
** PUBLIC INTERFACE
*******************************************************************************
*/

#define SHA1_HASH_BYTES		20	// Size of resulting SHA1 binary hash
#define SHA1_BLOCK_BYTES	64	// Size of data blocks used in SHA1 digest computation

#pragma pack(push, 1)

typedef struct esif_sha1_s {
	UInt8	hash[SHA1_HASH_BYTES];		// SHA1 produces a 20-byte binary hash
	UInt8	block[SHA1_BLOCK_BYTES];	// SHA1 is computed in 64-byte binary blocks
	UInt16	blockbytes;					// Number of bytes in current partially-filled block
	UInt32  digest_values[5];			// SHA1 digest values array used during computation
	UInt64  digest_bits;				// Total bits used to compute SHA1 digest
} esif_sha1_t;

#pragma pack(pop)

void esif_sha1_init(esif_sha1_t *self); 
void esif_sha1_update(esif_sha1_t *self, const void *source, size_t bytes);
void esif_sha1_finish(esif_sha1_t *self);

char *esif_base64_encode(char *dest, size_t dest_bytes, const void *source, size_t src_bytes);

#endif /* ESIF_WS_ALGO_H */
