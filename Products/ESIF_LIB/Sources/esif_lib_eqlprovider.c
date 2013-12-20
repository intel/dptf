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
#define _EQLCMD_CLASS
#define _EQLPROVIDER_CLASS

#include "esif_pm.h"
#include "esif_participant.h"
#include "esif_uf_primitive.h"
#include "esif_uf_shell.h"
#include "esif_uf_cfgmgr.h"

#include "esif_lib_eqlcmd.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// EQL Providers Interfaces
static eEsifError Provider_DataBank (EqlCmdPtr eqlcmd);
static eEsifError Provider_Primitive (EqlCmdPtr eqlcmd);

// friends
extern int g_dst;

#define MAXACTIONLEN    256

//////////////////////////////////////////////////////////////////////////////
// Provider Registry. Static for now
// Keep all lists sorted for binary search capability
//////////////////////////////////////////////////////////////////////////////

static ProviderAction g_DataBank_Actions[] = {
	{"EXEC",   (ProviderCallback)Provider_DataBank},
	{"GET",    (ProviderCallback)Provider_DataBank},
	{"SET",    (ProviderCallback)Provider_DataBank},
	{"DELETE", (ProviderCallback)Provider_DataBank},
	{       0,                                   0}
};

static ProviderAction g_Primitive_Actions[] = {
	{"EXEC", (ProviderCallback)Provider_Primitive},
	{"GET",  (ProviderCallback)Provider_Primitive},
	{"SET",  (ProviderCallback)Provider_Primitive},
	{     0,                                    0}
};
static Provider g_ProviderList[] = {
	{"DATABANK",  g_DataBank_Actions },
	{"PRIMITIVE", g_Primitive_Actions},
	{          0,                 0}
};
static Adapter g_AdapterList[] = {
	{"CONFIG",    0, 0},
	// {"HTTP",		0,	"http"},
	{"PRIMITIVE", 1, 0},
	{          0, 0}
};

static ProviderRegistry g_Registry = {
	g_AdapterList,  sizeof(g_AdapterList) / sizeof(Adapter) - 1,
	g_ProviderList, sizeof(g_ProviderList) / sizeof(Provider) - 1
};

//////////////////////////////////////////////////////////////////////////////
// Adapter Class

eEsifError Adapter_BindEqlCmd (EqlCmdPtr eqlcmd)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	ProviderRegistryPtr reg = &g_Registry;
	int i = 0, j = 0;

	// Sanity check
	if (!eqlcmd) {
		return rc;
	}

	// TODO: Binary Search
	for (i = 0; reg->adapters[i].name; i++)
		if (esif_ccb_stricmp(reg->adapters[i].name, eqlcmd->adapter) == 0) {
			ProviderActionPtr actions = reg->providers[reg->adapters[i].provider].actions;
			for (j = 0; actions[j].action; j++)
				if (esif_ccb_stricmp(actions[j].action, eqlcmd->action) == 0) {
					// Set Callback Handler
					eqlcmd->handler = actions[j].fnInterface;

					// For now, context = namespace
					if (reg->adapters[i].context && !eqlcmd->subtype) {
						eqlcmd->subtype = String_Clone((StringPtr)reg->adapters[i].context);
					}

					return ESIF_OK;
				}
		}
	return rc;
}


// Set Callback Function for a given Adapter
eEsifError Adapter_SetCallback (
	StringPtr adapter,
	StringPtr action,
	ProviderCallback fnProviderCallback,
	void *context
	)
{
	eEsifError rc = ESIF_E_NOT_FOUND;
	ProviderRegistryPtr reg = &g_Registry;
	int i, j;

	for (i = 0; reg->adapters[i].name; i++)
		if (esif_ccb_stricmp(reg->adapters[i].name, adapter) == 0) {
			ProviderActionPtr actions = reg->providers[reg->adapters[i].provider].actions;
			for (j = 0; actions[j].action; j++)
				if (NULL == action || esif_ccb_stricmp(actions[j].action, action) == 0) {
					actions[j].fnInterface   = fnProviderCallback;
					reg->adapters[i].context = context;
					rc = ESIF_OK;
					break;
				}
			break;
		}
	return rc;
}


//////////////////////////////////////////////////////////////////////////////
// Provider Class

// Null Provider
static int ProviderNull (EqlCmdPtr eqlcmd)
{
	int rc = 0;
	EqlCmd_DebugDump(eqlcmd);
	return rc;
}


