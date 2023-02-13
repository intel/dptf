/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#define LEFTROTATE(a,b)		(((a) << (b)) | ((a) >> ((sizeof(a)*8)-(b))))
#define RIGHTROTATE(a,b)	(((a) >> (b)) | ((a) << ((sizeof(a)*8)-(b))))
#define REORDERBYTES(a)		(((a) << 24) | (((a) & 0x00ff0000) >> 8) | (((a) & 0x0000ff00) << 8) | ((a) >> 24))
#define REORDERBYTES64(a)	(((a) << 56) | (((a) & 0x00ff000000000000) >> 40) | (((a) & 0x0000ff0000000000) >> 24) | (((a) & 0x000000ff00000000) >> 8) | (((a) & 0x00000000ff000000) << 8) | (((a) & 0x0000000000ff0000) << 24) | (((a) & 0x000000000000ff00) << 40) | ((a) >> 56))

#ifdef ESIF_ATTR_SHA1
/**************************************************************************************************
** SHA-1 Secure Hash Algorithm (SHA-1 with 512-bit block and 160-bit output) [WebSocket handshake only]
** Implementation based on psuedocode for SHA-1 algorithm at http://en.wikipedia.org/wiki/SHA-1
**************************************************************************************************/

#define SHA1_BLOCK_BYTES		64	// Size of data blocks used in SHA1 digest computation
#define SHA1_DIGESTS			5	// Number of SHA1 32-bit Digest words
#define SHA1_ROUNDS				80	// Number of SHA1 Rounds

