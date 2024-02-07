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
#define _ESIFDATA_CLASS

#include "esif.h"
#include "esif_lib_esifdata.h"
#include "esif_temp.h"

#include "esif_sdk_iface_compress.h"


#define tohexdigit(ch)  ((ch) >= '0' && (ch) <= '9' ? ((ch) - '0') : (toupper(ch) - 'A' + 10))

// Max String Lengths for each Data type class
#define MAXSTR_INT8         5
#define MAXSTR_INT16        7
#define MAXSTR_INT32        12
#define MAXSTR_INT64        22
#define MAXSTR_DEFAULT      MAX_LINE
#define MAXSTR_MINVALUE     4
#define MAXSTR_MAXVALUE     0x7ffffffe

#define MIN_TEMPERATURE		(-273.15)	// Min Supported Temperature in Celsius (Absolute Zero)
#define MAX_TEMPERATURE		(9999)		// Max Supported Temperature in Celsius

#define ISHEX(str)  (esif_ccb_strnicmp((str), "0x", 2) == 0 ? ESIF_TRUE : ESIF_FALSE)
#define IFHEX(str, hex, dec)    (esif_ccb_strnicmp((str), "0x", 2) == 0 ? (hex) : (dec))

#define MAX_COMPRESSED_DATA	0x7FFFFFFE	// Max supported Compressed and Decompressed EsifData size (not necessarily Compression Library max size)

// EsifData Class

// Constructor
void EsifData_ctor (EsifDataPtr self)
{
	if (self) {
		WIPEPTR(self);
	}
}


// Destructor
void EsifData_dtor (EsifDataPtr self)
{
	if (self) {
		if (self->buf_len) {
			esif_ccb_free(self->buf_ptr);
		}
		WIPEPTR(self);
	}
}


// new Operator
EsifDataPtr EsifData_Create ()
{
	EsifDataPtr self = (EsifDataPtr)esif_ccb_malloc(sizeof(*self));
	EsifData_ctor(self);
	return self;
}


// delete Operator
void EsifData_Destroy (EsifDataPtr self)
{
	EsifData_dtor(self);
	esif_ccb_free(self);
}


// sizeof() operator
size_t EsifData_Sizeof ()
{
	return sizeof(EsifData);
}


// sizeof(typeof()) operator
size_t EsifData_SizeofType (EsifDataPtr self)
{
	size_t size = 0;
	switch (self->type) {
	case ESIF_DATA_INT8:
	case ESIF_DATA_UINT8:
		size = 1;
		break;

	case ESIF_DATA_INT16:
	case ESIF_DATA_UINT16:
	case ESIF_DATA_QUALIFIER:	// ESIF_DATA_QUALIFIER
	case ESIF_DATA_INSTANCE:
		size = 2;
		break;

	case ESIF_DATA_INT32:
	case ESIF_DATA_UINT32:
	case ESIF_DATA_TEMPERATURE:
	case ESIF_DATA_POWER:
	case ESIF_DATA_TIME:
	case ESIF_DATA_PERCENT:
	case ESIF_DATA_IPV4:
		size = 4;
		break;

	case ESIF_DATA_INT64:
	case ESIF_DATA_UINT64:
	case ESIF_DATA_FREQUENCY:
		size = 8;
		break;

	case ESIF_DATA_GUID:
	case ESIF_DATA_IPV6:
		size = 16;
		break;

	case ESIF_DATA_REGISTER:
		size = sizeof(size_t);
		break;

	case ESIF_DATA_POINTER:
		size = sizeof(void*);
		break;

	case ESIF_DATA_ENUM:
		size = sizeof(EsifDataType);
		break;

	case ESIF_DATA_HANDLE:
		size = 4;				// For Windows, what about Linux?
		break;

	case ESIF_DATA_TABLE:
		size = self->data_len;	// Or 0xFFFFFFFF?
		break;

	// Variable-Length Types
	case ESIF_DATA_STRING:
	case ESIF_DATA_UNICODE:
	case ESIF_DATA_BINARY:
	case ESIF_DATA_BLOB:
	case ESIF_DATA_DSP:
	case ESIF_DATA_JSON:
	case ESIF_DATA_STRUCTURE:
	case ESIF_DATA_XML:
		size = self->data_len;	// Or 0xFFFFFFFF?
		break;

	case ESIF_DATA_VOID:
	default:
		size = 0;
		break;
	}
	return size;
}


