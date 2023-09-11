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

#include "esif_ccb_string.h"
#include "Base64.h"
#include <algorithm>
#include <stdexcept>

//Base64 Decoding Helper Macros
#define BASE64_DECODED_BUFFER_LEN(encodedLen)	((((encodedLen) + 3) / 4) * 3)
#define BASE64_DECODED_ISVALID(ch)				(isalpha(ch) || isdigit(ch) || (ch) == '+' || (ch) == '/')
#define BASE64_DECODED_BYTE(ch)					((unsigned char)((ch) == '=' ? 0x00 : (ch) == '/' ? 0x3F : (ch) == '+' ? 0x3E : isdigit(ch) ? (ch) - '0' + 0x34 : islower(ch) ? (ch) - 'a' + 0x1A : (ch) - 'A'))
#define BASE64_encodedLenGTH(bytes)			(((((bytes) + 2) / 3) * 4) + 1) // Compute Encoded String Length for a buffer, including NUL terminator
#define MAX_BASE64_encodedLen					(((((32 * 1024 * 1024) / 3 + 2) * 4) / 64 + 1) * 66) // 32MB Binary = ~44MB Encoded with newlines every 64 bytes

#define BASE64_ENCODED_LENGTH(bytes)                                                                                   \
	(((((bytes) + 2) / 3) * 4) + 1) // Compute Encoded String Length for a buffer, including NUL terminator
#define MAX_BASE64_ENCODED_LEN                                                                                         \
	(((((32 * 1024 * 1024) / 3 + 2) * 4) / 64 + 1) * 66) // 32MB Binary = ~44MB Encoded with newlines every 64 bytes


using namespace std;

vector<unsigned char> Base64::decode(const vector<unsigned char>& base64EncodedData)
{
	size_t decodedLen = 0;
	const size_t encodedLen = base64EncodedData.size();
	esif_error_t rc = base64Decode(
		nullptr, 
		&decodedLen, 
		reinterpret_cast<const char*>(base64EncodedData.data()), 
		encodedLen);
	if ((rc == ESIF_E_NEED_LARGER_BUFFER) && (decodedLen > 0))
	{
		vector<unsigned char> decodedData;
		decodedData.assign(decodedLen, '\0');
		rc = base64Decode(
			decodedData.data(), 
			&decodedLen, 
			reinterpret_cast<const char*>(base64EncodedData.data()), 
			encodedLen);
		if (rc == ESIF_OK)
		{
			return trimTrailingZeros(decodedData);
		}
	}
	throw runtime_error("Failed to decode base64 data");
}

std::vector<unsigned char> Base64::decode(const std::string& base64EncodedData)
{
	vector<unsigned char> encodedData;
	encodedData.reserve(base64EncodedData.size());
	encodedData.assign(base64EncodedData.begin(), base64EncodedData.end());
	return decode(encodedData);
}

vector<unsigned char> Base64::encode(const std::vector<unsigned char>& data)
{
	const size_t requiredLength = BASE64_ENCODED_LENGTH(data.size());
	vector<unsigned char> encodedData;
	encodedData.assign(requiredLength, '\0');
	const esif_error_t rc = base64Encode(
		reinterpret_cast<char*>(encodedData.data()),
		requiredLength,
		data.data(),
		data.size());
	if (rc == ESIF_OK)
	{
		return encodedData;
	}
	throw runtime_error("Failed to encode base64 data");
}

std::vector<unsigned char> Base64::encode(const std::string& stringData)
{
	vector<unsigned char> data;
	data.reserve(stringData.size());
	data.assign(stringData.begin(), stringData.end());
	return encode(data);
}

