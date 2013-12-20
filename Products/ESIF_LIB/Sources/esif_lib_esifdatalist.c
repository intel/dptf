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

#define _ESIFDATALIST_CLASS
#include "esif_lib_esifdatalist.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

///////////////////////////////////////////////////////
// EsifDataList Class

void EsifDataList_ctor (EsifDataListPtr self)
{
	if (self) {
		WIPEPTR(self);
	}
}


void EsifDataList_dtor (EsifDataListPtr self)
{
	if (self) {
		int i;
		for (i = 0; i < self->size; i++)
			EsifData_dtor(&self->elements[i]);
		esif_ccb_free(self->elements);
		WIPEPTR(self);
	}
}


EsifDataListPtr EsifDataList_Create ()
{
	EsifDataListPtr self = (EsifDataListPtr)esif_ccb_malloc(sizeof(*self));
	EsifDataList_ctor(self);
	return self;
}


void EsifDataList_Destroy (EsifDataListPtr self)
{
	EsifDataList_dtor(self);
	esif_ccb_free(self);
}


int EsifDataList_GetCount (EsifDataListPtr self)
{
	return self->size;
}


EsifDataPtr EsifDataList_GetList (EsifDataListPtr self)
{
	return self->elements;
}


void EsifDataList_Add (
	EsifDataListPtr self,
	EsifDataPtr data
	)
{
	if (data->buf_len == ESIF_DATA_ALLOCATE) {
		return;
	}
	self->elements = (EsifDataPtr)esif_ccb_realloc(self->elements, (self->size + 1) * sizeof(self->elements[0]));
	if (self->elements) {
		self->elements[self->size++] = *data;
		if (data->buf_len != ESIFSTATIC) {
			self->elements[self->size - 1].buf_ptr = esif_ccb_malloc(data->buf_len);
			esif_ccb_memcpy(self->elements[self->size - 1].buf_ptr, data->buf_ptr, data->buf_len);
		}
	}
}


