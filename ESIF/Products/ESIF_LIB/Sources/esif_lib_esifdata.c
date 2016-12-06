/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "esif_lib_esifdata.h"
#include "esif_temp.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define tohexdigit(ch)  ((ch) >= '0' && (ch) <= '9' ? ((ch) - '0') : (toupper(ch) - 'A' + 10))

// Max String Lengths for each Data type class
#define MAXSTR_INT8         5
#define MAXSTR_INT16        7
#define MAXSTR_INT32        12
#define MAXSTR_INT64        22
#define MAXDISPLAY_STRING   256
#define MAXDISPLAY_BINARY   (MAXDISPLAY_STRING / 2 - 2)

#define MIN_TEMPERATURE		(-273.15)	// Min Supported Temperature in Celsius (Absolute Zero)
#define MAX_TEMPERATURE		(9999)		// Max Supported Temperature in Celsius

#define ISHEX(str)  (esif_ccb_strnicmp((str), "0x", 2) == 0 ? ESIF_TRUE : ESIF_FALSE)
#define IFHEX(str, hex, dec)    (esif_ccb_strnicmp((str), "0x", 2) == 0 ? (hex) : (dec))

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
	case (EsifDataType)0:	// NA
	case ESIF_DATA_VOID:
		size = 0;
		break;

	case ESIF_DATA_BIT:
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
	case ESIF_DATA_STRUCTURE:
	case ESIF_DATA_XML:
	default:
		size = self->data_len;	// Or 0xFFFFFFFF?
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
char *EsifData_ToString (EsifDataPtr self)
{
	char *result   = 0;
	UInt32 alloc   = 0;
	UInt32 u32data = 0;
	UInt64 u64data = 0;
	Byte *ptrdata  = 0;
	UInt32 ptrlen  = 0;
	UInt32 idx     = 0;

	if (self == NULL || self->buf_ptr == NULL || self->data_len == 0) {
		return NULL;
	}

	// Determine Buffer Size and Conversion Values
	switch (self->type) {
	case ESIF_DATA_BIT:
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
		alloc   = esif_ccb_min(self->data_len, MAXDISPLAY_STRING + 1);
		ptrdata = (Byte*)self->buf_ptr;
		ptrlen  = self->data_len;
		break;

   case ESIF_DATA_VOID:
	   alloc   = 0;
	   ptrdata = NULL;
	   ptrlen  = 0;
	   break;

	/* TODO:
	   case ESIF_DATA_BLOB:
	   case ESIF_DATA_DSP:
	   case ESIF_DATA_ENUM:
	   case ESIF_DATA_GUID:
	   case ESIF_DATA_HANDLE:
	   case ESIF_DATA_INSTANCE:
	   case ESIF_DATA_IPV4:
	   case ESIF_DATA_IPV6:
	   case ESIF_DATA_POINTER:
	   case ESIF_DATA_QUALIFIER:
	   case ESIF_DATA_REGISTER:
	   case ESIF_DATA_STRUCTURE:
	   case ESIF_DATA_TABLE:
	   case ESIF_DATA_UNICODE:
	 */
	case ESIF_DATA_BINARY:
	default:
		alloc   = 2 + (2 * esif_ccb_min(self->data_len, MAXDISPLAY_BINARY)) + 2 + 1;
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
		case ESIF_DATA_BIT:
			esif_ccb_sprintf(sizeof(intstr), intstr, "%ld", (long)((Int8)u32data & 0x1));
			esif_ccb_strcpy(result, intstr, alloc);
			break;

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
			if (ptrlen <= MAXDISPLAY_STRING + 1) {
				esif_ccb_memcpy(result, ptrdata, ptrlen);
			} else {
				esif_ccb_memcpy(result, ptrdata, MAXDISPLAY_STRING - 3);
				esif_ccb_memcpy(result + MAXDISPLAY_STRING - 3, "...", 4);
			}
			break;

		/* TODO:
		   case ESIF_DATA_BLOB:
		   case ESIF_DATA_DSP:
		   case ESIF_DATA_ENUM:
		   case ESIF_DATA_GUID:
		   case ESIF_DATA_HANDLE:
		   case ESIF_DATA_INSTANCE:
		   case ESIF_DATA_IPV4:
		   case ESIF_DATA_IPV6:
		   case ESIF_DATA_POINTER:
		   case ESIF_DATA_QUALIFIER:
		   case ESIF_DATA_REGISTER:
		   case ESIF_DATA_STRUCTURE:
		   case ESIF_DATA_TABLE:
		   case ESIF_DATA_UNICODE:
		 */
		case ESIF_DATA_BINARY:
		default:
			esif_ccb_strcpy(result, "0x", alloc);
			for (idx = 0; idx < ptrlen && idx < MAXDISPLAY_BINARY; idx++)
				esif_ccb_sprintf(alloc - 2 - (idx * 2), result + 2 + (2 * idx), "%02X", ptrdata[idx]);
			if (ptrlen > MAXDISPLAY_BINARY) {
				esif_ccb_strcat(result, "..", alloc);
			}
			break;
		}
	}
	return result;
}


