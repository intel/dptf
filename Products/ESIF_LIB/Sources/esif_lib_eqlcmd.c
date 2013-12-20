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

// friend classes
#define _ISTRINGLIST_CLASS
#define _ESIFDATALIST_CLASS
#define _EQLCMD_CLASS

#include "esif_lib_eqlcmd.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

extern eEsifError Adapter_BindEqlCmd (EqlCmdPtr eqlcmd);

///////////////////////////////////////////////////////
// EqlCmd Class

// constructor
static ESIF_INLINE void EqlCmd_ctor (EqlCmdPtr self)
{
	ASSERT(self);
	self->adapter    = String_Create();
	self->subtype    = String_Create();
	self->action     = String_Create();
	self->messages   = StringList_Create();
	self->parameters = StringList_Create();
	self->datatypes  = StringList_Create();
	self->values     = StringList_Create();
	self->options    = StringList_Create();
	self->results    = EsifDataList_Create();
	self->handler    = 0;
}


// destructor
static ESIF_INLINE void EqlCmd_dtor (EqlCmdPtr self)
{
	ASSERT(self);
	String_Destroy(self->adapter);
	String_Destroy(self->subtype);
	String_Destroy(self->action);
	StringList_Destroy(self->messages);
	StringList_Destroy(self->parameters);
	StringList_Destroy(self->datatypes);
	StringList_Destroy(self->values);
	StringList_Destroy(self->options);
	EsifDataList_Destroy(self->results);
	WIPEPTR(self);
}


// new operator
EqlCmdPtr EqlCmd_Create ()
{
	EqlCmdPtr self = (EqlCmdPtr)esif_ccb_malloc(sizeof(*self));
	EqlCmd_ctor(self);
	return self;
}


// delete operator
void EqlCmd_Destroy (EqlCmdPtr self)
{
	EqlCmd_dtor(self);
	esif_ccb_free(self);
}


// methods
void EqlCmd_Reset (EqlCmdPtr self)
{
	EqlCmd_dtor(self);
	EqlCmd_ctor(self);
}


// Bind Optional Binary Arguments to the Command
eEsifError EqlCmd_BindArguments (
	EqlCmdPtr self,
	EsifDataPtr *arguments
	)
{
	eEsifError rc = ESIF_OK;
	int bin = 0, val = 0;

	// Replace Binary Values with parameters
	for (val = 0; val < self->values->items; val++)
		if (strcmp(self->datatypes->list[val], "%") == 0) {
			if (arguments && arguments[bin]) {
				self->datatypes->list[val] = esif_data_type_str(arguments[bin]->type);
				esif_ccb_free(self->values->list[val]);
				self->values->list[val]    = EsifData_ToString(arguments[bin]);	// temp hack: convert to string
				bin++;
			} else {
				return ESIF_E_PARAMETER_IS_NULL;
			}
		}
	return rc;
}


// Execute Parsed and Bound Command
eEsifError EqlCmd_Execute (EqlCmdPtr self)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	if (self->handler) {
		rc = (*self->handler)(self);
	}
	return rc;
}


// Dispatch and Execute a prepared EqlCmd using optional EsifData argument(s)
eEsifError EqlCmd_Dispatch (
	EqlCmdPtr self,
	EsifDataPtr *arguments
	)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;

	// Bind Adapter Callback Function to this Command
	if ((rc = Adapter_BindEqlCmd(self)) == ESIF_OK) {
		// Bind Optional Arguments
		if (arguments) {
			rc = EqlCmd_BindArguments(self, arguments);
		}

		// Call Handler
		if (self->handler) {
			rc = EqlCmd_Execute(self);
		}
	}
	return rc;
}


// Display Results
void EqlCmd_DisplayResults (EqlCmdPtr self)
{
	if (self) {
		int i;
		for (i = 0; i < self->results->size; i++) {
			EsifDataPtr data = &self->results->elements[i];
			char *strdata    = EsifData_ToString(data);
			if (strdata) {
				CMD_OUT("%s\n", strdata);
				esif_ccb_free(strdata);
			}
		}
		CMD_OUT("\n");
	}
}


// Debug Output
void EqlCmd_DebugDump (EqlCmdPtr self)
{
	if (self) {
		int i;
		CMD_DEBUG(
			"\n"
			"EQLCMD:\n"
			"  adapter       = %s\n"
			"  subtype       = %s\n"
			"  action        = %s\n",
			String_Get(self->adapter),
			String_Get(self->subtype),
			String_Get(self->action));

		for (i = 0; i < self->messages->items; i++)
			CMD_DEBUG(
				"  messages[%d]   = %s\n",
				i, self->messages->list[i]);
		for (i = 0; i < self->parameters->items; i++)
			CMD_DEBUG(
				"  parameters[%d] = %s\n",
				i, self->parameters->list[i]);
		for (i = 0; i < self->values->items; i++)
			CMD_DEBUG(
				"  values[%d]     = %s (%s)\n",
				i, self->datatypes->list[i], self->values->list[i]);
		for (i = 0; i < self->options->items; i++)
			CMD_DEBUG(
				"  options[%d]    = %s\n",
				i, self->options->list[i]);
		for (i = 0; i < self->results->size; i++) {
			EsifDataPtr data = &self->results->elements[i];
			char *strdata    = EsifData_ToString(data);
			if (strdata) {
				CMD_DEBUG(
					"  results[%d]    = %s (buf_len=%d data_len=%d)\n"
					"%s\n",
					i, esif_data_type_str(self->results->elements[i].type),
					self->results->elements[i].buf_len, self->results->elements[i].data_len,
					strdata);
				esif_ccb_free(strdata);
			}
		}
		CMD_DEBUG("\n");
	}
}


