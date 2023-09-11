/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include <string>
#include <stdexcept>
#include "sdl/sdl.h"
#include "LzmaDataCompressor.h"
#include <lzma/LzmaEnc.h>
#include <lzma/Alloc.h>
#include "lzma/LzmaDec.h"
using namespace std;

/*
 * Hardcoded LZMA Compression Property Values (and their LZMA_SDK v18.01 lzma.exe command line equivalents)
 * Items marked with "##" should never be changed since they affect the 5-byte LZMA Properties Header Signature
 * The following parameters correspond to to the ESIF_COMPRESS_SIGNATURE defined in esif_sdk_iface_compress.h,
 * which always maps to [5D 00 XX XX XX] for -lc3 -lp0 -pb2 and -d12 to -d27 lzma.exe options.
 */
#define LZMA_PROPS_LEVEL 9 // Compression Level [-a1 = 9]
#define LZMA_PROPS_DICTSIZE (1 << 24) // Dictionary Size [-d24] ##
#define LZMA_PROPS_LITCTXBITS 3 // Literal Context Bits [-lc3] ##
#define LZMA_PROPS_LITPOSBITS 0 // Literal Pos Bits [-lp0] ##
#define LZMA_PROPS_NUMPOSBITS 2 // Number of Pos Bits [-pb2] ##
#define LZMA_PROPS_FASTBYTES 128 // Number of Fast Bytes [-fb128]
#define LZMA_PROPS_THREADS 1 // Number of Threads [-mt1]


// Standard LZMA File Header
#define LZMA_PROPS_SIZE 5 // [XX YY YY YY YY] Where XX = Encoded -lc -lp -pb options, YY = Encoded -d option
#pragma pack(push, 1)
struct LzmaHeader
{
	unsigned char properties[LZMA_PROPS_SIZE]; // encoded LZMA_PROPS options
	unsigned long long original_size; // original uncompressed data size
};
#pragma pack(pop)

string getMessageFromReturnCode(SRes returnCode)
{
	switch (returnCode)
	{
	case SZ_OK:
		return "OK"s;
	case SZ_ERROR_DATA:
		return "Data"s;
	case SZ_ERROR_MEM:
		return "Memory"s;
	case SZ_ERROR_CRC:
		return "Cyclic Redundency Check"s;
	case SZ_ERROR_UNSUPPORTED:
		return "Unsupported"s;
	case SZ_ERROR_PARAM:
		return "Parameter"s;
	case SZ_ERROR_INPUT_EOF:
		return "Input End Of File"s;
	case SZ_ERROR_OUTPUT_EOF:
		return "Output End Of File"s;
	case SZ_ERROR_READ:
		return "Read"s;
	case SZ_ERROR_WRITE:
		return "Write"s;
	case SZ_ERROR_PROGRESS:
		return "Progress"s;
	case SZ_ERROR_FAIL:
		return "Fail"s;
	case SZ_ERROR_THREAD:
		return "Thread"s;
	case SZ_ERROR_ARCHIVE:
		return "Archive"s;
	case SZ_ERROR_NO_ARCHIVE:
		return "No Archive"s;
	default:
		return "Other"s;
	}
}

void throwIfReturnCodeError(SRes returnCode)
{
	if (returnCode != SZ_OK)
	{
		const string errorMessage = "LZMA error: "s + getMessageFromReturnCode(returnCode);
		throw runtime_error(errorMessage.c_str());
	}
}

void throwIfDataTruncated(size_t destinationLength, const struct LzmaHeader* header)
{
	// Validate Data not Truncated since LzmaDecode returns OK if destLen too small
	if (destinationLength < header->original_size)
	{
		throw runtime_error("LZMA error: data truncated");
	}
}

void throwIfStatusCodeError(ELzmaStatus statusCode)
{
	if (statusCode == LZMA_STATUS_NOT_FINISHED)
	{
		throw runtime_error("LZMA error: decode did not finish");
	}

	if (statusCode == LZMA_STATUS_NEEDS_MORE_INPUT)
	{
		throw runtime_error("LZMA error: decode needs more input bytes");
	}
}

