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
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
# include <sys/stat.h>

#define  _IOSTREAM_CLASS
#include "esif_lib_iostream.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// constructor
static ESIF_INLINE void IOStream_ctor (IOStreamPtr self)
{
	if (self) {
		WIPEPTR(self);
		self->type = StreamNull;
	}
}


// destructor
static ESIF_INLINE void IOStream_dtor (IOStreamPtr self)
{
	if (self) {
		IOStream_Close(self);
		switch (self->type) {
		case StreamFile:
			esif_ccb_free(self->file.name);
			esif_ccb_free(self->file.mode);
			break;

		case StreamMemory:
			if (self->memory.buf_len) {
				esif_ccb_free(self->memory.buffer);
			}
			break;
		}
		WIPEPTR(self);
	}
}


// new operator
IOStreamPtr IOStream_Create ()
{
	IOStreamPtr self = (IOStreamPtr)esif_ccb_malloc(sizeof(*self));
	IOStream_ctor(self);
	return self;
}


// new operator
IOStreamPtr IOStream_CreateAs (StreamType type)
{
	IOStreamPtr self = (IOStreamPtr)esif_ccb_malloc(sizeof(*self));
	if (self) {
		IOStream_ctor(self);
		self->type = type;
	}
	return self;
}


// delete operator
void IOStream_Destroy (IOStreamPtr self)
{
	if (self) {
		IOStream_dtor(self);
		esif_ccb_free(self);
	}
}


// methods

// Set IOStream to use a file (but don't open)
int IOStream_SetFile (
	IOStreamPtr self,
	StringPtr filename,
	StringPtr mode
	)
{
	ASSERT(self);
	IOStream_dtor(self);
	self->type = StreamFile;
	self->file.name = esif_ccb_strdup(filename);
	if (mode) {
		self->file.mode = esif_ccb_strdup(mode);
	}
	return self->file.name ? EOK : ENOMEM;
}


// Set IOStream to use a Memory Buffer (but don't access)
int IOStream_SetMemory (
	IOStreamPtr self,
	BytePtr buffer,
	size_t size
	)
{
	ASSERT(self);
	IOStream_dtor(self);
	self->type = StreamMemory;
	self->memory.buffer   = buffer;
	self->memory.data_len = size;
	self->memory.buf_len  = 0;
	self->memory.offset   = 0;
	return self->memory.buffer ? EOK : ENOMEM;
}


// Set IOStream to use a File and open it
int IOStream_OpenFile (
	IOStreamPtr self,
	StringPtr filename,
	StringPtr mode
	)
{
	if (IOStream_SetFile(self, filename, mode) == EOK) {
		return IOStream_Open(self);
	} else {
		return ENOMEM;
	}
}


// Set IOStream to use a Memory Buffer and open it
int IOStream_OpenMemory (
	IOStreamPtr self,
	BytePtr buffer,
	size_t size
	)
{
	if (IOStream_SetMemory(self, buffer, size) == EOK) {
		return IOStream_Open(self);
	} else {
		return ENOMEM;
	}
}


// Open the IOStream (File or Memory)
int IOStream_Open (IOStreamPtr self)
{
	int rc = EPERM;
	StringPtr mode = "rb";	// Default File Open Mode

	IOStream_Close(self);
	switch (self->type) {
	case StreamFile:
		if (self->file.mode) {
			mode = self->file.mode;
		}
		rc = esif_ccb_fopen(&(self->file.handle), self->file.name, mode);
		break;

	case StreamMemory:
		// TODO: set data_len to 0 if write mode
		if (self->memory.buffer) {
			rc = EOK;
		}

	default:
		break;
	}
	return rc;
}


// Close the IOStream. Leave File & Memory Buffer data intact so it can be Re-Opened
int IOStream_Close (IOStreamPtr self)
{
	int rc = EPERM;
	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			esif_ccb_fclose(self->file.handle);
			self->file.handle = 0;
		}
		break;

	case StreamMemory:
		self->memory.offset = 0;
		break;

	default:
		break;
	}
	return rc;
}


// Create a new Memory Buffer IOStream, copying the contents from the given Memory Buffer
int IOStream_OpenMemoryClone (
	IOStreamPtr self,
	BytePtr source,
	size_t size
	)
{
	ASSERT(self);
	IOStream_Close(self);
	self->memory.buffer = (BytePtr)esif_ccb_malloc(size);
	if (self->memory.buffer != NULL) {
		self->type = StreamMemory;
		esif_ccb_memcpy(self->memory.buffer, source, size);
		self->memory.data_len = size;
		self->memory.buf_len  = size;
		self->memory.offset   = 0;
		return EOK;
	}
	return ENOMEM;
}


// Create a new Memory Buffer IOStream, copying the contents from the given File
int IOStream_OpenMemoryCloneFile (
	IOStreamPtr self,
	StringPtr filename
	)
{
	int rc = EPERM;
	IOStreamPtr source = 0;
	size_t bytes = 0;
	size_t bytesread   = 0;

	ASSERT(self);
	IOStream_Close(self);
	source = (IOStreamPtr)IOStream_Create();
	bytes  = IOStream_GetFileSize(filename) + 1;// Include Hidden Null Terminator
	if (source && bytes > 0 && ((rc = IOStream_OpenFile(source, filename, "rb")) == EOK)) {
		self->memory.buffer = (BytePtr)esif_ccb_malloc(bytes);
		if (self->memory.buffer != NULL) {
			self->type = StreamMemory;
			self->memory.data_len = bytes-1;
			self->memory.buf_len  = bytes-1;
			self->memory.offset   = 0;
			bytesread = IOStream_Read(source, self->memory.buffer, bytes);
			self->memory.buffer[bytesread] = 0;	// Null Terminate
			rc = EOK;
		}
		IOStream_Close(source);
	}
	IOStream_Destroy(source);
	return rc;
}


