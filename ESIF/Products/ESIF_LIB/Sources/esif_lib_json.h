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

#pragma once

#include "esif_lib_istring.h"
#include "esif_lib_istringlist.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////
// JSON Classes

// JSON Object Key/Value Pair. For now, all values are represented as strings.
typedef struct JsonKeyPair_s {
	esif_data_type_t type;
	StringPtr key;
	StringPtr value;
} JsonKeyPair, *JsonKeyPairPtr;

// JSON Object. For now only flat Key/Value Pairs are supported.
typedef struct JsonObj_s {
	size_t			count;
	JsonKeyPairPtr	items;
} JsonObj, *JsonObjPtr;

JsonObjPtr JsonObj_Create();
void JsonObj_Destroy(JsonObjPtr self);
esif_error_t JsonObj_AddKeyPair(JsonObjPtr self, esif_data_type_t type, StringPtr key, StringPtr value);
IStringPtr JsonObj_ToString(JsonObjPtr self);
esif_error_t JsonObj_FromString(JsonObjPtr self, StringPtr str);
StringPtr JsonObj_GetValue(JsonObjPtr self, StringPtr key);

#ifdef __cplusplus
}
#endif