void throwIfBufferIsEmpty(const vector<unsigned char>& buffer)
{
	if (buffer.empty() || (buffer.data() == nullptr))
	{
		throw runtime_error("LZMA error: data buffer is empty");
	}
}

#define LZMA_PADDING_MINSIZE 256 // Minimum Padding Bytes for Compression Buffer
#define LZMA_PADDING_PERCENT 0.05 // Percent Padding Bytes for Compression Buffer (0.0-1.0)
#define LZMA_MAX_COMPRESSED_SIZE (((size_t)(-1) >> 1) - 1)
size_t estimateCompressedBufferSize(size_t uncompressedSize)
{
	const auto padding = (size_t)(uncompressedSize * LZMA_PADDING_PERCENT);
	return uncompressedSize + sizeof(struct LzmaHeader)
		   + (padding < LZMA_PADDING_MINSIZE ? LZMA_PADDING_MINSIZE : padding);
}

CLzmaEncProps createEncodingProperties()
{
	CLzmaEncProps props{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	LzmaEncProps_Init(&props);
	props.level = LZMA_PROPS_LEVEL;
	props.dictSize = LZMA_PROPS_DICTSIZE;
	props.lc = LZMA_PROPS_LITCTXBITS;
	props.lp = LZMA_PROPS_LITPOSBITS;
	props.pb = LZMA_PROPS_NUMPOSBITS;
	props.fb = LZMA_PROPS_FASTBYTES;
	props.numThreads = LZMA_PROPS_THREADS;
	return props;
}

vector<unsigned char> createDestinationBuffer(const vector<unsigned char>& source)
{
	const auto estimatedSize = estimateCompressedBufferSize(source.size());
	vector<unsigned char> destination(estimatedSize);
	return destination;
}

struct LzmaHeader createDestinationBufferHeader(const vector<unsigned char>& source)
{
	struct LzmaHeader header{0};
	header.original_size = source.size();
	return header;
}

vector<unsigned char> LzmaDataCompressor::encode(const vector<unsigned char>& source) const
{
	throwIfBufferIsEmpty(source);
	const auto properties = createEncodingProperties();
	auto header = createDestinationBufferHeader(source);
	auto compressedData = createDestinationBuffer(source);

	size_t destinationSize = estimateCompressedBufferSize(source.size()) - sizeof(header);
	size_t lzmaOutPropsSize = sizeof(header.properties);

	const auto returnCode = LzmaEncode(
		compressedData.data() + sizeof(header),
		&destinationSize,
		source.data(),
		source.size(),
		&properties,
		header.properties,
		&lzmaOutPropsSize,
		0,
		NULL,
		&g_Alloc,
		&g_Alloc);
	throwIfReturnCodeError(returnCode);

	throwIfBufferIsEmpty(compressedData);
	MyMemcpy(compressedData.data(), (void *)&header ,sizeof(header));
	compressedData.resize(destinationSize + sizeof(header));

	return compressedData;
}

vector<unsigned char> LzmaDataCompressor::decode(const vector<unsigned char>& source) const
{
	throwIfBufferIsEmpty(source);
	const auto header = (struct LzmaHeader*)source.data();
	size_t lzmaSrcLen = source.size() - sizeof(*header);
	size_t destinationLength = (size_t)header->original_size;

	ELzmaStatus statusCode;
	vector<unsigned char> uncompressedData(destinationLength);
	const auto returnCode = LzmaDecode(
		uncompressedData.data(),
		&destinationLength,
		source.data() + sizeof(*header),
		&lzmaSrcLen,
		source.data(),
		sizeof(*header),
		LZMA_FINISH_ANY,
		&statusCode,
		&g_Alloc);

	throwIfReturnCodeError(returnCode);
	throwIfStatusCodeError(statusCode);
	throwIfDataTruncated(destinationLength, header);
	return uncompressedData;
}
