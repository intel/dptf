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
#include <stdlib.h>
#include <memory.h>
#include <string.h>
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
static ESIF_INLINE void IOStream_ctor(IOStreamPtr self)
{
	if (self) {
		WIPEPTR(self);
		self->type = StreamNull;
	}
}


// destructor
static ESIF_INLINE void IOStream_dtor(IOStreamPtr self)
{
	if (self) {
		IOStream_Close(self);
		switch (self->type) {
		case StreamFile:
			esif_ccb_free(self->file.name);
			esif_ccb_free(self->file.mode);
			break;

		case StreamMemory:
			esif_ccb_free(self->memory.buffer);
			break;
		}
		WIPEPTR(self);
	}
}


// new operator
IOStreamPtr IOStream_Create()
{
	IOStreamPtr self = (IOStreamPtr)esif_ccb_malloc(sizeof(*self));
	IOStream_ctor(self);
	return self;
}


// delete operator
void IOStream_Destroy(IOStreamPtr self)
{
	if (self) {
		IOStream_dtor(self);
		esif_ccb_free(self);
	}
}


// methods

// Set IOStream to use a file (but don't open)
int IOStream_SetFile(
	IOStreamPtr self,
	StringPtr filename,
	StringPtr mode
	)
{
	StringPtr filenameCopyPtr = NULL;
	StringPtr modeCopyPtr = NULL;
	
	if (NULL == self) {
		return EINVAL;
	}

	//
	// Get a copy of the items before destroying the stream, as they may be
	// from the stream we are destroying below...
	//
	filenameCopyPtr = esif_ccb_strdup(filename);
	modeCopyPtr = esif_ccb_strdup(mode);

	IOStream_dtor(self);

	self->type = StreamFile;
	self->file.name = filenameCopyPtr;
	self->file.mode = modeCopyPtr;

	return self->file.name ? EOK : ENOMEM;
}


// Set IOStream to use a Memory Buffer (but don't access)
int IOStream_SetMemory(
	IOStreamPtr self,
	BytePtr buffer,
	size_t size
	)
{
	size_t bufSize = 0;

	if (NULL == self) {
		return EINVAL;
	}

	// Clear everything when setting new contents
	IOStream_dtor(self);

	self->type = StreamMemory;
	bufSize = ((size / IOSTREAM_MEMORY_STREAM_BLOCK_SIZE) + 1) * IOSTREAM_MEMORY_STREAM_BLOCK_SIZE;
	self->memory.buffer = esif_ccb_malloc(bufSize);
	if (NULL == self->memory.buffer) {
		goto exit;
	}
	if (buffer != NULL) {
		esif_ccb_memcpy(self->memory.buffer, buffer, size);
	}
	self->memory.buf_len  = bufSize;
	self->memory.data_len = size;
	self->memory.offset   = 0;
exit:
	return self->memory.buffer ? EOK : ENOMEM;
}


// Set IOStream to use a File and open it
int IOStream_OpenFile(
	IOStreamPtr self,
	StringPtr filename,
	StringPtr mode
	)
{
	int rc = EOK;

	if (IOStream_SetFile(self, filename, mode) == EOK) {
		rc = IOStream_Open(self);
	} else {
		rc = ENOMEM;
	}
	return rc;
}


// Open the IOStream (File or Memory)
int IOStream_Open(IOStreamPtr self)
{
	int rc = EPERM;
	StringPtr mode = "rb";	// Default File Open Mode

	if (NULL == self) {
		rc = EINVAL;
		goto exit;
	}

	IOStream_Close(self);
	switch (self->type) {
	case StreamFile:
		if (self->file.mode) {
			mode = self->file.mode;
		}
		self->file.handle = esif_ccb_fopen(self->file.name, mode, &rc);
		break;

	case StreamMemory:
		if (self->memory.buffer) {
			self->memory.offset = 0;
			rc = EOK;
		}

	default:
		break;
	}
exit:
	return rc;
}