// Transform a 512-bit block and add it to the SHA1 Digest
static void esif_sha1_transform(esif_sha_t *self)
{
	static const u32 kvalues[4] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 }; // algorithm variable k
	u32 chunk[SHA1_DIGESTS] = { 0 };	// algorithm variable a,b,c,d,e
	u32 rounds[SHA1_ROUNDS] = { 0 };	// algorithm variable m
	u32 temp = 0;						// algorithm variable temp
	size_t i = 0;						// algorithm variable i
	u32 *digest_values = (u32 *)self->hash;

	// Process message in 512-bit chunks
	for (i = 0; i < SHA1_DIGESTS; i++) {
		chunk[i] = digest_values[i];
	}
	// Break chunk into sixteen 32-bit words
	for (i = 0; i < 16; i++) {
		rounds[i] = REORDERBYTES(((u32 *)self->block)[i]);
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
** SHA-256 Secure Hash Algorithm (SHA-2 with 512-bit block and 256-bit output)
** Implementation based on psuedocode for SHA-2 algorithms at http://en.wikipedia.org/wiki/SHA-2
***************************************************************************************************/

#define SHA256_BLOCK_BYTES		64	// Size of data blocks used in SHA256 digest computation
#define SHA256_DIGESTS			8	// Number of SHA256 32-bit Digest words
#define SHA256_ROUNDS			64	// Number of SHA256 rounds in message schedule array

// Transform a 512-bit block and add it to the SHA256 Digest
static void esif_sha256_transform(esif_sha_t *self)
{
	static const u32 kvalues[SHA256_ROUNDS] = { // algorithm variable k
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};
	u32 chunk[SHA256_DIGESTS] = { 0 };	// algorithm variable a,b,c,d,e,f,g,h
	u32 rounds[SHA256_ROUNDS] = { 0 };	// algorithm variable w
	u32 temp1 = 0, temp2 = 0;			// algorithm variable t1,t2
	size_t i = 0;						// algorithm variable i
	u32 *digest_values = (u32 *)self->hash;

	// Process message in 512-bit chunks
	for (i = 0; i < SHA256_DIGESTS; i++) {
		chunk[i] = digest_values[i];
	}
	// Break chunk into sixteen 32-bit words
	for (i = 0; i < 16; i++) {				
		rounds[i] = REORDERBYTES(((u32 *)self->block)[i]);
	}
	// Extend the sixteen 32-bit words into the remaining forty-eight 32-bit words of the message schedule array
	for (; i < SHA256_ROUNDS; ++i) {
		u32 s0 = (RIGHTROTATE(rounds[i - 15], 7) ^ RIGHTROTATE(rounds[i - 15], 18) ^ (rounds[i - 15] >> 3));// algorithm variable s0
		u32 s1 = (RIGHTROTATE(rounds[i - 2], 17) ^ RIGHTROTATE(rounds[i - 2], 19) ^ (rounds[i - 2] >> 10));	// algorithm variable s1
		rounds[i] = rounds[i - 16] + s0 + rounds[i - 7] + s1;
	}

	// Apply Hash to each 512-bit chunk for the required sixty-four rounds
	for (i = 0; i < SHA256_ROUNDS; i++) {
		u32 S1 = (RIGHTROTATE(chunk[4], 6) ^ RIGHTROTATE(chunk[4], 11) ^ RIGHTROTATE(chunk[4], 25));	// algorithm variable S1
		u32 ch = ((chunk[4] & chunk[5]) ^ (~chunk[4] & chunk[6]));										// algorithm variable ch

		u32 S0 = (RIGHTROTATE(chunk[0], 2) ^ RIGHTROTATE(chunk[0], 13) ^ RIGHTROTATE(chunk[0], 22));	// algorithm variable S0
		u32 maj = ((chunk[0] & chunk[1]) ^ (chunk[0] & chunk[2]) ^ (chunk[1] & chunk[2]));				// algorithm variable maj

		temp1 = chunk[7] + S1 + ch + kvalues[i] + rounds[i];
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

#if defined(ESIF_ATTR_SHA384) || defined(ESIF_ATTR_SHA512)
/**************************************************************************************************
** SHA-384 Secure Hash Algorithm (SHA-2 with 1024-bit block and 384-bit output)
** SHA-512 Secure Hash Algorithm (SHA-2 with 1024-bit block and 512-bit output)
** Implementation based on psuedocode for SHA-2 algorithms at http://en.wikipedia.org/wiki/SHA-2
***************************************************************************************************/

#define SHA512_BLOCK_BYTES		128	// Size of data blocks used in SHA512 digest computation
#define SHA512_DIGESTS			8	// Number of SHA512 64-bit Digest words
#define SHA512_ROUNDS			80	// Number of SHA512 rounds in message schedule array

// Transform a 512-bit block and add it to the SHA512 Digest
static void esif_sha512_transform(esif_sha_t *self)
{
	static const u64 kvalues[SHA512_ROUNDS] = { // algorithm variable k
		0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538, 
		0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe, 
		0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 
		0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 
		0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab, 
		0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725, 
		0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 
		0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b, 
		0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218, 
		0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 
		0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 
		0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec, 
		0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c, 
		0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6, 
		0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 
		0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
	};
	u64 chunk[SHA512_DIGESTS] = { 0 };	// algorithm variable a,b,c,d,e,f,g,h
	u64 rounds[SHA512_ROUNDS] = { 0 };	// algorithm variable w
	u64 temp1 = 0, temp2 = 0;			// algorithm variable t1,t2
	size_t i = 0;						// algorithm variable i
	u64 *digest_values = (u64 *)self->hash;

	// Process message in 1024-bit chunks
	for (i = 0; i < SHA512_DIGESTS; i++) {
		chunk[i] = digest_values[i];
	}
	// Break chunk into sixteen 64-bit words
	for (i = 0; i < 16; i++) {
		rounds[i] = REORDERBYTES64(((u64 *)self->block)[i]);
	}
	// Extend the sixteen 64-bit words into the remaining sixty-four 64-bit words of the message schedule array
	for (; i < SHA512_ROUNDS; ++i) {
		u64 s0 = (RIGHTROTATE(rounds[i - 15], 1) ^ RIGHTROTATE(rounds[i - 15], 8) ^ (rounds[i - 15] >> 7)); // algorithm variable s0
		u64 s1 = (RIGHTROTATE(rounds[i - 2], 19) ^ RIGHTROTATE(rounds[i - 2], 61) ^ (rounds[i - 2] >> 6));	// algorithm variable s1
		rounds[i] = rounds[i - 16] + s0 + rounds[i - 7] + s1;
	}

	// Apply Hash to each 1024-bit chunk for the required eighty rounds
	for (i = 0; i < SHA512_ROUNDS; i++) {
		u64 S1 = (RIGHTROTATE(chunk[4], 14) ^ RIGHTROTATE(chunk[4], 18) ^ RIGHTROTATE(chunk[4], 41));	// algorithm variable S1
		u64 ch = ((chunk[4] & chunk[5]) ^ (~chunk[4] & chunk[6]));										// algorithm variable ch

		u64 S0 = (RIGHTROTATE(chunk[0], 28) ^ RIGHTROTATE(chunk[0], 34) ^ RIGHTROTATE(chunk[0], 39));	// algorithm variable S0
		u64 maj = ((chunk[0] & chunk[1]) ^ (chunk[0] & chunk[2]) ^ (chunk[1] & chunk[2]));				// algorithm variable maj

		temp1 = chunk[7] + S1 + ch + kvalues[i] + rounds[i];
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
	for (i = 0; i < SHA512_DIGESTS; i++) {
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
			esif_sha1_transform(self);
			break;
#endif
#ifdef ESIF_ATTR_SHA256
		case SHA256_TYPE:
			esif_sha256_transform(self);
			break;
#endif
#ifdef ESIF_ATTR_SHA384
		case SHA384_TYPE:
#endif
#ifdef ESIF_ATTR_SHA512
		case SHA512_TYPE:
#endif
#if defined(ESIF_ATTR_SHA384) || defined(ESIF_ATTR_SHA512)
			esif_sha512_transform(self);
			break;
#endif
		default:
			break;
		}
	}
}

// Initialize a SHA digest
void esif_sha_init(esif_sha_t *self, u16 hashtype)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));

		switch (hashtype) {
#ifdef ESIF_ATTR_SHA1
		case SHA1_TYPE:
		{
			u32 *digest_values = (u32 *)self->hash;
			self->hashtype = SHA1_TYPE;
			self->hashsize = SHA1_HASH_BYTES;
			self->blocksize = SHA1_BLOCK_BYTES;
			digest_values[0] = 0x67452301;
			digest_values[1] = 0xefcdab89;
			digest_values[2] = 0x98badcfe;
			digest_values[3] = 0x10325476;
			digest_values[4] = 0xc3d2e1f0;
			break;
		}
#endif
#ifdef ESIF_ATTR_SHA256
		case SHA256_TYPE:
		{
			u32 *digest_values = (u32 *)self->hash;
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
		}
#endif
#ifdef ESIF_ATTR_SHA384
		case SHA384_TYPE:
		{
			u64* digest_values = (u64*)self->hash;
			self->hashtype = SHA384_TYPE;
			self->hashsize = SHA384_HASH_BYTES;
			self->blocksize = SHA512_BLOCK_BYTES;
			digest_values[0] = 0xcbbb9d5dc1059ed8;
			digest_values[1] = 0x629a292a367cd507;
			digest_values[2] = 0x9159015a3070dd17;
			digest_values[3] = 0x152fecd8f70e5939;
			digest_values[4] = 0x67332667ffc00b31;
			digest_values[5] = 0x8eb44a8768581511;
			digest_values[6] = 0xdb0c2e0d64f98fa7;
			digest_values[7] = 0x47b5481dbefa4fa4;
			break;
		}
#endif
#ifdef ESIF_ATTR_SHA512
		case SHA512_TYPE:
		{
			u64 *digest_values = (u64 *)self->hash;
			self->hashtype = SHA512_TYPE;
			self->hashsize = SHA512_HASH_BYTES;
			self->blocksize = SHA512_BLOCK_BYTES;
			digest_values[0] = 0x6a09e667f3bcc908;
			digest_values[1] = 0xbb67ae8584caa73b;
			digest_values[2] = 0x3c6ef372fe94f82b;
			digest_values[3] = 0xa54ff53a5f1d36f1;
			digest_values[4] = 0x510e527fade682d1;
			digest_values[5] = 0x9b05688c2b3e6c1f;
			digest_values[6] = 0x1f83d9abfb41bd6b;
			digest_values[7] = 0x5be0cd19137e2179;
			break;
		}
#endif
		default:
			break;
		}
	}
}

