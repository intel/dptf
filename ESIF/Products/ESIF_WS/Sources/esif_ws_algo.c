/*****************************************************************************
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
#define ESIF_TRACE_ID ESIF_TRACEMODULE_WEBSERVER

#include "esif_ws_algo.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Common Shared Macros
#define LEFTROTATE(a,b)		(((a) << (b)) | ((a) >> (32-(b))))
#define RIGHTROTATE(a,b)	(((a) >> (b)) | ((a) << (32-(b))))
#define REORDERBYTES(a)		(((a) << 24) | (((a) & 0x00ff0000) >> 8) | (((a) & 0x0000ff00) << 8) | ((a) >> 24))

// Convert a completed SHA1 or SHA256 Hash into a string
const char *esif_sha_tostring(UInt8 hash_bytes[], size_t hash_len, char *buffer, size_t buf_len)
{
	if (buffer && buf_len) {
		esif_ccb_memset(buffer, 0, buf_len);
		if (hash_bytes && buf_len > (2 * hash_len)) {
			size_t j = 0;
			for (j = 0; j < hash_len / sizeof(UInt32); j++) {
				UInt32 val = REORDERBYTES(((UInt32 *)hash_bytes)[j]);
				esif_ccb_sprintf_concat(buf_len, buffer, "%08X", val);
			}
			return buffer;
		}
	}
	return NULL;
}

/**************************************************************************************************
** SHA-1 Secure Hash Algorithm (For use by websocket code only)
**************************************************************************************************/

#define SHA1_ROUNDS			80	// Number of SHA1 Rounds

// Transform a 512-bit block and add it to the SHA1 Digest
static void esif_sha1_transform(esif_sha1_t *self, UInt8 block[])
{
	static const UInt32 kvalues[4] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 }; // algorithm variable k
	UInt32 chunk[SHA1_DIGESTS] = { 0 };	// algorithm variable a,b,c,d,e
	UInt32 rounds[SHA1_ROUNDS] = { 0 };	// algorithm variable m
	UInt32 temp = 0;					// algorithm variable temp
	size_t i = 0;						// algorithm variable i

	// Process message in 512-bit chunks
	for (i = 0; i < SHA1_DIGESTS; i++) {
		chunk[i] = self->digest_values[i];
	}
	// Break chunk into sixteen 32-bit words
	for (i = 0; i < 16; i++) {
		rounds[i] = (block[i * 4] << 24) + (block[(i * 4) + 1] << 16) + (block[(i * 4) + 2] << 8) + (block[(i * 4) + 3]);
	}
	// Extend the sixteen 32-bit words into eighty 32-bit words
	for (; i < SHA1_ROUNDS; i++) {
		rounds[i] = rounds[i - 3] ^ rounds[i - 8] ^ rounds[i - 14] ^ rounds[i - 16];
		rounds[i] = LEFTROTATE(rounds[i], 1);
	}

	// Apply Hash to each 512-bit chunk for the required eighty rounds
	for (i = 0; i < SHA1_ROUNDS; i++) {
		switch (i / 20) {
		case 0:
			temp = LEFTROTATE(chunk[0], 5) + ((chunk[1] & chunk[2]) ^ (~chunk[1] & chunk[3])) + chunk[4] + kvalues[0] + rounds[i];
			break;
		case 1:
			temp = LEFTROTATE(chunk[0], 5) + (chunk[1] ^ chunk[2] ^ chunk[3]) + chunk[4] + kvalues[1] + rounds[i];
			break;
		case 2:
			temp = LEFTROTATE(chunk[0], 5) + ((chunk[1] & chunk[2]) ^ (chunk[1] & chunk[3]) ^ (chunk[2] & chunk[3])) + chunk[4] + kvalues[2] + rounds[i];
			break;
		case 3:
			temp = LEFTROTATE(chunk[0], 5) + (chunk[1] ^ chunk[2] ^ chunk[3]) + chunk[4] + kvalues[3] + rounds[i];
			break;
		default:
			break;
		}
		chunk[4] = chunk[3];
		chunk[3] = chunk[2];
		chunk[2] = LEFTROTATE(chunk[1], 30);
		chunk[1] = chunk[0];
		chunk[0] = temp;
	}

	// Add chunk's hash to result so far
	for (i = 0; i < SHA1_DIGESTS; i++) {
		self->digest_values[i] += chunk[i];
	}
}

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
	}
}

