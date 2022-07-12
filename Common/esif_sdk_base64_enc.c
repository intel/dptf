/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

// Base64 Encode a Binary buffer into a Target string
esif_error_t esif_base64_encode(
	char *target_str,
	size_t target_len,
	const void *source_buf,
	size_t source_len)
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
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (BASE64_ENCODED_LENGTH(source_len) > MAX_BASE64_ENCODED_LEN) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	else if (target_len < BASE64_ENCODED_LENGTH(source_len)) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
	}
	else if (source_buf && target_str) {
		UInt8 block_of_bytes[4] = { 0 };
		const UInt8* data = (const UInt8*)source_buf;
		size_t i = 0, j = 0;

		for (i = 0; i < source_len / 3; ++i) {
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = ((data[i * 3 + 0] << 4) | (data[i * 3 + 1] >> 4)) & 0x3F;
			block_of_bytes[2] = ((data[i * 3 + 1] << 2) | (data[i * 3 + 2] >> 6)) & 0x3F;
			block_of_bytes[3] = data[i * 3 + 2] & 0x3F;

			for (j = 0; j < 4; ++j)
				*target_str++ = base64_asciimap[block_of_bytes[j]];
		}

		switch (source_len % 3) {
		case 0:
			break;

		case 1:
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = (data[i * 3 + 0] << 4) & 0x3F;

			*target_str++ = base64_asciimap[block_of_bytes[0]];
			*target_str++ = base64_asciimap[block_of_bytes[1]];
			*target_str++ = '=';
			*target_str++ = '=';
			break;

		case 2:
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = ((data[i * 3 + 0] << 4) | (data[i * 3 + 1] >> 4)) & 0x3F;
			block_of_bytes[2] = (data[i * 3 + 1] << 2) & 0x3F;

			*target_str++ = base64_asciimap[block_of_bytes[0]];
			*target_str++ = base64_asciimap[block_of_bytes[1]];
			*target_str++ = base64_asciimap[block_of_bytes[2]];
			*target_str++ = '=';
			break;

		default:
			break;
		}

		*target_str = '\0';
		rc = ESIF_OK;
	}
	return rc;
}