// FromString operator
eEsifError EsifData_FromString (
	EsifDataPtr self,
	char *str,
	EsifDataType type	/* , int maxlen=-1 */
	)
{
	eEsifError rc  = ESIF_E_UNSPECIFIED;
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
	case ESIF_DATA_BIT:
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
		alloc   = (UInt32)esif_ccb_strlen(str, MAXAUTOLEN) + 1;
		ptrdata = (Byte*)str;
		ptrlen  = alloc;
		break;

   case ESIF_DATA_VOID:
	   alloc   = 0;
	   ptrdata = NULL;
	   ptrlen  = 0;
	   break;

	/* TODO:
	   case ESIF_DATA_BLOB:
	   case ESIF_DATA_DSP:
	   case ESIF_DATA_ENUM:
	   case ESIF_DATA_GUID:
	   case ESIF_DATA_HANDLE:
	   case ESIF_DATA_INSTANCE:
	   case ESIF_DATA_IPV4:
	   case ESIF_DATA_IPV6:
	   case ESIF_DATA_POINTER:
	   case ESIF_DATA_QUALIFIER:
	   case ESIF_DATA_REGISTER:
	   case ESIF_DATA_STRUCTURE:
	   case ESIF_DATA_TABLE:
	   case ESIF_DATA_UNICODE:
	 */
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
		rc = ESIF_OK;

		// Convert Data
		switch (type) {
		case ESIF_DATA_BIT:
		case ESIF_DATA_INT8:
			esif_ccb_sscanf(str, IFHEX(str, "%x", "%d"), &u32data);
			u32data = (self->type == ESIF_DATA_BIT ? u32data & 0x1 : u32data);
			*STATIC_CAST(Int8*, buffer) = (Int8)u32data;
			break;

		case ESIF_DATA_INT16:
			esif_ccb_sscanf(str, IFHEX(str, "%x", "%d"), &u32data);
			*STATIC_CAST(Int16*, buffer) = (Int16)u32data;
			break;

		case ESIF_DATA_INT32:
			esif_ccb_sscanf(str, IFHEX(str, "%x", "%d"), &u32data);
			*STATIC_CAST(Int32*, buffer) = (Int32)u32data;
			break;

		case ESIF_DATA_UINT8:
			esif_ccb_sscanf(str, IFHEX(str, "%x", "%u"), &u32data);
			*STATIC_CAST(UInt8*, buffer) = (UInt8)u32data;
			break;

		case ESIF_DATA_UINT16:
			esif_ccb_sscanf(str, IFHEX(str, "%x", "%u"), &u32data);
			*STATIC_CAST(UInt16*, buffer) = (UInt16)u32data;
			break;

		case ESIF_DATA_UINT32:
		case ESIF_DATA_POWER:
		case ESIF_DATA_TIME:
		case ESIF_DATA_PERCENT:
			esif_ccb_sscanf(str, IFHEX(str, "%x", "%u"), &u32data);
			*STATIC_CAST(UInt32*, buffer) = (UInt32)u32data;
			break;

		/*
		 * Temp input in floating point Celsius with 1 decimal point precision,
		 * but stored in 10ths K
		 */
		case ESIF_DATA_TEMPERATURE:
		{
			if (ISHEX(str)) {
				u32 temp = 0;
				esif_ccb_sscanf(str, "%x", &temp);
				temp = (temp > MAX_TEMPERATURE ? MAX_TEMPERATURE : temp);
				u32data = temp * 10;
			}
			else {
				double temp = 0.0;
				esif_ccb_sscanf(str,"%lf", &temp);
				temp = (temp < MIN_TEMPERATURE ? MIN_TEMPERATURE : temp > MAX_TEMPERATURE ? MAX_TEMPERATURE : temp);
				temp *= 10.0;
				u32data = (UInt32)temp;
			}
			esif_convert_temp(ESIF_TEMP_DECIC, NORMALIZE_TEMP_TYPE, &u32data);

			*STATIC_CAST(UInt32*, buffer) = (UInt32)u32data;
			break;
		}
		case ESIF_DATA_INT64:
			esif_ccb_sscanf(str, IFHEX(str, "%llx", "%lld"), &u64data);
			*STATIC_CAST(Int64*, buffer) = (UInt64)u64data;
			break;

		case ESIF_DATA_UINT64:
		case ESIF_DATA_FREQUENCY:
			esif_ccb_sscanf(str, IFHEX(str, "%llx", "%llu"), &u64data);
			*STATIC_CAST(UInt64*, buffer) = (UInt64)u64data;
			break;

		case ESIF_DATA_STRING:
			esif_ccb_memcpy(buffer, ptrdata, ptrlen);
			break;

		/* TODO:
		   case ESIF_DATA_BLOB:
		   case ESIF_DATA_DSP:
		   case ESIF_DATA_ENUM:
		   case ESIF_DATA_GUID:
		   case ESIF_DATA_HANDLE:
		   case ESIF_DATA_INSTANCE:
		   case ESIF_DATA_IPV4:
		   case ESIF_DATA_IPV6:
		   case ESIF_DATA_POINTER:
		   case ESIF_DATA_QUALIFIER:
		   case ESIF_DATA_REGISTER:
		   case ESIF_DATA_STRUCTURE:
		   case ESIF_DATA_TABLE:
		   case ESIF_DATA_UNICODE:
		 */
		case ESIF_DATA_BINARY:
		default:
			// TODO: Convert "0xABCD"
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
			break;
		}
	}

exit:
	return rc;
}