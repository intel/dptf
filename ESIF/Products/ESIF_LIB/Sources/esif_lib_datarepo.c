/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

// Friend Classes
#define _IOSTREAM_CLASS

#include "esif_lib_datarepo.h"
#include "esif_lib_iostream.h"

/////////////////////////////////////////////////////////////////////////
// DataRepo Class

// constructor
static esif_error_t DataRepo_ctor(DataRepoPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		WIPEPTR(self);
		rc = ESIF_OK;
		self->stream = IOStream_Create();
		if (self->stream == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
	}
	return rc;
}

// destructor
static void DataRepo_dtor(DataRepoPtr self)
{
	if (self) {
		IOStream_Destroy(self->stream);
		WIPEPTR(self);
	}
}

// new operator
DataRepoPtr DataRepo_Create()
{
	DataRepoPtr self = (DataRepoPtr)esif_ccb_malloc(sizeof(*self));
	if (DataRepo_ctor(self) != ESIF_OK) {
		DataRepo_dtor(self);
		esif_ccb_free(self);
		self = NULL;
	}
	return self;
}

// delete operator
void DataRepo_Destroy(DataRepoPtr self)
{
	DataRepo_dtor(self);
	esif_ccb_free(self);
}

// new operator, specifying stream type and name
DataRepoPtr DataRepo_CreateAs(
	StreamType type,
	StoreType store,
	StringPtr name
)
{
	DataRepoPtr self = DataRepo_Create();
	if (DataRepo_SetType(self, type, store, name) != ESIF_OK) {
		DataRepo_Destroy(self);
		self = NULL;
	}
	return self;
}

// Set Repo Stream and Store Type
esif_error_t DataRepo_SetType(
	DataRepoPtr self,
	StreamType type,
	StoreType store,
	StringPtr name
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self) {
		if (name) {
			esif_ccb_strcpy(self->name, name, sizeof(self->name));
		}
		if (self->stream) {
			IOStream_dtor(self->stream);

			switch (type) {
			case StreamFile:
				if (name) {
					char fullpath[MAX_PATH] = { 0 };

					// Use Full path if specified, otherwise use DV path
					if (esif_ccb_isfullpath(name)) {
						esif_ccb_strcpy(fullpath, name, sizeof(fullpath));
					}
					else {
						StringPtr ext = (esif_ccb_strchr(name, '.') ? NULL : ESIFDV_FILEEXT);
						esif_build_path(fullpath, sizeof(fullpath), ESIF_PATHTYPE_DV, name, ext);
					}

					if (IOStream_SetFile(self->stream, store, fullpath, "rb") != EOK) {
						IOStream_dtor(self->stream);
					}
				}
				break;
			case StreamMemory:
				IOStream_SetMemory(self->stream, store, NULL, 0);
				break;
			default:
				break;
			}
		}

		if (self->stream == NULL || self->stream->type != type) {
			rc = ESIF_E_NO_CREATE;
		}
		else {
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Get the Normalized Repo Name (lowercase without path or file extension)
void DataRepo_GetName(
	DataRepoPtr self,
	StringPtr name,
	size_t name_len
)
{
	if (self && name && name_len > 0) {
		StringPtr str = esif_ccb_strrchr(self->name, *ESIF_PATH_SEP);
		str = (str ? str + 1 : self->name);
		esif_ccb_strcpy(name, str, name_len);
		if ((str = esif_ccb_strchr(name, '.')) != NULL) {
			*str = 0;
		}
	}
}
