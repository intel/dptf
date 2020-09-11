/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

/* Main Loadable Library Module used by ESIF Compression Library
 * This is the only C module owned by ESIF, the rest are owned by LZMA_SDK.
 */

#include "Precomp.h"
#include "Alloc.h"
#include "LzmaDec.h"
#include "LzmaEnc.h"

#include "esif_sdk_iface_compress.h"

#include "EsifSdl.h" /* Always include last */

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNUSED_VAR(hInstance);
	UNUSED_VAR(dwReason);
	UNUSED_VAR(lpReserved);
	return TRUE;
}
#endif

// Standard LZMA File Header
#define LZMA_PROPS_SIZE	5	// [XX YY YY YY YY] Where XX = Encoded -lc -lp -pb options, YY = Encoded -d option
#pragma pack(push, 1)
struct LzmaHeader {
	unsigned char		properties[LZMA_PROPS_SIZE];// encoded LZMA_PROPS options
	unsigned long long	original_size;				// original uncompressed data size	
};
#pragma pack(pop)

/* Hardcoded LZMA Compression Property Values (and their LZMA_SDK v18.01 lzma.exe command line equivalents)
 * Items marked with "##" should never be changed since they affect the 5-byte LZMA Properties Header Signature
 * The following parameters correspond to to the ESIF_COMPRESS_SIGNATURE defined in esif_sdk_iface_compress.h,
 * which always maps to [5D 00 XX XX XX] for -lc3 -lp0 -pb2 and -d12 to -d27 lzma.exe options.
 */
#define LZMA_PROPS_LEVEL			9			// Compression Level [-a1 = 9]
#define LZMA_PROPS_DICTSIZE			(1 << 24)	// Dictionary Size [-d24] ##
#define LZMA_PROPS_LITCTXBITS		3			// Literal Context Bits [-lc3] ##
#define LZMA_PROPS_LITPOSBITS		0			// Literal Pos Bits [-lp0] ##
#define LZMA_PROPS_NUMPOSBITS		2			// Number of Pos Bits [-pb2] ##
#define LZMA_PROPS_FASTBYTES		128			// Number of Fast Bytes [-fb128]
#define LZMA_PROPS_THREADS			1			// Number of Threads [-mt1]

#define LZMA_PADDING_MINSIZE		256			// Minimum Padding Bytes for Compression Buffer
#define LZMA_PADDING_PERCENT		0.05		// Percent Padding Bytes for Compression Buffer (0.0-1.0)
#define LZMA_MAX_COMPRESSED_SIZE	(((size_t)(-1) >> 1) - 1)

/* Exported Data Compression Function
 * Call with NULL dest to compute required destLen
 */
ESIF_EXPORT int ESIF_CALLCONV EsifCompress(
	unsigned char *dest,
	size_t *destLen,
	const unsigned char *src,
	size_t srcLen
)
{
	int rc = SZ_ERROR_PARAM;
	struct LzmaHeader header = { 0 };

	// NULL dest = Return Required Buffer Size
	if (dest == NULL && destLen && src && srcLen) {
		// Estimate worst case scenario of original size + 5%
		size_t padding = (size_t)(srcLen * LZMA_PADDING_PERCENT);
		*destLen = srcLen + sizeof(header) + (padding < LZMA_PADDING_MINSIZE ? LZMA_PADDING_MINSIZE : padding);
		rc = SZ_OK;
	}
	else if (dest && destLen && src && srcLen && *destLen > sizeof(header)) {
		size_t lzmaDestSize = *destLen - sizeof(header);
		size_t lzmaOutPropsSize = sizeof(header.properties);
		header.original_size = srcLen;
		
		CLzmaEncProps props;
		LzmaEncProps_Init(&props);
		props.level = LZMA_PROPS_LEVEL;
		props.dictSize = LZMA_PROPS_DICTSIZE;
		props.lc = LZMA_PROPS_LITCTXBITS;
		props.lp = LZMA_PROPS_LITPOSBITS;
		props.pb = LZMA_PROPS_NUMPOSBITS;
		props.fb = LZMA_PROPS_FASTBYTES;
		props.numThreads = LZMA_PROPS_THREADS;

		rc = LzmaEncode(
			dest + sizeof(header),
			&lzmaDestSize,
			src,
			srcLen,
			&props,
			header.properties,
			&lzmaOutPropsSize,
			0,
			NULL,
			&g_Alloc, 
			&g_Alloc);


		MyMemcpy(dest, &header, sizeof(header));
		*destLen = lzmaDestSize + sizeof(header);

		// Bounds Check
		if (*destLen > LZMA_MAX_COMPRESSED_SIZE) {
			*destLen = LZMA_MAX_COMPRESSED_SIZE;
			rc = SZ_ERROR_OUTPUT_EOF;
		}
	}
	return rc;
}

/* Exported Data Decompression Function
 * Call with NULL dest to compute required destLen
 */
ESIF_EXPORT int ESIF_CALLCONV EsifDecompress(
	unsigned char *dest,
	size_t *destLen,
	const unsigned char *src,
	size_t srcLen
)
{
	int rc = SZ_ERROR_PARAM;
	struct LzmaHeader *header = NULL;

	// NULL dest = Return Required Buffer Size
	if (dest == NULL && destLen && src && srcLen > sizeof(*header)) {
		header = (struct LzmaHeader *)src;
		unsigned char encoded_signature[] = ESIFCMP_SIGNATURE;

		// Compute Original Decompressed Size if valid Header Properties
		if ((memcmp(header->properties, encoded_signature, sizeof(encoded_signature)) == 0) &&
			(header->original_size > 0 && header->original_size != (unsigned long long)(-1))) {
			*destLen = (size_t)header->original_size;
			rc = SZ_OK;
		}
		else {
			rc = SZ_ERROR_DATA;
		}
	}
	else if (dest && destLen && src && srcLen > sizeof(*header)) {
		header = (struct LzmaHeader *)src;
		size_t lzmaSrcLen = srcLen - sizeof(*header);

		ELzmaStatus status;
		rc = LzmaDecode(
			dest,
			destLen, 
			src + sizeof(*header),
			&lzmaSrcLen,
			src, 
			sizeof(*header),
			LZMA_FINISH_ANY, 
			&status,
			&g_Alloc);

		// Validate Data not Truncated since LzmaDecode returns OK if destLen too small
		if (*destLen < header->original_size) {
			rc = SZ_ERROR_OUTPUT_EOF;
		}
		// Bounds Check
		if (*destLen > LZMA_MAX_COMPRESSED_SIZE) {
			*destLen = LZMA_MAX_COMPRESSED_SIZE;
			rc = SZ_ERROR_OUTPUT_EOF;
		}
	}
	return rc;
}
