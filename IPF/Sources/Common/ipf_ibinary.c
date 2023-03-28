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

#include "esif_ccb_memory.h"
#include "ipf_ibinary.h"

#define IBINARY_DEFAULT_GROWBY	256		// Default buffer autogrow boundary size
#define IBINARY_WIPE_BUFFER				// #define to zero out unused memory in IBinary buffers

#ifdef IBINARY_WIPE_BUFFER
#  define IBinary_WipeMemory(buf_ptr, buf_len)	esif_ccb_memset(buf_ptr, 0, buf_len)
#else
# define IBinary_WipeMemory(buf_ptr, buf_len)	(void)(0)
#endif

// Constructor
IBinary *IBinary_Create(void)
{
	IBinary *self = esif_ccb_malloc(sizeof(*self));
	if (self) {
		self->growby_len = IBINARY_DEFAULT_GROWBY;
	}
	return self;
}

// Destructor
void IBinary_Destroy(IBinary *self)
{
	if (self) {
		if (self->buf_ptr && self->buf_len) {
			IBinary_WipeMemory(self->buf_ptr, self->buf_len);
		}
		esif_ccb_free(self->buf_ptr);
		esif_ccb_free(self);
	}
}

// Allocate or Reallocate Buffer to exact size. Truncates data if buf_len < data_len
void *IBinary_Realloc(IBinary *self, size_t buf_len)
{
	UInt8 *buf_ptr = NULL;
	if (self) {
		buf_ptr = self->buf_ptr;
		if (buf_len != self->buf_len) {
			if (buf_len == 0) {
				esif_ccb_free(self->buf_ptr);
				self->buf_ptr = NULL;
				self->buf_len = 0;
				self->data_len = 0;
				buf_ptr = NULL;
			}
			else {
				buf_ptr = esif_ccb_realloc(self->buf_ptr, buf_len);
				if (buf_ptr != NULL) {
					if (buf_len > self->buf_len && self->buf_len > 0) {
						IBinary_WipeMemory(buf_ptr + self->buf_len, buf_len - self->buf_len);
					}
					self->buf_ptr = buf_ptr;
					self->buf_len = buf_len;
					if (buf_len < self->data_len) {
						self->data_len = buf_len;
					}
				}
			}
		}
	}
	return buf_ptr;
}

// Copy over a binary buffer, reallocating if necessary to nearest growby boundary
void *IBinary_Copy(IBinary *self, const void *buffer, size_t bytes)
{
	IBinary_Truncate(self, 0);
	return IBinary_Insert(self, buffer, bytes, 0);
}

// Clone a binary buffer, reallocating if necessary to exact size
void *IBinary_Clone(IBinary *self, const void *buffer, size_t bytes)
{
	IBinary_Truncate(self, 0);
	IBinary_Realloc(self, bytes);
	return IBinary_Insert(self, buffer, bytes, 0);
}

// Append a binary buffer, reallocating if necessary to nearest growby boundary
void *IBinary_Append(IBinary *self, const void *buffer, size_t bytes)
{
	return IBinary_Insert(self, buffer, bytes, (self ? self->data_len : 0));
}

// Insert a binary buffer at given offset, reallocating if necessary to nearest growby boundary
void *IBinary_Insert(IBinary *self, const void *buffer, size_t bytes, size_t offset)
{
	UInt8 *buf_ptr = NULL;
	if (self) {
		buf_ptr = self->buf_ptr;
		if (buffer && bytes && offset <= self->data_len) {
			if (self->data_len + bytes > self->buf_len) {
				if (self->growby_len) {
					size_t growby_len = ((((self->data_len + bytes) / self->growby_len) + 1 - !((self->data_len + bytes) % self->growby_len)) * self->growby_len) - self->buf_len;
					buf_ptr = IBinary_Realloc(self, self->buf_len + growby_len);
				}
				else {
					buf_ptr = NULL;
				}
			}
			if (buf_ptr) {
				if (offset < self->data_len) {
					esif_ccb_memmove(buf_ptr + offset + bytes, buf_ptr + offset, self->data_len - offset);
				}
				esif_ccb_memcpy(buf_ptr + offset, buffer, bytes);
				self->data_len += bytes;
			}
		}
	}
	return buf_ptr;
}