// Decode a Base64-Encoded Buffer into Binary Buffer
esif_error_t Base64::base64Decode(
	void* targetBuf, // Target Buffer. May be NULL if *targetLen = 0. Data must be padded with '=' if encodedLen not divisible by 4.
	size_t* targetLen, // Target Length: Buffer size if targetBuf not NULL, Output: Decoded Length [if ESIF_OK] or Required Length [if ESIF_E_NEED_LARGER_BUFFER]
	const char* encodedBuf, // Base-64 Encoded Buffer [Null-terminator optional]. May be NULL if *targetLen = 0
	size_t encodedLen // Base-64 Encoded Length, including Padding [not including optional Null-terminator]. May include whitespace.
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (encodedLen > MAX_BASE64_encodedLen) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	else if (targetLen && *targetLen < BASE64_DECODED_BUFFER_LEN(encodedLen)) {
		*targetLen = BASE64_DECODED_BUFFER_LEN(encodedLen);
		rc = ESIF_E_NEED_LARGER_BUFFER;
	}
	else if (targetBuf && targetLen && encodedBuf) {
		size_t j = 0;
		size_t k = 0;
		size_t padding = 0;
		size_t whitespace = 0;
		const char *whitespace_chars = "\r\n\t ";
		unsigned char decoded[4] = { 0 };
		auto*outbuf = static_cast<unsigned char*>(targetBuf);
		rc = ESIF_E_COMMAND_DATA_INVALID;

		// Ignore trailing whitespace
		while (encodedLen > 0 && esif_ccb_strchr(whitespace_chars, encodedBuf[encodedLen - 1]) != NULL) {
			encodedLen--;
			whitespace++;
		}

		// Decode each 4-byte text chunk into a 3 byte binary chunk (final binary chunk will be 1-3 bytes)
		for (j = 0, k = 0; j + 3 < encodedLen && k + 2 < *targetLen; j += 4, k += 3) {
			if ((!BASE64_DECODED_ISVALID(encodedBuf[j]))
				|| (!BASE64_DECODED_ISVALID(encodedBuf[j + 1]))
				|| (!BASE64_DECODED_ISVALID(encodedBuf[j + 2]) && (j + 2 < encodedLen - 2 || encodedBuf[j + 2] != '='))
				|| (!BASE64_DECODED_ISVALID(encodedBuf[j + 3]) && (j + 3 < encodedLen - 2 || encodedBuf[j + 3] != '='))) {
				break;
			}
			decoded[0] = BASE64_DECODED_BYTE(encodedBuf[j]);
			decoded[1] = BASE64_DECODED_BYTE(encodedBuf[j + 1]);
			decoded[2] = BASE64_DECODED_BYTE(encodedBuf[j + 2]);
			decoded[3] = BASE64_DECODED_BYTE(encodedBuf[j + 3]);
			outbuf[k]     = ((decoded[0] & 0x3F) << 2) | ((decoded[1] & 0x30) >> 4);
			outbuf[k + 1] = ((decoded[1] & 0x0F) << 4) | ((decoded[2] & 0x3C) >> 2);
			outbuf[k + 2] = ((decoded[2] & 0x03) << 6) | ((decoded[3] & 0x3F));

			// Do not decode trailing padding bytes
			if (j + 4 == encodedLen) {
				if (encodedBuf[j + 2] == '=') {
					padding++;
				}
				if (encodedBuf[j + 3] == '=') {
					padding++;
				}
			}
			// Allow for whitespace every 4 bytes since many conversion tools add newlines every 64 bytes
			while (j + 4 < encodedLen && esif_ccb_strchr(whitespace_chars, encodedBuf[j + 4]) != NULL) {
				whitespace++;
				j++;
			}
		}
		// If buffer fully decoded, return success and actual decoded length
		if (j == encodedLen && k == *targetLen - BASE64_DECODED_BUFFER_LEN(whitespace)) {
			*targetLen = k - padding;
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Base64 Encode a Binary buffer into a Target string
esif_error_t Base64::base64Encode(char* target_str, size_t target_len, const void* source_buf, size_t source_len)
{
	static const char base64_asciimap[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
		'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
		's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (BASE64_ENCODED_LENGTH(source_len) > MAX_BASE64_ENCODED_LEN)
	{
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	else if (target_len < BASE64_ENCODED_LENGTH(source_len))
	{
		rc = ESIF_E_NEED_LARGER_BUFFER;
	}
	else if (source_buf && target_str)
	{
		UInt8 block_of_bytes[4] = {0};
		const auto data = static_cast<const UInt8*>(source_buf);
		size_t i = 0, j = 0;

		for (i = 0; i < source_len / 3; ++i)
		{
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = ((data[i * 3 + 0] << 4) | (data[i * 3 + 1] >> 4)) & 0x3F;
			block_of_bytes[2] = ((data[i * 3 + 1] << 2) | (data[i * 3 + 2] >> 6)) & 0x3F;
			block_of_bytes[3] = data[i * 3 + 2] & 0x3F;

			for (j = 0; j < 4; ++j)
				*target_str++ = base64_asciimap[block_of_bytes[j]];
		}

		switch (source_len % 3)
		{
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

vector<unsigned char> Base64::trimTrailingZeros(const vector<unsigned char>& decodedData)
{
	// find last zero from end
	auto posLastZero = decodedData.end()-1;
	while (posLastZero != decodedData.begin())
	{
		if (*posLastZero != '\0')
		{
			break;
		}
		--posLastZero;
	}

	vector<unsigned char> trimmedData;
	trimmedData.assign(decodedData.begin(), posLastZero+1);
	return trimmedData;
}
