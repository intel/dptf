/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#include <stdarg.h>
#include <ctype.h>

#define _ISTRING_CLASS
#include "esif_lib_istring.h"

#ifdef ESIF_ATTR_OS_WINDOWS
# define _SDL_BANNED_RECOMMENDED
# include "win/banned.h"
#endif

#define ISTRING_AUTOGROW    0	// Autogrow IString buffer if necessary and pad with this many extra bytes (0-N). Undefine this to disable Autogrow

// static members
static void IString_SetInstance (IStringPtr self, void *buf_ptr, u32 buf_len, u32 data_len);
static IStringPtr IString_CreateInstance (void *buf_ptr, u32 buf_len, u32 data_len);

// Replacement for strstr() that supports Case-Insensitive searches
static const char*strfind (
	const char *str,
	const char *strSearch,
	int IgnoreCase
	)
{
	if (IgnoreCase) {
		size_t len = esif_ccb_strlen(strSearch, 0x7FFFFFFF);
		while (*str != 0 && esif_ccb_strnicmp(str, strSearch, len) != 0)
			str++;
		return *str != 0 ? str : 0;
	}
	return strstr(str, strSearch);
}


// object management
void ESIF_INLINE IString_ctor (IStringPtr self)
{
	if (self) {
		WIPEPTR(self);
		self->type = ESIF_DATA_STRING;
	}
}


void ESIF_INLINE IString_dtor (IStringPtr self)
{
	ASSERT(self);
	if (self->buf_len) {
		esif_ccb_free(self->buf_ptr);
	}
	WIPEPTR(self);
}


// new operator
IStringPtr IString_Create ()
{
	IStringPtr self = (IStringPtr)esif_ccb_malloc(sizeof(*self));
	IString_ctor(self);
	return self;
}


// delete operator
void IString_Destroy (IStringPtr self)
{
	IString_dtor(self);
	esif_ccb_free(self);
}


// constructors
IStringPtr IString_CreateAs (u32 buf_len)
{
	IStringPtr self = IString_Create();
	if (self && buf_len) {
		IString_SetInstance(self, esif_ccb_malloc(buf_len), buf_len, 1);
	}
	return self;
}


IStringPtr IString_CreateFrom (IStringPtr src)
{
	IStringPtr self = IString_Create();
	if (self && src) {
		IString_SetInstance(self, esif_ccb_malloc(src->buf_len), src->buf_len, src->data_len);
		if (self->buf_ptr) {
			esif_ccb_strcpy((ZString)self->buf_ptr, (ZString)src->buf_ptr, src->data_len);
		}
	}
	return self;
}


// Low-Level operators. Caller is responsible for allocating buf_ptr and setting buf_len
static void IString_SetInstance (
	IStringPtr self,
	void *buf_ptr,
	u32 buf_len,
	u32 data_len
	)
{
	IString_dtor(self);
	self->type     = ESIF_DATA_STRING;
	self->buf_ptr  = buf_ptr;
	self->buf_len  = buf_len;
	self->data_len = data_len;
}


// Low Level Create Instance. Caller is responsible for allocating buf_ptr and setting buf_len
static IStringPtr IString_CreateInstance (
	void *buf_ptr,
	u32 buf_len,
	u32 data_len
	)
{
	IStringPtr self = IString_Create();
	if (self) {
		IString_SetInstance(self, buf_ptr, buf_len, data_len);
	}
	return self;
}


//////////////////////////////////////////////////////////////////////////////
// IString methods (Static or Dynamic)

// Return Pointer to Null Terminated String
ZString IString_GetString (IStringPtr self)
{
	return (ZString)(self && self->buf_ptr ? self->buf_ptr : "");
}


// Get IString Length, not including Null terminator
u32 IString_Strlen (IStringPtr self)
{
	return self && self->data_len ? self->data_len - 1 : 0;
}


// Return Pointer to Buffer
BytePtr IString_GetBuf (IStringPtr self)
{
	return (BytePtr)(self ? self->buf_ptr : 0);
}


// Buffer Length in bytes including Null terminator if Dynamically allocated, otherwise 0
u32 IString_BufLen (IStringPtr self)
{
	return self ? self->buf_len : 0;
}


