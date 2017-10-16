/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#include "esif_sdk_sha.h"

// Common Shared Macros
#define LEFTROTATE(a,b)		(((a) << (b)) | ((a) >> (32-(b))))
#define RIGHTROTATE(a,b)	(((a) >> (b)) | ((a) << (32-(b))))
#define REORDERBYTES(a)		(((a) << 24) | (((a) & 0x00ff0000) >> 8) | (((a) & 0x0000ff00) << 8) | ((a) >> 24))

#ifdef ESIF_ATTR_SHA1
/**************************************************************************************************
** SHA-1 Secure Hash Algorithm (For use by websocket code only)
**************************************************************************************************/

#define SHA1_BLOCK_BYTES		64	// Size of data blocks used in SHA1 digest computation
#define SHA1_DIGESTS			(SHA1_HASH_BYTES / sizeof(UInt32))	// Number of SHA1 32-bit Digest words
#define SHA1_ROUNDS				80	// Number of SHA1 Rounds

// Transform a 512-bit block and add it to the SHA1 Digest
static void esif_sha1_transform(esif_sha_t *self, UInt8 block[])
{
	static const UInt32 kvalues[4] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 }; // algorithm variable k
	UInt32 chunk[SHA1_DIGESTS] = { 0 };	// algorithm variable a,b,c,d,e
	UInt32 rounds[SHA1_ROUNDS] = { 0 };	// algorithm variable m
	UInt32 temp = 0;					// algorithm variable temp
	size_t i = 0;						// algorithm variable i
	UInt32 *digest_values = (UInt32 *)self->hash;

	// Process message in 512-bit chunks
	for (i = 0; i < SHA1_DIGESTS; i++) {
		chunk[i] = digest_values[i];
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
		digest_values[i] += chunk[i];
	}
}
#endif

#ifdef ESIF_ATTR_SHA256
/**************************************************************************************************
** SHA-256 Secure Hash Algorithm (SHA-2 with 256 bit output)
***************************************************************************************************/

#define SHA256_BLOCK_BYTES		64	// Size of data blocks used in SHA256 digest computation
#define SHA256_DIGESTS			(SHA256_HASH_BYTES / sizeof(UInt32))	// Number of SHA256 32-bit Digest words
#define SHA256_ROUNDS			64	// Number of SHA256 rounds in message schedule array

// Transform a 512-bit block and add it to the SHA256 Digest
static void esif_sha256_transform(esif_sha_t *self, const UInt8 block[])
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
	UInt32 *digest_values = (UInt32 *)self->hash;

	// Process message in 512-bit chunks
	for (i = 0; i < SHA256_DIGESTS; i++) {
		chunk[i] = digest_values[i];
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
		digest_values[i] += chunk[i];
	}
}
#endif

/**************************************************************************************************
** Secure Hash Algorithm Shared Code
**************************************************************************************************/

// Transform a 512-bit block and add it to the SHA1 Digest
static void esif_sha_transform(esif_sha_t *self)
{
	if (self) {
		switch (self->hashtype) {
#ifdef ESIF_ATTR_SHA1
		case SHA1_TYPE:
			esif_sha1_transform(self, self->block);
			break;
#endif
#ifdef ESIF_ATTR_SHA256
		case SHA256_TYPE:
			esif_sha256_transform(self, self->block);
			break;
#endif
		default:
			break;
		}
	}
}

// Initialize a SHA digest
void esif_sha_init(esif_sha_t *self, UInt16 hashtype)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));
		UInt32 *digest_values = (UInt32 *)self->hash;

		switch (hashtype) {
#ifdef ESIF_ATTR_SHA1
		case SHA1_TYPE:
			self->hashtype = SHA1_TYPE;
			self->hashsize = SHA1_HASH_BYTES;
			self->blocksize = SHA1_BLOCK_BYTES;
			digest_values[0] = 0x67452301;
			digest_values[1] = 0xefcdab89;
			digest_values[2] = 0x98badcfe;
			digest_values[3] = 0x10325476;
			digest_values[4] = 0xc3d2e1f0;
			break;
#endif
#ifdef ESIF_ATTR_SHA256
		case SHA2_TYPE:
		case SHA256_TYPE:
			self->hashtype = SHA256_TYPE;
			self->hashsize = SHA256_HASH_BYTES;
			self->blocksize = SHA256_BLOCK_BYTES;
			digest_values[0] = 0x6a09e667;
			digest_values[1] = 0xbb67ae85;
			digest_values[2] = 0x3c6ef372;
			digest_values[3] = 0xa54ff53a;
			digest_values[4] = 0x510e527f;
			digest_values[5] = 0x9b05688c;
			digest_values[6] = 0x1f83d9ab;
			digest_values[7] = 0x5be0cd19;
			break;
#endif
		default:
			break;
		}
	}
}

