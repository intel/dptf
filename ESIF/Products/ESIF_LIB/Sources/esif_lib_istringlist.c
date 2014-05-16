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
#include "esif_lib.h"

#define _ISTRINGLIST_CLASS
#include "esif_lib_istringlist.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// constructor
void StringList_ctor (StringListPtr self)
{
	if (self) {
		WIPEPTR(self);
	}
}


// destructor
void StringList_dtor (StringListPtr self)
{
	if (self) {
		int i;
		for (i = 0; i < self->items; i++)
			String_Destroy(self->list[i]);
		esif_ccb_free(self->list);
		WIPEPTR(self);
	}
}


// new operator
StringListPtr StringList_Create ()
{
	StringListPtr self = (StringListPtr)esif_ccb_malloc(sizeof(*self));
	StringList_ctor(self);
	return self;
}


// delete operator
void StringList_Destroy (StringListPtr self)
{
	StringList_dtor(self);
	esif_ccb_free(self);
}


// methods
int StringList_GetCount (StringListPtr self)
{
	return self->items;
}


char**StringList_GetList (StringListPtr self)
{
	return self->list;
}


void StringList_Add (
	StringListPtr self,
	char *str
	)
{
	ASSERT(str);
	self->list = (char**)esif_ccb_realloc(self->list, (self->items + 2) * sizeof(self->list[0]));
	if (self->list) {
		self->list[self->items++] = String_Clone(str);
		self->list[self->items]   = 0;
	}
}


