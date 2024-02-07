/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#pragma once

#include "esif_ccb.h"

#define	ESIFCMP_LIBRARY				"ipf_cmp"			// Name of Loadable .dll or .so Library
#define	ESIFCMP_COMPRESSOR			"IpfCompress"		// Name of Exported Symbol in Library to Compress data
#define	ESIFCMP_DECOMPRESSOR		"IpfDecompress"		// Name of Exported Symbol in Library to Decompress data
#define ESIFCMP_ERROR_OUTPUT_EOF	7					// Output Buffer too small return code (SZ_ERROR_OUTPUT_EOF)

/* Exported Data Compression Function Pointer (ESIFCMP_COMPRESSOR)
 * Call with NULL dest to compute required destLen
 * Returns 0 on success, nonzero on error
 */
typedef int (ESIF_CALLCONV *IpfCompressFuncPtr)(
	unsigned char *dest,
	size_t *destLen,
	const unsigned char *src,
	size_t srcLen
);

/* Exported Data Decompression Function Pointer (ESIFCMP_DECOMPRESSOR)
 * Call with NULL dest to compute required destLen
 * Returns 0 on success, nonzero on error
 */
typedef int (ESIF_CALLCONV *IpfDecompressFuncPtr)(
	unsigned char *dest,
	size_t *destLen,
	const unsigned char *src,
	size_t srcLen
);

/* Inline function to detect whether a data buffer appears to be compressed
 * without having to dynamically load the ESIF Compression Loadable Library first.
 * Returns nonzero if data appears to be compressed.
 *
 * Note that you can also verify that a data buffer is compressed by first loading
 * the ESIF Compression Loadable Library and then calling (*IpfDecompressFuncPtr)
 * with a NULL dest. If it returns 0, the data is compressed and *destLen=decompressed size
 */
#include <memory.h>

/* These are only used by EsfCmp_IsCompressed and IPF_CMP Library; Do not use in Client code.
 */
#define ESIFCMP_SIGNATURE		{'\x5D','\x00'}	// Always [5D 00] for LZMA Properties -lc3 -lp0 -pb2 and -d12 to -d27 lzma.exe options
#define ESIFCMP_HEADER_SIZE		13				// 5-byte LZMA Properties + UInt64 uncompressed size

static ESIF_INLINE int EsifCmp_IsCompressed(
	unsigned char *src,
	size_t srcLen
)
{
	int rc = ESIF_FALSE;
	unsigned char signature[] = ESIFCMP_SIGNATURE;
	if (src && srcLen > ESIFCMP_HEADER_SIZE && memcmp(src, &signature, sizeof(signature)) == 0) {
		rc = ESIF_TRUE;
	}
	return rc;
}