//////////////////////////////////////////////////////////////////////////////
// DataBank Provider
//////////////////////////////////////////////////////////////////////////////

static eEsifError Provider_DataBank (EqlCmdPtr eqlcmd)
{
	// List of option names and codes. TODO: Keep this list sorted alphabetically and do a binary search
	static struct OptionList_s {
		StringPtr  name;
		UInt32     option;
	}

	optionList[] = {
		{"PERSIST",  ESIF_SERVICE_CONFIG_PERSIST },
		{"ENCRYPT",  ESIF_SERVICE_CONFIG_ENCRYPT },
		{"READONLY", ESIF_SERVICE_CONFIG_READONLY},
		{"NOCACHE",  ESIF_SERVICE_CONFIG_NOCACHE },
		{"FILELINK", ESIF_SERVICE_CONFIG_FILELINK},
#ifdef ESIF_ATTR_OS_WINDOWS
		{"REGLINK",  ESIF_SERVICE_CONFIG_REGLINK },
#endif
		{"DELETE",   ESIF_SERVICE_CONFIG_DELETE  },
		{"STATIC",   ESIF_SERVICE_CONFIG_STATIC  },	// DataVault-Level Option
		{         0,                            0}
	};
	eEsifError rc     = ESIF_E_UNSPECIFIED;
	char message[OUT_BUF_LEN] = {0};
	EsifDataType type = ESIF_DATA_STRING;
	UInt32 options    = 0;
	StringPtr DefaultNamespace = "esif";
	int i;

	for (i = 0; i < eqlcmd->options->items; i++) {
		char *opt = eqlcmd->options->list[i];
		int j;
		// TODO: Do a binary search
		for (j = 0; optionList[j].name; j++)
			if (esif_ccb_stricmp(opt, optionList[j].name) == 0) {
				options |= optionList[j].option;
				break;
			}
		if (!optionList[j].name) {
			esif_ccb_sprintf(OUT_BUF_LEN, message, "Error: Invalid Option: %s\n", opt);
			StringList_Add(eqlcmd->messages, message);
			return rc;
		}
	}

	// "DELETE Adapter:Namespace(Parameters)" == "SET Adapter:Namespace(Parameters) WITH DELETE"
	if (esif_ccb_stricmp(eqlcmd->action, "DELETE") == 0) {
		esif_ccb_strcpy(eqlcmd->action, "SET", 4);
		options |= ESIF_SERVICE_CONFIG_DELETE;
	}

	// SET
	if (esif_ccb_stricmp(eqlcmd->action, "SET") == 0) {
		EsifDataPtr data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, ISNULL(eqlcmd->subtype, DefaultNamespace), ESIFSTATIC, ESIFAUTOLEN);
		EsifDataPtr data_path   = EsifData_CreateAs(ESIF_DATA_STRING, eqlcmd->parameters->list[0], ESIFSTATIC, ESIFAUTOLEN);
		EsifDataPtr data_value  = EsifData_Create();

		// Use supplied Data Value
		if (eqlcmd->values->items) {
			type = esif_data_type_string2enum(String_Get(eqlcmd->datatypes->list[0]));

			// If this is a link, use the supplied string, otherwise decode the value
			if (options & (ESIF_SERVICE_CONFIG_FILELINK | ESIF_SERVICE_CONFIG_REGLINK)) {
				EsifData_Set(data_value, type, eqlcmd->values->list[0], 0, ESIFAUTOLEN);
			} else if ((rc = EsifData_FromString(data_value, eqlcmd->values->list[0], type)) != ESIF_OK) {
				EsifData_Destroy(data_nspace);
				EsifData_Destroy(data_path);
				EsifData_Destroy(data_value);
				return rc;
			}
		}
		rc = EsifConfigSet(data_nspace, data_path, options, data_value);

		// Hack: Delete & Re-Add if buffer in NameSpace too small
		if (ESIF_E_NEED_LARGER_BUFFER == rc) {
			rc = EsifConfigSet(data_nspace, data_path, options | ESIF_SERVICE_CONFIG_DELETE, data_value);
			if (rc == ESIF_OK) {
				rc = EsifConfigSet(data_nspace, data_path, options, data_value);
			}
		}
		if (rc != ESIF_OK) {
			StringList_Add(eqlcmd->messages, esif_rc_str(rc));
		}

		EsifData_Destroy(data_nspace);
		EsifData_Destroy(data_path);
		EsifData_Destroy(data_value);
	} else {// GET
		EsifDataPtr data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, ISNULL(eqlcmd->subtype, DefaultNamespace), ESIFSTATIC, ESIFAUTOLEN);
		EsifDataPtr data_path   = EsifData_CreateAs(ESIF_DATA_STRING, eqlcmd->parameters->list[0], ESIFSTATIC, ESIFAUTOLEN);
		EsifDataPtr data_value  = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

		rc = EsifConfigGet(data_nspace, data_path, data_value);

		// Return Results
		if (ESIF_OK != rc) {
			StringList_Add(eqlcmd->messages, esif_rc_str(rc));
		} else {
			EsifDataList_Add(eqlcmd->results, data_value);
		}
		EsifData_Destroy(data_nspace);
		EsifData_Destroy(data_path);
		EsifData_Destroy(data_value);
	}
	if (*message) {
		StringList_Add(eqlcmd->messages, message);
	}

	return rc;
}


