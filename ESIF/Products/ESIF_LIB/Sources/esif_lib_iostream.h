/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "esif_lib.h"
#include "esif_lib_istringlist.h"

#include <stdio.h>
#include <errno.h>

// errno values. See also: errno.h
#define     EOK         0
#define		IOSTREAM_MEMORY_STREAM_BLOCK_SIZE 512

//////////////////////////////////////////////////////////////////////////////
// IOStream Class

union IOStream_s;
typedef union IOStream_s IOStream, *IOStreamPtr, **IOStreamPtrLocation;

typedef enum stream_type {
	StreamNull,
	StreamFile,
	StreamMemory,
} StreamType;

typedef enum store_type {
	StoreReadOnly,	// Stream is Read-Only
	StoreStatic,	// Stream is Read-Only and Static (Permanently Loaded)
	StoreReadWrite,	// Stream is Read/Write
} StoreType;

#ifdef _IOSTREAM_CLASS

union IOStream_s {
	StreamType  type;			// Stream Type (File, Buffer)

	struct {
		StreamType  type;		// Stream Type
		StoreType   store;		// Store Type
	} base;

	struct {
		StreamType  type;		// Stream Type
		StoreType   store;		// Store Type
		StringPtr   name;		// File Name (Full or Relative Pathname)
		FILE        *handle;	// File Handle
		StringPtr   mode;		// File Open Mode ("rb", "wb", "ab")
	} file;

	struct {
		StreamType  type;		// Store Type
		StoreType   store;		// Store Type
		Byte        *buffer;	// Buffer Pointer
		size_t      buf_len;	// Buffer Size if Dynamically Allocated
		size_t      data_len;	// Buffer Data Length
		size_t      offset;		// Current Offset from Buffer Pointer
	} memory;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

// object management
void IOStream_ctor(IOStreamPtr self);	// constructor
void IOStream_dtor(IOStreamPtr self);	// destructor
IOStreamPtr IOStream_Create();			// new operator
void IOStream_Destroy(IOStreamPtr self);// delete operator

// methods
// Open/Close File or Memory Buffer
int IOStream_SetFile(IOStreamPtr self, StoreType store, StringPtr filename, StringPtr mode);
int IOStream_SetMemory(IOStreamPtr self, StoreType store, BytePtr buffer, size_t size);
int IOStream_OpenFile(IOStreamPtr self, StoreType store, StringPtr filename, StringPtr mode);
int IOStream_Open(IOStreamPtr self);	// fopen equivalent
int IOStream_Close(IOStreamPtr self);	// fclose equivalent

// Stream I/O
size_t IOStream_Read(IOStreamPtr self, void *dest_buffer, size_t bytes);		// fread equivalent
size_t IOStream_ReadAt(IOStreamPtr self, void *dest_buffer, size_t bytes, size_t offset);
size_t IOStream_Write(IOStreamPtr self, void *src_buffer, size_t bytes);		// fwrite equivalent

StringPtr IOStream_GetLine(IOStreamPtr self, StringPtr dest_buffer, int bytes);	// fgets equivalent
int IOStream_Seek(IOStreamPtr self, size_t offset, int origin);			// fseek equivalent
size_t IOStream_GetOffset(IOStreamPtr self);									// ftell equivalent [fgetpos?]

// Member Access and Helper Functions
int IOStream_LoadBlock(IOStreamPtr self, void *buf_ptr, size_t buf_len, size_t offset);  // Open and Read a Data Block
StreamType IOStream_GetType(IOStreamPtr self);
StoreType IOStream_GetStore(IOStreamPtr self);
size_t IOStream_GetSize(IOStreamPtr self);
BytePtr IOStream_GetMemoryBuffer(IOStreamPtr self);
size_t IOStream_GetFileSize(StringPtr filename);	// static member

#ifdef __cplusplus
}
#endif

#endif
