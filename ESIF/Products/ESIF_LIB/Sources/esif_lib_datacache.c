/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_DATAVAULT

#define _DATACACHE_CLASS
#include "esif_lib_datacache.h"


///////////////////////////////////////////////////////
// DataCache Class

// private members
static DataCacheEntryPtr DataCache_GetList(DataCachePtr self);
static int DataCache_Search(DataCachePtr self, esif_string key);
static int DataCache_FindInsertionPoint(DataCachePtr self, esif_string key);
static EsifDataPtr CloneCacheData(EsifDataPtr dataPtr);

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

	if (NULL == self) {
		return;
	}
	for (i = 0; i < self->size; i++) {
		EsifData_dtor(&self->elements[i].key);
		EsifData_dtor(&self->elements[i].value);
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
	int index = 0;

	if (NULL == self) {
		return NULL;
	}
	
	index = DataCache_Search(self, key);
	if (index == EOF) {
		return NULL;
	} else {
		return &self->elements[index];
	}
}

//
// Inserts a key value pair based on alphabetical position of the key.
// Warning: The valuePtr parameter represent a "cache data" item, so a 0 buf_len
// member indicates the buf_ptr represents a file offset; not whether the EsifData
// items owns the associated buffer.
// Notes: The pair is inserted even if a key with the same name already exists.
// The cache array will be re-allocated to fit the new member and old members
// will be copied down if needed to allow insertion
//
eEsifError DataCache_InsertValue(
	DataCachePtr self,
	esif_string key,
	EsifDataPtr valuePtr,
	esif_flags_t flags
	)
{
	eEsifError rc = ESIF_OK;
	DataCacheEntryPtr old_elements = NULL;
	EsifData keydata;
	EsifDataPtr valueClonePtr = NULL;
	int node = 0;

	if ((NULL == self) || (NULL == valuePtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	valueClonePtr = CloneCacheData(valuePtr);
	if (NULL == valueClonePtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	node = DataCache_FindInsertionPoint(self, key);

	// Reallocate the elements to fit the new pair
	old_elements = self->elements;
	self->elements = (DataCacheEntryPtr)esif_ccb_realloc(self->elements, (self->size + 1) * sizeof(*self->elements));
	if (NULL == self->elements) {
		self->elements = old_elements;
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	
	// Move old pairs down to fit the new pair
	if (node < (int)self->size) {
		esif_ccb_memmove(&self->elements[node + 1], &self->elements[node], (self->size - node) * sizeof(*self->elements));
	}

	// Insert the new pair
	EsifData_ctor(&keydata);
	EsifData_Set(&keydata, ESIF_DATA_STRING, esif_ccb_strdup(key), ESIFAUTOLEN, ESIFAUTOLEN);
	self->elements[node].key   = keydata;
	self->elements[node].value = *valueClonePtr;
	self->elements[node].flags = flags;
	self->size++;
exit:
	if (rc == ESIF_OK) {
		esif_ccb_free(valueClonePtr); // Free pointer but don't destroy as the element owns buffer now
	} else {
		EsifData_Destroy(valueClonePtr);
	}
	return rc;
}


eEsifError DataCache_DeleteValue(
	DataCachePtr self,
	esif_string key
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 node = 0;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	node = DataCache_Search(self, key);
	if (node == EOF) {
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}
	EsifData_dtor(&self->elements[node].key);
	EsifData_dtor(&self->elements[node].value);

	if (node < self->size - 1) {
		esif_ccb_memmove(&self->elements[node], &self->elements[node + 1], (self->size - node - 1) * sizeof(*self->elements));
		esif_ccb_memset(&self->elements[self->size - 1], 0, sizeof(self->elements[0]));
	}

	if (self->size > 1) {
		DataCacheEntryPtr old_elements = self->elements;
		self->elements = (DataCacheEntryPtr)esif_ccb_realloc(self->elements, (self->size - 1) * sizeof(*self->elements));
		if (NULL == self->elements) {
			self->elements = old_elements;
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
	} else {
		esif_ccb_free(self->elements);
		self->elements = NULL;
	}
	self->size--;
exit:
	return rc;
}


UInt32 DataCache_GetCount (DataCachePtr self)
{
	return (self ? self->size : 0);
}


// Private Members
DataCacheEntryPtr DataCache_GetList (DataCachePtr self)
{
	return (self ? self->elements : NULL);
}


static int DataCache_Search (
	DataCachePtr self,
	esif_string key
	)
{
	// Do Binary Search on Sorted Array
	int items = 0;
	
	ESIF_ASSERT(self != NULL);

	items = self->size;
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


//
// Finds the insertion point for a new key/value pair based on insertion in an
// alphabetized list of keys
//
static int DataCache_FindInsertionPoint (
	DataCachePtr self,
	esif_string key
	)
{
	int items = 0;
	int start = 0;
	int end = 0;
	int node = 0;

	ESIF_ASSERT(self != NULL);

	items = self->size;
	start = 0;
	end = items - 1;
	node = items / 2;

	// Do insersion sort using binary search on sorted array
	while (start <= end) {
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
	return node;
}

// Make a clone of NOCACHE entries only so they can be restored in the event of I/O Failure
DataCachePtr DataCache_CloneOffsets(
	DataCachePtr self
	)
{
	eEsifError rc = ESIF_OK;
	DataCachePtr clonePtr = NULL;
	UInt32 idx = 0;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	clonePtr = DataCache_Create();
	if (NULL == clonePtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	for (idx = 0; idx < self->size; idx++) {
		if (self->elements[idx].flags & ESIF_SERVICE_CONFIG_NOCACHE) {
			rc = DataCache_InsertValue(clonePtr,
				self->elements[idx].key.buf_ptr,
				&self->elements[idx].value,
				self->elements[idx].flags);
			if (rc != ESIF_OK) {
				break;
			}
		}
	}
exit:
	if (rc != ESIF_OK) {
		DataCache_Destroy(clonePtr);
		clonePtr = NULL;
	}
	return clonePtr;
}

// Restore NOCACHE offset pointers from a cloned backup after I/O Failure
eEsifError DataCache_RestoreOffsets(
	DataCachePtr self,
	DataCachePtr clonePtr
)
{
	eEsifError rc = ESIF_OK;
	UInt32 idx = 0;

	if (NULL == self || NULL == clonePtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
	}

	if (rc == ESIF_OK) {
		for (idx = 0; idx < clonePtr->size; idx++) {
			int found = DataCache_Search(self, clonePtr->elements[idx].key.buf_ptr);
			if (found != EOF && self->elements[found].flags & ESIF_SERVICE_CONFIG_NOCACHE) {
				// Replace NOCACHE value with allocated value in cache so GETs always return updated value
				self->elements[found].value.buf_ptr = clonePtr->elements[idx].value.buf_ptr;
				self->elements[found].value.data_len = clonePtr->elements[idx].value.data_len;
				self->elements[found].value.buf_len = clonePtr->elements[idx].value.buf_len;
				esif_ccb_memset(&clonePtr->elements[idx].value, 0, sizeof(clonePtr->elements[idx].value));
			}
		}
	}

	return rc;
}

EsifDataPtr CloneCacheData(
	EsifDataPtr dataPtr
	)
{
	EsifDataPtr clonePtr = NULL;

	if (NULL == dataPtr) {
		goto exit;
	}

	clonePtr = EsifData_Create();
	if (NULL == clonePtr) {
		goto exit;
	}

	*clonePtr = *dataPtr;

	/* For cache data, a 0 buf_len means the buf_ptr is a file offset */
	if ((NULL == dataPtr->buf_ptr) || (0 == dataPtr->buf_len)) {
		goto exit;
	}

	clonePtr->buf_ptr = esif_ccb_malloc(esif_ccb_max(dataPtr->buf_len, dataPtr->data_len));
	if (NULL == clonePtr->buf_ptr) {
		EsifData_Destroy(clonePtr);
		clonePtr = NULL;
		goto exit;
	}

	esif_ccb_memcpy(clonePtr->buf_ptr, dataPtr->buf_ptr, dataPtr->data_len);
exit:
	return clonePtr;
}