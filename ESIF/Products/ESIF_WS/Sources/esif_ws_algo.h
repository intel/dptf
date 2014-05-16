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

#ifndef ESIF_WS_ALGO_H
#define ESIF_WS_ALGO_H

#include "esif.h"

#define HASH_BITS_NUM  160
#define SHA1_HASH_BYTES (HASH_BITS_NUM / 8)
#define NUM_BITS_BLOCK 512
#define SHA1_BLOCK_BYTES (NUM_BITS_BLOCK / 8)

#define FUNC0_TO_19_KVALUE         0x5a827999
#define FUNC20_TO_39_KVALUE        0x6ed9eba1
#define FUNC40_TO_59_KVALUE        0x8f1bbcdc
#define FUNC60_TO_79_KVALUE        0xca62c1d6


typedef struct _shaCtx {
	UInt32  hash_values_array[5];
	UInt64  length;
}shaCtx;


/*
 *******************************************************************************
 ** EXTERN
 *******************************************************************************
 */


/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */


/*
 *******************************************************************************
 ** PUBLIC INTERFACE
 *******************************************************************************
 */
void esif_ws_algo_sha_context_init (shaCtx *state);


void esif_ws_algo_add_block_to_sha (shaCtx *state, const void *block);


void esif_ws_algo_add_last_block_to_sha (shaCtx *state, const void *block, UInt16 length_b);


void esif_ws_algo_hash_sha_context (void *dest, shaCtx *state);


void esif_ws_algo_hash_sha_algo (void *dest, const void *msg, UInt32 length_b);


void esif_ws_algo_encode_base64_value (char *dest, const void *src, UInt16 length);


#endif /* ESIF_WS_ALGO_H */