// Additional new Operators
EsifDataPtr EsifData_CreateAs (
	EsifDataType type,
	void *buf_ptr,
	u32 buf_len,
	u32 data_len
	)
{
	EsifDataPtr self = EsifData_Create();
	EsifData_Set(self, type, buf_ptr, buf_len, data_len);
	return self;
}


// new[] Operator
EsifDataPtr EsifData_CreateArray (int size)
{
	EsifDataPtr self = (size ? (EsifDataPtr)esif_ccb_malloc(size * sizeof(*self)) : 0);
	int idx;

	ESIF_ASSERT(self || !size);
	for (idx = 0; idx < size; idx++)
		EsifData_ctor(&self[idx]);
	return self;
}


// delete[] Operator
void EsifData_DestroyArray (
	EsifDataPtr self,
	int size
	)
{
	int idx;

	ESIF_ASSERT(self || !size);
	for (idx = 0; idx < size; idx++)
		EsifData_dtor(&self[idx]);
	esif_ccb_free(self);
}


// [] operator
EsifDataPtr EsifData_GetArray (
	EsifDataPtr self,
	int item
	)
{
	ESIF_ASSERT(self || !item);
	return &self[item];
}


// methods
void EsifData_Set (
	EsifDataPtr self,
	EsifDataType type,
	void *buf_ptr,
	u32 buf_len,
	u32 data_len
	)
{
	if (self) {
		int bytes = ((buf_ptr && (buf_len == ESIFAUTOLEN || data_len == ESIFAUTOLEN)) ? (UInt32)esif_ccb_strlen((char*)buf_ptr, MAXAUTOLEN) + 1 : 0);
		if (self->buf_len) {
			esif_ccb_free(self->buf_ptr);
		}
		self->type     = type;
		self->buf_ptr  = buf_ptr;
		self->buf_len  = (buf_len == ESIFAUTOLEN ?  bytes : buf_len);
		self->data_len = (data_len == ESIFAUTOLEN ? bytes : data_len);
	}
}

//
// Clones an EsifData object
// Notes:  If the data object has a buffer owned elsewhere, buf_len == 0, the
// cloned data object shall contain a buffer large enough to contain the
// associated data (data_len) and the buf_len member shall contain the same
// value, so as to indicate this new structure is not owned elsewhere
//
EsifDataPtr EsifData_Clone(
	EsifDataPtr self
	)
{
	EsifDataPtr clonePtr = NULL;
	u32 buf_len = 0; 

	if (NULL == self) {
		goto exit;
	}
	
	clonePtr = EsifData_Create();
	if (NULL == clonePtr) {
		goto exit;
	}

	*clonePtr = *self;
	clonePtr->buf_ptr = NULL;

	if (NULL == self->buf_ptr) {
		goto exit;
	}

	buf_len = esif_ccb_max(self->buf_len, self->data_len);
	if (0 == buf_len) {
		goto exit;
	}
	clonePtr->buf_len = buf_len;

	clonePtr->buf_ptr = esif_ccb_malloc(buf_len);
	if (NULL == clonePtr->buf_ptr) {
		EsifData_Destroy(clonePtr);
		clonePtr = NULL;
		goto exit;
	}

	esif_ccb_memcpy(clonePtr->buf_ptr, self->buf_ptr, self->data_len);
exit:
	return clonePtr;
}


// cast operators
char*EsifData_AsString (EsifDataPtr self)
{
	return STATIC_CAST(char*, self->buf_ptr);
}


Byte*EsifData_AsPointer (EsifDataPtr self)
{
	return STATIC_CAST(Byte*, self->buf_ptr);
}


Int32 EsifData_AsInt32 (EsifDataPtr self)
{
	ESIF_ASSERT(self && self->data_len == 4);
	return *STATIC_CAST(Int32*, self->buf_ptr);
}


UInt32 EsifData_AsUInt32 (EsifDataPtr self)
{
	ESIF_ASSERT(self && self->data_len == 4);
	return *STATIC_CAST(UInt32*, self->buf_ptr);
}

