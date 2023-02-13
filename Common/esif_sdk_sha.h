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

#pragma once

#include "esif_ccb.h"
#include "esif_ccb_string.h"

/*********************************************************************************
 * Secure Hash Algorithm Public Interface (SHA-1, SHA-256, SHA-384, SHA-512)
 *********************************************************************************/

// Suppport only SHA-1 and SHA-256 by default unless one or more algorithms are specified
#if !defined(ESIF_ATTR_SHA1) && !defined(ESIF_ATTR_SHA256) && !defined(ESIF_ATTR_SHA384) && !defined(ESIF_ATTR_SHA512)
# define ESIF_ATTR_SHA1
# define ESIF_ATTR_SHA256
#endif

#if defined(ESIF_ATTR_SHA384) || defined(ESIF_ATTR_SHA512)
#define SHA_HASH_BYTES			64	// SHA512=64: Max size of binary hash in bytes (512 bits)
#define SHA_BLOCK_BYTES			128	// SHA512=128: Max size of SHA block size in bytes (1024 bits)
#elif defined(ESIF_ATTR_SHA256)
#define SHA_HASH_BYTES			32	// SHA256=32: Size of binary hash in bytes (256 bits)
#define SHA_BLOCK_BYTES			64	// SHA256=64 SHA512=128: Size of SHA block size in bytes (512 bits)
#elif defined(ESIF_ATTR_SHA1)
#define SHA_HASH_BYTES			20	// SHA1=20: Size of binary hash in bytes (160 bits)
#define SHA_BLOCK_BYTES			64	// SHA1=64: Size of SHA block size in bytes (512 bits)
#endif
#define SHA_STRING_BYTES		((SHA_HASH_BYTES * 2) + 1)			// Max size of SHA string plus NUL

typedef struct esif_sha_s {
	u16 hashtype;					// SHA1=1 SHA256=256 SHA512=512: Hash Type
	u8  hashsize;					// SHA1=20 SHA256=32 SHA512=64 : Variable Length Hash Size in bytes
	u8  blocksize;					// SHA1=64 SHA256=64 SHA512=128: Variable-length blocksize in bytes
	u8  hash[SHA_HASH_BYTES];		// SHA1=20 SHA256=32 SHA512=64 : Variable-length Hash
	u8  block[SHA_BLOCK_BYTES];		// SHA1=64 SHA256=64 SHA512=128: Current partially-filled block
	u16 blockused;					// Number of bytes in current partially-filled block
    u64 digest_bits;  				// Total bits used to compute SHA digest (Low-Order 64 bits)  [All SHAs]
    u64 digest_hibits;	   			// Total bits used to compute SHA digest (High-Order 64 bits) [SHA-384+ only]
} esif_sha_t;

void esif_sha_init(esif_sha_t *self, u16 hashtype);
void esif_sha_update(esif_sha_t *self, const void *source, size_t bytes);
void esif_sha_finish(esif_sha_t *self);
const char *esif_sha_tostring(esif_sha_t *self, char *buffer, size_t buf_len);
const char *esif_hash_tostring(u8 hash_bytes[], size_t hash_len, char *buffer, size_t buf_len);

#ifdef ESIF_ATTR_SHA1
/*
 * SHA-1 Digest (SHA-1 with 512-bit block and 160-bit output)
 */
#define	SHA1_TYPE				1	// Hash Type
#define SHA1_HASH_BYTES			20	// Size of resulting SHA1 binary hash
#define SHA1_STRING_BYTES		((SHA1_HASH_BYTES * 2) + 1)		// Size of SHA1 string plus NUL

#define esif_sha1_t								esif_sha_t
#define esif_sha1_init(self)					esif_sha_init(self, SHA1_TYPE)
#define esif_sha1_update(self, src, len)		esif_sha_update(self, src, len)
#define esif_sha1_finish(self)					esif_sha_finish(self)
#define esif_sha1_tostring(self, buf, len)		esif_sha_tostring(self, buf, len)
#endif

#ifdef ESIF_ATTR_SHA256
/*
 * SHA-256 Digest (SHA-2 with 512-bit block and 256-bit output)
 */
#define SHA256_TYPE				256	// Hash Type
#define SHA256_HASH_BYTES		32	// Size of resulting SHA256 binary hash
#define SHA256_STRING_BYTES		((SHA256_HASH_BYTES * 2) + 1)	// Size of SHA256 string plus NUL

#define esif_sha256_t							esif_sha_t
#define esif_sha256_init(self)					esif_sha_init(self, SHA256_TYPE)
#define esif_sha256_update(self, src, len)		esif_sha_update(self, src, len)
#define esif_sha256_finish(self)				esif_sha_finish(self)
#define esif_sha256_tostring(self, buf, len)	esif_sha_tostring(self, buf, len)
#endif

#ifdef ESIF_ATTR_SHA384
/*
 * SHA-384 Digest (SHA-2 with 1024-bit block and 384-bit output)
 */
#define SHA384_TYPE				384	// Hash Type
#define SHA384_HASH_BYTES		48	// Size of resulting SHA384 binary hash
#define SHA384_STRING_BYTES		((SHA384_HASH_BYTES * 2) + 1) // Size of SHA384 string plus NUL

#define esif_sha384_t                           esif_sha_t
#define esif_sha384_init(self)					esif_sha_init(self, SHA384_TYPE)
#define esif_sha384_update(self, src, len)		esif_sha_update(self, src, len)
#define esif_sha384_finish(self)				esif_sha_finish(self)
#define esif_sha384_tostring(self, buf, len)    esif_sha_tostring(self, buf, len)
#endif

#ifdef ESIF_ATTR_SHA512
 /*
  * SHA-512 Digest (SHA-2 with 1024-bit block and 512-bit output)
  */
#define SHA512_TYPE				512	// Hash Type
#define SHA512_HASH_BYTES		64	// Size of resulting SHA512 binary hash
#define SHA512_STRING_BYTES		((SHA512_HASH_BYTES * 2) + 1)	// Size of SHA512 string plus NUL

#define esif_sha512_t							esif_sha_t
#define esif_sha512_init(self)					esif_sha_init(self, SHA512_TYPE)
#define esif_sha512_update(self, src, len)		esif_sha_update(self, src, len)
#define esif_sha512_finish(self)				esif_sha_finish(self)
#define esif_sha512_tostring(self, buf, len)	esif_sha_tostring(self, buf, len)
#endif

// Include SHA Main Module if this symbol defined
#ifdef ESIF_SDK_SHA_MAIN
#include "esif_sdk_sha.c"
#endif