// Update SHA digest with arbitrary-sized data buffer
void esif_sha_update(esif_sha_t *self, const void *source, size_t bytes)
{
	if (self && source && bytes > 0 && self->blocksize >= sizeof(u64) && self->blocksize <= sizeof(self->block) && self->blockused < self->blocksize) {
		const u8 *octets = (const u8 *)source;

		// Add data to digest and hash each bit block
		while (self->blockused + bytes >= self->blocksize) {
			u64 bits = 0;
			esif_ccb_memcpy(&self->block[self->blockused], octets, self->blocksize - self->blockused);
			esif_sha_transform(self);
			bits = self->digest_bits;
			self->digest_bits += (u64)self->blocksize * 8;
			self->digest_hibits += (self->digest_bits < bits);
			octets += self->blocksize - self->blockused;
			bytes -= self->blocksize - self->blockused;
			self->blockused = 0;
		}
		esif_ccb_memcpy(&self->block[self->blockused], octets, bytes);
		self->blockused += (u16)bytes;
	}
}

// Finalize SHA digest and compute hash
void esif_sha_finish(esif_sha_t *self)
{
	if (self && self->blocksize >= sizeof(u64) && self->blocksize <= sizeof(self->block) && self->blockused < self->blocksize) {
		size_t j = 0;
		size_t digest_size = 0;
		u64 bits = 0;

		// Terminate data with 0x80 and pad remaining block with zeros
		self->block[self->blockused] = 0x80;
		if ((self->blockused + 1) < self->blocksize) {
			esif_ccb_memset(&self->block[self->blockused + 1], 0, (size_t)self->blocksize - self->blockused - 1);
		}
		// Transform block and start a new one if block too full to append digest length
		digest_size += sizeof(self->digest_bits);
		switch (self->hashtype) {
#ifdef ESIF_ATTR_SHA384
		case SHA384_TYPE:
#endif
#ifdef ESIF_ATTR_SHA512
		case SHA512_TYPE:
#endif
#if defined(ESIF_ATTR_SHA384) || defined(ESIF_ATTR_SHA512)
			// Use 128-bit digest length for SHA-512-based algorithms
			digest_size += sizeof(self->digest_hibits);
			break;
#endif
		default:
			break;
		}
		if (self->blockused >= self->blocksize - digest_size) {
			esif_sha_transform(self);
			esif_ccb_memset(self->block, 0, self->blocksize);
		}

		// Add Blocksize to Digest Length
		bits = self->digest_bits;
		self->digest_bits += (u64)self->blockused * 8;
		self->digest_hibits += (self->digest_bits < bits);
		
		switch (self->hashtype) {
#if defined(ESIF_ATTR_SHA1)
		case SHA1_TYPE:
#endif
#if defined(ESIF_ATTR_SHA256)
		case SHA256_TYPE:
#endif
#if defined(ESIF_ATTR_SHA1) || defined(ESIF_ATTR_SHA256)
		{
			// Append digest length in bits to message and do one final transform
			u64 digest_length = REORDERBYTES64(self->digest_bits);
			esif_ccb_memcpy(&self->block[self->blocksize - sizeof(digest_length)], &digest_length, sizeof(digest_length));
			esif_sha_transform(self);

			// Convert byte ordering from little-endian digests to big-endian binary hash
			u32 *digest_values = (u32 *)self->hash;
			for (j = 0; j < (self->hashsize / sizeof(u32)); j++) {
				digest_values[j] = REORDERBYTES(digest_values[j]);
			}
			break;
		}
#endif
#ifdef ESIF_ATTR_SHA384
		case SHA384_TYPE:
#endif
#ifdef ESIF_ATTR_SHA512
		case SHA512_TYPE:
#endif
#if defined(ESIF_ATTR_SHA384) || defined(ESIF_ATTR_SHA512)
		{
			// Append digest length in bits to message and do one final transform
			u64 *digest_values = NULL;
			u64 digest_length = REORDERBYTES64(self->digest_hibits);
			esif_ccb_memcpy(&self->block[self->blocksize - digest_size], &digest_length, sizeof(digest_length));
			digest_length = REORDERBYTES64(self->digest_bits);
			esif_ccb_memcpy(&self->block[self->blocksize - sizeof(digest_length)], &digest_length, sizeof(digest_length));
			esif_sha_transform(self);

			// Convert byte ordering from little-endian digests to big-endian binary hash
			digest_values = (u64 *)self->hash;
			for (j = 0; j < (self->hashsize / sizeof(u64)); j++) {
				digest_values[j] = REORDERBYTES64(digest_values[j]);
			}
			break;
		}
#endif
		default:
			break;
		}
		esif_ccb_memset(self->block, 0, self->blocksize);
		self->blockused = 0;
		self->blocksize = 0;
	}
}


// Convert a completed SHA1 or SHA256 Hash into a string
const char *esif_sha_tostring(esif_sha_t *self, char *buffer, size_t buf_len)
{
	if (self && self->blocksize == 0) {
		return esif_hash_tostring(self->hash, self->hashsize, buffer, buf_len);
	}
	return NULL;
}

// Convert a Hash buffer into a string
const char *esif_hash_tostring(u8 hash_bytes[], size_t hash_len, char *buffer, size_t buf_len)
{
	if (buffer && buf_len) {
		esif_ccb_memset(buffer, 0, buf_len);
		if (hash_bytes && hash_len && buf_len > (2 * hash_len)) {
			size_t j = 0;
			for (j = 0; j < hash_len; j++) {
				u8 hi = ((hash_bytes[j] & 0xf0) >> 4);
				u8 lo = (hash_bytes[j] & 0x0f);
				buffer[(j * 2)] = (hi < 10 ? hi + '0' : hi - 10 + 'a');
				buffer[(j * 2) + 1] = (lo < 10 ? lo + '0' : lo - 10 + 'a');
			}
			return buffer;
		}
	}
	return NULL;
}