// Update SHA1 digest with arbitrary-sized data buffer
void esif_sha1_update(esif_sha1_t *self, const void *source, size_t bytes)
{
	if (self && source && bytes > 0 && self->blockbytes < sizeof(self->block)) {
		const UInt8 *octets = source;

		// Add data to digest and hash each 512-bit block
		while (self->blockbytes + bytes >= sizeof(self->block)) {
			esif_ccb_memcpy(&self->block[self->blockbytes], octets, sizeof(self->block) - self->blockbytes);
			esif_sha1_transform(self, self->block);
			self->digest_bits += (sizeof(self->block) * 8);
			octets += sizeof(self->block) - self->blockbytes;
			bytes -= sizeof(self->block) - self->blockbytes;
			self->blockbytes = 0;
		}
		esif_ccb_memcpy(&self->block[self->blockbytes], octets, bytes);
		self->blockbytes += (UInt16)bytes;
	}
}

// Finalize SHA1 digest and compute hash
void esif_sha1_finish(esif_sha1_t *self)
{
	if (self && self->blockbytes < sizeof(self->block)) {
		size_t j = 0;

		// Terminate data with 0x80 and pad remaining block with zeros
		self->block[self->blockbytes] = 0x80;
		if ((self->blockbytes + 1) < sizeof(self->block)) {
			esif_ccb_memset(&self->block[self->blockbytes + 1], 0, sizeof(self->block) - self->blockbytes - 1);
		}
		// Transform block and start a new one if block too full to append digest length
		if (self->blockbytes >= sizeof(self->block) - sizeof(self->digest_bits)) {
			esif_sha1_transform(self, self->block);
			esif_ccb_memset(self->block, 0, sizeof(self->block));
		}

		// Append digest length in bits to message and do one final transform
		self->digest_bits += (self->blockbytes * 8);
		UInt64 digest_length = ((UInt64)REORDERBYTES((UInt32)self->digest_bits) << 32) | REORDERBYTES((UInt32)(self->digest_bits >> 32));
		esif_ccb_memcpy(&self->block[sizeof(self->block) - sizeof(digest_length)], &digest_length, sizeof(digest_length));
		esif_sha1_transform(self, self->block);

		// Convert byte ordering from little-endian digests to big-endian binary hash
		for (j = 0; j < sizeof(self->digest_values) / sizeof(self->digest_values[0]); j++) {
			((UInt32*)self->hash)[j] = REORDERBYTES(self->digest_values[j]);
		}
		esif_ccb_memset(self->block, 0, sizeof(self->block));
		esif_ccb_memset(self->digest_values, 0, sizeof(self->digest_values));
		self->blockbytes = 0;
	}
}

// Convert a SHA1 hash to a null-termianted string
const char *esif_sha1_tostring(esif_sha1_t *self, char *buffer, size_t buf_len)
{
	if (self) {
		return esif_sha_tostring(self->hash, sizeof(self->hash), buffer, buf_len);
	}
	return NULL;
}

/**************************************************************************************************
** SHA-256 Secure Hash Algorithm (SHA-2 with 256 bit output)
***************************************************************************************************/

#define SHA256_ROUNDS			64	// Number of SHA256 Rounds

// Transform a 512-bit block and add it to the SHA256 Digest
static void esif_sha256_transform(esif_sha256_t *self, const UInt8 block[])
{
	static const UInt32 kvalues[SHA256_ROUNDS] = { // algorithm variable k
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};
	UInt32 chunk[SHA256_DIGESTS] = { 0 };	// algorithm variable a,b,c,d,e,f,g,h
	UInt32 rounds[SHA256_ROUNDS] = { 0 };	// algorithm variable w
	UInt32 temp1 = 0, temp2 = 0;			// algorithm variable t1,t2
	size_t i = 0;							// algorithm variable i

	// Process message in 512-bit chunks
	for (i = 0; i < SHA256_DIGESTS; i++) {
		chunk[i] = self->digest_values[i];
	}
	// Break chunk into sixteen 32-bit words
	for (i = 0; i < 16; i++) {
		rounds[i] = (block[i * 4] << 24) | (block[(i * 4) + 1] << 16) | (block[(i * 4) + 2] << 8) | (block[(i * 4) + 3]);
	}
	// Extend the sixteen 32-bit words into the remaining forty-eight 32-bit words of the message schedule array
	for (; i < SHA256_ROUNDS; ++i) {
		UInt32 s0 = (RIGHTROTATE(rounds[i - 15], 7) ^ RIGHTROTATE(rounds[i - 15], 18) ^ (rounds[i - 15] >> 3)); // algorithm variable s0
		UInt32 s1 = (RIGHTROTATE(rounds[i - 2], 17) ^ RIGHTROTATE(rounds[i - 2], 19) ^ (rounds[i - 2] >> 10));	// algorithm variable s1					
		rounds[i] = rounds[i - 16] + s0 + rounds[i - 7] + s1;
	}

	// Apply Hash to each 512-bit chunk for the required sixty-four rounds
	for (i = 0; i < SHA256_ROUNDS; i++) {
		UInt32 S1 = (RIGHTROTATE(chunk[4], 6) ^ RIGHTROTATE(chunk[4], 11) ^ RIGHTROTATE(chunk[4], 25));	// algorithm variable S1
		UInt32 ch = ((chunk[4] & chunk[5]) ^ (~chunk[4] & chunk[6]));									// algorithm variable ch
		temp1 = chunk[7] + S1 + ch + kvalues[i] + rounds[i];

		UInt32 S0 = (RIGHTROTATE(chunk[0], 2) ^ RIGHTROTATE(chunk[0], 13) ^ RIGHTROTATE(chunk[0], 22));	// algorithm variable S0
		UInt32 maj = ((chunk[0] & chunk[1]) ^ (chunk[0] & chunk[2]) ^ (chunk[1] & chunk[2]));			// algorithm variable maj
		temp2 = S0 + maj;

		chunk[7] = chunk[6];
		chunk[6] = chunk[5];
		chunk[5] = chunk[4];
		chunk[4] = chunk[3] + temp1;
		chunk[3] = chunk[2];
		chunk[2] = chunk[1];
		chunk[1] = chunk[0];
		chunk[0] = temp1 + temp2;
	}

	// Add chunk's hash to result so far
	for (i = 0; i < SHA256_DIGESTS; i++) {
		self->digest_values[i] += chunk[i];
	}
}