// ToString operator [dynamic, caller-owned]
char *EsifData_ToString(EsifDataPtr self)
{
	return EsifData_ToStringMax(self, MAXSTR_DEFAULT);
}

// ToString operator [dynamic, caller-owned]
char *EsifData_ToStringMax(
	EsifDataPtr self,
	UInt32 max_length
)
{
	char *result   = 0;
	UInt32 alloc   = 0;
	UInt32 u32data = 0;
	UInt64 u64data = 0;
	Byte *ptrdata  = 0;
	UInt32 ptrlen  = 0;
	UInt32 idx     = 0;
	UInt32 max_string = (max_length == 0 ? MAXSTR_DEFAULT : esif_ccb_max(MAXSTR_MINVALUE, esif_ccb_min(MAXSTR_MAXVALUE, max_length)));
	UInt32 max_binary = (max_string / 2 - 2);

	if (self == NULL || self->buf_ptr == NULL || self->data_len == 0) {
		return NULL;
	}

	// Determine Buffer Size and Conversion Values
	switch (self->type) {
	case ESIF_DATA_INT8:
	case ESIF_DATA_UINT8:
		alloc   = MAXSTR_INT8;
		u32data = (UInt32) * STATIC_CAST(UInt8*, self->buf_ptr);
		break;

	case ESIF_DATA_INT16:
	case ESIF_DATA_UINT16:
		alloc   = MAXSTR_INT16;
		u32data = (UInt32) * STATIC_CAST(UInt16*, self->buf_ptr);
		break;

	case ESIF_DATA_INT32:
	case ESIF_DATA_UINT32:
	case ESIF_DATA_TEMPERATURE:
	case ESIF_DATA_POWER:
	case ESIF_DATA_TIME:
	case ESIF_DATA_PERCENT:
		alloc   = MAXSTR_INT32;
		u32data = (UInt32) * STATIC_CAST(UInt32*, self->buf_ptr);
		break;

	case ESIF_DATA_INT64:
	case ESIF_DATA_UINT64:
	case ESIF_DATA_FREQUENCY:
		alloc   = MAXSTR_INT64;
		u64data = (UInt64) * STATIC_CAST(UInt64*, self->buf_ptr);
		break;

	case ESIF_DATA_STRING:
	case ESIF_DATA_JSON:
	case ESIF_DATA_XML:
		alloc   = esif_ccb_min(self->data_len, max_string) + 1;
		ptrdata = (Byte*)self->buf_ptr;
		ptrlen  = self->data_len;
		break;

   case ESIF_DATA_VOID:
	   alloc   = 0;
	   ptrdata = NULL;
	   ptrlen  = 0;
	   break;

	case ESIF_DATA_BINARY:
	default:
		alloc   = 2 + (2 * esif_ccb_min(self->data_len, max_binary)) + 2 + 1;
		ptrdata = (Byte*)self->buf_ptr;
		ptrlen  = self->data_len;
		break;
	}

	// Allocate Memory and Convert Data
	if (alloc == 0) {
		return NULL;
	}
	else {
		char intstr[MAXSTR_INT64] = { 0 };
		result = (char*)esif_ccb_malloc(alloc);
		if (result == NULL) {
			return NULL;
		}
		switch (self->type) {
		case ESIF_DATA_INT8:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%ld", (long)((Int8)u32data));
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_INT16:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%hd", (Int16)u32data);
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_INT32:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%ld", (long)u32data);
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_UINT8:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%lu", (long unsigned)(UInt8)u32data);
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_UINT16:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%hu", (UInt16)u32data);
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_UINT32:
		case ESIF_DATA_TEMPERATURE:
		case ESIF_DATA_POWER:
		case ESIF_DATA_TIME:
		case ESIF_DATA_PERCENT:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%lu", (long unsigned)u32data);
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_INT64:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%lld", (Int64)u64data);
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_UINT64:
		case ESIF_DATA_FREQUENCY:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%llu", (UInt64)u64data);
			esif_ccb_strcpy(result, intstr, alloc);
			break;

		case ESIF_DATA_STRING:
		case ESIF_DATA_JSON:
		case ESIF_DATA_XML:
			if (ptrlen <= max_string + 1) {
				esif_ccb_memcpy(result, ptrdata, ptrlen);
			} else {
				esif_ccb_memcpy(result, ptrdata, max_string - 2);
				esif_ccb_memcpy(result + max_string - 2, "..", 3);
			}
			break;

		case ESIF_DATA_BINARY:
		default:
			esif_ccb_strcpy(result, "0x", alloc);
			for (idx = 0; idx < ptrlen && idx < max_binary; idx++)
				esif_ccb_sprintf(alloc - 2 - (idx * 2), result + 2 + (2 * idx), "%02X", ptrdata[idx]);
			if (ptrlen > max_binary) {
				esif_ccb_strcat(result, "..", alloc);
			}
			break;
		}
	}
	return result;
}


