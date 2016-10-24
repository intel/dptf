/*****************************************************************************
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
#define ESIF_TRACE_ID ESIF_TRACEMODULE_WEBSERVER

#include "esif_ws_algo.h"
#include "esif_ccb_memory.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */

#define SHA1_HASH_BITS	(SHA1_HASH_BYTES * 8)
#define SHA1_BLOCK_BITS	(SHA1_BLOCK_BYTES * 8)
#define SHA1_FOURBITMASK 0x0000000f

static UInt32 sha1_circular_left_shift(UInt32 shift_num, UInt8 value);
static UInt32 sha1_byteswap_uint32(UInt32 byte_to_swap);
static void sha1_add_block(esif_sha1_t *self, UInt8 *block);
static void sha1_add_last_block(esif_sha1_t *self, UInt8 *block, UInt16 bits);
static void sha1_compute_hash(esif_sha1_t *self);

static UInt32 sha1_circular_left_shift(
	UInt32 shitf_num,
	UInt8 value
	)
{
	return (shitf_num << value) | (shitf_num >> (32 - value));
}


static UInt32 sha1_byteswap_uint32(UInt32 byte_to_swap)
{
	return ((byte_to_swap) << 24) | ((byte_to_swap) >> 24) | (((byte_to_swap) & 0x0000ff00) << 8) | (((byte_to_swap) & 0x00ff0000) >> 8);
}


static void sha1_add_block(
	esif_sha1_t *self,
	UInt8 *block
)
{
	UInt32 chunk_of_initial_values[5] = { 0 };
	UInt32 array_of_16_words[16] = { 0 };
	UInt32 temp_value1 = 0;
	UInt32 temp_kvalue = 0;
	UInt8 index = 0;
	UInt8 word_index = 0;
	UInt8 kvalues_index = 0;
	UInt8 twenty_value_processed_flag = 0;
	UInt32 array_of_kvalues[4] = {
		0x5a827999,
		0x6ed9eba1,
		0x8f1bbcdc,
		0xca62c1d6
	};

	for (index = 0; index < 16; ++index)
		array_of_16_words[index] = sha1_byteswap_uint32(((UInt32*)block)[index]);

	esif_ccb_memcpy(chunk_of_initial_values, self->digest_values, 5 * sizeof(UInt32));

	for (kvalues_index = 0, twenty_value_processed_flag = 0, index = 0; index <= 79; ++index) {
		word_index = index & SHA1_FOURBITMASK;

		if (index >= 16) {
			array_of_16_words[word_index] = sha1_circular_left_shift(array_of_16_words[(word_index + 13) & SHA1_FOURBITMASK] ^
				array_of_16_words[(word_index + 8) & SHA1_FOURBITMASK] ^
				array_of_16_words[(word_index + 2) & SHA1_FOURBITMASK] ^
				array_of_16_words[word_index],
				1);
		}

		switch (kvalues_index) {
		case 0:
			temp_kvalue = (chunk_of_initial_values[1] & chunk_of_initial_values[2]) ^ ((~chunk_of_initial_values[1]) & chunk_of_initial_values[3]);
			break;
		case 1:
		case 3:
			temp_kvalue = (chunk_of_initial_values[1] ^ chunk_of_initial_values[2]) ^ chunk_of_initial_values[3];
			break;
		case 2:
			temp_kvalue = (chunk_of_initial_values[1] & chunk_of_initial_values[2]) ^ (chunk_of_initial_values[1] & chunk_of_initial_values[3]) ^ (chunk_of_initial_values[2] & chunk_of_initial_values[3]);
			break;
		}

		temp_value1 =
			sha1_circular_left_shift(chunk_of_initial_values[0], 5) +
			temp_kvalue +
			chunk_of_initial_values[4] +
			array_of_kvalues[kvalues_index] +
			array_of_16_words[word_index];

		esif_ccb_memmove(&(chunk_of_initial_values[1]), &(chunk_of_initial_values[0]), 4 * sizeof(UInt32));

		chunk_of_initial_values[0] = temp_value1;
		chunk_of_initial_values[2] = sha1_circular_left_shift(chunk_of_initial_values[2], 30);

		twenty_value_processed_flag++;

		if (twenty_value_processed_flag == 20) {
			twenty_value_processed_flag = 0;
			kvalues_index = (kvalues_index + 1) % 4;
		}
	}

	for (index = 0; index < 5; ++index)
		self->digest_values[index] += chunk_of_initial_values[index];

	self->digest_bits += SHA1_BLOCK_BITS;
}


