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

#pragma once

#include "esif_ccb.h"
#include "esif_ccb_memory.h"

#if defined(ESIF_ATTR_KERNEL)

#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_string_win_kern.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_string_lin_kern.h"
#endif

#elif defined(ESIF_ATTR_USER)

#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_string_win_user.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_string_lin_user.h"
#endif

#endif /* USER */

/* OS/Kernel Agnostic */

/* Case-insensitive string pattern match function (only "*" and "?" supported) */
static ESIF_INLINE int esif_ccb_strmatch(char *string, char *pattern)
{
	while (*pattern || *string) {
		if (*pattern == '*') {
			return (esif_ccb_strmatch(string, pattern + 1) || (*string && esif_ccb_strmatch(string + 1, pattern)));
		}
		if (*string && ((*pattern == '?') || (*pattern && (tolower(*string) == tolower(*pattern))))) {
			string++, pattern++;
			continue;
		}
		return ESIF_FALSE;
	}
	return ESIF_TRUE;
}

/* Copy up to dest_len bytes from src string to dst, padding with zeros.
 * Note: Behaves like strncpy() - dest is NOT null terminated if strlen(src) >= dest_len
 * Use for copying non-null terminated string buffers to/from null terminated strings
 */
static ESIF_INLINE void *esif_ccb_strmemcpy(
	char *dest,
	size_t dest_len,
	const char *src,
	size_t src_len
)
{
	size_t bytes = esif_ccb_min(dest_len, esif_ccb_strlen(src, src_len));
	if (bytes > 0)
		esif_ccb_memcpy(dest, src, bytes);
	if (bytes < dest_len)
		esif_ccb_memset(dest + bytes, 0, dest_len - bytes);
	return dest;
}