// FromString operator
esif_error_t EsifData_FromString (
	EsifDataPtr self,
	char *str,
	EsifDataType type
	)
{
	esif_error_t rc  = ESIF_E_UNSPECIFIED;
	UInt32 alloc   = 0;
	UInt32 u32data = 0;
	UInt64 u64data = 0;
	Byte *ptrdata  = 0;
	UInt32 ptrlen  = 0;
	Byte *buffer   = 0;
	UInt32 idx     = 0;

	ESIF_ASSERT(self);

	// Store NULL as 1 byte buffers, 0 length so they get freed
	if (NULL == str || '\0' == *str) {
		EsifData_Set(self, type, esif_ccb_malloc(1), 1, alloc);
		return ESIF_OK;
	}

	if (type == ESIF_DATA_AUTO) {
		int ch=0;

		// Default = STRING unless one of the recognized formats below
		type = ESIF_DATA_STRING;

		// 0x[0-9A-F]+ = UINT32 or BINARY (STRING if invalid hex string)
		if (esif_ccb_strnicmp(str, "0x", 2) == 0) {
			for (ch = 2; str[ch] != 0 && isxdigit(str[ch]); ch++) {
				;
			}
			if (str[ch] == 0) {
				if (ch <= 10) {	// 0xAA, 0xAABB, 0xAABBCC, 0xAABBCCDD
					type = ESIF_DATA_UINT32;
				}
				else {
					type = ESIF_DATA_BINARY;
				}
			}
		}
		// [-+][0-9]+ = INT32, UINT32, INT64, UINT64 (STRING if non-numeric)
		else if (str[0] == '+' || str[0] == '-' || isdigit(str[0])) {
			int sign = str[0];
			for (ch = 1; str[ch] != 0 && isdigit(str[ch]); ch++) {
				;
			}
			if (str[ch] == 0) {
				if (isdigit(sign)) {
					if (ch <= 10)
						type = ESIF_DATA_UINT32;
					else if (ch <= 20)
						type = ESIF_DATA_UINT64;
				}
				else {
					if (ch <= 11)
						type = ESIF_DATA_UINT32;
					else if (ch <= 21)
						type = ESIF_DATA_UINT64;
				}
			}
		}
		self->type = type;
	}

	// Determine Storage Requirements
	switch (type) {
	case ESIF_DATA_INT8:
	case ESIF_DATA_UINT8:
		alloc = sizeof(UInt8);
		break;

	case ESIF_DATA_INT16:
	case ESIF_DATA_UINT16:
		alloc = sizeof(UInt16);
		break;

	case ESIF_DATA_INT32:
	case ESIF_DATA_UINT32:
	case ESIF_DATA_TEMPERATURE:
	case ESIF_DATA_POWER:
	case ESIF_DATA_TIME:
	case ESIF_DATA_PERCENT:
		alloc = sizeof(UInt32);
		break;

	case ESIF_DATA_INT64:
	case ESIF_DATA_UINT64:
	case ESIF_DATA_FREQUENCY:
		alloc = sizeof(UInt64);
		break;

	case ESIF_DATA_STRING:
	case ESIF_DATA_JSON:
	case ESIF_DATA_XML:
		alloc   = (UInt32)esif_ccb_strlen(str, MAXAUTOLEN) + 1;
		ptrdata = (Byte*)str;
		ptrlen  = alloc;
		break;

   case ESIF_DATA_VOID:
	   alloc   = 1;
	   break;

	case ESIF_DATA_BINARY:
	default:
		ptrdata = (Byte*)str;
		ptrlen  = (UInt32)esif_ccb_strlen(str, MAXAUTOLEN);
		if (esif_ccb_strnicmp(str, "0x", 2) == 0) {
			alloc = ptrlen / 2 - 1;	// "0xABCD"
		} else {
			// else treat as string for now
			alloc = (UInt32)esif_ccb_strlen(str, MAXAUTOLEN) + 1;
		}
		break;
	}

	// Allocate Memory and Convert Data
	if (alloc == 0) {
		EsifData_Set(self, type, ptrdata, 0, 0);
		rc = ESIF_OK;
	}
	else {
		EsifData_Set(self, type, esif_ccb_malloc(alloc), alloc, alloc);
		ESIF_ASSERT(self->buf_ptr);
		SAFETY(memset(self->buf_ptr, 0, alloc));
		buffer = (Byte*)self->buf_ptr;
		if (buffer == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		rc = ESIF_E_COMMAND_DATA_INVALID;

		// Convert Data
		switch (type) {
		case ESIF_DATA_INT8:
			if (esif_ccb_sscanf(str, IFHEX(str, "%x", "%d"), &u32data) && u32data <= 0xff) {
				*STATIC_CAST(Int8*, buffer) = (Int8)u32data;
				rc = ESIF_OK;
			}
			break;

		case ESIF_DATA_INT16:
			if (esif_ccb_sscanf(str, IFHEX(str, "%x", "%d"), &u32data) && u32data <= 0xffff) {
				*STATIC_CAST(Int16*, buffer) = (Int16)u32data;
				rc = ESIF_OK;
			}
			break;

		case ESIF_DATA_INT32:
			if (esif_ccb_sscanf(str, IFHEX(str, "%x", "%d"), &u32data)) {
				*STATIC_CAST(Int32*, buffer) = (Int32)u32data;
				rc = ESIF_OK;
			}
			break;

		case ESIF_DATA_UINT8:
			if (esif_ccb_sscanf(str, IFHEX(str, "%x", "%u"), &u32data) && u32data <= 0xff) {
				*STATIC_CAST(UInt8*, buffer) = (UInt8)u32data;
				rc = ESIF_OK;
			}
			break;

		case ESIF_DATA_UINT16:
			if (esif_ccb_sscanf(str, IFHEX(str, "%x", "%u"), &u32data) && u32data <= 0xffff) {
				*STATIC_CAST(UInt16*, buffer) = (UInt16)u32data;
				rc = ESIF_OK;
			}
			break;

		case ESIF_DATA_UINT32:
		case ESIF_DATA_POWER:
		case ESIF_DATA_TIME:
		case ESIF_DATA_PERCENT:
			if (esif_ccb_sscanf(str, IFHEX(str, "%x", "%u"), &u32data)) {
				*STATIC_CAST(UInt32*, buffer) = (UInt32)u32data;
				rc = ESIF_OK;
			}
			break;

		/*
		 * Temp input in floating point Celsius with 1 decimal point precision,
		 * but stored in 10ths K
		 */
		case ESIF_DATA_TEMPERATURE:
		{
			if (ISHEX(str)) {
				u32 temp = 0;
				if (esif_ccb_sscanf(str, "%x", &temp)) {
					temp = (temp > MAX_TEMPERATURE ? MAX_TEMPERATURE : temp);
					u32data = temp * 10;
					rc = ESIF_OK;
				}
			}
			else {
				double temp = 0.0;
				if (esif_ccb_sscanf(str,"%lf", &temp)) {
					temp = (temp < MIN_TEMPERATURE ? MIN_TEMPERATURE : temp > MAX_TEMPERATURE ? MAX_TEMPERATURE : temp);
					temp *= 10.0;
					u32data = (UInt32)temp;
					rc = ESIF_OK;
				}
			}

			if (rc == ESIF_OK) {
				esif_convert_temp(ESIF_TEMP_DECIC, NORMALIZE_TEMP_TYPE, &u32data);
				*STATIC_CAST(UInt32*, buffer) = (UInt32)u32data;
			}
			break;
		}
		case ESIF_DATA_INT64:
			if (esif_ccb_sscanf(str, IFHEX(str, "%llx", "%lld"), &u64data)) {
				*STATIC_CAST(Int64*, buffer) = (UInt64)u64data;
				rc = ESIF_OK;
			}
			break;

		case ESIF_DATA_UINT64:
		case ESIF_DATA_FREQUENCY:
			if (esif_ccb_sscanf(str, IFHEX(str, "%llx", "%llu"), &u64data)) {
				*STATIC_CAST(UInt64*, buffer) = (UInt64)u64data;
				rc = ESIF_OK;
			}
			break;

		case ESIF_DATA_STRING:
		case ESIF_DATA_JSON:
		case ESIF_DATA_XML:
			esif_ccb_memcpy(buffer, ptrdata, ptrlen);
			rc = ESIF_OK;
			break;
			
		case ESIF_DATA_VOID:
			rc = ESIF_OK;
			break;

		case ESIF_DATA_BINARY:
		default:
			// Convert "0xABCD"
			if (esif_ccb_strnicmp(str, "0x", 2) == 0) {
				idx = 0;
				if (ptrlen % 2 == 1) {	// Handle odd-length hex vaules: 0xABC 0xABCDE
					buffer[idx++] = (UInt8)tohexdigit(str[0]);
					str++;
				} 
				else {
					str += 2;
				}
				for ( ; idx < alloc && isxdigit(str[idx * 2]) && isxdigit(str[idx * 2 + 1]); idx++)
					buffer[idx] = (UInt8)(tohexdigit(str[idx * 2]) << 4 | tohexdigit(str[idx * 2 + 1]));
				self->buf_len = self->data_len = idx;
			} else {
				esif_ccb_memcpy(buffer, ptrdata, ptrlen);
			}
			rc = ESIF_OK;
			break;
		}
	}

exit:
	return rc;
}

// Include Compression Support?
#if defined(ESIF_FEAT_OPT_COMPRESS)

/* Return True if the given data buffer appears to be compressed (without loading compression library) */
Bool EsifData_IsCompressed(EsifDataPtr self)
{
	Bool rc = ESIF_FALSE;
	if (self) {
		rc = (Bool)EsifCmp_IsCompressed(self->buf_ptr, self->data_len);
	}
	return rc;
}

/* Compress an EsifData buffer and return updated buffer and size to caller if successful.
 * ESIF Compression Library is dynamically loaded and used to compress data unless the data appears to
 * already be compressed. buf_ptr is replaced if successfully compressed, otherwise it is unchanged.
 * If the original buf_ptr passed in is not dynamically allocated, buf_len MUST be 0 (but not data_len).
 * The Compression Library is unloaded after compression is complete.
 *
 * Note that the compression library is optional for Compression (i.e. returns OK if the loadable
 * libary is not found), but mandatory for Decompression. This allows the caller to use the original
 * uncompressed data the compression library is not installed on the OS image.
 *
 * Returns ESIF_OK if data is already compressed, or if library loaded and data successfully compressed,
 * or if loadable library was not found.
 */
esif_error_t EsifData_Compress(EsifDataPtr self)
{
	esif_error_t rc = (self ? ESIF_OK : ESIF_E_PARAMETER_IS_NULL);
	int esifCmp_result = 0;
	BytePtr expandedData = NULL;
	size_t expandedSize = 0;
	BytePtr compressedData = NULL;
	size_t compressedSize = 0;

	// Load Compression Library and Compress data if it is not already compressed
	if (rc == ESIF_OK && EsifCmp_IsCompressed(self->buf_ptr, self->data_len) == ESIF_FALSE) {
		char esifCmpLibPath[MAX_PATH] = { 0 };
		IpfDecompressFuncPtr fnCompress = NULL;

		expandedData = self->buf_ptr;
		expandedSize = self->data_len;

		esif_build_path(esifCmpLibPath, sizeof(esifCmpLibPath), ESIF_PATHTYPE_DLL, ESIFCMP_LIBRARY, ESIF_LIB_EXT);
		esif_lib_t esifCmpLib = esif_ccb_library_load(esifCmpLibPath);

		if (esifCmpLib == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if (esifCmpLib->handle == NULL) {
			rc = esif_ccb_library_error(esifCmpLib);
			
			// Compression Library is optional for compression; required for decompression
			// If loadable library does not exist, return OK with original buffer
			if (rc == ESIF_E_NOT_FOUND) {
				rc = ESIF_OK;
			}
		}
		else if ((fnCompress = (IpfCompressFuncPtr)esif_ccb_library_get_func(esifCmpLib, ESIFCMP_COMPRESSOR)) == NULL) {
			rc = ESIF_E_IFACE_NOT_SUPPORTED;
		}
		else {
			// 1. Get Estimated Compressed Buffer Size
			esifCmp_result = (*fnCompress)(
				NULL,
				&compressedSize,
				expandedData,
				expandedSize
				);

			// 2. Allocate Buffer and Compress
			if (esifCmp_result != 0) {
				rc = ESIF_E_COMPRESSION_ERROR;
			}
			else if (compressedSize > MAX_COMPRESSED_DATA) {
				rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			}
			else if (compressedSize > 0) {
				compressedData = (BytePtr)esif_ccb_malloc(compressedSize);

				if (compressedData == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					int retries = 3;			// Retry this many times
					double growpct = 1.25;		// Grow by this much each retry (3 x 125% = 200%)
					size_t compressedBufLen = compressedSize;

					/* Unlike decompression, calling Compressor with a NULL dest only computes an ESTIMATED buffer size.
					 * In the event that the computed compression buffer is too small to hold the compressed data,
					 * keep retrying compression with a larger buffer until it succeeds or until retries are exceeded.
					 * This is rare and usually only happens when compressing compressed data, but handle anyway.
					 */
					do {
						rc = ESIF_OK;
						if (esifCmp_result == ESIFCMP_ERROR_OUTPUT_EOF) {
							rc = ESIF_E_NO_MEMORY;
							compressedBufLen = (size_t)(compressedBufLen * growpct);
							BytePtr newBuffer = (BytePtr)esif_ccb_realloc(compressedData, compressedBufLen);
							if (newBuffer != NULL) {
								rc = ESIF_OK;
								compressedData = newBuffer;
								compressedSize = compressedBufLen;
							}
						}

						if (rc == ESIF_OK) {
							esifCmp_result = (*fnCompress)(
								compressedData,
								&compressedSize,
								expandedData,
								expandedSize
								);
						}
					} while (rc == ESIF_OK && esifCmp_result == ESIFCMP_ERROR_OUTPUT_EOF && retries-- > 0);

					if (esifCmp_result != 0) {
						rc = ESIF_E_COMPRESSION_ERROR;
					}
					// Shrink compressed buffer down to actual compressed size
					else if (rc == ESIF_OK && compressedSize < compressedBufLen) {
						rc = ESIF_E_NO_MEMORY;
						BytePtr newBuffer = (BytePtr)esif_ccb_realloc(compressedData, compressedSize);
						if (newBuffer != NULL) {
							rc = ESIF_OK;
						}
						compressedData = newBuffer;
					}
						
					if (rc == ESIF_OK) {
						ESIF_TRACE_INFO("EsifCmp Compressed: %u => %zd bytes\n", expandedSize, compressedSize);

						EsifData_Set(self,
							self->type,
							compressedData,
							(u32)compressedSize,
							(u32)compressedSize);

						compressedData = NULL; // Now owned by self
					}
				}
				esif_ccb_free(compressedData);
			}
		}

		// Unload Library
		esif_ccb_library_unload(esifCmpLib);
	}

	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("EsifCmp Compress Error: %s (%d) [result=%d, original=%zd, compressed=%zd]\n",
			esif_rc_str(rc), rc, esifCmp_result, expandedSize, compressedSize);
	}
	return rc;
}

/* Decompress an EsifData buffer and return updated buffer and size to caller if successful.
 * ESIF Compression Library is dynamically loaded and used to decompress data unless the data appears to
 * already be uncompressed. buf_ptr is replaced if successfully decompressed, otherwise it is unchanged.
 * If the original buf_ptr passed in is not dynamically allocated, buf_len MUST be 0 (but not data_len).
 * The Compression Library is unloaded after decompression is complete.
 *
 * Note that the compression library is mandatory for Decompression (if the data appears to be compressed)
 * and an error is returned if it is not found or otherwise cannot be loaded.
 *
 * Returns ESIF_OK if data is already uncompressed, or if library loaded and data successfully decompressed.
 */
esif_error_t EsifData_Decompress(EsifDataPtr self)
{
	esif_error_t rc = (self ? ESIF_OK : ESIF_E_PARAMETER_IS_NULL);
	int esifCmp_result = 0;
	BytePtr compressedData = NULL;
	size_t compressedSize = 0;
	BytePtr expandedData = NULL;
	size_t expandedSize = 0;

	// Load Compression Library and Decompress data if it is compressed
	if (rc == ESIF_OK && EsifCmp_IsCompressed(self->buf_ptr, self->data_len) == ESIF_TRUE) {
		char esifCmpLibPath[MAX_PATH] = { 0 };
		IpfDecompressFuncPtr fnDecompress = NULL;

		compressedData = self->buf_ptr;
		compressedSize = self->data_len;

		esif_build_path(esifCmpLibPath, sizeof(esifCmpLibPath), ESIF_PATHTYPE_DLL, ESIFCMP_LIBRARY, ESIF_LIB_EXT);
		esif_lib_t esifCmpLib = esif_ccb_library_load(esifCmpLibPath);

		if (esifCmpLib == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if (esifCmpLib->handle == NULL) {
			rc = esif_ccb_library_error(esifCmpLib);
		}
		else if ((fnDecompress = (IpfDecompressFuncPtr)esif_ccb_library_get_func(esifCmpLib, ESIFCMP_DECOMPRESSOR)) == NULL) {
			rc = ESIF_E_IFACE_NOT_SUPPORTED;
		}
		else {
			// 1. Get Actual Decompressed Buffer Size
			esifCmp_result = (*fnDecompress)(
				NULL,
				&expandedSize,
				compressedData,
				compressedSize
				);

			// 2. Allocate Buffer and Decompress
			if (esifCmp_result != 0) {
				rc = ESIF_E_COMPRESSION_ERROR;
			}
			else if (expandedSize > MAX_COMPRESSED_DATA) {
				rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			}
			else if (expandedSize > 0) {
				expandedData = (BytePtr)esif_ccb_malloc(expandedSize);

				if (expandedData == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					esifCmp_result = (*fnDecompress)(
						expandedData,
						&expandedSize,
						compressedData,
						compressedSize
						);

					if (esifCmp_result != 0) {
						rc = ESIF_E_COMPRESSION_ERROR;
					}
					else {
						ESIF_TRACE_INFO("EsifCmp Decompressed: %u => %zd bytes\n", compressedSize, expandedSize);

						EsifData_Set(self,
							self->type,
							expandedData,
							(u32)expandedSize,
							(u32)expandedSize);

						expandedData = NULL; // Now owned by self
						rc = ESIF_OK;
					}
				}
				esif_ccb_free(expandedData);
			}
		}

		// Unload Library
		esif_ccb_library_unload(esifCmpLib);
	}

	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("EsifCmp Decompress Error: %s (%d) [result=%d, compressed=%zd, expanded=%zd]\n",
			esif_rc_str(rc), rc, esifCmp_result, compressedSize, expandedSize);
	}
	return rc;
}

#else // No Compression Support

Bool EsifData_IsCompressed(EsifDataPtr self)
{
	Bool rc = ESIF_FALSE;
	if (self) {
		rc = (Bool)EsifCmp_IsCompressed(self->buf_ptr, self->data_len);
	}
	return rc;
}

esif_error_t EsifData_Compress(EsifDataPtr self)
{
	return (self && self->buf_ptr ? ESIF_OK : ESIF_E_PARAMETER_IS_NULL);
}

esif_error_t EsifData_Decompress(EsifDataPtr self)
{
	esif_error_t rc = (self && self->buf_ptr ? ESIF_OK : ESIF_E_PARAMETER_IS_NULL);
	if (EsifData_IsCompressed(self)) {
		rc = ESIF_E_NOT_IMPLEMENTED;
	}
	return rc;
}

#endif
