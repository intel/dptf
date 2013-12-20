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
#ifndef _ISTRINGLIST_H
#define _ISTRINGLIST_H

///////////////////////////////////////////////////////
// StringPtr Class (always char *)
typedef char*StringPtr;

#define String_Create()         0
#define String_CreateAs(size)   (char*)esif_ccb_malloc(size)
#define String_Clone(str)       esif_ccb_strdup(str)
#define String_Destroy(str)     esif_ccb_free(str);
#define String_Set(str, value)  (str) = esif_ccb_strdup(value)
#define String_Get(str)         (str)

///////////////////////////////////////////////////////
// StringList Class

struct StringList_s;
typedef struct StringList_s StringList, *StringListPtr, **StringListPtrLocation;

#ifdef _ISTRINGLIST_CLASS
struct StringList_s {
	int  items;		// TODO: Rename to size
	char * *list;	// TODO: Rename to elements, use StringArray class?
};

#endif

// object management
void StringList_ctor (StringListPtr self);
void StringList_dtor (StringListPtr self);
StringListPtr StringList_Create ();
void StringList_Destroy (StringListPtr self);

// methods
int StringList_GetCount (StringListPtr self);
char**StringList_GetList (StringListPtr self);
void StringList_Add (StringListPtr self, char *str);

#endif