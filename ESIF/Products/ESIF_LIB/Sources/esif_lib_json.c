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

#include "esif_ccb_rc.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include "esif_lib_json.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

char *esif_shell_strtok(char *str, char *seps, char **context);
void strip_illegal_chars(char *srcStringPtr, const char *illegalCharsPtr);

///////////////////////////////////////////////////////////////////////////////
// JSON Object for Dynamic Participants
///////////////////////////////////////////////////////////////////////////////

// Constructors and Object Management
static void JsonObj_ctor(JsonObjPtr self)
{
	if (self) {
		WIPEPTR(self);
	}
}

static void JsonObj_dtor(JsonObjPtr self)
{
	if (self) {
		for (size_t j = 0; j < self->count && self->items; j++) {
			esif_ccb_free(self->items[j].key);
			esif_ccb_free(self->items[j].value);
		}
		esif_ccb_free(self->items);
		WIPEPTR(self);
	}
}

JsonObjPtr JsonObj_Create()
{
	JsonObjPtr self = (JsonObjPtr)esif_ccb_malloc(sizeof(*self));
	JsonObj_ctor(self);
	return self;
}

void JsonObj_Destroy(JsonObjPtr self)
{
	JsonObj_dtor(self);
	esif_ccb_free(self);
}

// Add a Key/Value Pair to a JSON object. For now, all values are represented as Strings.
esif_error_t JsonObj_AddKeyPair(
	JsonObjPtr self,
	esif_data_type_t type,
	StringPtr key,
	StringPtr value)
{
	static char *illegalCharsForJSON = "\\\"";
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		JsonKeyPairPtr items = esif_ccb_realloc(self->items, (self->count + 1) * sizeof(*self->items));
		if (items == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			self->items = items;
			esif_ccb_memset(&self->items[self->count++], 0, sizeof(*self->items));
			self->items[self->count - 1].type = type;
			self->items[self->count - 1].key = esif_ccb_strdup(key);
			self->items[self->count - 1].value = (value ? esif_ccb_strdup(value) : value);
			strip_illegal_chars(self->items[self->count - 1].key, illegalCharsForJSON);
			strip_illegal_chars(self->items[self->count - 1].value, illegalCharsForJSON);
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Serialize a JSON object into an IString. Caller is responsible for destroying the returned IString.
IStringPtr JsonObj_ToString(JsonObjPtr self)
{
	IStringPtr result = (self ? IString_Create() : NULL);
	if (result) {
		IString_Strcat(result, "{\n");
		for (size_t j = 0; j < self->count && self->items; j++) {
			StringPtr quote = (self->items[j].type == ESIF_DATA_STRING && self->items[j].value ? "\"" : "");
			IString_SprintfConcat(result,
				"  \"%s\": %s%s%s%s\n",
				self->items[j].key,
				quote,
				(self->items[j].value ? self->items[j].value : "null"),
				quote,
				(j + 1 < self->count ? "," : "")
			);
		}
		IString_Strcat(result, "}\n");
	}
	return result;
}

// Deserialize a String into a JSON Object
esif_error_t JsonObj_FromString(
	JsonObjPtr self,
	StringPtr str)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && str && (str = esif_ccb_strdup(str)) != NULL) {
		rc = ESIF_E_INVALID_REQUEST_TYPE;

		// opening brace
		StringPtr ctx = NULL;
		StringPtr tok = esif_shell_strtok(str, " \t\r\n,", &ctx);
		StringPtr key = NULL;
		StringPtr value = NULL;

		while (tok) {
			// opening brace or colon
			if ((*tok == '{' && tok == str) || (*tok == ':' && key != NULL && value == NULL)) {

			}
			// closing brace
			else if (*tok == '}') {
				rc = ESIF_OK;
				break;
			}
			// key
			else if (key == NULL) {
				key = tok;
			}
			// value
			else if (value == NULL) {
				esif_data_type_t type = ESIF_DATA_STRING;
				if (esif_ccb_strnicmp(tok, "null", 4) == 0) {
					type = ESIF_DATA_VOID;
				}
				else {
					value = tok;
					StringPtr ch = value;
					while (*ch && isdigit(*ch)) {
						ch++;
					}
					if (ch > value && *ch == '\0') {
						type = ESIF_DATA_UINT32;
					}
				}
				rc = JsonObj_AddKeyPair(self, type, key, value);
				key = value = NULL;
			}
			tok = esif_shell_strtok(NULL, " \t\r\n,", &ctx);
		}
		esif_ccb_free(str);
	}
	return rc;
}

// Get the Value of a JSON Object Key
StringPtr JsonObj_GetValue(
	JsonObjPtr self,
	StringPtr key)
{
	StringPtr value = NULL;
	if (self && self->items) {
		for (size_t j = 0; j < self->count; j++) {
			if (esif_ccb_stricmp(self->items[j].key, key) == 0) {
				value = (self->items[j].value && self->items[j].value[0] ? self->items[j].value : NULL);
				break;
			}
		}
	}
	return value;
}