// Update SHA digest with arbitrary-sized data buffer
void esif_sha_update(esif_sha_t *self, const void *source, size_t bytes)
{
	if (self && source && bytes > 0 && self->blocksize >= sizeof(UInt64) && self->blocksize <= sizeof(self->block) && self->blockused < self->blocksize) {
		const UInt8 *octets = source;

		// Add data to digest and hash each 512-bit block
		while (self->blockused + bytes >= self->blocksize) {
			esif_ccb_memcpy(&self->block[self->blockused], octets, self->blocksize - self->blockused);
			esif_sha_transform(self);
			self->digest_bits += (self->blocksize * 8);
			octets += self->blocksize - self->blockused;
			bytes -= self->blocksize - self->blockused;
			self->blockused = 0;
		}
		esif_ccb_memcpy(&self->block[self->blockused], octets, bytes);
		self->blockused += (UInt16)bytes;
	}
}

// Finalize SHA digest and compute hash
void esif_sha_finish(esif_sha_t *self)
{
	if (self && self->blocksize >= sizeof(UInt64) && self->blocksize <= sizeof(self->block) && self->blockused < self->blocksize) {
		size_t j = 0;
		UInt32 *digest_values = (UInt32 *)self->hash;

		// Terminate data with 0x80 and pad remaining block with zeros
		self->block[self->blockused] = 0x80;
		if ((self->blockused + 1) < self->blocksize) {
			esif_ccb_memset(&self->block[self->blockused + 1], 0, self->blocksize - self->blockused - 1);
		}
		// Transform block and start a new one if block too full to append digest length
		if (self->blockused >= self->blocksize - sizeof(self->digest_bits)) {
			esif_sha_transform(self);
			esif_ccb_memset(self->block, 0, self->blocksize);
		}

		// Append digest length in bits to message and do one final transform
		self->digest_bits += (self->blockused * 8);
		UInt64 digest_length = ((UInt64)REORDERBYTES((UInt32)self->digest_bits) << 32) | REORDERBYTES((UInt32)(self->digest_bits >> 32));
		esif_ccb_memcpy(&self->block[self->blocksize - sizeof(digest_length)], &digest_length, sizeof(digest_length));
		esif_sha_transform(self);

		// Convert byte ordering from little-endian digests to big-endian binary hash
		for (j = 0; j < (self->hashsize / sizeof(UInt32)); j++) {
			digest_values[j] = REORDERBYTES(digest_values[j]);
		}
		esif_ccb_memset(self->block, 0, self->blocksize);
		self->blockused = 0;
		self->blocksize = 0;
	}
}

// Convert a completed SHA1 or SHA256 Hash into a string
const char *esif_sha_tostring(esif_sha_t *self, char *buffer, size_t buf_len)
{
	if (self && self->hashsize && self->blocksize == 0) {
		return esif_hash_tostring(self->hash, self->hashsize, buffer, buf_len);
	}
	return NULL;
}

// Convert a Hash buffer into a string
const char *esif_hash_tostring(UInt8 hash_bytes[], size_t hash_len, char *buffer, size_t buf_len)
{
	if (buffer && buf_len) {
		esif_ccb_memset(buffer, 0, buf_len);
		if (hash_bytes && buf_len > (2 * hash_len)) {
			UInt32 *digest_values = (UInt32 *)hash_bytes;
			size_t j = 0;
			for (j = 0; j < hash_len / sizeof(UInt32); j++) {
				UInt32 val = REORDERBYTES(digest_values[j]);
				esif_ccb_sprintf_concat(buf_len, buffer, "%08X", val);
			}
			return buffer;
		}
	}
	return NULL;
}
