/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "esif_ccb_string.h"
#include "esif_sdk_base64.h"

// Base64 Decoding Helper Macros
#define BASE64_DECODED_BUFFER_LEN(encoded_len)	((((encoded_len) + 3) / 4) * 3)
#define BASE64_DECODED_ISVALID(ch)				(isalpha(ch) || isdigit(ch) || (ch) == '+' || (ch) == '/')
#define BASE64_DECODED_BYTE(ch)					((unsigned char)((ch) == '=' ? 0x00 : (ch) == '/' ? 0x3F : (ch) == '+' ? 0x3E : isdigit(ch) ? (ch) - '0' + 0x34 : islower(ch) ? (ch) - 'a' + 0x1A : (ch) - 'A'))

// Decode a Base64-Encoded Buffer into Binary Buffer
esif_error_t esif_base64_decode(
	unsigned char *target_buf,	// Target Buffer. May be NULL if *target_len = 0. Data must be padded with '=' if encoded_len not divisible by 4.
	size_t *target_len,			// Target Length: Buffer size if target_buf not NULL, Output: Decoded Length [if ESIF_OK] or Required Length [if ESIF_E_NEED_LARGER_BUFFER]
	const char *encoded_buf,	// Base-64 Encoded Buffer [Null-terminator optional]. May be NULL if *target_len = 0
	size_t encoded_len			// Base-64 Encoded Length, including Padding [not including optional Null-terminator]. May include whitespace.
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (encoded_len > MAX_BASE64_ENCODED_LEN) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	else if (target_len && *target_len < BASE64_DECODED_BUFFER_LEN(encoded_len)) {
		*target_len = BASE64_DECODED_BUFFER_LEN(encoded_len);
		rc = ESIF_E_NEED_LARGER_BUFFER;
	}
	else if (target_buf && target_len && encoded_buf) {
		size_t j = 0;
		size_t k = 0;
		size_t padding = 0;
		size_t whitespace = 0;
		const char *whitespace_chars = "\r\n\t ";
		unsigned char decoded[4] = { 0 };
		rc = ESIF_E_COMMAND_DATA_INVALID;

		// Ignore trailing whitespace
		while (encoded_len > 0 && esif_ccb_strchr(whitespace_chars, encoded_buf[encoded_len - 1]) != NULL) {
			encoded_len--;
			whitespace++;
		}

		// Decode each 4-byte text chunk into a 3 byte binary chunk (final binary chunk will be 1-3 bytes)
		for (j = 0, k = 0; j + 3 < encoded_len && k + 2 < *target_len; j += 4, k += 3) {
			if ((!BASE64_DECODED_ISVALID(encoded_buf[j]))
				|| (!BASE64_DECODED_ISVALID(encoded_buf[j + 1]))
				|| (!BASE64_DECODED_ISVALID(encoded_buf[j + 2]) && (j + 2 < encoded_len - 2 || encoded_buf[j + 2] != '='))
				|| (!BASE64_DECODED_ISVALID(encoded_buf[j + 3]) && (j + 3 < encoded_len - 2 || encoded_buf[j + 3] != '='))) {
				break;
			}
			decoded[0] = BASE64_DECODED_BYTE(encoded_buf[j]);
			decoded[1] = BASE64_DECODED_BYTE(encoded_buf[j + 1]);
			decoded[2] = BASE64_DECODED_BYTE(encoded_buf[j + 2]);
			decoded[3] = BASE64_DECODED_BYTE(encoded_buf[j + 3]);
			target_buf[k]     = ((decoded[0] & 0x3F) << 2) | ((decoded[1] & 0x30) >> 4);
			target_buf[k + 1] = ((decoded[1] & 0x0F) << 4) | ((decoded[2] & 0x3C) >> 2);
			target_buf[k + 2] = ((decoded[2] & 0x03) << 6) | ((decoded[3] & 0x3F));

			// Do not decode trailing padding bytes
			if (j + 4 == encoded_len) {
				if (encoded_buf[j + 2] == '=') {
					padding++;
				}
				if (encoded_buf[j + 3] == '=') {
					padding++;
				}
			}
			// Allow for whitespace every 4 bytes since many conversion tools add newlines every 64 bytes
			while (j + 4 < encoded_len && esif_ccb_strchr(whitespace_chars, encoded_buf[j + 4]) != NULL) {
				whitespace++;
				j++;
			}
		}
		// If buffer fully decoded, return success and actual decoded length
		if (j == encoded_len && k == *target_len - BASE64_DECODED_BUFFER_LEN(whitespace)) {
			*target_len = k - padding;
			rc = ESIF_OK;
		}
	}
	return rc;
}