static void sha1_add_last_block(
	esif_sha1_t *self,
	UInt8 *block,
	UInt16 bits
)
{
	UInt8 i = 0;
	UInt8 lb[SHA1_BLOCK_BYTES] = { 0 };

	while (bits >= SHA1_BLOCK_BITS) {
		sha1_add_block(self, block);
		bits -= SHA1_BLOCK_BITS;
		block = (UInt8*)block + SHA1_BLOCK_BYTES;
	}

	self->digest_bits += bits;
	esif_ccb_memset(lb, 0, SHA1_BLOCK_BYTES);
	esif_ccb_memcpy(lb, block, (bits + 7) >> 3);

	lb[bits >> 3] |= 0x80 >> (bits & 0x07);

	if (bits > SHA1_BLOCK_BITS - 64 - 1) {
		sha1_add_block(self, lb);
		self->digest_bits -= SHA1_BLOCK_BITS;
		esif_ccb_memset(lb, 0, SHA1_BLOCK_BYTES);
	}

	for (i = 0; i < 8; ++i)
		lb[56 + i] = ((UInt8*)&(self->digest_bits))[7 - i];

	sha1_add_block(self, lb);
	self->digest_bits -= SHA1_BLOCK_BITS;
}

static void sha1_compute_hash(esif_sha1_t *self)
{
	int i = 0;
	for (i = 0; i < 5; ++i)
		((UInt32*)(self->hash))[i] = sha1_byteswap_uint32(self->digest_values[i]);
}

/*
 *******************************************************************************
 ** PUBLIC INTERFACE
 *******************************************************************************
 */

// Initialize a SHA1 digest
void esif_sha1_init(esif_sha1_t *self)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));
		self->digest_values[0] = 0x67452301;
		self->digest_values[1] = 0xefcdab89;
		self->digest_values[2] = 0x98badcfe;
		self->digest_values[3] = 0x10325476;
		self->digest_values[4] = 0xc3d2e1f0;
		self->digest_bits = 0;
	}
}

// Update SHA1 digest with arbitrary-sized data buffer
void esif_sha1_update(esif_sha1_t *self, const void *source, size_t bytes)
{
	if (self && source && bytes > 0) {
		const UInt8 *data = (const UInt8 *)source;
		while (self->blockbytes + bytes >= sizeof(self->block)) {
			esif_ccb_memcpy(self->block + self->blockbytes, data, sizeof(self->block) - self->blockbytes);
			sha1_add_block(self, self->block);

			data += sizeof(self->block) - self->blockbytes;
			bytes -= sizeof(self->block) - self->blockbytes;
			self->blockbytes = 0;
		}
		if (bytes > 0) {
			esif_ccb_memcpy(self->block + self->blockbytes, data, bytes);
			self->blockbytes += (UInt16)bytes;
		}
	}
}

// Finalize SHA1 digest and compute hash
void esif_sha1_finish(esif_sha1_t *self)
{
	if (self) {
		sha1_add_last_block(self, self->block, self->blockbytes * 8);
		sha1_compute_hash(self);

		esif_ccb_memset(self->block, 0, sizeof(self->block));
		esif_ccb_memset(self->digest_values, 0, sizeof(self->digest_values));
		self->blockbytes = 0;
	}
}

// Base64 Encode a Binary buffer into a Destination string
char *esif_base64_encode(
	char *destination,
	size_t dest_bytes,
	const void *source,
	size_t src_bytes)
{
	static const char base64_asciimap[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};
	char *result = NULL;
	size_t i = 0, j = 0;
	UInt8 block_of_bytes[4] = { 0 };
	const UInt8 *data = (const UInt8 *)source;

	if (dest_bytes >= (((src_bytes + 2) / 3) * 4) + 1) {

		for (i = 0; i < src_bytes / 3; ++i) {
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = ((data[i * 3 + 0] << 4) | (data[i * 3 + 1] >> 4)) & 0x3F;
			block_of_bytes[2] = ((data[i * 3 + 1] << 2) | (data[i * 3 + 2] >> 6)) & 0x3F;
			block_of_bytes[3] = data[i * 3 + 2] & 0x3F;

			for (j = 0; j < 4; ++j)
				*destination++ = base64_asciimap[block_of_bytes[j]];
		}

		switch (src_bytes % 3) {
		case 0:
			break;

		case 1:
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = (data[i * 3 + 0] << 4) & 0x3F;

			*destination++ = base64_asciimap[block_of_bytes[0]];
			*destination++ = base64_asciimap[block_of_bytes[1]];
			*destination++ = '=';
			*destination++ = '=';
			break;

		case 2:
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = ((data[i * 3 + 0] << 4) | (data[i * 3 + 1] >> 4)) & 0x3F;
			block_of_bytes[2] = (data[i * 3 + 1] << 2) & 0x3F;

			*destination++ = base64_asciimap[block_of_bytes[0]];
			*destination++ = base64_asciimap[block_of_bytes[1]];
			*destination++ = base64_asciimap[block_of_bytes[2]];
			*destination++ = '=';
			break;

		default:
			break;
		}

		*destination = '\0';
		result = destination;
	}
	return result;
}
