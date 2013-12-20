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
#define _DATACACHE_CLASS
#include "esif_lib_datacache.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

///////////////////////////////////////////////////////
// DataCache Class

// private members
static int DataCache_GetCount (DataCachePtr self);
static DataCacheEntryPtr DataCache_GetList (DataCachePtr self);
static int DataCache_Search (DataCachePtr self, esif_string key);

// constructor
DataCachePtr DataCache_Create ()
{
	DataCachePtr self = (DataCachePtr)esif_ccb_malloc(sizeof(*self));
	return self;
}


// destructor
void DataCache_Destroy (DataCachePtr self)
{
	UInt32 i;
	for (i = 0; i < self->size; i++) {
		if (self->elements[i].key.buf_len) {
			esif_ccb_free(self->elements[i].key.buf_ptr);
		}
		if (self->elements[i].value.buf_len) {
			esif_ccb_free(self->elements[i].value.buf_ptr);
		}
	}
	esif_ccb_free(self->elements);
	WIPEPTR(self);
	esif_ccb_free(self);
}


DataCacheEntryPtr DataCache_GetValue (
	DataCachePtr self,
	esif_string key
	)
{
	int index = DataCache_Search(self, key);
	if (index == EOF) {
		return NULL;
	} else {
		return &self->elements[index];
	}
}


void DataCache_SetValue (
	DataCachePtr self,
	esif_string key,
	EsifData value,
	esif_flags_t flags
	)
{
	// Do Insersion Sort using Binary Search on Sorted Array
	int items = self->size, start = 0, end = items - 1, node = items / 2;
	while (node < items && start <= end) {
		int comp = esif_ccb_stricmp(key, (esif_string)self->elements[node].key.buf_ptr);
		if (comp == 0) {
			break;
		} else if (comp > 0) {
			start = node + 1;
		} else {
			end = node - 1;
		}
		node = (end - start) / 2 + start;
	}

	if (node == EOF) {
		node = 0;
	}
	self->elements = (DataCacheEntryPtr)esif_ccb_realloc(self->elements, (self->size + 1) * sizeof(self->elements[0]));
	if (self->elements) {
		EsifData keydata;
		EsifData_ctor(&keydata);
		EsifData_Set(&keydata, ESIF_DATA_STRING, esif_ccb_strdup(key), ESIFAUTOLEN, ESIFAUTOLEN);
		if (node < (int)self->size) {
			memmove(&self->elements[node + 1], &self->elements[node], (self->size - node) * sizeof(*self->elements));
		}
		self->elements[node].key   = keydata;
		self->elements[node].value = value;
		self->elements[node].flags = flags;
		self->size++;
	}
}


void DataCache_Delete (
	DataCachePtr self,
	esif_string key
	)
{
	// Do Insersion Sort using Binary Search on Sorted Array
	UInt32 node = DataCache_Search(self, key);
	if (node == EOF) {
		return;
	}
	if (node < self->size - 1) {
		memmove(&self->elements[node], &self->elements[node + 1], (self->size - node - 1) * sizeof(*self->elements));
	}
	self->elements = (DataCacheEntryPtr)esif_ccb_realloc(self->elements, (self->size - 1) * sizeof(self->elements[0]));
	self->size--;
}


// Private Members
static int DataCache_GetCount (DataCachePtr self)
{
	return self->size;
}


static DataCacheEntryPtr DataCache_GetList (DataCachePtr self)
{
	return self->elements;
}


static int DataCache_Search (
	DataCachePtr self,
	esif_string key
	)
{
	// Do Binary Search on Sorted Array
	int items = self->size;
	int start = 0, end = items - 1, node = items / 2;
	while (start <= end) {
		int comp = esif_ccb_stricmp(key, (esif_string)(self->elements[node].key.buf_ptr));
		if (comp == 0) {
			return node;
		} else if (comp > 0) {
			start = node + 1;
		} else {
			end = node - 1;
		}
		node = (end - start) / 2 + start;
	}
	return EOF;
}