// Initialize a SHA256 digest
void esif_sha256_init(esif_sha256_t *self)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));
		self->digest_values[0] = 0x6a09e667;
		self->digest_values[1] = 0xbb67ae85;
		self->digest_values[2] = 0x3c6ef372;
		self->digest_values[3] = 0xa54ff53a;
		self->digest_values[4] = 0x510e527f;
		self->digest_values[5] = 0x9b05688c;
		self->digest_values[6] = 0x1f83d9ab;
		self->digest_values[7] = 0x5be0cd19;
	}
}

// Update SHA256 digest with arbitrary-sized data buffer
void esif_sha256_update(esif_sha256_t *self, const void *source, size_t bytes)
{
	if (self && source && bytes > 0 && self->blockbytes < sizeof(self->block)) {
		const UInt8 *octets = source;

		// Add data to digest and hash each 512-bit block
		while (self->blockbytes + bytes >= sizeof(self->block)) {
			esif_ccb_memcpy(&self->block[self->blockbytes], octets, sizeof(self->block) - self->blockbytes);
			esif_sha256_transform(self, self->block);
			self->digest_bits += (sizeof(self->block) * 8);
			octets += sizeof(self->block) - self->blockbytes;
			bytes -= sizeof(self->block) - self->blockbytes;
			self->blockbytes = 0;
		}
		esif_ccb_memcpy(&self->block[self->blockbytes], octets, bytes);
		self->blockbytes += (UInt16)bytes;
	}
}

// Finalize SHA256 digest and compute hash
void esif_sha256_finish(esif_sha256_t *self)
{
	if (self && self->blockbytes < sizeof(self->block)) {
		size_t j = 0;

		// Terminate data with 0x80 and pad remaining block with zeros
		self->block[self->blockbytes] = 0x80;
		if ((self->blockbytes + 1) < sizeof(self->block)) {
			esif_ccb_memset(&self->block[self->blockbytes + 1], 0, sizeof(self->block) - self->blockbytes - 1);
		}
		// Transform block and start a new one if block too full to append digest length
		if (self->blockbytes >= sizeof(self->block) - sizeof(self->digest_bits)) {
			esif_sha256_transform(self, self->block);
			esif_ccb_memset(self->block, 0, sizeof(self->block));
		}

		// Append digest length in bits to message and do one final transform
		self->digest_bits += (self->blockbytes * 8);
		UInt64 digest_length = ((UInt64)REORDERBYTES((UInt32)self->digest_bits) << 32) | REORDERBYTES((UInt32)(self->digest_bits >> 32));
		esif_ccb_memcpy(&self->block[sizeof(self->block) - sizeof(digest_length)], &digest_length, sizeof(digest_length));
		esif_sha256_transform(self, self->block);

		// Convert byte ordering from little-endian digests to big-endian binary hash
		for (j = 0; j < sizeof(self->digest_values) / sizeof(self->digest_values[0]); j++) {
			((UInt32*)self->hash)[j] = REORDERBYTES(self->digest_values[j]);
		}
		esif_ccb_memset(self->block, 0, sizeof(self->block));
		esif_ccb_memset(self->digest_values, 0, sizeof(self->digest_values));
		self->blockbytes = 0;
	}
}

// Convert a SHA256 hash to a null-termianted string
const char *esif_sha256_tostring(esif_sha256_t *self, char *buffer, size_t buf_len)
{
	if (self) {
		return esif_sha_tostring(self->hash, sizeof(self->hash), buffer, buf_len);
	}
	return NULL;
}

/***************************** Base64 Encoding ******************************/

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
