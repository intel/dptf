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
#ifndef _IOSTREAM_H_
#define _IOSTREAM_H_

#include <stdio.h>

#include "esif_lib.h"
#include "esif_lib_istringlist.h"

// errno values. See also: errno.h
#define     EOK         0
#define		IOSTREAM_MEMORY_STREAM_BLOCK_SIZE 512

//////////////////////////////////////////////////////////////////////////////
// IOStream Class

typedef unsigned char Byte, *BytePtr, **BytePtrLocation;

union IOStream_s;
typedef union IOStream_s IOStream, *IOStreamPtr, **IOStreamPtrLocation;

typedef enum stream_type {
	StreamNull,
	StreamFile,
	StreamMemory,
} StreamType;

#ifdef _IOSTREAM_CLASS

union IOStream_s {
	StreamType  type;			// Stream Type
	struct {
		StreamType  type;
		StringPtr   name;		// File Name (Full or Relative Pathname)
		FILE        *handle;	// File Handle
		StringPtr   mode;		// File Open Mode ("rb", "wb", "ab")
	}

	file;

	struct {
		StreamType  type;
		Byte        *buffer;	// Buffer Pointer
		size_t      buf_len;	// Buffer Size if Dynamically Allocated, Otherwize 0
		size_t      data_len;	// Buffer Data Length
		size_t      offset;		// Current Offset from Buffer Pointer
	}

	memory;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

// object management
IOStreamPtr IOStream_Create();
void IOStream_Destroy(IOStreamPtr self);

// methods
// Open/Close File or Memory Block
int IOStream_SetFile(IOStreamPtr self, StringPtr filename, StringPtr mode);
int IOStream_SetMemory(IOStreamPtr self, BytePtr buffer, size_t size);
int IOStream_OpenFile(IOStreamPtr self, StringPtr filename, StringPtr mode);
int IOStream_Open(IOStreamPtr self);	// fopen equivalent
int IOStream_Close(IOStreamPtr self);	// fclose equivalent

// Clone another IOStream as a new memory IOStream [dynamic, self-owned]
eEsifError IOStream_CloneAsMemoryStream(IOStreamPtr self, IOStreamPtr *clonePtr);

// Stream I/O
size_t IOStream_Read(IOStreamPtr self, void *dest_buffer, size_t bytes);		// fread equivalent
size_t IOStream_ReadAt(IOStreamPtr self, void *dest_buffer, size_t bytes, size_t offset);
size_t IOStream_Write(IOStreamPtr self, void *src_buffer, size_t bytes);		// fwrite equivalent
// Note: Applicable to memory streams only
size_t IOStream_WriteAt(IOStreamPtr self, void *src_buffer, size_t bytes, size_t offset);

StringPtr IOStream_GetLine(IOStreamPtr self, StringPtr dest_buffer, int bytes);	// fgets equivalent
int IOStream_Seek(IOStreamPtr self, size_t offset, int origin);			// fseek equivalent
size_t IOStream_GetOffset(IOStreamPtr self);									// ftell equivalent [fgetpos?]

// TODO:	fputs, fprintf, fscanf, fgetpos, fsetpos, fflush, fgetc, fputc

// Member Access and Helper Functions
StreamType IOStream_GetType(IOStreamPtr self);
size_t IOStream_GetSize(IOStreamPtr self);
BytePtr IOStream_GetMemoryBuffer(IOStreamPtr self);
size_t IOStream_GetFileSize(StringPtr filename);	// static member

#ifdef __cplusplus
}
#endif

#endif