//////////////////////////////////////////////////////////////////////////////
// Primitive Provider
//////////////////////////////////////////////////////////////////////////////

// Primitive Provider
static eEsifError Provider_Primitive (EqlCmdPtr eqlcmd)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	char message[OUT_BUF_LEN]  = {0};
	u32 id = 0;
	char primitive_name[256]   = {0};
	char qualifier_str[32 + 1] = "D0";
	u8 instance = 255;
	EsifDataPtr request  = EsifData_CreateAs(ESIF_DATA_VOID, NULL, 0, 0);
	EsifDataPtr response = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	int destination = g_dst;

	// Participant Destination Name
	if (eqlcmd->subtype) {
		UInt8 i;
		for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
			EsifUpPtr up_ptr = EsifUpManagerGetAvailableParticipantByInstance(i);
			if (NULL != up_ptr && esif_ccb_stricmp(eqlcmd->subtype, up_ptr->fMetadata.fName) == 0) {
				destination = up_ptr->fInstance;
				break;
			}
		}
	}

	// Primitive ID
	if (eqlcmd->parameters->items > 0) {
		id = esif_atoi(eqlcmd->parameters->list[0]);	// Allow Numbers
		if (!id) {
			// prepend "GET_" or "SET_" to name if not specified
			if (esif_ccb_strnicmp(eqlcmd->parameters->list[0], eqlcmd->action, esif_ccb_strlen(eqlcmd->action, MAXACTIONLEN)) != 0) {
				esif_ccb_sprintf(sizeof(primitive_name), primitive_name, "%s_%s", eqlcmd->action, eqlcmd->parameters->list[0]);
			} else {
				esif_ccb_strcpy(primitive_name, eqlcmd->parameters->list[0], sizeof(primitive_name));
			}
			id = (u32)esif_primitive_type_string2enum(primitive_name);
		}
	}
	if (!id) {
		EsifData_Destroy(request);
		EsifData_Destroy(response);
		return rc;
	}

	// Qualifier
	if (eqlcmd->parameters->items > 1) {
		esif_ccb_strcpy(qualifier_str, eqlcmd->parameters->list[1], sizeof(qualifier_str));
	}
	// Instance
	if (eqlcmd->parameters->items > 2) {
		instance = (u8)esif_atoi(eqlcmd->parameters->list[2]);
	}

	// Value
	if (esif_ccb_stricmp(eqlcmd->action, "SET") == 0 && eqlcmd->values->list[0]) {
		EsifData_FromString(request, eqlcmd->values->list[0], esif_data_type_string2enum(eqlcmd->datatypes->list[0]));
	} else if (esif_ccb_stricmp(eqlcmd->action, "GET") == 0 && eqlcmd->datatypes->items) {
		// TODO: Support for more than INT data types
		EsifData_FromString(response, "0", esif_data_type_string2enum(eqlcmd->datatypes->list[0]));
	}

	// Execute Primitive
	rc = EsifExecutePrimitive(
			(u8)destination,
			id,
			qualifier_str,
			instance,
			request,
			response);

	// Return Results
	if (rc == ESIF_OK) {
		if (request->type == ESIF_DATA_VOID) {	// get
			EsifDataList_Add(eqlcmd->results, response);
		}
	}
	if (*message) {
		StringList_Add(eqlcmd->messages, message);
	}

	EsifData_Destroy(request);
	EsifData_Destroy(response);
	return rc;
}