// Close the IOStream. Leave File & Memory Buffer data intact so it can be Re-Opened
int IOStream_Close(IOStreamPtr self)
{
	int rc = EPERM;

	if (NULL == self) {
		rc = EINVAL;
		goto exit;
	}
	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			esif_ccb_fclose(self->file.handle);

			esif_ccb_free(self->file.mode);
			self->file.mode = NULL;

			self->file.handle = 0;
		}
		break;

	case StreamMemory:
		self->memory.offset = 0;
		break;

	default:
		break;
	}
exit:
	return rc;
}


// Create a new Memory Buffer IOStream, copying the contents from the given stream
eEsifError IOStream_CloneAsMemoryStream(
	IOStreamPtr self,
	IOStreamPtr *clonePtr
	)
{
	eEsifError rc = ESIF_OK;
	BytePtr bufPtr = NULL;
	IOStreamPtr newStreamPtr = NULL;
	size_t size = 0;
	size_t bytesRead = 0;

	if ((NULL == self) || (NULL == clonePtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	size = IOStream_GetSize(self);

	//
	// Create the memory buffer for the memory IOStream
	// Note: +1 to allow for null terminator (In original code, not sure why)
	//
	bufPtr = (BytePtr)esif_ccb_malloc(size);
	if (NULL == bufPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	newStreamPtr = (IOStreamPtr)IOStream_Create();
	if (NULL == newStreamPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	if (IOStream_SetMemory(newStreamPtr, bufPtr, size) != 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	if (IOStream_Open(self) != EOK) {
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}
	bytesRead = IOStream_Read(self, newStreamPtr->memory.buffer, size);
	IOStream_Close(self);

	if (bytesRead != size) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	*clonePtr = newStreamPtr;
exit:
	esif_ccb_free(bufPtr);
	if (rc != ESIF_OK) {
		IOStream_Destroy(newStreamPtr);
	}
	return rc;
}


// Read an open IOStream into a given Memory Buffer
size_t IOStream_Read(
	IOStreamPtr self,
	void *dest_buffer,
	size_t bytes
	)
{
	size_t rc = 0;

	if ((NULL == self) || (NULL == dest_buffer)) {
		goto exit;
	}

	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			rc = esif_ccb_fread(dest_buffer, bytes, sizeof(Byte), bytes, self->file.handle);
		}
		break;

	case StreamMemory:
		if (self->memory.buffer) {
			//
			// We allow setting the offset before memory grows during writes, but
			// that means you can try to read from an offset that is not within
			// the data region...
			//
			if (self->memory.offset > self->memory.data_len) {
				break;
			}
			bytes = esif_ccb_min(bytes, self->memory.data_len - self->memory.offset);
			esif_ccb_memcpy(dest_buffer, self->memory.buffer + self->memory.offset, bytes);
			self->memory.offset += bytes;
			rc = bytes;
		}
		break;

	default:
		break;
	}
exit:
	return rc;
}


// Reads from a stream at the specified position/offset into the supplied buffer
size_t IOStream_ReadAt(
	IOStreamPtr self,
	void *dest_buffer,
	size_t bytes,
	size_t offset
	)
{
	size_t bytesRead = 0;

	// Open DB File or Memory Block
	if ((NULL == self) || (NULL == dest_buffer)) {
		goto exit;
	}

	// Seek and Read Buffer
	if (IOStream_Seek(self, (long)offset, SEEK_SET) != 0) {
		goto exit;
	}
	
	bytesRead = IOStream_Read(self, dest_buffer, bytes);
exit:
	return bytesRead;
}


//
// Writes to a stream at the specified position/offset from the supplied buffer
// Note: Applicable to memory streams only
//
size_t IOStream_WriteAt(
	IOStreamPtr self,
	void *src_buffer,
	size_t bytes,
	size_t offset
	)
{
	size_t bytesWritten = 0;

	// Open DB File or Memory Block
	if ((NULL == self) || (NULL == src_buffer)) {
		goto exit;
	}

	ESIF_ASSERT(self->type == StreamMemory);

	if(self->type != StreamMemory) {
		goto exit;
	}

	// Seek and write Buffer
	if (IOStream_Seek(self, (long)offset, SEEK_SET) != 0) {
		goto exit;
	}
	
	bytesWritten = IOStream_Write(self, src_buffer, bytes);
exit:
	return bytesWritten;
}


// Write a Memory Buffer to an open IOStream
size_t IOStream_Write(
	IOStreamPtr self,
	void *src_buffer,
	size_t bytes
	)
{
	size_t rc = 0;
	size_t reqSize = 0;
	size_t newSize = 0;
	BytePtr newBuffer = NULL;

	if ((NULL == self) || (NULL == src_buffer)) {
		goto exit;
	}

	switch (self->type) {
	case StreamFile:
		if (self->file.handle) {
			rc = esif_ccb_fwrite(src_buffer, sizeof(Byte), bytes, self->file.handle);
		}
		break;

	case StreamMemory:
		if (NULL == self->memory.buffer) {
			break;
		}
		//
		// We will grow the memory buffer as needed for writes
		//
		reqSize = self->memory.offset + bytes;
		if (self->memory.buf_len < reqSize) {
			newSize = ((reqSize / IOSTREAM_MEMORY_STREAM_BLOCK_SIZE) + 1) * IOSTREAM_MEMORY_STREAM_BLOCK_SIZE;

			newBuffer = esif_ccb_realloc(self->memory.buffer, newSize);
			if (NULL == newBuffer) {
				break;
			}
			self->memory.buffer = newBuffer;
			self->memory.buf_len = newSize;
		}
		esif_ccb_memcpy(self->memory.buffer + self->memory.offset, src_buffer, bytes);
		self->memory.offset += bytes;
		self->memory.data_len = (self->memory.offset > self->memory.data_len) ? self->memory.offset : self->memory.data_len;
		rc = bytes;
		break;

	default:
		break;
	}
exit:
	return rc;
}


// Read a line from a IOStream
StringPtr IOStream_GetLine(
	IOStreamPtr self,
	StringPtr dest_buffer,
	int bytes
	)
{
	StringPtr result = 0;

	if (NULL == self) {
		goto exit;
	}

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
exit:
	return result;
}


// Seek to the given offset in the open IOStream, based on origin starting point
int IOStream_Seek(
	IOStreamPtr self,
	size_t offset,
	int origin
	)
{
	int rc = EBADF;

	if (NULL == self) {
		rc = EINVAL;
		goto exit;
	}

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

			//
			// As we grow the memory buffer as needed for writes, we allow the
			// offset to be specified outside of the data region
			//
			switch (origin) {
			case SEEK_SET:
				newoffset = offset;
				break;

			case SEEK_CUR:
				newoffset = self->memory.offset + offset;
				break;

			case SEEK_END:
				newoffset = self->memory.data_len - offset;
				break;
			}
			self->memory.offset = newoffset;
			rc = EOK;
		}
		break;

	default:
		break;
	}
exit:
	return rc;
}


// Get the current offset (from start) of the open IOStream
size_t IOStream_GetOffset(IOStreamPtr self)
{
	if (NULL == self) {
		goto exit;
	}

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
exit:
	return 0;
}


// Return the IOStream type
StreamType IOStream_GetType(IOStreamPtr self)
{
	ESIF_ASSERT(self);
	return self->type;
}


// Get the current size of the open IOStream
size_t IOStream_GetSize(IOStreamPtr self)
{
	size_t size = 0;

	if (NULL == self) {
		goto exit;
	}

	switch (self->type) {
	case StreamFile:
		size = IOStream_GetFileSize(self->file.name);
		break;

	case StreamMemory:
		size = self->memory.data_len;
		break;

	default:
		break;
	}
exit:
	return size;
}


// Return the Memory Buffer address of a Memory Buffer IOStream (or NULL if not StreamMemory)
BytePtr IOStream_GetMemoryBuffer(IOStreamPtr self)
{
	if (NULL == self) {
		return NULL;
	}
	switch (self->type) {
	case StreamMemory:
		return self->memory.buffer;
		break;
	default:
		return NULL;
		break;
	}
}


// Helper Function: Get the size of the given File
size_t IOStream_GetFileSize(StringPtr filename)
{
	int rc = EPERM;
	struct stat st;
	if ((rc = esif_ccb_stat(filename, &st)) == 0) {
		return st.st_size;
	}
	return 0;
}


