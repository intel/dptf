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
#ifndef _ISTRING_H
#define _ISTRING_H

#include "esif.h"
#include "esif_lib.h"

//////////////////////////////////////////////////////////////////////////////
// Primitive Objects
// ZString Class = Null Terminated ASCII String
typedef char*ZString;

typedef unsigned char Byte, *BytePtr;

#define ISTRING_NOIGNORECASE    0			// Case-Sensitive Compare or Search/Replace
#define ISTRING_IGNORECASE      1			// Case-Insensitive Compare or Search/Replace
#define ZSTRING_MAXLEN          MAXAUTOLEN	// Max length of a Null-Terminated ASCII string when the length is not specified

//////////////////////////////////////////////////////////////////////////////
// IString Class = Intelligent String Class [or Intel String] (implemeted as EsifData)
#ifdef _ISTRING_CLASS

// IString storage is currently implmented as struct esif_data. Redefine here to use a new data type
#define IString_s esif_data
#define ISTRING_STATIC(str)     {ESIF_DATA_STRING, (str), 0, ((str) ? (u32)esif_ccb_strlen((str), ZSTRING_MAXLEN) + 1 : 0)}

#else
struct IString_s;	// Encapsulation Placeholder
#endif

typedef struct IString_s IString, *IStringPtr, **IStringPtrLocation;

// object management
void IString_ctor (IStringPtr self);	// constructor
void IString_dtor (IStringPtr self);	// destructor
IStringPtr IString_Create ();					// new operator
void IString_Destroy (IStringPtr self);	// delete operator

// overloaded constructors
IStringPtr IString_CreateAs (u32 buf_len);						// Create an empty IString using the specified buffer length
IStringPtr IString_CreateFrom (IStringPtr src);					// Duplicate an IString into a new Dynamically-Allocated IString

// IString methods (Dynamic or Static)
ZString IString_GetString (IStringPtr self);				// Return Pointer to Null Terminated String
u32 IString_Strlen (IStringPtr self);					// String Length in characters not including Null terminator
BytePtr IString_GetBuf (IStringPtr self);					// Return Pointer to Buffer
u32 IString_BufLen (IStringPtr self);					// Buffer Length in bytes including Null terminator, if Dynamically allocated, otherwise 0
u32 IString_DataLen (IStringPtr self);					// Data Length in bytes including Null terminator
void IString_Truncate (IStringPtr self);				// Truncate an IString without releasing the buffer
int IString_Compare (IStringPtr self, IStringPtr str2, int IgnoreCase);				// Compare two Istrings (optional IgnoreCase)
int IString_CompareMax (IStringPtr self, IStringPtr str2, u32 count, int IgnoreCase);	// Compare up to count characters of two IStrings (optional IgnoreCase)
void IString_ToUpper (IStringPtr self);					// Convert an IString to UPPERCASE
void IString_ToLower (IStringPtr self);					// Convert an IString to lowercase
int IString_Sscanf (IStringPtr self, ZString format, ...);	// SScanf from IString into a variable. Must include buffer size if target is a String

// TODO:    Find, FindAny, InsertAt, DeleteAt, StrTok, ...

// IString methods (Dynamic only)
ZString IString_Resize (IStringPtr self, u32 buf_len);					// Resize an IString to the given buffer length. Allocate if necessary
ZString IString_GrowBy (IStringPtr self, u32 bytes);					// Grow an IString by the given bytes. Allocate if necessary
ZString IString_Copy (IStringPtr self, IStringPtr src);					// Copy an IString to another IString. AutoGrow if necessary
ZString IString_Concat (IStringPtr self, IStringPtr src);				// Concatenate (Append) an IString to another Istring. AutoGrow if necessary
int IString_Sprintf (IStringPtr self, ZString format, ...);							// Sprintf to IString, overwriting current string. AutoGrow if necessary
int IString_SprintfConcat (IStringPtr self, ZString format, ...);					// Sprintf to IString, concatenate (append) to current string. AutoGrow if
																					// necessary
int IString_VSprintfTo (IStringPtr self, u32 offset, ZString format, va_list args);	// VSprintf to IString, starting at given offset. AutoGrow if necessary
ZString IString_ReplaceIString (IStringPtr self, IStringPtr what, IStringPtr with, int IgnoreCase);	// Search & Replace IStrings within an IString. AutoGrow if
																									// necessary

// ZString methods
IStringPtr IString_StrdupSubstr (ZString src, u32 chars);					// Duplicate a Substring into an IString. Auto-Allocate new buffer
IStringPtr IString_Strdup (ZString src);									// Duplicate a ZString into an IString. Auto-Allocate new buffer
ZString IString_Strcpy (IStringPtr self, ZString src);					// Copy a ZString to an IString, AutoGrow if necessary
ZString IString_Strcat (IStringPtr self, ZString src);					// Concatenate (Append) a ZString to an IString, AutoGrow if necessary
int IString_Strcmp (IStringPtr self, ZString str2);					// Compare an IString to a ZString (Case-Sensitive)
int IString_Stricmp (IStringPtr self, ZString str2);				// Compare an IString to a ZString (Case-Insensitive)
int IString_Strncmp (IStringPtr self, ZString str2, u32 count);		// Compare up to count characters of an IString and ZString (Case-Sensitive)
int IString_Strnicmp (IStringPtr self, ZString str2, u32 count);	// Compare up to count characters of an IString and ZString (Case-Insensitive)
ZString IString_Replace (IStringPtr self, ZString what, ZString with);	// Search & Replace ZStrings within na IString. AutoGrow if necessary

#endif