// Data Length in bytes including Null terminator
u32 IString_DataLen (IStringPtr self)
{
	return self ? self->data_len : 0;
}


// Truncate an IString
void IString_Truncate (IStringPtr self)
{
	ASSERT(self);
	if (self->data_len) {
		*((ZString)(self->buf_ptr)) = 0;
		self->data_len = 1;
	}
}


// Compare two IStrings
int IString_Compare (
	IStringPtr self,
	IStringPtr str2,
	int IgnoreCase
	)
{
	if (self && str2 && self->buf_ptr && str2->buf_ptr) {
		if (IgnoreCase) {
			return esif_ccb_stricmp((ZString)self->buf_ptr, (ZString)str2->buf_ptr);
		} else {
			return esif_ccb_strcmp((ZString)self->buf_ptr, (ZString)str2->buf_ptr);
		}
	}
	return 0;
}


// Compare two IStrings, up to count characters
int IString_CompareMax (
	IStringPtr self,
	IStringPtr str2,
	u32 count,
	int IgnoreCase
	)
{
	if (self && str2 && self->buf_ptr && str2->buf_ptr) {
		if (IgnoreCase) {
			return esif_ccb_strnicmp((ZString)self->buf_ptr, (ZString)str2->buf_ptr, count);
		} else {
			return esif_ccb_strncmp((ZString)self->buf_ptr, (ZString)str2->buf_ptr, count);
		}
	}
	return 0;
}


// Convert IString to uppercase
void IString_ToUpper (IStringPtr self)
{
	if (self && self->buf_ptr) {
		esif_ccb_strupr((ZString)self->buf_ptr, self->data_len);
	}
}


// Convert IString to lowercase
void IString_ToLower (IStringPtr self)
{
	if (self && self->buf_ptr) {
		esif_ccb_strlwr((ZString)self->buf_ptr, self->data_len);
	}
}


// sscanf from an IString into a ZString or other data type, including buffer length of strings for sscanf_s compatibility
int IString_Sscanf (
	IStringPtr self,
	ZString format,
	...
	)
{
	int result = 0;
	const char *fmtstr = 0;
	va_list args;

	ASSERT(self);
	if (self->data_len <= 1) {
		return result;
	}

	va_start(args, format);

	// If format specifier is a string (%s, %S, %[...]), get buffer length from arguments
	fmtstr = strchr(format, '%');
	if (fmtstr) {
		const char *fmt = fmtstr + 1;
		u32 len = 0;
		BytePtr arg     = va_arg(args, BytePtr);

		// Skip optional width & size4
		while (*fmt != 0 && (isdigit(*fmt) || strchr("ILlh", *fmt) != 0))
			fmt++;
		if (strchr("sS[]", *fmt) != 0) {// %s, %S, %[a-z]
			len = va_arg(args, u32);
		}
		result = esif_ccb_sscanf((ZString)self->buf_ptr, format, arg, len);
	}
	va_end(args);
	return result;
}


//////////////////////////////////////////////////////////////////////////////
// IString methods (Dynamic only)

// Resize an IString buffer (if dynamically allocated)
ZString IString_Resize (
	IStringPtr self,
	u32 buf_len
	)
{
	ASSERT(self);
	// Allocate initial buffer if it has never been allocated
	if (self->buf_ptr == 0) {
		self->buf_ptr = esif_ccb_malloc(buf_len);
		if (self->buf_ptr) {
			self->buf_len  = buf_len;
			self->data_len = 1;
			return (ZString)self->buf_ptr;
		}
	}
	// Resize buffer if it is not a static string
	if (self->buf_len > 0) {
		ZString buf_ptr = (ZString)esif_ccb_realloc(self->buf_ptr, buf_len);
		if (buf_ptr) {
			if (buf_len > self->buf_len) {
				esif_ccb_memset(buf_ptr + self->buf_len, 0, buf_len - self->buf_len);
			}
			self->buf_ptr = buf_ptr;
			self->buf_len = buf_len;
			return (ZString)self->buf_ptr;
		}
	}
	return 0;
}


