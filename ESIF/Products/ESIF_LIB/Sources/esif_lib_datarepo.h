/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "esif_lib_datavault.h"
#include "esif_lib_iostream.h"

// Data Repository = Named Stream Container for one or more DataVaults
// Repos are associated with Files or Memory buffers and are single-threaded
typedef struct DataRepo_s {
	char					name[MAX_PATH];	// file.dv, file.dvx, buffername
	IOStreamPtr				stream;			// File or Memory Buffer Stream for this Repo
} DataRepo, *DataRepoPtr;

typedef union DataVaultHeader_u DataVaultHeader, *DataVaultHeaderPtr;

// DataRepo object management
DataRepoPtr DataRepo_Create();
DataRepoPtr DataRepo_CreateAs(StreamType type, StoreType store, StringPtr name);
void DataRepo_Destroy(DataRepoPtr self);

esif_error_t DataRepo_SetType(DataRepoPtr self, StreamType type, StoreType store, StringPtr name);
void DataRepo_GetName(DataRepoPtr self, StringPtr name, size_t name_len);

esif_error_t DataRepo_ReadHeader(DataRepoPtr self, DataVaultHeaderPtr header);
esif_error_t DataRepo_LoadSegments(DataRepoPtr self);

