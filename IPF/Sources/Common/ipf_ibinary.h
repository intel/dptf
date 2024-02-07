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

#pragma once

#include "esif_ccb.h"

// Intelligent Binary Buffer Object
typedef struct IBinary_s {
	void  *buf_ptr;
	size_t buf_len;
	size_t data_len;
	size_t growby_len;
} IBinary;

// Constructors
IBinary *IBinary_Create(void);
void IBinary_Destroy(IBinary *self);

// Getter/Setter Methods
void   IBinary_SetGrowBy(IBinary *self, size_t growby_len);
size_t IBinary_GetGrowBy(IBinary *self);
void  *IBinary_GetBuf(IBinary *self);
size_t IBinary_GetBufLen(IBinary *self);
size_t IBinary_GetLen(IBinary *self);

// Methods that may autogrow the buffer
void *IBinary_Copy(IBinary *self, const void *buffer, size_t bytes);
void *IBinary_CopyOver(IBinary *self, const void *buffer, size_t bytes, size_t offset);
void *IBinary_Insert(IBinary *self, const void *buffer, size_t bytes, size_t offset);
void *IBinary_Append(IBinary *self, const void *buffer, size_t bytes);
void *IBinary_Delete(IBinary *self, size_t offset, size_t bytes);
void *IBinary_Truncate(IBinary *self, size_t data_len);

// Methods that may autogrow or shrink the buffer
void *IBinary_Clone(IBinary *self, const void *buffer, size_t bytes);
void *IBinary_Realloc(IBinary *self, size_t buf_len);
void *IBinary_Shrink(IBinary *self, size_t data_len);
void *IBinary_Resize(IBinary *self, size_t data_len);
