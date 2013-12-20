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
#ifndef _ESIFDATALIST_H
#define _ESIFDATALIST_H

#include "esif_lib.h"
#include "esif_lib_esifdata.h"

//////////////////////////////////////////////////////////////////////////////
// EsifDataList Class
struct EsifDataList_s;
typedef struct EsifDataList_s EsifDataList, *EsifDataListPtr, **EsifDataListPtrLocation;

#ifdef _ESIFDATALIST_CLASS
struct EsifDataList_s {
	int  size;		// Number of EsifData items in List
	EsifDataPtr elements;	// EsifData array
};

#endif

void EsifDataList_ctor (EsifDataListPtr self);
void EsifDataList_dtor (EsifDataListPtr self);
EsifDataListPtr EsifDataList_Create ();
void EsifDataList_Destroy (EsifDataListPtr self);
int EsifDataList_GetCount (EsifDataListPtr self);
EsifDataPtr EsifDataList_GetList (EsifDataListPtr self);
void EsifDataList_Add (EsifDataListPtr self, EsifDataPtr data);

#endif