// Clone a IOStream (Memory or File) into another, with optional offset and length
int IOStream_Clone (
	IOStreamPtr self,
	IOStreamPtr source,
	long offset,
	size_t length
	)
{
	// TODO
	UNREFERENCED_PARAMETER(self);
	UNREFERENCED_PARAMETER(source);
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(length);
	return EPERM;
}


// Read an open IOStream into a given Memory Buffer
size_t IOStream_Read (
	IOStreamPtr self,
	void *dest_buffer,
	size_t bytes
	)
{
	size_t rc = 0;
	ASSERT(self && dest_buffer);
	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			rc = esif_ccb_fread(dest_buffer, bytes, sizeof(Byte), bytes, self->file.handle);
		}
		break;

	case StreamMemory:
		if (self->memory.buffer) {
			bytes = esif_ccb_min(bytes, self->memory.data_len - self->memory.offset);
			esif_ccb_memcpy(dest_buffer, self->memory.buffer + self->memory.offset, bytes);
			self->memory.offset += bytes;
			rc = bytes;
		}
		break;

	default:
		break;
	}
	return rc;
}


// Write a Memory Buffer to an open IOStream
size_t IOStream_Write (
	IOStreamPtr self,
	void *src_buffer,
	size_t bytes
	)
{
	size_t rc = 0;
	ASSERT(self && src_buffer);
	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			rc = esif_ccb_fwrite(src_buffer, sizeof(Byte), bytes, self->file.handle);
		}
		break;

	case StreamMemory:
		// TODO
		break;

	default:
		break;
	}
	return rc;
}


// Read a line from a IOStream
StringPtr IOStream_GetLine (
	IOStreamPtr self,
	StringPtr dest_buffer,
	int bytes
	)
{
	StringPtr result = 0;
	ASSERT(self);
	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			result = esif_ccb_fgets(dest_buffer, bytes, self->file.handle);
		} else {
			errno = EBADF;
		}
		break;

	case StreamMemory:
		if (self->memory.buffer) {
			int count = 0;
			while (count < bytes && self->memory.offset < self->memory.data_len) {
				dest_buffer[count++] = self->memory.buffer[self->memory.offset++];
				if (dest_buffer[count - 1] == '\n') {
					break;
				}
			}
			dest_buffer[count] = 0;
			if (count) {
				result = dest_buffer;
			}
		} else {
			errno = EFAULT;
		}
		break;

	default:
		errno = EBADF;
		break;
	}
	return result;
}


// Seek to the given offset in the open IOStream, based on origin starting point
int IOStream_Seek (
	IOStreamPtr self,
	size_t offset,
	int origin
	)
{
	int rc = EBADF;
	ASSERT(self);
	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			if (offset > 0x7FFFFFFF) {	// fseek() only accepts (long), not (size_t)
				rc = E2BIG;
			} else {
				rc = esif_ccb_fseek(self->file.handle, (long)offset, origin);
			}
		}
		break;

	case StreamMemory:
		if (self->memory.buffer) {
			size_t newoffset = 0;
			switch (origin) {
			case SEEK_SET:
				newoffset = offset;
				break;

			case SEEK_CUR:
				if (self->memory.offset + offset > self->memory.data_len) {
					rc = EFAULT;
				} else {
					newoffset = self->memory.offset + offset;
				}
				break;

			case SEEK_END:
				if ((size_t)offset > self->memory.data_len) {
					rc = EFAULT;
				} else {
					newoffset = self->memory.offset - offset;
				}
				break;
			}
			if (rc != EFAULT) {
				self->memory.offset = newoffset;
				rc = EOK;
			}
		}
		break;

	default:
		break;
	}
	return rc;
}


// Get the current offset (from start) of the open IOStream
size_t IOStream_GetOffset (IOStreamPtr self)
{
	ASSERT(self);
	switch (self->type) {
	case StreamFile:
		return esif_ccb_ftell(self->file.handle);

		break;

	case StreamMemory:
		return self->memory.offset;

		break;

	default:
		break;
	}
	return 0;
}


// Return the IOStream type
StreamType IOStream_GetType (IOStreamPtr self)
{
	ASSERT(self);
	return self->type;
}


// Get the current size of the open IOStream
size_t IOStream_GetSize (IOStreamPtr self)
{
	long offset = 0;
	size_t size = 0;
	switch (self->type) {
	case StreamFile:
		offset = esif_ccb_ftell(self->file.handle);
		esif_ccb_fseek(self->file.handle, 0, SEEK_END);
		size   = ftell(self->file.handle);
		esif_ccb_fseek(self->file.handle, offset, SEEK_SET);
		break;

	case StreamMemory:
		size = self->memory.data_len;
		break;

	default:
		break;
	}
	return size;
}


// Return the Memory Buffer address of a Memory Buffer IOStream (or NULL if not StreamMemory)
BytePtr IOStream_GetMemoryBuffer (IOStreamPtr self)
{
	ASSERT(self);
	switch (self->type) {
	case StreamMemory:
		return self->memory.buffer;

		break;

	default:
		return 0;

		break;
	}
}


// Helper Function: Get the size of the given File
size_t IOStream_GetFileSize (StringPtr filename)
{
	int rc = EPERM;
	struct stat st;
	if ((rc = esif_ccb_stat(filename, &st)) == 0) {
		return st.st_size;
	}
	return 0;
}