// Grow an IString buffer
ZString IString_GrowBy (
	IStringPtr self,
	u32 bytes
	)
{
	return IString_Resize(self, self->buf_len + bytes);
}


// Copy an IString, overwriting the existing IString
ZString IString_Copy (
	IStringPtr self,
	IStringPtr src
	)
{
	ASSERT(self && src);
	if (src->data_len > self->buf_len) {
#ifdef ISTRING_AUTOGROW
		if (IString_Resize(self, src->data_len + ISTRING_AUTOGROW) == NULL)
#endif
		return 0;
	}
	esif_ccb_strcpy((ZString)self->buf_ptr, (ZString)src->buf_ptr, self->buf_len);
	self->data_len = src->data_len;
	return (ZString)self->buf_ptr;
}


// Concatenate (Append) an IString to the current IString
ZString IString_Concat (
	IStringPtr self,
	IStringPtr src
	)
{
	u32 self_len;
	ASSERT(self && src);
	self_len = (self->data_len ? self->data_len - 1 : 0);
	if (self_len + src->data_len > self->buf_len) {
#ifdef ISTRING_AUTOGROW
		if (IString_Resize(self, self_len + src->data_len + ISTRING_AUTOGROW) == NULL)
#endif
		return 0;
	}
	esif_ccb_strcpy((ZString)self->buf_ptr + self_len, (ZString)src->buf_ptr, self->buf_len - self_len);
	self->data_len += src->data_len - 1;
	return (ZString)self->buf_ptr;
}


// vsprintf to an IString, starting at offset
int IString_VSprintfTo (
	IStringPtr self,
	u32 offset,
	ZString format,
	va_list args
	)
{
	u32 len;
	int result = 0;
	ASSERT(self);

	len    = esif_ccb_vscprintf(format, args);
	offset = (self->data_len > 0 ? esif_ccb_min(offset, self->data_len - 1) : 0);
	if (offset + len + 1 > self->buf_len) {
#ifdef ISTRING_AUTOGROW
		if (IString_Resize(self, offset + len + 1 + ISTRING_AUTOGROW) == NULL)
#endif
		return 0;
	}
	result = esif_ccb_vsprintf(self->buf_len - offset, (ZString)self->buf_ptr + offset, format, args);
	self->data_len = offset + result + 1;
	return result;
}


// sprintf to an IString, overwriting current string
int IString_Sprintf (
	IStringPtr self,
	ZString format,
	...
	)
{
	int result;
	va_list args;
	va_start(args, format);
	result = IString_VSprintfTo(self, 0, format, args);
	va_end(args);
	return result;
}


// sprintf to an IString, concatenate (append) to current string
int IString_SprintfConcat (
	IStringPtr self,
	ZString format,
	...
	)
{
	int result;
	va_list args;
	va_start(args, format);
	result = IString_VSprintfTo(self, IString_Strlen(self), format, args);
	va_end(args);
	return result;
}