// Copy Into (Overwrite) a binary buffer at given offset, reallocating if necessary to nearest growby boundary
void *IBinary_CopyOver(IBinary *self, const void *buffer, size_t bytes, size_t offset)
{
	UInt8 *buf_ptr = NULL;
	if (self) {
		buf_ptr = self->buf_ptr;
		if (buffer && bytes && offset <= self->data_len) {
			if (offset + bytes > self->buf_len) {
				if (self->growby_len) {
					size_t growby_len = ((((offset + bytes) / self->growby_len) + 1 - !((offset + bytes) % self->growby_len)) * self->growby_len) - self->buf_len;
					buf_ptr = IBinary_Realloc(self, self->buf_len + growby_len);
				}
				else {
					buf_ptr = NULL;
				}
			}
			if (buf_ptr) {
				esif_ccb_memcpy(buf_ptr + offset, buffer, bytes);
				if (offset + bytes > self->data_len) {
					self->data_len += (bytes - offset);
				}
			}
		}
	}
	return buf_ptr;
}

// Delete bytes from the buffer without reallocating buffer
void *IBinary_Delete(IBinary *self, size_t offset, size_t bytes)
{
	UInt8 *buf_ptr = NULL;
	if (self) {
		buf_ptr = self->buf_ptr;
		if (buf_ptr && bytes && offset + bytes <= self->data_len) {
			size_t data_len = self->data_len - bytes;
			esif_ccb_memmove(buf_ptr + offset, buf_ptr + offset + bytes, self->data_len - offset - bytes);
			IBinary_WipeMemory(buf_ptr + data_len, bytes);
			self->data_len = data_len;
		}
	}
	return buf_ptr;
}

// Truncate data without reallocating buffer
void *IBinary_Truncate(IBinary *self, size_t data_len)
{
	UInt8 *buf_ptr = NULL;
	if (self) {
		buf_ptr = self->buf_ptr;
		if (buf_ptr && data_len < self->data_len) {
			IBinary_WipeMemory(buf_ptr + data_len, self->buf_len - data_len);
			self->data_len = data_len;
		}
	}
	return buf_ptr;
}

// Truncate data and reallocate buffer to nearest growby boundary
void *IBinary_Shrink(IBinary *self, size_t data_len)
{
	UInt8 *buf_ptr = IBinary_Truncate(self, data_len);
	if (self && (data_len == 0 || self->buf_len - self->data_len > self->growby_len)) {
		size_t buf_len = 0;
		if (data_len && self->growby_len) {
			buf_len = ((self->data_len + self->growby_len - 1) / self->growby_len * self->growby_len);
		}
		buf_ptr = IBinary_Realloc(self, buf_len);
	}
	return buf_ptr;
}

// Resize data and reallocate buffer to nearest growby boundary
void *IBinary_Resize(IBinary *self, size_t data_len)
{
	UInt8 *buf_ptr = IBinary_GetBuf(self);
	if (self) {
		size_t buf_len = 0;
		if (data_len && self->growby_len) {
			buf_len = ((data_len + self->growby_len - 1) / self->growby_len * self->growby_len);
		}
		buf_ptr = IBinary_Realloc(self, buf_len);
	}
	return buf_ptr;
}

// Get Buffer Byte Pointer
void *IBinary_GetBuf(IBinary *self)
{
	UInt8 *buf_ptr = NULL;
	if (self) {
		buf_ptr = self->buf_ptr;
	}
	return buf_ptr;
}

// Get Buffer Allocated Length
size_t IBinary_GetBufLen(IBinary *self)
{
	size_t buf_len = 0;
	if (self) {
		buf_len = self->buf_len;
	}
	return buf_len;
}

// Get Buffer Data Length
size_t IBinary_GetLen(IBinary *self)
{
	size_t data_len = 0;
	if (self) {
		data_len = self->data_len;
	}
	return data_len;
}

// Get Buffer Growby Boundary
size_t IBinary_GetGrowBy(IBinary *self)
{
	size_t growby_len = 0;
	if (self) {
		growby_len = self->growby_len;
	}
	return growby_len;
}

// Set Buffer Growby Boundary
void IBinary_SetGrowBy(IBinary *self, size_t growby_len)
{
	if (self && growby_len) {
		self->growby_len = growby_len;
	}
}