// Search and Replace
ZString IString_ReplaceIString (
	IStringPtr self,
	IStringPtr what,
	IStringPtr with,
	int IgnoreCase
	)
{
	ZString from, to, find;
	u32 count, oldsize, newsize;

	// Sanity checks. Cannot replace an empty string
	ASSERT(self && what && with);
	if (self->data_len <= 1 || what->data_len <= 1) {
		return 0;
	}

	// Count occurances of replacment string in original string
	for (count = 0, find = (ZString)self->buf_ptr; (find = (ZString)strfind(find, (ZString)what->buf_ptr, IgnoreCase)) != NULL; count++)
		find += what->data_len - 1;

	// Compute new string size and Resize if necessary
	oldsize = self->data_len;
	newsize = self->data_len + (count * (int)(esif_ccb_max(with->data_len, 1) - what->data_len));
	if (newsize > self->buf_len) {
#ifdef ISTRING_AUTOGROW
		if (IString_Resize(self, newsize + ISTRING_AUTOGROW) == NULL)
#endif
		return 0;
	}

	// Do an in-string replacement so that another copy of the string does not need to be allocated
	// a) newsize <= oldsize: Do a left-to-right copy replacment
	// b) newsize >  oldsize: Move string to end of newsize buffer, then do a left-to-right copy replacement
	from = to = (ZString)self->buf_ptr;
	self->data_len = newsize;
	if (newsize > oldsize) {
		// Move string to end of reallocated (data_len) buffer
		esif_ccb_memmove(((ZString)self->buf_ptr) + (newsize - oldsize), (ZString)self->buf_ptr, oldsize);
		from += newsize - oldsize;
	}
	// Do a left-to-right copy (from -> to), replacing each occurance of old string (what) with new string (with)
	while ((find = (ZString)strfind(from, (ZString)what->buf_ptr, IgnoreCase)) != NULL) {
		if (from > to) {
			esif_ccb_memcpy(to, from, (size_t)(find - from));
		}
		to += (size_t)(find - from);
		if (with->data_len > 0) {
			esif_ccb_memcpy(to, (ZString)with->buf_ptr, with->data_len - 1);
			to += with->data_len - 1;
		}
		from = find + (what->data_len > 0 ? what->data_len - 1 : 0);
	}
	// Copy remainder of string, if any
	if (to < from) {
		esif_ccb_memcpy(to, from, newsize - (size_t)(to - (ZString)self->buf_ptr));
	}
	to += newsize - (size_t)(to - (ZString)self->buf_ptr);
	// zero out remainder of old string, if any
	if (oldsize > newsize) {
		esif_ccb_memset(to, 0, oldsize - newsize);
	}
	return (ZString)self->buf_ptr;
}


//////////////////////////////////////////////////////////////////////////////
// ZString methods

// Duplicate a ZString substring into a new Dynamically-Allocated IString
IStringPtr IString_StrdupSubstr (
	ZString src,
	u32 chars
	)
{
	IStringPtr self = IString_CreateAs(chars + 1);
	if (self && self->buf_ptr) {
		esif_ccb_strcpy((ZString)self->buf_ptr, src, self->buf_len);
		self->data_len = self->buf_len;
	}
	return self;
}


// Duplicate a ZString into a new Dynamically-Allocated IString
IStringPtr IString_Strdup (ZString src)
{
	return IString_StrdupSubstr(src, (u32)esif_ccb_strlen(src, ZSTRING_MAXLEN));
}


// Copy a ZString into an existing IString, autogrow if necessary
ZString IString_Strcpy (
	IStringPtr self,
	ZString src
	)
{
	IString iSrc = ISTRING_STATIC(src);
	return (ZString)(IString_Copy(self, &iSrc) ? self->buf_ptr : 0);
}


// Concatenate a ZString to an IString, autogrow if necessary
ZString IString_Strcat (
	IStringPtr self,
	ZString src
	)
{
	IString iSrc = ISTRING_STATIC(src);
	return (ZString)(IString_Concat(self, &iSrc) ? self->buf_ptr : 0);
}


// Compare Functions
int IString_Strcmp (
	IStringPtr self,
	ZString str2
	)
{
	ASSERT(self);
	if (self->buf_ptr && str2) {
		return esif_ccb_strcmp((ZString)self->buf_ptr, str2);
	}
	return 0;
}


int IString_Stricmp (
	IStringPtr self,
	ZString str2
	)
{
	ASSERT(self);
	if (self->buf_ptr && str2) {
		return esif_ccb_stricmp((ZString)self->buf_ptr, str2);
	}
	return 0;
}


int IString_Strncmp (
	IStringPtr self,
	ZString str2,
	u32 count
	)
{
	ASSERT(self);
	if (self->buf_ptr && str2) {
		return esif_ccb_strncmp((ZString)self->buf_ptr, str2, count);
	}
	return 0;
}


int IString_Strnicmp (
	IStringPtr self,
	ZString str2,
	u32 count
	)
{
	ASSERT(self);
	if (self->buf_ptr && str2) {
		return esif_ccb_strnicmp((ZString)self->buf_ptr, str2, count);
	}
	return 0;
}


ZString IString_Replace (
	IStringPtr self,
	ZString what,
	ZString with
	)
{
	IString iWhat = ISTRING_STATIC(what);
	IString iWith = ISTRING_STATIC(with);
	return IString_ReplaceIString(self, &iWhat, &iWith, 0);
}


