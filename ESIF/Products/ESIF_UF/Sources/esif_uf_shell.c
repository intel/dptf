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
#define ESIF_TRACE_ID ESIF_TRACEMODULE_SHELL

// Friend classes
#define _DATABANK_CLASS
#define _DATACACHE_CLASS

/* ESIF */
#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_shell.h"	/* Shell / Command Line Interface */
#include "esif_uf_ipc.h"	/* IPC */
#include "esif_temp.h"

/* Managers */
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_uf_appmgr.h"	/* Application Manager */
#include "esif_uf_actmgr.h"	/* Action Manager */
#include "esif_uf_cnjmgr.h"	/* Conjure Manager */
#include "esif_uf_cfgmgr.h"	/* Config Manager */
#include "esif_uf_eventmgr.h"

#include "esif_uf_primitive.h"

#include "esif_command.h"	/* Commands */
#include "esif_ipc.h"		/* IPC Abstraction */
#include "esif_primitive.h"	/* Primitive */
#include "esif_cpc.h"		/* Compact Primitive Catalog */
#include "esif_uf_ccb_thermalapi.h"
#include "esif_uf_loggingmgr.h"

// SDK
#include "esif_sdk_capability_type.h" /* For Capability Id Description*/
#include "esif_sdk_data_misc.h"
#include "esif_sdk_base64.h"
#include "esif_sdk_iface_ws.h"

#include "esif_lib_datacache.h"
#include "esif_lib_databank.h"
#include "esif_lib_datarepo.h"
#include "esif_lib_istring.h"
#include "esif_lib_json.h"

#include "esif_dsp.h"
#include "esif_uf_fpc.h"
#include "esif_uf_ccb_system.h"
#include "esif_uf_app.h"
#include "esif_uf_tableobject.h"
#include "esif_sdk_iface_esif.h"
#include "esif_uf_service.h"

#ifdef ESIF_ATTR_OS_WINDOWS
// Tool
#include "win\esif_uf_tool.h"
#endif
const char *g_esif_shell_version = ESIF_UF_VERSION;
extern char *g_esif_etf_version;
char g_esif_kernel_version[64] = "NA";

extern struct esif_uf_dm g_dm;

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Shell strings limited to current size of output buffer
#define ESIF_SHELL_STRLEN(s)        esif_ccb_strlen(s, OUT_BUF_LEN)

// Alias Dispatcher
static char *esif_shell_exec_dispatch(const char *line, char *output);

#define FILE_READ         "rb"
#define FILE_WRITE        "w"
#define PATH_STR_LEN      MAX_PATH
#define REG_EXPR_STR_LEN  256

// Limits
#define MAX_CPC_SIZE         0x7ffffffe
#define MAX_FPC_SIZE         0x7ffffffe
#define MAX_PRIMITIVES       0x7ffffffe
#define MAX_ALGORITHMS       0x7ffffffe
#define MAX_DOMAINS          ESIF_DOMAIN_MAX
#define MAX_EVENTS           0x7ffffffe
#define MAX_ARGV             32
#define MAX_REPEAT			 0x7ffffffe
#define MAX_REPEAT_DELAY	 0x7ffffffe
#define MAX_START_ID		 0x7ffffffe
#define MAX_SLEEP_MS		 (0x7fffffff / 1000) // ~35min; Required for OS Abstraction
#define MAX_STRINGLEN		 0x7ffffffe
#define MAX_FILENAME		 100
#define MAX_FILE_LINE_LENGTH 260
#define MAX_FILE_DECODED_LEN (((MAX_BASE64_ENCODED_LEN / 4) + 3) * 3) // ~33MB
#define MAX_FILE_ENCODED_LEN ((MAX_FILE_DECODED_LEN * 5) + (((MAX_FILE_DECODED_LEN / 16) + 1) * 2)) // ~169MB
#define MIN_PARAMETERS_FOR_GET_PRIMITIVE 2
#define MIN_PARAMETERS_FOR_SET_PRIMITIVE 5
#define MIN_PARAMETERS_FOR_APP_STATUS 2

/* Participant Creation Defaults */
#define DYNAMIC_PARTICIPANT_VERSION ESIF_PARTICIPANT_VERSION
#define DYNAMIC_PARTICIPANT_FLAGS 0X0

/* Friends */
extern EsifAppMgr g_appMgr;
extern EsifCnjMgr g_cnjMgr;
extern EsifUppMgr g_uppMgr;

esif_handle_t g_dst   = ESIF_HANDLE_PRIMARY_PARTICIPANT;
char *g_dstName = NULL;


// StopWatch
struct timeval g_timer = {0};

enum output_format g_format = FORMAT_TEXT;
int g_shell_enabled = 0;	// user shell enabled?
int g_shell_stopped = 0;    // Used to stop shell processing when exiting ESIF
int g_cmdshell_enabled = 1;	// "!cmd" type shell commands enabled (if shell enabled)?
static UInt8 g_isRest = 0;

//
// NOT Declared In Header Only This Module Should Use These
//
int g_binary_buf_size = 4096;	// Buffer Size
extern int g_errorlevel;	// Exit Errorlevel
extern int g_quit;			// Quit Application?
extern int g_disconnectClient;	// Disconnect shell client
int g_repeat = 1;		// Repeat N Times
int g_repeat_delay = 0;		// Repeat Delay In ms
extern int g_timestamp;		// Timestamp on / off?
char g_os[64];
char g_illegalXmlChars[] = { '<','>','&','\0' };

int g_soe = 1;
static const char *g_shellStartScript = NULL;

static eEsifError esif_shell_get_participant_id(char *participantNameOrId, esif_handle_t *targetParticipantIdPtr);

// Global Shell lock to limit parse_cmd to one thread at a time
static esif_ccb_mutex_t g_shellLock;
static esif_ccb_event_t g_shellStopEvent = { 0 };

// ESIF Global Shell Output Buffer
char *g_outbuf = NULL;						// Dynamically created and can grow
UInt32 g_outbuf_len = OUT_BUF_LEN_DEFAULT;	// Current (or Default) Size of ESIF Shell Output Buffer

size_t g_cmdlen = 0;

struct esif_data_binary_bst_package {
	union esif_data_variant battery_state;
	union esif_data_variant battery_present_rate;
	union esif_data_variant battery_remaining_capacity;
	union esif_data_variant battery_present_voltage;
};

// For use with ltrim()
#define PREFIX_ALGORITHM_TYPE	20	// ESIF_ALGORITHM_TYPE_
#define PREFIX_ACTION_TYPE		12	// ESIF_ACTION_
#define PREFIX_DATA_TYPE		10	// ESIF_DATA_
#define PREFIX_EVENT_GROUP		17	// ESIF_EVENT_GROUP_
#define PREFIX_EVENT_TYPE		11	// ESIF_EVENT_
#define PREFIX_PARTICIPANT_ENUM 22	// ESIF_PARTICIPANT_ENUM_

// String Type for displaying DataVault Header/Item Flags as char mask ("SC--NRXP")
typedef struct {
	char mask[8 + 1];	// "--------"
} config_flags_str_t;

// Convert DataVault Header or Item Flags to a readable mask string ("SC--NRXP")
static config_flags_str_t config_flags_str(esif_flags_t flags)
{
	// Define bits and relative position in mask string right-to-left
	struct { esif_flags_t bit; char code; } bitmasks[] = {
		{ ESIF_SERVICE_CONFIG_PERSIST,		'P' },
		{ ESIF_SERVICE_CONFIG_SCRAMBLE,		'X' },
		{ ESIF_SERVICE_CONFIG_READONLY,		'R' },
		{ ESIF_SERVICE_CONFIG_NOCACHE,		'N' },
		{ 0,								'-' },
		{ 0,								'-' },
		{ ESIF_SERVICE_CONFIG_COMPRESSED,	'C' },
		{ ESIF_SERVICE_CONFIG_STATIC,		'S' },
		{ 0, 0 }
	};
	config_flags_str_t flagstr = { 0 };
	int j = 0;

	// Build string char mask if flag is set
	esif_ccb_memset(&flagstr, '-', sizeof(flagstr) - 1);
	for (j = 0; j < sizeof(flagstr) - 1 && bitmasks[j].code; j++) {
		if (FLAGS_TEST(flags, bitmasks[j].bit)) {
			flagstr.mask[sizeof(flagstr.mask) - 2 - j] = bitmasks[j].code;
		}
	}
	return flagstr;
}

static int count_cmd_args(const char *command);

// for shell commands with xml output
void strip_illegal_chars(char *srcStringPtr, const char *illegalCharsPtr);

// Return str with the first prefix characters removed if str length > prefix
static char *ltrim(char *str, size_t prefix)
{
	return ((esif_ccb_strlen(str, prefix + 2) > prefix) ? (str + prefix) : (str));
}

// low-level vsprintf to an auto-growing buffer, starting at offset
static int esif_shell_vsprintfto(
	size_t *buf_len,
	char **buffer,
	size_t offset,
	const char *format,
	va_list args
	)
{
	int result = 0;
	int len = esif_ccb_vscprintf(format, args);
	size_t buf_needed = offset + len + 1;

	if (buf_needed > *buf_len) {
		char *new_buffer = esif_ccb_realloc(*buffer, buf_needed);
		if (new_buffer != NULL) {
			*buffer = new_buffer;
			*buf_len = buf_needed;
		}
	}
	if (*buffer) {
		result = esif_ccb_vsprintf(*buf_len - offset, *buffer + offset, format, args);
	}
	return result;
}

// sprintf to an auto-growing string
static int esif_shell_sprintf(
	size_t *buf_len,
	char **buffer,
	const char *format,
	...
	)
{
	int result;
	va_list args;
	va_start(args, format);
	result = esif_shell_vsprintfto(buf_len, buffer, 0, format, args);
	va_end(args);
	return result;
}

// sprintf_concat to an auto-growing string
static int esif_shell_sprintf_concat(
	size_t *buf_len,
	char **buffer,
	const char *format,
	...
	)
{
	int result;
	va_list args;
	va_start(args, format);
	result = esif_shell_vsprintfto(buf_len, buffer, esif_ccb_strlen(*buffer, *buf_len), format, args);
	va_end(args);
	return result;
}

// Use our own strtok() function, which understands "quoted strings with spaces"
#define ESIF_SHELL_STRTOK_SEP   " \t\r\n"
#undef  esif_ccb_strtok
#define esif_ccb_strtok(str, sep, ctxt) esif_shell_strtok(str, sep, ctxt)

char *esif_shell_strtok(
	char *str,
	char *seps,
	char **context
	)
{
	char *result  = 0;
	char quote[2] = {0};
	if (!str) {
		str = *context;
	}
	if (str) {
		while (*str && strchr(seps, *str)) {
			str++;
		}

		if (*str) {
			// Allow "Quoted Strings" or 'Quoted Strings' with embedded spaces or other separators
			if (*str == '\"' || *str == '\'') {
				quote[0] = *str;
				seps     = quote;
				str++;
			}
			result = str;
			str    = strpbrk(str, seps);
			if (str) {
				*str++   = 0;
				*context = str;
			} else {
				*context = result + ESIF_SHELL_STRLEN(result);
			}
		}
	}
	return result;
}


// Shell ATOI
unsigned int esif_atoi(const esif_string str)
{
	unsigned int val = 0;
	if (NULL == str) {
		return 0;
	}

	if (!strncmp(str, "0x", 2)) {
		esif_ccb_sscanf(str + 2, "%x", &val);
	} else {
		esif_ccb_sscanf(str, "%d", (int *)&val);
	}

	return val;
}


UInt64 esif_atoi64(const esif_string str)
{
	UInt64 val = 0;
	if (NULL == str) {
		return 0;
	}

	if (!strncmp(str, "0x", 2)) {
		esif_ccb_sscanf(str + 2, "%llx", &val);
	} else {
		esif_ccb_sscanf(str, "%lld", (Int64 *) &val);
	}

	return val;
}


/* Need Header */
char *esif_str_replace(
	char *orig,
	char *rep,
	char *with
	)
{
	char *result;	// the return string
	char *ins;		// the next insert point
	char *tmp;		// varies
	size_t tmp_len   = 0;
	size_t len_rep   = 0;	// length of rep
	size_t len_with  = 0;	// length of with
	size_t len_front = 0;	// distance between rep and end of last rep
	int count;		// number of replacements

	if (NULL == orig) {
		return NULL;
	}
	len_rep = (rep ? ESIF_SHELL_STRLEN(rep) : 0);

	if (NULL == rep || 0 == len_rep) {
		return NULL;
	}
	ins = strstr(orig, rep);

	if (NULL == ins) {
		return NULL;
	}

	if (NULL == with) {
		with = (char *)"";
	}
	len_with = ESIF_SHELL_STRLEN(with);

	/* Count the number of replacement instances */
	for (count = 0; NULL != (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	if (0 == count) {
		return NULL;
	}

	/*
	** first time through the loop, all the variable are set correctly
	** from here on,
	**    tmp points to the end of the result string
	**    ins points to the next occurrence of rep in orig
	**    orig points to the remainder of orig after "end of rep"
	*/
	tmp_len = ESIF_SHELL_STRLEN(orig) + (len_with - len_rep) * count + 1;

	tmp     = result = (char *)esif_ccb_malloc(tmp_len);

	if (NULL == result) {
		return NULL;
	}

	while (count--) {
		ins = strstr(orig, rep);/* Find the replacement in origin */

		len_front = ins - orig;	/* Calculate bytes before replacement */
		esif_ccb_strcpy(tmp, orig, len_front+1); /* Copy entire string */
		tmp += len_front;					/* Move To Repalement Loc */

		/* Copy the replacement @ the replace locaiton */
		esif_ccb_strcpy(tmp, with, len_with+1);
		tmp  += len_with;

		orig += len_front + len_rep;// move to next "end of rep"
	}

	/*
	** Append what we have left of orig to the end.
	*/
	esif_ccb_strcat(result, orig, tmp_len);
	return result;
}

// Filespecs that contain Relative Paths like .. are prohibited
static Bool shell_isprohibited_filespec(const StringPtr filespec)
{
	return (filespec == NULL || esif_ccb_strstr(filespec, "..") != NULL);
}

// Filenames that contain Relative Paths and wildcard characters are prohibited
static Bool shell_isprohibited(const StringPtr name)
{
	return (shell_isprohibited_filespec(name) || esif_ccb_strpbrk(name, "*?[]") != NULL);
}

// Is the given string a number?
static Bool shell_isnumber(StringPtr str)
{
	Bool rc = ESIF_FALSE;
	Bool isHex = ESIF_FALSE;

	if (str) {
		size_t length = 0;
		if (*str == '-') {
			str++;
		}
		else if (esif_ccb_strnicmp(str, "0x", 2) == 0) {
			str += 2;
			isHex = ESIF_TRUE;
		}
		while (isdigit(*str) || (isHex && isxdigit(*str))) {
			str++;
			length++;
		}

		if (*str == 0 && length > 0) {
			rc = ESIF_TRUE;
		}
	}
	return rc;
}

// Is the given string a valid name or string?
static Bool shell_isstring(StringPtr str, Bool isName, size_t buf_len)
{
	Bool rc = ESIF_FALSE;
	if (str) {
		size_t length = 0;
		while (length < buf_len && isascii(*str) && ((isName && isalnum(*str)) || (!isName && isprint(*str)))) {
			str++;
			length++;
		}
		if (*str == 0 && length > 0 && length < buf_len) {
			rc = ESIF_TRUE;
		}
	}
	return rc;
}

// Transate ESIF_PARTICIPANT_ENUM_TYPENAME to TYPENAME with special translation
static esif_string esif_participant_enum_shortname(esif_participant_enum_t enumerator)
{
	esif_string name = ltrim(esif_participant_enum_str(enumerator), PREFIX_PARTICIPANT_ENUM);
	if (esif_ccb_stricmp(name, "CONJURE") == 0) {
		name = "CNJR";
	}
	return name;
}

// Create a Dynamic Participant from a JSON string
esif_error_t CreateParticipantFromJson(esif_string jsonStr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	JsonObjPtr obj = JsonObj_Create();
	if (obj) {
		if (JsonObj_FromString(obj, jsonStr) == ESIF_OK) {
			StringPtr type = JsonObj_GetValue(obj, "type");
			StringPtr revision = JsonObj_GetValue(obj, "revision");
			StringPtr name = JsonObj_GetValue(obj, "name");
			StringPtr enumerator = JsonObj_GetValue(obj, "enum");
			StringPtr description = JsonObj_GetValue(obj, "description");

			// Do not create if a participant already exists with the same name
			if (EsifUpPm_DoesAvailableParticipantExistByName(name)) {
				rc = ESIF_E_IO_ALREADY_EXISTS;
			}
			// Translate the Dynamic Participant JSON object to an addpart or addpartk shell command
			else if (type && revision && (esif_ccb_stricmp(type, DYNAMIC_PARTICIPANTS_OBJTYPE) == 0)) {
				switch (esif_atoi(revision)) {
				case 1: // Revision 1: implied enumerator=CONJURE only
					enumerator = ltrim(esif_participant_enum_str(ESIF_PARTICIPANT_ENUM_CONJURE), PREFIX_PARTICIPANT_ENUM);;
					rc = ESIF_OK;
					break;
				case 2: // Revision 2: explicit enumerator
					if (enumerator != NULL) {
						rc = ESIF_OK;
					}
					break;
				default:
					rc = ESIF_E_NOT_SUPPORTED;
					break;
				}

				// Create and Execute shell command to create Dynamic Participant
				IStringPtr cmd = IString_Create();
				if (cmd == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				if (rc == ESIF_OK) {
					esif_participant_enum_t enum_type = esif_participant_enum_str2enum(enumerator);
					switch (enum_type) {

					// Dynamic Kernel APCI Participant
					case ESIF_PARTICIPANT_ENUM_ACPI:
					{
						StringPtr hid = JsonObj_GetValue(obj, "hid");
						StringPtr ptype = JsonObj_GetValue(obj, "ptype");
						if (name && description && hid && ptype) {
							IString_Sprintf(cmd,
								"addpartk ACPI %s \"%s\" %s %s",
								name,
								description,
								hid,
								ptype
							);
						}
						break;
					}

					// Dynamic Kernel PCI Participant
					case ESIF_PARTICIPANT_ENUM_PCI:
					{
						StringPtr vendorid = JsonObj_GetValue(obj, "vendorid");
						StringPtr deviceid = JsonObj_GetValue(obj, "deviceid");
						if (name && description && vendorid && deviceid) {
							IString_Sprintf(cmd,
								"addpartk PCI %s \"%s\" %s %s",
								name,
								description,
								vendorid,
								deviceid
							);
						}
						break;
					}

					// Dynamic Upper Framework Conjure Participant
					case ESIF_PARTICIPANT_ENUM_CONJURE:
					{
						StringPtr hid = JsonObj_GetValue(obj, "hid");
						StringPtr ptype = JsonObj_GetValue(obj, "ptype");
						StringPtr flags = JsonObj_GetValue(obj, "flags");
						if (name && description && hid && ptype) {
							IString_Sprintf(cmd,
								"addpart %s \"%s\" \"%s\" %s",
								name,
								description,
								hid,
								ptype
							);
							if (flags) {
								IString_SprintfConcat(cmd, " %s", flags);
							}
						}
						break;
					}

					default:
						rc = ESIF_E_NOT_SUPPORTED;
						break;
					}

					// Create Dynamic Participant
					if (IString_DataLen(cmd) > 0) {
						char *output = esif_shell_exec_command(IString_GetString(cmd), IString_BufLen(cmd) + 1, ESIF_FALSE, ESIF_TRUE);
						if (output && esif_ccb_strstr(output, "ESIF_E_") != NULL) {
							rc = (EsifUpPm_ParticipantCount() >= MAX_PARTICIPANT_ENTRY ? ESIF_E_MAXIMUM_CAPACITY_REACHED : ESIF_E_NO_CREATE);
						}
						else if (!EsifUpPm_DoesAvailableParticipantExistByName(name)) {
							// Conjured Participants are created synchronously, so if it doesn't exist by now, it failed.
							if (enum_type == ESIF_PARTICIPANT_ENUM_CONJURE) {
								rc = (EsifUpPm_ParticipantCount() >= MAX_PARTICIPANT_ENTRY ? ESIF_E_MAXIMUM_CAPACITY_REACHED : ESIF_E_NO_CREATE);
							}
							// Kernel Participants are created in the kernel synchronously, but are sent to the UF asynchronously,
							// so if it doesn't exist by now, it may not have arrived yet, so return ESIF_OK unless PM is full
							// and the UI can decide whether to check again later or just assume that it worked.
							else {
								rc = (EsifUpPm_ParticipantCount() >= MAX_PARTICIPANT_ENTRY ? ESIF_E_MAXIMUM_CAPACITY_REACHED : ESIF_I_AGAIN);
							}
						}
					}
					else if (rc == ESIF_OK) {
						rc = ESIF_E_PARAMETER_IS_NULL;
					}
				}
				IString_Destroy(cmd);
			}
		}
		JsonObj_Destroy(obj);
	}
	return rc;
}

// Create a JSON String from Participant Data. Caller is responsible for destroying returned IStringPtr
IStringPtr CreateJsonFromParticipantData(
	esif_participant_enum_t enumerator,
	StringPtr name,
	StringPtr description,
	StringPtr param1,
	StringPtr param2,
	StringPtr param3)
{
	StringPtr enumerator_name = ltrim(esif_participant_enum_str(enumerator), PREFIX_PARTICIPANT_ENUM);
	IStringPtr jsonStr = NULL;
	JsonObjPtr obj = JsonObj_Create();
	if (obj) {
		JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "type", DYNAMIC_PARTICIPANTS_OBJTYPE);
		JsonObj_AddKeyPair(obj, ESIF_DATA_UINT32, "revision", DYNAMIC_PARTICIPANTS_REVISION);
		JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "name", name);
		JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "enum", enumerator_name);
		JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "description", description);

		switch (enumerator) {
		case ESIF_PARTICIPANT_ENUM_ACPI:
			JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "hid", param1);
			JsonObj_AddKeyPair(obj, ESIF_DATA_UINT32, "ptype", param2);
			break;
		case ESIF_PARTICIPANT_ENUM_PCI:
			JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "vendorid", param1);
			JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "deviceid", param2);
			break;
		case ESIF_PARTICIPANT_ENUM_CONJURE:
			JsonObj_AddKeyPair(obj, ESIF_DATA_STRING, "hid", param1);
			JsonObj_AddKeyPair(obj, ESIF_DATA_UINT32, "ptype", param2);
			if (param3 && enumerator == ESIF_PARTICIPANT_ENUM_CONJURE) {
				JsonObj_AddKeyPair(obj, ESIF_DATA_UINT32, "flags", param3);
			}
			break;
		default:
			enumerator_name = NULL;
			break;
		}
		if (enumerator_name) {
			jsonStr = JsonObj_ToString(obj);
		}
		JsonObj_Destroy(obj);
	}
	return jsonStr;
}

// Create all Persisted Dynamic Participants (that have not yet been created)
esif_error_t CreateDynamicParticipants()
{
	eEsifError rc = ESIF_OK;
	StringPtr dvname = DYNAMIC_PARTICIPANTS_DATAVAULT;
	StringPtr keyspec = DYNAMIC_PARTICIPANTS_KEYSPEC;
	EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, dvname, 0, ESIFAUTOLEN);
	EsifDataPtr key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
	EsifDataPtr value = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	EsifConfigFindContext context = NULL;

	// Find all matching JSON strings in Participants DataVault and create them
	if (nameSpace && key && value && key->buf_ptr && (rc = EsifConfigFindFirst(nameSpace, key, value, &context)) == ESIF_OK) {
		do {
			StringPtr jsonStr = value->buf_ptr;
			if (value->type == ESIF_DATA_JSON && jsonStr && *jsonStr) {
				CreateParticipantFromJson(jsonStr);
			}
			EsifData_Set(key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		} while ((rc = EsifConfigFindNext(nameSpace, key, value, &context)) == ESIF_OK);

		EsifConfigFindClose(&context);
		if (rc == ESIF_E_ITERATION_DONE) {
			rc = ESIF_OK;
		}
	}
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return rc;
}

// Destroy all Persisted Dynamic Participants (that have been created)
esif_error_t DestroyDynamicParticipants()
{
	eEsifError rc = ESIF_OK;
	StringPtr dvname = DYNAMIC_PARTICIPANTS_DATAVAULT;
	StringPtr keyspec = DYNAMIC_PARTICIPANTS_KEYSPEC;
	EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, dvname, 0, ESIFAUTOLEN);
	EsifDataPtr key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
	EsifDataPtr value = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	EsifConfigFindContext context = NULL;

	// Find all matching JSON strings in Participants DataVault and Destroy them
	if (nameSpace && key && value && key->buf_ptr && (rc = EsifConfigFindFirst(nameSpace, key, value, &context)) == ESIF_OK) {
		do {
			StringPtr jsonStr = value->buf_ptr;
			if (value->type == ESIF_DATA_JSON && jsonStr && *jsonStr) {
				// Synchronously Destroy Participant so name can be reused
				JsonObjPtr obj = JsonObj_Create();
				StringPtr name = NULL;
				if (obj && (JsonObj_FromString(obj, jsonStr) == ESIF_OK) && ((name = JsonObj_GetValue(obj, "name")) != NULL)) {
					Bool isConjured = ESIF_FALSE;
					char partname[ESIF_NAME_LEN] = { 0 };
					esif_ccb_strcpy(partname, name, sizeof(partname));
					esif_ccb_strupr(partname, sizeof(partname));

					// Only Destroy Conjured Dynamic Participants
					EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByName(partname);
					if (EsifUp_GetEnumerator(upPtr) == ESIF_PARTICIPANT_ENUM_CONJURE) {
						isConjured = ESIF_TRUE;
					}
					EsifUp_PutRef(upPtr);

					if (isConjured && EsifUpPm_DestroyParticipant(partname) == ESIF_OK) {
						CMD_OUT("Participant %s destroyed.\n", partname);

						// Reset Default Participant to IETM if we just destroyed it
						if (g_dstName && esif_ccb_stricmp(partname, g_dstName) == 0) {
							g_dst = ESIF_HANDLE_PRIMARY_PARTICIPANT;
							esif_ccb_free(g_dstName);
							g_dstName = esif_ccb_strdup(ESIF_PARTICIPANT_DPTF_NAME);
						}
					}
				}
				JsonObj_Destroy(obj);
			}
			EsifData_Set(key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		} while ((rc = EsifConfigFindNext(nameSpace, key, value, &context)) == ESIF_OK);

		EsifConfigFindClose(&context);
		if (rc == ESIF_E_ITERATION_DONE) {
			rc = ESIF_OK;
		}
	}
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return rc;
}

///////////////////////////////////////////////////////////////////////////////
// INITIALIZE
///////////////////////////////////////////////////////////////////////////////

// Init Shell
eEsifError esif_uf_shell_init()
{
	esif_ccb_mutex_init(&g_shellLock);

	esif_ccb_event_init(&g_shellStopEvent);
	esif_ccb_event_set(&g_shellStopEvent);

	g_dstName = esif_ccb_strdup(ESIF_PARTICIPANT_DPTF_NAME);

	if ((g_outbuf = esif_ccb_malloc(OUT_BUF_LEN)) == NULL) {
		return ESIF_E_NO_MEMORY;
	}

	return ESIF_OK;
}

// Uninit Shell
void esif_uf_shell_exit()
{
	esif_uf_shell_stop(); // Stop in case not already stopped

	esif_ccb_free(g_dstName);

	esif_ccb_event_uninit(&g_shellStopEvent);
	esif_ccb_mutex_uninit(&g_shellLock);
	esif_ccb_free(g_outbuf);
	g_outbuf = NULL;
}

void esif_uf_shell_stop()
{
	g_shell_stopped = ESIF_TRUE;
	esif_ccb_event_wait(&g_shellStopEvent);
}

// Exclusively Lock Shell
void esif_uf_shell_lock()
{
	esif_ccb_mutex_lock(&g_shellLock);
}

// Unlock Lock Shell
void esif_uf_shell_unlock()
{
	esif_ccb_mutex_unlock(&g_shellLock);
}

// Resize ESIF Shell Buffer if necessary
char *esif_shell_resize(size_t buf_len)
{
	if (buf_len > OUT_BUF_LEN) {
		char *buf_ptr = esif_ccb_realloc(g_outbuf, buf_len);
		if (buf_ptr != NULL) {
			g_outbuf = buf_ptr;
			g_outbuf_len = (UInt32)buf_len;
		}
	}
	return g_outbuf;
}

void esif_shell_set_start_script(const char *script)
{
	g_shellStartScript = script;
}
const char *esif_shell_get_start_script(void)
{
	return g_shellStartScript;
}
eEsifError esif_uf_shell_banner_init(void)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	CMD_OUT("Start ESIF Upper Framework Shell\n");

	// Build OS Display String
	esif_ccb_sprintf(sizeof(g_os), g_os, "%s %s %s", ESIF_ATTR_OS, ESIF_PLATFORM_TYPE, ESIF_BUILD_TYPE);
	g_os[0] = (char)toupper(g_os[0]);

	// Display Banner if Shell Enabledappex. Initial Prompt will follow
	if (g_shell_enabled) {
		CMD_OUT("\n\nEEEEEEEEEE   SSSSSSSSSS   IIIIIIIII   FFFFFFFFFF\n"
					"EEE          SSS             III      FFF\n"
					"EEE          SSS             III      FFF\n"
					"EEEEEEEEEE   SSSSSSSSSS      III      FFFFFFFFFF\n"
					"EEE                 SSS      III      FFF\n"
					"EEE                 SSS      III      FFF     OS:      %s\n"
					"EEEEEEEEEE   SSSSSSSSSS   IIIIIIIII   FFF     Version: %s\n\n",
				g_os, g_esif_shell_version);
	}
	else {
		CMD_OUT("ESIF Shell Disabled\n");
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);

	return rc;
}


///////////////////////////////////////////////////////////////////////////////
// PARSED COMMANDS
///////////////////////////////////////////////////////////////////////////////


static char *esif_shell_cmd_dspquery(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	EsifDspQuery query = { 0 };
	EsifString dspName = NULL;
	char participantName[MAX_ACPI_DEVICE_STRING_LENGTH] = { 0 };
	char vendorId[MAX_VENDOR_ID_STRING_LENGTH] = { 0 };
	char deviceId[MAX_DEVICE_ID_STRING_LENGTH] = { 0 };
	char participantEnum[ENUM_TO_STRING_LEN] = { 0 };
	char participantPType[ENUM_TO_STRING_LEN] = { 0 };
	char hid[MAX_ACPI_DEVICE_STRING_LENGTH] = { 0 };
	char uid[MAX_ACPI_UID_STRING_LENGTH] = { 0 };
	int opt = 1;

	if (argc > opt) {
		esif_ccb_strcpy(participantName, argv[opt++], sizeof(participantName));
	}

	if (argc > opt) {
		esif_ccb_strcpy(vendorId, argv[opt++], sizeof(vendorId));
	}

	if (argc > opt) {
		esif_ccb_strcpy(deviceId, argv[opt++], sizeof(deviceId));
	}

	if (argc > opt) {
		esif_ccb_strcpy(participantEnum, argv[opt++], sizeof(participantEnum));
	}

	if (argc > opt) {
		esif_ccb_strcpy(participantPType, argv[opt++], sizeof(participantPType));
	}

	if (argc > opt) {
		esif_ccb_strcpy(hid, argv[opt++], sizeof(hid));
	}

	if (argc > opt) {
		esif_ccb_strcpy(uid, argv[opt++], sizeof(uid));
	}

	query.vendorId = vendorId;
	query.deviceId = deviceId;
	query.enumerator = participantEnum;
	query.hid = hid;
	query.uid = uid;
	query.ptype = participantPType;
	query.participantName = participantName;

	dspName = EsifDspMgr_SelectDsp(query);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", (dspName ? dspName : ESIF_NOT_AVAILABLE));
	return output;
}


static char *esif_shell_cmd_dsps(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	u8 i = 0;
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (g_format == FORMAT_XML) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

		for (i = 0; i < g_dm.dme_count; i++) {
			struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
			char version[8];

			if (NULL == dsp_ptr) {
				continue;
			}

			esif_ccb_sprintf(8, version, "%u.%u",
							 *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <dsp>\n");
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "    <id>%u</id>\n"
							 "    <enum>%u</enum>\n"
							 "    <package>%s</package>\n"
							 "    <name>%s</name>\n"
							 "    <guid>%s</guid>\n"
							 "    <version>%s</version>\n"
							 "    <acpiHID>%s</acpiHID>\n"
							 "    <acpiUID>%s</acpiUID>\n"
							 "    <acpiType>%s</acpiType>\n"
							 "    <acpiScope>%s</acpiScope>\n"
							 "    <pciVendor>%s</pciVendor>\n"
							 "    <pciDevice>%s</pciDevice>\n"
							 "    <pciBus>%u</pciBus>\n"
							 "    <pciBusDevice>%u</pciBusDevice>\n"
							 "    <pciFunction>%u</pciFunction>\n"
							 "    <pciRevision></pciRevision>\n"
							 "    <pciClass></pciClass>\n"
							 "    <pciSubClass></pciSubClass>\n"
							 "    <pciProgIf></pciProgIf>\n",
							 i,
							 *dsp_ptr->bus_enum,
							 dsp_ptr->code_ptr,
							 dsp_ptr->code_ptr + 3,
							 dsp_ptr->type,
							 version,
							 dsp_ptr->acpi_device,
							 "",
							 dsp_ptr->acpi_type,
							 dsp_ptr->acpi_scope,
							 dsp_ptr->vendor_id,
							 dsp_ptr->device_id,
							 *dsp_ptr->pci_bus,
							 *dsp_ptr->pci_bus_device,
							 *dsp_ptr->pci_function);

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  </dsp>\n");
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");

		return output;
	}


	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nLoaded Device Support Packages (DSP):\n"
					 "Count:  %u\n\n",
					 g_dm.dme_count);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "ACPI Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: = (HID(8) & TYPE(4) & UID(2) & SCOPE(1))\n"
					 "Minterms: 4\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION HID      TYPE UID SCOPE\n"
					 "-- ------------ ------- -------- ---- --- ------------------------------\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 0) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-8s %-4s %-3s %-30s\n", 
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->acpi_device,
						 dsp_ptr->acpi_type,
						 "",
						 dsp_ptr->acpi_scope);
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "\nPCI Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (VENDOR(128)& DEVICE(64)& BS(32)& DV(16)& FN(8)& RV(4)& SC(2)& PI(1))\n"
					 "Minterms: 8\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION VENDOR DEVICE BS DV FN RV SC PI\n"
					 "-- ------------ ------- ------ ------ -- -- -- -- -- --\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 1) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-6s %-6s %2s %2s %2s %2s %2s %2s\n",
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->vendor_id,
						 dsp_ptr->device_id,
						 "",
						 "",
						 "",
						 "",
						 "",
						 "");
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "\nConjure Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (GUID(1))\n"
					 "Minterms: 1\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION GUID\n"
					 "-- ------------ ------- ------------------------------------\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr	/* || *dsp_ptr->bus_enum != 3 */) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-36s\n", 
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->type);
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					 "\nPlatform Enumerated DSP Candidates:\n\n"
					 "Weighted Eq: (GUID(1))\n"
					 "Minterms: 1\n"
					 "\n"
					 "ID DSP PACKAGE  VERSION GUID\n"
					 "-- ------------ ------- ------------------------------------\n");

	for (i = 0; i < g_dm.dme_count; i++) {
		struct esif_up_dsp *dsp_ptr = g_dm.dme[i].dsp_ptr;
		char version[8];

		if (NULL == dsp_ptr || *dsp_ptr->bus_enum != 2) {
			continue;
		}

		esif_ccb_sprintf(8, version, "%u.%u", *dsp_ptr->ver_major_ptr, *dsp_ptr->ver_minor_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-7s %-36s\n", 
						 i,
						 dsp_ptr->code_ptr,
						 version,
						 dsp_ptr->type);
	}


	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	return output;
}


static char *esif_shell_cmd_conjures(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	u8 i = 0;
	
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output,
					 "\nLoaded Conjures:\n\n"
					 "ID Name         Description                         Type   Version     \n"
					 "-- ------------ ----------------------------------- ------ ------------\n"

					 );
	/* Enumerate Conjures */
	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		EsifCnjPtr a_conjure_ptr    = &g_cnjMgr.fEnrtries[i];

		if (NULL == a_conjure_ptr->fLibNamePtr) {
			continue;
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, 
					"%02u %-12s %-35s %-6s %d\n", i,
						 a_conjure_ptr->fLibNamePtr,
						 (esif_string)a_conjure_ptr->fInterface.desc,
						 "plugin",
						 a_conjure_ptr->fInterface.cnjVersion);
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	return output;
}

// Kernel Actions (Static/dynamic)
static char *esif_shell_cmd_actionsk(EsifShellCmdPtr shell)
{
	struct esif_ipc_command *command_ptr = NULL;
	u32 data_len = sizeof(struct esif_command_get_actions);
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	struct esif_command_get_actions *data_ptr = NULL;
	struct esif_action_info *act_ptr = NULL;
	u32 count = 0;
	char *output = shell->outbuf;
	u32 i = 0;

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"esif_ipc_alloc_command failed for %u bytes\n",
			data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_ACTIONS;
	command_ptr->req_data_type = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len = 0;
	command_ptr->rsp_data_type = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len = data_len;

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"IPC error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}


	/*
	* If the buffer is too small, reallocate based on the number of actions that are present and retry
	*/
	if (ESIF_E_NEED_LARGER_BUFFER == command_ptr->return_code) {
		data_ptr = (struct esif_command_get_actions *)(command_ptr + 1);
		count = data_ptr->available_count;
		if (0 == count) {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"Invalid action count (0) returned for ESIF_E_NEED_LARGER_BUFFER\n");
			goto exit;
		}
		data_len = sizeof(struct esif_command_get_actions) + ((count - 1) * sizeof(*data_ptr->action_info));

		esif_ipc_free(ipc_ptr);

		ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

		if (NULL == ipc_ptr || NULL == command_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN,
				output,
				"esif_ipc_alloc_command failed for %u bytes\n",
				data_len);
			goto exit;
		}

		command_ptr->type = ESIF_COMMAND_TYPE_GET_ACTIONS;
		command_ptr->req_data_type = ESIF_DATA_VOID;
		command_ptr->req_data_offset = 0;
		command_ptr->req_data_len = 0;
		command_ptr->rsp_data_type = ESIF_DATA_STRUCTURE;
		command_ptr->rsp_data_offset = 0;
		command_ptr->rsp_data_len = data_len;

		ipc_execute(ipc_ptr);
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"ipc error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"Command error code = %s(%d)\n",
			esif_rc_str(command_ptr->return_code),
			command_ptr->return_code);
		goto exit;
	}

	// out data
	data_ptr = (struct esif_command_get_actions *)(command_ptr + 1);
	count = data_ptr->returned_count;

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(
			OUT_BUF_LEN,
			output,
			"\nKERNEL ACTIONS:\n\n"
			"ID Name                                Type   \n"
			"-- ----------------------------------- -------\n");
	}
	else {// XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<actionsk>\n");
	}

	for (i = 0; i < count; i++) {
		act_ptr = &data_ptr->action_info[i];
		char *type_ptr = "dynamic";

		if (ESIF_FALSE == act_ptr->dynamic_action){
			type_ptr = "static";
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN,
				output,
				"%-2d %-35s %s\n",
				act_ptr->action_type,
				esif_action_type_str(act_ptr->action_type),
				type_ptr);
		}
		else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"<action>\n"
				"  <id>%d</id>\n"
				"  <name>%s</name>\n"
				"  <type>%s</type>\n"
				"</action>\n",
				act_ptr->action_type,
				esif_action_type_str(act_ptr->action_type),
				type_ptr);
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	}
	else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</actionsk>\n");
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


static char *esif_shell_cmd_actionsu(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	eEsifError iterRc = ESIF_OK;
	ActMgrIterator actIter = {0};
	EsifActPtr curActPtr = NULL;

	iterRc = EsifActMgr_InitIterator(&actIter);
	if (iterRc == ESIF_OK) {

		if (g_format == FORMAT_XML) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

			iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
			while (ESIF_OK == iterRc) {
				if (curActPtr != NULL) {
					char *type_ptr = "dynamic";

					if (!EsifAct_IsPlugin(curActPtr)) {
						type_ptr = "static";
					}

					esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
									 "<action>\n"
									 "    <id>%u</id>\n"
									 "    <name>%s</name>\n"
									 "    <type>%s</type>\n"
									 "  </action>\n",
									 EsifAct_GetType(curActPtr),
									 EsifAct_GetName(curActPtr),
									 type_ptr);
				}
				iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
			}
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");
			goto exit;
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nUSER ACTIONS:\n\n"
						 "ID Name         Description                         Type    Version\n"
						 "-- ------------ ----------------------------------- ------- -------\n"
						 );

		iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
		while (ESIF_OK == iterRc) {
			if (curActPtr != NULL) {
				char *type_ptr = "dynamic";

				if (!EsifAct_IsPlugin(curActPtr)) {
					type_ptr = "static";
				}
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-35s %-7s %7u\n",
								 EsifAct_GetType(curActPtr),
								 EsifAct_GetName(curActPtr),
								 EsifAct_GetDesc(curActPtr),
								 type_ptr,
								 EsifAct_GetVersion(curActPtr));
			}
			iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
		}
	}
exit:
	EsifAct_PutRef(curActPtr);
	return output;
}


static char *esif_shell_cmd_actions(EsifShellCmdPtr shell)
{
	char *actionskString = NULL;
	char *output = shell->outbuf;

	// Get and save the actionsk output
	esif_shell_exec_dispatch("actionsk", output);
	actionskString = esif_ccb_strdup(output);
		
	output = esif_shell_exec_dispatch("actionsu", output);
	if (g_format == FORMAT_XML) {
		goto exit;
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, actionskString);

exit:
	esif_ccb_free(actionskString);
	return output;
}


static char *esif_shell_cmd_upes(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	eEsifError iterRc = ESIF_OK;
	ActMgrIterator actIter = {0};
	EsifActPtr curActPtr = NULL;

	iterRc = EsifActMgr_InitIterator(&actIter);
	if (iterRc == ESIF_OK) {

		if (g_format == FORMAT_XML) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "<result>\n");

			iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
			while (ESIF_OK == iterRc) {
				if ((curActPtr != NULL) && EsifAct_IsPlugin(curActPtr)) {

					esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
									 "<action>\n"
									 "    <id>%u</id>\n"
									 "    <action_type_str>%s</action_type_str>\n"
									 "    <name>%s</name>\n"
									 "    <desc>%s</desc>\n"
									 "    <version>%u</version>\n"
									 "</action>\n",
									 EsifAct_GetType(curActPtr),
									 esif_action_type_str(EsifAct_GetType(curActPtr)),
									 EsifAct_GetName(curActPtr),
									 EsifAct_GetDesc(curActPtr),
									 EsifAct_GetVersion(curActPtr));
				}
				iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
			}
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");
			goto exit;
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nUSER PARTICIPANT EXTENSIONS:\n\n"
						 "ID Name         Description                         Version\n"
						 "-- ------------ ----------------------------------- -------\n"
						 );

		iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
		while (ESIF_OK == iterRc) {
				if ((curActPtr != NULL) && EsifAct_IsPlugin(curActPtr)) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-12s %-35s %7u\n",
								 EsifAct_GetType(curActPtr),
								 EsifAct_GetName(curActPtr),
								 EsifAct_GetDesc(curActPtr),
								 EsifAct_GetVersion(curActPtr));
			}
			iterRc = EsifActMgr_GetNextAction(&actIter, &curActPtr);
		}
	}
exit:
	EsifAct_PutRef(curActPtr);
	return output;
}


static char *esif_shell_cmd_apps_append_events(
	esif_context_t key,
	esif_handle_t participantID,
	UInt16 domainId,
	char *outputPtr,
	size_t bufferSize
	)
{
	int i = 0;
	char *eventNamePtr = NULL;

	ESIF_ASSERT(outputPtr != NULL);

	for (i = 0; i <= MAX_ESIF_EVENT_ENUM_VALUE; i ++) {
		eventNamePtr = esif_event_type_str(i);
		if (EsifEventMgr_IsEventRegistered(i, key, participantID, domainId)) {
			esif_ccb_sprintf_concat(bufferSize, outputPtr, "        %s\n", eventNamePtr);
		}
	}
	return outputPtr;
}

static char *esif_shell_cmd_app(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	int optarg = 1;

	// app <subcmd> ...
	if (argc > optarg) {
		esif_string subcmd = argv[optarg++];

		// app cmd <appname> <app-argv> [...]
		if (argc > (optarg + 1) && esif_ccb_stricmp(subcmd, "cmd") == 0) {
			esif_string appname = argv[optarg++];
			EsifAppPtr appPtr = EsifAppMgr_GetAppFromName(appname);

			if (appPtr) {
				UInt32 argcEsifData = (UInt32)(argc - optarg);
				EsifDataArray argvEsifData = esif_ccb_malloc(argcEsifData * sizeof(*argvEsifData));
				struct esif_data response = { 0 };
				UInt32 j = 0;

				if (argvEsifData == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					response.type = ESIF_DATA_STRING;
					response.buf_len = OUT_BUF_LEN_DEFAULT;
					response.buf_ptr = esif_ccb_malloc(response.buf_len);
					response.data_len = 0;

					for (j = 0; j < argcEsifData; j++) {
						argvEsifData[j].type = ESIF_DATA_STRING;
						argvEsifData[j].buf_ptr = argv[optarg + j];
						argvEsifData[j].buf_len = (u32)ESIF_SHELL_STRLEN(argv[optarg + j]) + 1;
						argvEsifData[j].data_len = argvEsifData[j].buf_len;
					}

					// Send argc/argv array to app
					esif_uf_shell_unlock();
					rc = EsifApp_SendCommand(appPtr, argcEsifData, argvEsifData, &response);
					esif_uf_shell_lock();

					// Resize response buffer and call again if insufficent size
					if (rc == ESIF_E_NEED_LARGER_BUFFER) {
						char *new_ptr = esif_ccb_realloc(response.buf_ptr, response.data_len);
						if (new_ptr) {
							response.buf_ptr = new_ptr;
							response.buf_len = response.data_len;
							esif_uf_shell_unlock();
							rc = EsifApp_SendCommand(appPtr, argcEsifData, argvEsifData, &response);
							esif_uf_shell_lock();
						}
					}
					if (response.data_len > g_outbuf_len) {
						esif_shell_resize(response.data_len);
					}

					if (response.buf_ptr && g_outbuf_len >= response.data_len) {
						esif_ccb_strcpy(g_outbuf, response.buf_ptr, g_outbuf_len);
					}
					else {
						rc = ESIF_E_NEED_LARGER_BUFFER;
					}
					esif_ccb_free(response.buf_ptr);
				}
				esif_ccb_free(argvEsifData);
			}
			else {
				rc = ESIF_E_NOT_FOUND;
			}
			EsifAppMgr_PutRef(appPtr);
		}
		// app suspend <appname>
		else if (argc > optarg && esif_ccb_stricmp(subcmd, "suspend") == 0) {
			esif_string appname = argv[optarg++];
			EsifAppPtr appPtr = EsifAppMgr_GetAppFromName(appname);
			rc = ESIF_E_NOT_FOUND;
			if (appPtr) {
				rc = EsifApp_SuspendApp(appPtr);
				if (rc == ESIF_OK) {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s Suspended\n", appPtr->fAppNamePtr);
				}
			}
			EsifAppMgr_PutRef(appPtr);
		}
		// app resume <appname>
		else if (argc > optarg && esif_ccb_stricmp(subcmd, "resume") == 0) {
			esif_string appname = argv[optarg++];
			EsifAppPtr appPtr = EsifAppMgr_GetAppFromName(appname);
			rc = ESIF_E_NOT_FOUND;
			if (appPtr) {
				rc = EsifApp_ResumeApp(appPtr);
				if (rc == ESIF_OK) {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s Resumed\n", appPtr->fAppNamePtr);
				}
			}
			EsifAppMgr_PutRef(appPtr);
		}
		// app status <appname>
		else if (argc > optarg && esif_ccb_stricmp(subcmd, "status") == 0) {
			esif_string appname = argv[optarg++];
			EsifAppPtr appPtr = EsifAppMgr_GetAppFromName(appname);
			char appStatusBuf[1024] = { 0 };
			EsifData appStatusOut = { ESIF_DATA_STRING, appStatusBuf, (u32)sizeof(appStatusBuf), 0 };
			rc = ESIF_E_NOT_FOUND;
			if (appPtr) {
				rc = EsifApp_GetStatus(
					appPtr,
					eAppStatusCommandGetGroups,
					0,
					&appStatusOut
				);
				if (rc == ESIF_OK) {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", appStatusBuf);
				}
			}
			EsifAppMgr_PutRef(appPtr);
		}
		// app rename <appname> <newname>
		else if (argc > optarg + 1 && esif_ccb_stricmp(subcmd, "rename") == 0) {
			esif_string appname = argv[optarg++];
			esif_string newname = argv[optarg++];
			rc = EsifAppMgr_AppRename(appname, newname);
			if (rc == ESIF_OK) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: Renamed App '%s' to '%s'\n", esif_rc_str(rc), appname, newname);
			}
		}
	}
	if (rc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s (%d)\n", esif_rc_str(rc), rc);
	}
	return output;
}


static char *esif_shell_cmd_apps(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	int arg = 1;
	u8 i = 0;

	eEsifError appIterRc = ESIF_OK;
	AppMgrIterator appIter = { 0 };
	EsifAppPtr appPtr = NULL;

	eEsifError appPartIterRc = ESIF_OK;
	EsifAppPartDataIterator appPartIter = { 0 };
	AppParticipantDataMapPtr appPartPtr = NULL;

	eEsifError appDomIterRc = ESIF_OK;
	EsifAppDomainDataIterator appDomIter = { 0 };
	AppDomainDataMapPtr appDomPtr = NULL;

	Bool showEvents = ESIF_FALSE;
	if (argc > arg && (esif_ccb_stricmp(argv[arg], "verbose") == 0 || esif_ccb_strnicmp(argv[arg], "events", 5) == 0)) {
		showEvents = ESIF_TRUE;
		arg++;
	}
	Bool showIntro = ESIF_FALSE;
	if (argc > arg && (esif_ccb_stricmp(argv[arg], "about") == 0 || esif_ccb_stricmp(argv[arg], "intro") == 0)) {
		showIntro = ESIF_TRUE;
		arg++;
	}
	char delimiter = 0;
	if (argc > arg && (esif_ccb_stricmp(argv[arg], "delimited") == 0 || esif_ccb_stricmp(argv[arg], "tsv") == 0 || esif_ccb_stricmp(argv[arg], "bsv") == 0 || esif_ccb_stricmp(argv[arg], "csv") == 0)) {
		if (esif_ccb_stricmp(argv[arg], "tsv") == 0) {
			delimiter = '\t';
		}
		else if (esif_ccb_stricmp(argv[arg], "csv") == 0) {
			delimiter = ',';
		}
		else {
			delimiter = '|';
		}
		arg++;
	}

	if (delimiter) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"Name%cDescription%cVersion%cType\n",
			delimiter, delimiter, delimiter
		);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"\nRUNNING APPLICATIONS:\n\n"
			"ID Name                   Library      Description                         Type   Version     \n"
			"-- ---------------------- ------------ ----------------------------------- ------ ------------\n");
	}

	/* Enumerate Applications */
	appIterRc = AppMgr_InitIterator(&appIter);
	if (ESIF_OK == appIterRc) {
		for (appIterRc = AppMgr_GetNextApp(&appIter, &appPtr); ESIF_OK == appIterRc; appIterRc = AppMgr_GetNextApp(&appIter, &appPtr)) {
			u8 j = 0;
			esif_handle_t appHandle = ESIF_INVALID_HANDLE;
			char desc[ESIF_DESC_LEN] = { 0 };
			char version[ESIF_DESC_LEN] = { 0 };

			i++;
			ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);
			ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_DESC_LEN);

			if (appPtr == NULL) {
				continue;
			}

			appHandle = EsifApp_GetAppHandle(appPtr);
			EsifApp_GetDescription(appPtr, &data_desc);
			EsifApp_GetVersion(appPtr, &data_version);

			if (delimiter) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s%c%s%c%s%c%s\n",
					EsifApp_GetAppName(appPtr), delimiter,
					(esif_string)data_desc.buf_ptr, delimiter,
					(esif_string)data_version.buf_ptr, delimiter,
					(appPtr->isRestartable ? "plugin" : "client"));
			}
			else {
				if (showEvents) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
				}

				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02u %-22s %-12s %-35s %-6s %-13s\n",
					i,
					EsifApp_GetAppName(appPtr),
					EsifApp_GetLibName(appPtr),
					(esif_string)data_desc.buf_ptr,
					(appPtr->isRestartable ? "plugin" : "client"),
					(esif_string)data_version.buf_ptr);

				if (showIntro) {
					u32 introBufLen = 1024;
					esif_string introBuffer = esif_ccb_malloc(introBufLen);
					if (introBuffer) {
						EsifData introData = { ESIF_DATA_STRING, introBuffer, introBufLen, 0 };
						esif_error_t introRc = EsifApp_GetIntro(appPtr, &introData);
						if (introRc == ESIF_E_NEED_LARGER_BUFFER) {
							esif_string newBuffer = esif_ccb_realloc(introBuffer, introData.data_len);
							if (newBuffer) {
								introBuffer = newBuffer;
								introData.buf_ptr = newBuffer;
								introData.buf_len = introData.data_len;
								introRc = ESIF_OK;
							}
						}
						if (introRc == ESIF_OK) {
							esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n%s%s\n", (esif_string)introData.buf_ptr, (esif_ccb_strchr((esif_string)introData.buf_ptr, '\n') ? "" : "\n"));
						}
						esif_ccb_free(introBuffer);
					}
				}
				if (showEvents) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  System Registered Events:\n");
					esif_shell_cmd_apps_append_events(appHandle, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, output, OUT_BUF_LEN);

					appPartIterRc = EsifApp_InitPartIterator(&appPartIter);
					if (ESIF_OK == appPartIterRc) {
						appPartIterRc = EsifApp_GetNextPart(&appPartIter, appPtr, &appPartPtr);

						/* Now dump participant data here */
						while (ESIF_OK == appPartIterRc) {
							u8 k = 0;

							ESIF_ASSERT(appPartPtr != NULL);

							if (EsifApp_GetPartHandle(appPartPtr) != ESIF_INVALID_HANDLE) {

								esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  [" ESIF_HANDLE_FMT "] %s: Handle(" ESIF_HANDLE_FMT ")\n",
									esif_ccb_handle2llu(EsifUp_GetInstance(EsifApp_GetParticipant(appPartPtr))),
									EsifUp_GetName(EsifApp_GetParticipant(appPartPtr)),
									esif_ccb_handle2llu(EsifApp_GetPartHandle(appPartPtr)));

								appDomIterRc = EsifApp_InitDomainIterator(&appDomIter);
								if (ESIF_OK == appDomIterRc) {
									appDomIterRc = EsifApp_GetNextDomain(&appDomIter, appPartPtr, &appDomPtr);

									while (ESIF_OK == appDomIterRc) {

										ESIF_ASSERT(appDomPtr != NULL);

										if (EsifApp_GetDomainHandle(appDomPtr) != ESIF_INVALID_HANDLE) {
											esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "    D%d: Handle(" ESIF_HANDLE_FMT ")\n", k, esif_ccb_handle2llu(EsifApp_GetDomainHandle(appDomPtr)));

											esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "      Registered Events:\n");
											esif_shell_cmd_apps_append_events(appHandle, EsifApp_GetPartHandle(appPartPtr), domain_str_to_short(EsifApp_GetDomainQualifier(appDomPtr)), output, OUT_BUF_LEN);
										}
										appDomIterRc = EsifApp_GetNextDomain(&appDomIter, appPartPtr, &appDomPtr);
										k++;
									}
								}
							}
							appPartIterRc = EsifApp_GetNextPart(&appPartIter, appPtr, &appPartPtr);
							j++;
						}
					}
				}
			}
		}
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	EsifAppMgr_PutRef(appPtr);
	return output;
}


static char *esif_shell_cmd_events(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	esif_error_t rc = ESIF_OK;
	UfEventIterator eventIter = { 0 };
	EventMgr_IteratorData curEventData = { 0 };
	EsifAppPtr appPtr = NULL;
	char *appNamePtr = NULL;
	char *eventMatch = NULL;
	char *appMatch = NULL;

	// events [namespec] [appspec]
	if (argc > 1) {
		eventMatch = argv[1];
	}
	if (argc > 2) {
		appMatch = argv[2];
	}

	/* Print header */
	esif_ccb_sprintf(OUT_BUF_LEN, output,
		"\nREGISTERED EVENTS:\n\n"
		"Type  Event Name                                                           Part  Domain  Application\n"
		"----  -------------------------------------------------------------------  ----  ------  -----------\n");
	
	/* Iterate through events present in the Event Manager 
	* Note: There is no guarantee of display of events that come/go during iteration
	*/
	rc = EsifEventMgr_InitIterator(&eventIter);

	if (ESIF_OK == rc) {
		rc = EsifEventMgr_GetNextEvent(&eventIter, &curEventData);
		while (ESIF_OK == rc) {

			/* Check if event is for and ESIF app; if so, get name */
			appPtr = NULL;
			appNamePtr = NULL;
			if (curEventData.callback == EsifSvcEventCallback) {
				appPtr = EsifAppMgr_GetAppFromHandle(curEventData.context);
				if (appPtr) {
					appNamePtr = EsifApp_GetAppName(appPtr);
				}
			}

			/* If using string matching parameter, skip event if no match for event name or app name */
			if ((eventMatch && esif_ccb_strmatch(esif_event_type_str(curEventData.eventType), eventMatch) == ESIF_FALSE) ||
				(appMatch && esif_ccb_strmatch((appNamePtr ? appNamePtr : "esif"), appMatch) == ESIF_FALSE)) {
				EsifAppMgr_PutRef(appPtr); /* Release reference from EsifAppMgr_GetAppFromHandle */
				rc = EsifEventMgr_GetNextEvent(&eventIter, &curEventData);
				continue;
			}

			/* Print event type */
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%4u  %-67s",
				curEventData.eventType,
				esif_event_type_str(curEventData.eventType));

			/* Print participant associated with event */
			if (EVENT_MGR_MATCH_ANY == curEventData.participantId) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "   ANY");
			}
			else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  %4d", curEventData.participantId);
			}

			/* Print participant domain with event */
			if (EVENT_MGR_MATCH_ANY_DOMAIN == curEventData.domainId) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "     ANY");
			}
			else if (EVENT_MGR_DOMAIN_NA == curEventData.domainId) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "     N/A");
			}
			else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  0x%04X", curEventData.domainId);
			}

			/* Print application associated with event
			* Note: EsifSvcEventCallback is used for all app events and the context
			* provided by the services interface when registering the events is the
			* app handle
			*/
			if (curEventData.callback == EsifSvcEventCallback) {
				if (appNamePtr) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  %11s ", appNamePtr);
				}
				else {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  %11s ", "orphan");
				}
			}
			else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  %11s ", "esif");
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");

			EsifAppMgr_PutRef(appPtr); /* Release reference from EsifAppMgr_GetAppFromHandle */
			rc = EsifEventMgr_GetNextEvent(&eventIter, &curEventData);
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	}

	if ((rc != ESIF_OK) && ((rc != ESIF_E_ITERATION_DONE))) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed to get event data: [%s (%d)]\n", esif_rc_str(rc), rc);
	}
	return output;
}

static char *esif_shell_cmd_appstart(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName = 0;

	// appstart [appname=]libname
	if (argc < 2) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid AppName\n");
		return output;
	}
	libName = argv[1];

	// Release Shell Lock while Starting App in case it calls into the ESIF SendCommand Interface after AppCreate completes
	esif_uf_shell_unlock();
	rc = EsifAppMgr_AppStart(libName);
	esif_uf_shell_lock();

	output = shell->outbuf = g_outbuf;
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Start App: %s [%s (%d)]\n", libName, esif_rc_str(rc), rc);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Started App: %s\n\n", libName);
	}

	return output;
}


static char *esif_shell_cmd_appstop(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName = 0;

	// appstop appname
	if (argc < 2) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid AppName\n");
		return output;
	}
	libName = argv[1];

	// Release Shell Lock while Stopping App in case it calls into the ESIF SendCommand Interface while Stopping
	esif_uf_shell_unlock();
	rc = EsifAppMgr_AppStop(libName);
	esif_uf_shell_lock();

	output = shell->outbuf = g_outbuf;
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Stop App: %s [%s (%d)]\n", libName, esif_rc_str(rc), rc);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Stopped App: %s\n\n", libName);
	}

	return output;
}


static char *esif_shell_cmd_actionstart(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	char *libName  = 0;
	eEsifError rc = ESIF_OK;

	if (argc < 2) {
		return NULL;
	}

	/* Parse The Library Name */
	libName = argv[1];

	rc = EsifActMgr_StartUpe(libName);

	switch(rc){
	case ESIF_OK:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Action %s started\n\n", libName);
		break;
	case ESIF_E_ACTION_ALREADY_STARTED:
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Action %s already started\n\n", libName);
		break;
	default:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Failed to start action %s, error = %s(%d)\n\n", libName, esif_rc_str(rc), rc);
		break;
	}
	return output;
}


static char *esif_shell_cmd_actionstop(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	eEsifError rc = ESIF_OK;
	char *libName  = 0;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}
	libName = argv[1];

	rc = EsifActMgr_StopUpe(libName);

	switch(rc){
	case ESIF_OK:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Action %s stopped\n\n", libName);
		break;
	case ESIF_E_ACTION_NOT_IMPLEMENTED:
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Action %s not found\n\n", libName);
		break;
	default:
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Failed to stop action %s, error = %s(%d)\n\n", libName, esif_rc_str(rc), rc);
		break;
	}
	return output;
}


// Execute Default Start Script, if any
static char *esif_shell_cmd_autoexec(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	int optarg = 1;
	char *command = NULL;

	// autoexec ["shell command"] [...]
	while (optarg < argc) {
		esif_uf_shell_unlock();
		parse_cmd(argv[optarg++], ESIF_FALSE, ESIF_TRUE);
		esif_uf_shell_lock();
	}
	if ((esif_shell_get_start_script() != NULL) && ((command = esif_ccb_strdup(esif_shell_get_start_script())) != NULL)) {
		esif_uf_shell_unlock();
		parse_cmd(command, ESIF_FALSE, ESIF_TRUE);
		esif_uf_shell_lock();
		esif_ccb_free(command);
	}
	// Output has already been shown by parse_cmd subfunctions
	output = shell->outbuf = g_outbuf;
	*output = '\0';
	return output;
}

// Stop On Error
static char *esif_shell_cmd_soe(EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char **argv   = shell->argv;
	char *output  = shell->outbuf;
	char *soe_str = 0;

	if (argc < 2) {
		return NULL;
	}
	soe_str = argv[1];

	if (!strcmp(soe_str, "on")) {
		g_soe = 1;
	} else {
		g_soe = 0;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "stop on error = %d\n", g_soe);
	return output;
}


// Debug Level
static char *esif_shell_cmd_debuglvl(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc *ipc_ptr = NULL;
	struct esif_command_set_debug_module_level data;
	const u32 data_len = sizeof(struct esif_command_set_debug_module_level);
	struct esif_ipc_command *command_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}

	// debuglvl <tracelevel>
	if (argc < 3) {
		data.module = (u32)(-1);
		data.level  = esif_atoi(argv[1]);
		if (data.level > ESIF_TRACELEVEL_DEBUG) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "invalid tracelevel\n");
			return NULL;
		}
	}
	// debuglvl <module> <categorymask>
	else {
		data.module = esif_atoi(argv[1]);
		data.level  = esif_atoi(argv[2]);
	}

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_SET_DEBUG_MODULE_LEVEL;
	command_ptr->req_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = data_len;
	command_ptr->rsp_data_type   = ESIF_DATA_VOID;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = 0;

	// Execute Command multiple times for "debuglvl -1 <categorymask>"
	if (argc >= 3 && data.module == (u32)(-1)) {
		u32 mod=0;
		for (mod = 0; mod < ESIF_DEBUG_MOD_MAX; mod++) {
			data.module = mod;
			esif_ccb_memcpy((command_ptr + 1), &data, data_len);
			ipc_execute(ipc_ptr);
		}
		data.module = (u32)(-1);
	}
	else {
		esif_ccb_memcpy((command_ptr + 1), &data, data_len);
		ipc_execute(ipc_ptr);
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	if (argc < 3)
		esif_ccb_sprintf(OUT_BUF_LEN, output, "kernel tracelevel = %d\n", data.level);
	else
		esif_ccb_sprintf(OUT_BUF_LEN, output, "kernel module = %d, category = 0x%08X\n", data.module, data.level);

exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// Debug Set
static char *esif_shell_cmd_debugset(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc *ipc_ptr = NULL;
	struct esif_command_debug_modules data;
	const u32 data_len = sizeof(struct esif_command_debug_modules);
	struct esif_ipc_command *command_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}
	data.modules = esif_atoi(argv[1]);

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_SET_DEBUG_MODULES;
	command_ptr->req_data_type   = ESIF_DATA_UINT32;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = data_len;
	command_ptr->rsp_data_type   = ESIF_DATA_VOID;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = 0;

	// Command
	esif_ccb_memcpy((command_ptr + 1), &data, data_len);
	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), command_ptr->return_code);
		goto exit;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "modules = 0x%08X\n", data.modules);
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

// Select Destination By ID
char *esif_shell_cmd_dst(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	esif_handle_t participantDst = ESIF_INVALID_HANDLE;
	EsifUpPtr upPtr = NULL;

	if (argc < 2) {
		return NULL;
	}

	participantDst = (esif_handle_t)esif_atoi64(argv[1]);
	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantDst);
	if (upPtr != NULL) {
		g_dst = participantDst;

		esif_ccb_sprintf(OUT_BUF_LEN, output, "Destination participant = " ESIF_HANDLE_FMT " selected (%s)\n", esif_ccb_handle2llu(g_dst), EsifUp_GetName(upPtr));

		esif_ccb_free(g_dstName);
		g_dstName = esif_ccb_strdup(EsifUp_GetName(upPtr));
		EsifUp_PutRef(upPtr);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Requested participant invalid, " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(participantDst));
	}
	return output;
}


// Select Destination By Name
char *esif_shell_cmd_dstn(EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char **argv      = shell->argv;
	char *output     = shell->outbuf;
	char *name_str   = 0;
	EsifUpPtr upPtr = NULL;

	if (argc < 2) {
		return NULL;
	}
	name_str = argv[1];

	upPtr   = EsifUpPm_GetAvailableParticipantByName(name_str);

	if (upPtr != NULL) {
		g_dst = EsifUp_GetInstance(upPtr);

		esif_ccb_sprintf(OUT_BUF_LEN, output, "Destination participant = %s selected (" ESIF_HANDLE_FMT ")\n", name_str, esif_ccb_handle2llu(g_dst));

		esif_ccb_free(g_dstName);
		g_dstName = esif_ccb_strdup(EsifUp_GetName(upPtr));
		EsifUp_PutRef(upPtr);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Requested participant, %s, not found\n", name_str);
	}

	return output;
}


// Echo
static char *esif_shell_cmd_echo(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *sep    = " ";
	int j = 1;

	// "echo ? arg arg ..." causes each argument to appear on separate line
	if (argc > 1 && strcmp(argv[1], "?") == 0) {
		sep = "\n";
		j++;
	}
	for ( ; j < argc; j++) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s%s", argv[j], (j + 1 < argc ? sep : "\n"));
	}

	return output;
}


// Format
static char *esif_shell_cmd_format(EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char **argv      = shell->argv;
	char *output     = shell->outbuf;
	char* format_str = "";

	if (argc > 1) {
		format_str = argv[1];
	}

	if (esif_ccb_strcmp(format_str, "xml") == 0) {
		g_format = FORMAT_XML;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "format=%s\n", format_str);
	}
	else if (esif_ccb_stricmp(format_str, "text") == 0) {
		g_format = FORMAT_TEXT;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "format=%s\n", format_str);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "invalid format requested=%s\n", format_str);
	}
	return output;
}


static void dump_binary_object(u8 *byte_ptr, u32 byte_len);
static void dump_table_hdr(struct esif_table_hdr *tab);
static void dump_binary_data(u8 *byte_ptr, u32 byte_len);

// Get From File
static char *esif_shell_cmd_getf(EsifShellCmdPtr shell)
{
	int argc         = shell->argc;
	char **argv      = shell->argv;
	char *output     = shell->outbuf;
	char *filename   = 0;
	char full_path[PATH_STR_LEN] = {0};
	FILE *fp_ptr     = NULL;
	struct stat file_stat = { 0 };
	size_t file_size = 0;
	size_t file_read = 0;
	u8 *buf_ptr      = NULL;
	char *suffix     = &argv[0][4];	// _b or _bd
	int dump         = 0;

	UNREFERENCED_PARAMETER(suffix);
	if (argv[0][ESIF_SHELL_STRLEN(argv[0]) - 1] == 'd') {	// getf_bd
		dump = 1;
	}

	if (argc < 2) {
		return NULL;
	}
	filename = argv[1];
	esif_build_path(full_path, sizeof(full_path), ESIF_PATHTYPE_BIN, filename, NULL);

	if (esif_ccb_stat(full_path, &file_stat) == 0 && file_stat.st_size > 0) {
		file_size = file_stat.st_size;
		fp_ptr = esif_ccb_fopen(full_path, (char *)FILE_READ, NULL);
	}
	if (NULL == fp_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: file not found (%s)\n", ESIF_FUNC, full_path);
		goto exit;
	}

	// allocate space
	buf_ptr = (u8 *)esif_ccb_malloc(file_size);
	if (NULL == buf_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: malloc failed to allocate %u bytes\n",
						 ESIF_FUNC, (u32)file_size);
		goto exit;
	}

	// read file contents
	file_read = fread(buf_ptr, 1, file_size, fp_ptr);
	if (file_read < file_size) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: read short received %u of %u bytes\n",
						 ESIF_FUNC, (int)file_read, (u32)file_size);
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "BIN Dump/Decode For %s Size %u:\n", full_path, (u32)file_size);

	if (0 == dump) {
		dump_binary_object(buf_ptr, (u32)file_size);
	} else {
		dump_binary_data(buf_ptr, (u32)file_size);
	}

exit:
	if (NULL != fp_ptr) {
		fclose(fp_ptr);
	}

	if (NULL != buf_ptr) {
		esif_ccb_free(buf_ptr);
	}
	return output;
}

//
//  WARNING:  Caller is responsible for releasing returned pointer
//
static struct esif_data *EsifCreateSpecificActionRequest(
	const UInt32 primitiveId,
	const EsifString domainStr,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr,
	EsifPrimitiveActionSelectorPtr actSelPtr
	)
{
	struct esif_data *reqPtr = NULL;
	EsifSpecificActionRequestPtr sarPtr = NULL;
	EsifPrimitiveTuplePtr tuplePtr = NULL;
	UInt16 domain = domain_str_to_short(domainStr);

	reqPtr = (struct esif_data *)esif_ccb_malloc(sizeof(*reqPtr) + sizeof(*sarPtr));
	if (NULL == reqPtr) {
		goto exit;
	}
	sarPtr = (EsifSpecificActionRequestPtr)(reqPtr + 1);
	tuplePtr = &sarPtr->tuple;

	reqPtr->type = ESIF_DATA_STRUCTURE;
	reqPtr->buf_ptr = sarPtr;
	reqPtr->buf_len = sizeof(*sarPtr);
	reqPtr->data_len = sizeof(*sarPtr);

	sarPtr->req_ptr = requestPtr;
	sarPtr->rsp_ptr = responsePtr;

	tuplePtr->id = (u16)primitiveId;
	tuplePtr->domain = domain;
	tuplePtr->instance = instance;

	esif_ccb_memcpy(&sarPtr->selector, actSelPtr, sizeof(sarPtr->selector));
exit:
	return reqPtr;
}


eEsifError EsifParseSpecificActionArg(
	char *argPtr,
	EsifPrimitiveActionSelectorPtr actSelectorPtr
	)
{
	eEsifError rc = ESIF_OK;
	u32 index = 0;
	esif_action_type_t actType = 0;

	ESIF_ASSERT(argPtr != NULL);
	ESIF_ASSERT(actSelectorPtr != NULL);

	if (*argPtr == '~') {
		argPtr++;
		actSelectorPtr->flags |= ESIF_PRIM_ACT_SEL_FLAG_EXCLUDE;
	}
	if (isdigit(*argPtr)) {
		index = (esif_action_type_t)esif_atoi(argPtr);
		actSelectorPtr->index = (u8)index;
		actSelectorPtr->flags |= ESIF_PRIM_ACT_SEL_FLAG_INDEX_VALID;
	}
	else {
		actType = esif_action_type_str2enum(argPtr);
		if ((esif_action_type_t)0 == actType) {
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
		actSelectorPtr->type = actType;
		actSelectorPtr->flags |= ESIF_PRIM_ACT_SEL_FLAG_TYPE_VALID;
	}
exit:
	return rc;
}

static eEsifError esif_shell_get_participant_id(char *participantNameOrId, esif_handle_t *targetParticipantIdPtr)
{ 
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	esif_handle_t participantID = ESIF_INVALID_HANDLE;

	ESIF_ASSERT(participantNameOrId != NULL);
	ESIF_ASSERT(targetParticipantIdPtr != NULL);

	// Name could be a number, so check if we have a participant by name first
	upPtr = EsifUpPm_GetAvailableParticipantByName(participantNameOrId);
	if (upPtr != NULL) {
		participantID = EsifUp_GetInstance(upPtr);
		EsifUp_PutRef(upPtr);
	}
	else {
		//If not found, try as a ID vs. name
		participantID = (esif_handle_t)esif_atoi64(participantNameOrId);

		// Error if not a number
		if (participantID == 0 && esif_ccb_strcmp(participantNameOrId, "0")) {
			rc = ESIF_E_PARTICIPANT_NOT_FOUND;
			goto exit;
		}
	}
	*targetParticipantIdPtr = participantID;
exit:
	return rc;
}


// Get Primitive
static char *esif_shell_cmd_getp(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	int opt = 1;

	u32 id      = 0;
	char *qualifier_str = "D0";
	u8 instance = 255;
	char full_path[PATH_STR_LEN]={0};
	char desc[32];
	u8 *data_ptr  = NULL;
	u16 qualifier = EVENT_MGR_DOMAIN_D0;

	struct esif_data response = {ESIF_DATA_VOID, NULL, 0};

	char *suffix = "";
	char *targetParticipant = NULL;
	enum esif_data_type type = ESIF_DATA_VOID;
	u32 buf_size = 0;
	int dump     = 0;
	esif_handle_t participantID = g_dst;
	EsifDataPtr primitiveInDataPtr = NULL;
	char dataIndicator[] = "data=";
	
	struct esif_data *sarPtr = NULL;
	EsifPrimitiveActionSelector actSelector = {0};

	if (argc <= opt) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Deduce suffix, data type, and dump options based on command
	if (esif_ccb_stricmp(argv[0], "getp") == 0) {
		suffix   = "";
		type     = ESIF_DATA_AUTO;
		buf_size = ESIF_DATA_ALLOCATE;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_u32") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_UINT32;
		buf_size = 4;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_t") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_TEMPERATURE;
		buf_size = 4;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_pw") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_POWER;
		buf_size = 4;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_s") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_STRING;
		buf_size = 128;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_t") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_UINT32;
		buf_size = g_binary_buf_size;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_b") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_BINARY;
		buf_size = g_binary_buf_size;
		dump     = 0;
	} else if (esif_ccb_stricmp(argv[0], "getp_bd") == 0) {
		suffix   = &argv[0][4];
		type     = ESIF_DATA_BINARY;
		buf_size = g_binary_buf_size;
		dump     = 1;
	} else if ((esif_ccb_stricmp(argv[0], "getp_bf") == 0) || (esif_ccb_stricmp(argv[0], "getp_bs") == 0)) {
		suffix   = &argv[0][4];
		type = ESIF_DATA_AUTO;
		buf_size = ESIF_DATA_ALLOCATE;
		dump     = 2;
	}
	else if (esif_ccb_stricmp(argv[0], "getp_part") == 0) {
		//now first parameter should be participant
		if (argc < MIN_PARAMETERS_FOR_GET_PRIMITIVE + 1) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		targetParticipant = argv[opt++];
		suffix = "";
		type = ESIF_DATA_AUTO;
		buf_size = ESIF_DATA_ALLOCATE;
		dump = 0;
		rc = esif_shell_get_participant_id(targetParticipant, &participantID);
		if (rc != ESIF_OK) {
			goto exit;
		}
	}

	// Primitive ID or Name
	id = (isdigit(argv[opt][0]) ? esif_atoi(argv[opt++]) : (u32)esif_primitive_type_str2enum(argv[opt++]));

	// Qualifier
	if (opt < argc) {
		qualifier_str = argv[opt++];
	}
	qualifier = domain_str_to_short(qualifier_str);

	// Instance ID
	if (opt < argc) {
		instance = (u8)esif_atoi(argv[opt++]);
	}

	// For File Dump
	if (2 == dump) {
		char *filename = 0;
		if (argc <= opt) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		filename = argv[opt++];
		char *extension = (esif_ccb_strchr(filename, '.') == NULL ? ".bin" : NULL);
		esif_build_path(full_path, sizeof(full_path), ESIF_PATHTYPE_BIN, filename, extension);
	}

	// Optional - input data (indicated by "data=[input value]")
	if (opt < argc && strncmp(dataIndicator,argv[opt], sizeof(dataIndicator) - 1) == 0) {
		primitiveInDataPtr = EsifData_Create();
		if (primitiveInDataPtr == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		rc = EsifData_FromString(primitiveInDataPtr, argv[opt++] + sizeof(dataIndicator), ESIF_DATA_AUTO);
		if (rc != ESIF_OK || primitiveInDataPtr->buf_ptr == NULL) {
			goto exit;
		}
	}

	// Optional - Action selector
	if (opt < argc) {
		//
		// Check for a '-' so deprecated options aren't seen as action selectors
		//
		if (*argv[opt] != '-') {
			rc = EsifParseSpecificActionArg(argv[opt++], &actSelector);
			if (rc != ESIF_OK) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Action option invalid\n");
				goto exit;
			}
		}
	}

	// Setup Response
	response.type = type;
	if (ESIF_DATA_ALLOCATE != buf_size) {
		data_ptr = (u8 *)esif_ccb_malloc(buf_size);
		if (NULL == data_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "esif_ccb_malloc failed for %u bytes\n", buf_size);
			goto exit;
		}
		response.buf_ptr  = (void *)data_ptr;
		response.buf_len  = buf_size;
		response.data_len = 0;
	} else {
		response.buf_len  = ESIF_DATA_ALLOCATE;
		response.buf_ptr  = NULL;
		response.data_len = 0;
	}

	// set up request (if not already set up by input)
	if (primitiveInDataPtr == NULL) {
		primitiveInDataPtr = esif_ccb_malloc(sizeof(*primitiveInDataPtr));
		if (primitiveInDataPtr == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		primitiveInDataPtr->type = ESIF_DATA_VOID;
		primitiveInDataPtr->buf_len = 0;
		primitiveInDataPtr->buf_ptr = NULL;
	}

	// Verify this is a GET
	if (EsifPrimitiveVerifyOpcode(
			participantID,
			id,
			qualifier_str,
			instance,
			ESIF_PRIMITIVE_OP_GET) == ESIF_FALSE) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Primitive (%d) not a GET: %s\n", id, esif_primitive_str(id));
		rc = ESIF_E_INVALID_REQUEST_TYPE;
		goto exit;
	}

	sarPtr = EsifCreateSpecificActionRequest(
		id,
		qualifier_str,
		instance,
		primitiveInDataPtr,
		&response,
		&actSelector);
	if (NULL == sarPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed to create specific action request\n");
		goto exit;
	}

	rc = EsifExecutePrimitive(
		participantID,
		SET_SPECIFIC_ACTION_PRIMITIVE,
		"D0",
		255,
		sarPtr,
		NULL);

	data_ptr = (u8 *)response.buf_ptr;

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s getp%s(%03u.%s.%03d)",
			esif_primitive_str((enum esif_primitive_type)id), suffix, id, qualifier_str, instance);
	}

	if (ESIF_E_NEED_LARGER_BUFFER == rc) {
		//
		// PRIMITIVE Error Code NEED Larger Buffer
		//
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 " error code = %s(%d) HAVE %u bytes NEED ATLEAST %u bytes\n",
						 esif_rc_str(rc),
						 rc,
						 response.buf_len,
						 response.data_len);
		g_errorlevel = -(ESIF_E_NEED_LARGER_BUFFER);
		goto exit;
	} else if (ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT == rc) {
		//
		// Not An Error Just A Warning
		//
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " info code = %s(%d)\n",
						 esif_rc_str(rc), rc);

		goto exit;
	} else if (ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP == rc) {

		esif_ccb_sprintf(OUT_BUF_LEN, output, "Required primitive %s %03u.D0.255 not found in DSP\n",
			esif_primitive_str((enum esif_primitive_type)SET_SPECIFIC_ACTION_PRIMITIVE), SET_SPECIFIC_ACTION_PRIMITIVE);
		goto exit;
	} else if (ESIF_OK != rc) {
		//
		// PRIMITIVE Error Code
		//
		if (ESIF_E_PRIMITIVE_SUR_NOT_FOUND_IN_DSP == rc) {
			rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " error code = %s(%d)\n", esif_rc_str(rc), rc);
		g_errorlevel = -(rc);
		goto exit;
	}
	
	if (NULL == response.buf_ptr) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "NULL buffer returned\n");
		goto exit;
	}

	//
	// WE have data so now process and format for output
	//

	/* Assign Respond Data Type Description */
	if (dump != 2) {
		type = response.type;
	}
	switch (type) {
	case ESIF_DATA_UINT32:
	case ESIF_DATA_UINT64:
		esif_ccb_strcpy(&desc[0], "", sizeof(desc));
		break;

	case ESIF_DATA_TEMPERATURE:
		esif_ccb_strcpy(&desc[0], "Degrees C", sizeof(desc));
		break;

	case ESIF_DATA_POWER:
		esif_ccb_strcpy(&desc[0], "MilliWatts", sizeof(desc));
		break;

	case ESIF_DATA_STRING:
		esif_ccb_strcpy(&desc[0], "ASCII", sizeof(desc));
		break;

	case ESIF_DATA_BINARY:
		esif_ccb_strcpy(&desc[0], "BINARY", sizeof(desc));
		break;

	case ESIF_DATA_TABLE:
		esif_ccb_strcpy(&desc[0], "TABLE", sizeof(desc));
		break;

	case ESIF_DATA_PERCENT:
		esif_ccb_strcpy(&desc[0], "Centi-Percent", sizeof(desc));
		break;

	case ESIF_DATA_FREQUENCY:
		esif_ccb_strcpy(&desc[0], "Frequency(Hz)", sizeof(desc));
		break;

	case ESIF_DATA_TIME:
		esif_ccb_strcpy(&desc[0], "Time (ms)", sizeof(desc));
		break;

	default:
		esif_ccb_strcpy(&desc[0], "", sizeof(desc));
		break;
	}

	if (ESIF_DATA_TEMPERATURE == type) {
		// Our Data
		u32 val = *(u32 *)(response.buf_ptr);
		float temp = 0.0;
		Bool disabled = ESIF_FALSE;

		// Magic number used to disable trip-points...
		if (ESIF_DISABLED_TEMP_VALUE == val) {
			disabled = ESIF_TRUE;
		}

		/* Internally we use 1/10ths K, but display in float C */
		esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, &val);

		temp = (float)((int)val / 10.0);
		if (FORMAT_TEXT == g_format) {
			if (!disabled) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = %.1f %s\n",temp, desc);
			} else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = DISABLED\n",temp);
			}
		} else {
			if (!disabled) {
				esif_ccb_sprintf(OUT_BUF_LEN, output,
					"<result>\n"
					"    <value>%.1f</value>\n"
					"    <valueDesc>%s</valueDesc>\n",
					temp, desc);
			} else {
				esif_ccb_sprintf(OUT_BUF_LEN, output,
					"<result>\n"
					"    <value>DISABLED</value>\n"
					"    <valueDesc> </valueDesc>\n");
			}
		}
	}
	else if ((ESIF_DATA_UINT32 == type) ||
		(ESIF_DATA_POWER == type) ||
		(ESIF_DATA_TIME == type) ||
		(ESIF_DATA_PERCENT == type)) {
		// Our Data
		u32 val = *(u32 *)(response.buf_ptr);

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = 0x%08x %u %s\n",
								val, val, desc);
		} else {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
								"<result>\n"
								"    <value>%u</value>\n"
								"    <valueDesc>%s</valueDesc>\n",
								val, desc);
		}
	}
	else if (ESIF_DATA_STRING == type) {
		char *str_ptr = (char *)(response.buf_ptr);
		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " string(%u of %u) = %s\n",
							 response.data_len,
							 response.buf_len,
							 str_ptr);
		}
		else {
			char *val_str = esif_ccb_strdup(str_ptr);
			if (val_str) {
				strip_illegal_chars(val_str, g_illegalXmlChars);

				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					"<result>\n"
					"    <value>%s</value>\n"
					"    <valueDesc>%s</valueDesc>\n",
					val_str, desc);

				esif_ccb_free(val_str);
			}
		}
	} else if (ESIF_DATA_UINT64 == type ||
			   ESIF_DATA_FREQUENCY == type) {
		u64 val = *(u64 *)(response.buf_ptr);
		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " value = 0x%016llx %llu %s\n",
							 val, val, desc);
		} else {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "<result>\n"
							 "    <value>%llu</value>\n"
							 "    <valueDesc>%s</valueDesc>\n",
							 val, desc);
		}
	} else {// Binary Dump
		u8 *byte_ptr = (u8 *)(response.buf_ptr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " Binary Data(%u of %u):",
						 response.data_len,
						 response.buf_len);

		switch (dump) {
		case 0:
			if (ESIF_DATA_TABLE == type) {
				dump_table_hdr((struct esif_table_hdr *)byte_ptr);
			} else {
				dump_binary_object(byte_ptr, response.data_len);
			}
			break;
		case 1:
			dump_binary_data(byte_ptr, response.data_len);
			break;
		case 2:
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, " %s Data To File: %s (%u of %u):\n",
							 ltrim(esif_data_type_str(response.type), PREFIX_DATA_TYPE),
							 full_path,
							 response.data_len,
							 response.buf_len);
			FILE *fp_ptr = NULL;

			if ((fp_ptr = esif_ccb_fopen(full_path, (char *)"wb", NULL)) != NULL) {
				fwrite(byte_ptr, 1, response.data_len, fp_ptr);
				fclose(fp_ptr);
			}
			break;
		default:
			break;
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</result>\n");
	}

exit:
	esif_ccb_free(sarPtr);
	esif_ccb_free(data_ptr);
	if (primitiveInDataPtr) {
		esif_ccb_free(primitiveInDataPtr->buf_ptr);
	}
	esif_ccb_free(primitiveInDataPtr);

	if (output && *output == 0 && rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	return output;
}

static char *esif_shell_cmd_ufpoll(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	int pollInterval = 0;
	char **argv = shell->argv;
	char *output = shell->outbuf;

	// ufpoll [status]
	if (argc < 2 || esif_ccb_stricmp(argv[1], "status") == 0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Upper framework polling is: %s\n", (EsifUFPollStarted() ? "started" : "stopped"));
	}
	// ufpoll start
	else if (esif_ccb_stricmp(argv[1], "start") == 0) {
		if (argc > 2) {
			if ((int) esif_atoi(argv[2]) >= ESIF_UFPOLL_PERIOD_MIN) {
				pollInterval = (esif_ccb_time_t) esif_atoi(argv[2]);
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid polling period specified (minimum is %d ms).\n", ESIF_UFPOLL_PERIOD_MIN);
				goto exit;
			}
		}
		/* Calling start when it is already started is allowed...it will update the poll period */
		EsifUFPollStart(pollInterval);
	}
	// ufpoll stop
	else if (esif_ccb_stricmp(argv[1], "stop") == 0) {
			EsifUFPollStop();
	}
exit:
	return output;
}

// FPC Info
static char *esif_shell_cmd_infofpc(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char cpc_domain_str[8] = "";
	char *cpc_pattern = "";
	char *edp_filename     = 0;
	char edp_name[PATH_STR_LEN]={0};
	char edp_full_path[PATH_STR_LEN]={0};
	EsifFpcPtr fpcPtr = NULL;
	EsifFpcDomainPtr domainPtr;
	EsifFpcPrimitivePtr primitivePtr;
	EsifFpcActionPtr fpcActionPtr;
	EsifFpcAlgorithmPtr algorithmPtr;
	EsifFpcEventPtr eventPtr;
	u32 fpc_size = 0;
	u8 *base_ptr = NULL;
	u32 num_prim = 0, i, j, k;
	IOStreamPtr io_ptr    = IOStream_Create();
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key = 0;
	EsifDataPtr value     = 0;
	struct edp_dir edp_dir;
	size_t r_bytes;
	size_t fpc_read;
	u32 offset; /* For debug only, not used for functional code */
	u32 edp_size;


	if (argc < 2 || io_ptr == NULL) {
		IOStream_Destroy(io_ptr);
		return NULL;
	}

	// parse filename no extension
	edp_filename = argv[1];

	// parse option regular expression
	if (argc > 2) {
		cpc_pattern = argv[2];
	}

	// Look for EDP file either on disk or in a DataVault (static or file), depending on priority setting
	esif_ccb_sprintf(PATH_STR_LEN, edp_name, "%s.edp", edp_filename);
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key       = EsifData_CreateAs(ESIF_DATA_STRING, edp_name, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	if (nameSpace == NULL || key == NULL || value == NULL) {
		goto exit;
	}
	esif_build_path(edp_full_path, sizeof(edp_full_path), ESIF_PATHTYPE_DSP, edp_name, NULL);

	if (!esif_ccb_file_exists(edp_full_path) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(edp_full_path, edp_name, PATH_STR_LEN);
		IOStream_SetMemory(io_ptr, StoreReadOnly, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, StoreReadOnly, edp_full_path, (char *)FILE_READ);
	}

	if (IOStream_Open(io_ptr) == 0) {
		edp_size = (UInt32)IOStream_GetSize(io_ptr);
		if (!edp_size) {
			goto exit;
		}
		r_bytes  = IOStream_Read(io_ptr, &edp_dir, sizeof(struct edp_dir));
		if (!esif_verify_edp(&edp_dir, r_bytes)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid EDP Header: Signature=%4.4s Version=%d\n", (char *)&edp_dir.signature, edp_dir.version);
			goto exit;
		}
		if (edp_dir.fpc_offset > MAX_CPC_SIZE || edp_dir.fpc_offset > edp_size) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: Invalid fpc size: edp_size %u, fpc_offset %u\n",
							 ESIF_FUNC, edp_size, edp_dir.fpc_offset);
			goto exit;
		}
		fpc_size = edp_size - edp_dir.fpc_offset;
		IOStream_Seek(io_ptr, edp_dir.fpc_offset, SEEK_SET);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: file not found (%s)\n", ESIF_FUNC, edp_full_path);
		goto exit;
	}

	// Find FPC and read out
	IOStream_Seek(io_ptr, edp_dir.fpc_offset, SEEK_SET);

	// allocate space for our FPC file contents
	fpcPtr = (EsifFpcPtr)esif_ccb_malloc(fpc_size);
	if (NULL == fpcPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: FPC malloc failed to allocate %u bytes for fpc\n",
						 ESIF_FUNC, fpc_size);
		goto exit;
	}
	base_ptr = (u8 *)fpcPtr;

	// read FPC file contents
	fpc_read = IOStream_Read(io_ptr, fpcPtr, fpc_size);
	if (fpc_read < fpc_size) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: FPC read short received %u of %u bytes\n",
						 ESIF_FUNC, (int)fpc_read, fpc_size);
		goto exit;
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "FPC File Info For %s Size %u\n\n",
						 edp_filename, fpc_size);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "FPC Source:     %s\n"
						 "FPC Size:       %d\n"
						 "FPC Name:       %s\n"
						 "FPC Version:    %x.%x\n"
						 "FOC Desc:       %s\n"
						 "FPC Primitives: %d\n"
						 "FPC Algorithms: %d\n"
						 "FPC Domains:    %d\n",
						 (IOStream_GetType(io_ptr) == StreamMemory ? "DataVault" : "File"),
						 fpcPtr->size,
						 fpcPtr->header.name,
						 fpcPtr->header.ver_major, fpcPtr->header.ver_minor,
						 fpcPtr->header.description,
						 fpcPtr->number_of_domains,
						 fpcPtr->number_of_algorithms,
						 fpcPtr->number_of_events);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<fpcinfo>\n"
						 "  <size>%d<size>\n"
						 "  <fpcSource>%s\n"
						 "  <fpcSize>%d</fpcSize>\n"
						 "  <fpcName>%s</fpcName>\n"
						 "  <fpcVersion>%x.%x<fpcVersion>\n"
						 "  <fpcDesc>%s<fpcDesc>\n"
						 "  <fpcPrimitives>%d<fpcPrimitives>\n"
						 "  <fpcAlgorithms>%d<fpcAlgorithms>\n"
						 "  <fpcDomains>%d</fpcDomains>\n",
						 fpc_size,
						 (IOStream_GetType(io_ptr) == StreamMemory ? "DataVault" : "File"),
						 fpcPtr->size,
						 fpcPtr->header.name,
						 fpcPtr->header.ver_major, fpcPtr->header.ver_minor,
						 fpcPtr->header.description,
						 fpcPtr->number_of_domains,
						 fpcPtr->number_of_algorithms,
						 fpcPtr->number_of_events);
	}


	/* First Domain, Ok To Have Zero Primitive Of A Domain */
	domainPtr = (EsifFpcDomainPtr)(fpcPtr + 1);
	for (i = 0; i < esif_ccb_min(fpcPtr->number_of_domains, MAX_DOMAINS); i++) {
		int show = 0;
		char temp_buf[1024]    = "";
		char qualifier_str[64] = "";

		offset = (unsigned long)((u8 *)domainPtr - base_ptr);

		if (!strcmp(cpc_pattern, "")) {
			show = 1;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(1024, temp_buf,
							 "\nDOMAINS:\n\n"
							 "Name     Description          Domain   Priority Capability Type               \n"
							 "-------- -------------------- -------- -------- ---------- -----------------------------\n");

			esif_ccb_sprintf_concat(1024, temp_buf,
							 "%-8s %-20s %-8s %08x %08x   %s(%d)\n",
							 domainPtr->descriptor.name,
							 domainPtr->descriptor.description,
							 esif_primitive_domain_str(domainPtr->descriptor.domain, qualifier_str, 64),
							 domainPtr->descriptor.priority,
							 domainPtr->capability_for_domain.capability_flags,
							 esif_domain_type_str(domainPtr->descriptor.domainType),
							 domainPtr->descriptor.domainType);


			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}
		} else {	// FORMAT_XML
			esif_ccb_sprintf(1024, temp_buf,
							 "<domain>%s</domain>\n"
							 "<domainSize>%u</domainSize>\n"
							 "<numPrimitive>%u</numPrimitive>\n"
							 "<numCapability>%d</numCapability>\n"
							 "<capability>%x</capability>\n",
							 domainPtr->descriptor.name,
							 domainPtr->size,
							 domainPtr->number_of_primitives,
							 domainPtr->capability_for_domain.number_of_capability_flags,
							 domainPtr->capability_for_domain.capability_flags);
			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}
		}


		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\nCAPABILITY:\n\n");
		}


		/* Capability */
		for (j = 0; j < MAX_CAPABILITY_MASK; j++) {
			offset = (unsigned long)(((u8 *)&domainPtr->capability_for_domain) - base_ptr);

			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf(1024, temp_buf,
								 "Capability[%d] 0x%x\n",
								 j, domainPtr->capability_for_domain.capability_mask[j]);
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			} else {	// FORMAT_XML
				esif_ccb_sprintf(1024, temp_buf,
								 "<Capability[%d]>%x</Capability[%d]>\n",
								 j, domainPtr->capability_for_domain.capability_mask[j], j);
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(1024, temp_buf, "\nPRIMITIVES AND ACTIONS:\n\n");
		}
		if (1 == show) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
		}

		/* First Primtive */
		primitivePtr = (EsifFpcPrimitivePtr)(domainPtr + 1);
		for (j = 0; j < domainPtr->number_of_primitives; j++, num_prim++) {
			offset = (unsigned long)(((u8 *)primitivePtr) - base_ptr);

			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf(1024, temp_buf,
								 "%03d.%s.%03d %s actions %d %s req %s rsp %s\n",
								 primitivePtr->tuple.id,
								 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
								 primitivePtr->tuple.instance,
								 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
								 primitivePtr->num_actions,
								 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id),
								 esif_data_type_str(primitivePtr->request_type),
								 esif_data_type_str(primitivePtr->result_type));

				// TODO REGEXP Here For Now Simple Pattern Match
				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				// Show Row?
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			} else {	// FORMAT_XML
				esif_ccb_sprintf(1024, temp_buf,
								 "    <primitive>\n"
								 "      <id>%d</id>\n"
								 "      <primitiveStr>%s</primitiveStr>\n"
								 "      <domain>%s</domain>\n"
								 "      <instance>%d</instance>\n"
								 "      <opcode>%d</opcode>\n"
								 "      <opcodeStr>%s</opcodeStr>\n"
								 "      <actionCount>%d</actionCount>\n"
								 "    </primitive>\n",
								 primitivePtr->tuple.id,
								 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id),
								 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
								 primitivePtr->tuple.instance,
								 primitivePtr->operation,
								 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
								 primitivePtr->num_actions);

				// TODO REGEXP Here For Now Simple Pattern Match
				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				// Show Row?
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}

			/* First Action */
			fpcActionPtr = (EsifFpcActionPtr)(primitivePtr + 1);
			for (k = 0; k < primitivePtr->num_actions; k++) {
				offset = (unsigned long)(((u8 *)fpcActionPtr) - base_ptr);

				if (FORMAT_TEXT == g_format) {
					esif_ccb_sprintf(1024, temp_buf,
									 "  %s param_valid %1d:%1d:%1d:%1d:%1d param %x:%x:%x:%x:%x\n",
									 esif_action_type_str(fpcActionPtr->type),
									 fpcActionPtr->param_valid[0],
									 fpcActionPtr->param_valid[1],
									 fpcActionPtr->param_valid[2],
									 fpcActionPtr->param_valid[3],
									 fpcActionPtr->param_valid[4],
									 fpcActionPtr->param_offset[0],
									 fpcActionPtr->param_offset[1],
									 fpcActionPtr->param_offset[2],
									 fpcActionPtr->param_offset[3],
									 fpcActionPtr->param_offset[4]);

					// TODO REGEXP Here For Now Simple Pattern Match
					if (0 == show && strstr(temp_buf, cpc_pattern)) {
						show = 1;
					}
					if (1 == show) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
					}
				} else {// FORMAT_XML
					esif_ccb_sprintf(1024, temp_buf,
									 "   <action>\n"
									 "      <type>%d</type>\n"
									 "      <typeStr>%s</typeStr>\n"
									 "      <paramValid>%1d:%1d:%1d:%1d:%1d</paramValid>\n"
									 "      <param[0]>%x</param[0]>\n"
									 "      <param[1]>%x</param[1]>\n"
									 "      <param[2]>%x</param[2]>\n"
									 "      <param[3]>%x</param[3]>\n"
									 "      <param[4]>%x</param[4]>\n"
									 "   </action>\n",
									 fpcActionPtr->type,
									 esif_action_type_str(fpcActionPtr->type),
									 fpcActionPtr->param_valid[0],
									 fpcActionPtr->param_valid[1],
									 fpcActionPtr->param_valid[2],
									 fpcActionPtr->param_valid[3],
									 fpcActionPtr->param_valid[4],
									 fpcActionPtr->param_offset[0],
									 fpcActionPtr->param_offset[1],
									 fpcActionPtr->param_offset[2],
									 fpcActionPtr->param_offset[3],
									 fpcActionPtr->param_offset[4]);
					// TODO REGEXP Here For Now Simple Pattern Match
					if (0 == show && strstr(temp_buf, cpc_pattern)) {
						show = 1;
					}
					if (1 == show) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
					}
				}
				/* Next Action */
				fpcActionPtr = (EsifFpcActionPtr)((u8 *)fpcActionPtr + fpcActionPtr->size);
			}
			/* Next Primitive */
			primitivePtr = (EsifFpcPrimitivePtr)((u8 *)primitivePtr + primitivePtr->size);
		}
		/* Next Domain */
		domainPtr = (EsifFpcDomainPtr)((u8 *)domainPtr + domainPtr->size);
	}


	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "ALGORITHMS:\n\n"
						 "Action Type                tempxform         tc1      tc2      powerxform         timexform\n"

						 "------ ------------------- ----------------- -------- -------- ----------------- ----------------\n");
	}

	/* First Algorithm (Laid After The Last Domain) */
	algorithmPtr = (EsifFpcAlgorithmPtr)domainPtr;
	for (i = 0; i < fpcPtr->number_of_algorithms && fpcPtr->number_of_algorithms <= MAX_ALGORITHMS; i++) {
		offset = (unsigned long)((u8 *)algorithmPtr - base_ptr);

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%-2d     %-19s %-17s %08x %08x %-17s %-17s\n",
							 algorithmPtr->action_type,
							 esif_action_type_str(algorithmPtr->action_type),
							 ltrim(esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->temp_xform)), PREFIX_ALGORITHM_TYPE),
							 algorithmPtr->tempC1,
							 algorithmPtr->percent_xform,
							 ltrim(esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->power_xform)), PREFIX_ALGORITHM_TYPE),
							 ltrim(esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->time_xform)), PREFIX_ALGORITHM_TYPE));
		} else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "<algorithmType>%s</algorithmType>\n"
							 "   <tempXform>%s</tempXform>\n"
							 "   <tempC1>%d</tempC1>\n"
							 "   <percent_xform>%d</percent_xform>\n"
							 "   <powerXform>%s</powerXform>\n"
							 "   <timeXform>%s</timeXform>\n",
							 esif_action_type_str(algorithmPtr->action_type),
							 esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->temp_xform)),
							 algorithmPtr->tempC1,
							 algorithmPtr->percent_xform,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->power_xform)),
							 esif_algorithm_type_str((enum esif_algorithm_type)(algorithmPtr->time_xform)));
		}

		/* Next Algorithm */
		algorithmPtr = (EsifFpcAlgorithmPtr)(algorithmPtr + 1);
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "Mapped Events:\n\n"
						 "Alias Event Identification             Group    Event Data           Event Description\n"
						 "----- -------------------------------- -------- -------------------- --------------------------------------\n");
	}	// FORMAT_TEXT

	/* First Event (Laid After The Last Algorithm) */
	eventPtr = (EsifFpcEventPtr)algorithmPtr;
	for (i = 0; i < fpcPtr->number_of_events && i < MAX_EVENTS; i++) {
		offset = (unsigned long)((u8 *)eventPtr - base_ptr);

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%5s ",
							 eventPtr->event_name);

			for (j = 0; j < 16; j++) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02x", eventPtr->event_key[j]);
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 " %-8s %-20s %s(%d)\n",
							 ltrim(esif_event_group_enum_str(eventPtr->esif_group), PREFIX_EVENT_GROUP),
							 esif_data_type_str(eventPtr->esif_group_data_type),
							 ltrim(esif_event_type_str(eventPtr->esif_event), PREFIX_EVENT_TYPE),
							 eventPtr->esif_event);
		} else {// FORMAT_XML
			char eventKey[64] = "";

			for (j = 0; j < 16; j++) {
				esif_ccb_sprintf_concat(64, eventKey, "%02x", eventPtr->event_key[j]);
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "<eventName>%s</eventName>\n"
							 "   <key>%s</key>\n"
							 "   <group>%d</group>\n"
							 "   <groupString>%s</groupString>\n"
							 "   <groupData>%d</groupData>\n"
							 "   <groupDataString>%s</groupDataString>\n"
							 "   <eventNum>%d</evenNum>\n"
							 "   <event>%s</event>\n",
							 eventPtr->event_name,
							 eventKey,
							 eventPtr->esif_group,
							 esif_event_group_enum_str(eventPtr->esif_group),
							 eventPtr->esif_group_data_type,
							 esif_data_type_str(eventPtr->esif_group_data_type),
							 eventPtr->esif_event,
							 esif_event_type_str(eventPtr->esif_event));
		}

		/* Next Event */
		eventPtr = (EsifFpcEventPtr)(eventPtr + 1);
	}

exit:
	// Cleanup
	esif_ccb_free(fpcPtr);
	IOStream_Destroy(io_ptr);
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return output;
}


// CPC Info
static char *esif_shell_cmd_infocpc(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char cpc_domain_str[8] = "";
	char *cpc_pattern = "";
	char *edp_filename     = 0;
	char edp_name[PATH_STR_LEN];
	char edp_full_path[PATH_STR_LEN];
	u32 cpc_size    = 0;
	size_t cpc_read = 0;
	u8 *byte_ptr    = NULL;
	struct esif_lp_cpc *cpc_ptr = NULL;
	struct esif_cpc_primitive *primitivePtr = NULL;
	struct esif_cpc_action *actionPtr = NULL;
	u32 i = 0, j = 0;
	IOStreamPtr io_ptr    = IOStream_Create();
	EsifDataPtr nameSpace = 0;
	EsifDataPtr key = 0;
	EsifDataPtr value     = 0;

	if (argc < 2 || io_ptr == NULL) {
		IOStream_Destroy(io_ptr);
		return NULL;
	}

	// parse filename no extension
	edp_filename = argv[1];

	// parse option regular expression
	if (argc > 2) {
		cpc_pattern = argv[2];
	}

	// Look for EDP file either on disk or in a DataVault (static or file), depending on priority setting
	esif_ccb_sprintf(PATH_STR_LEN, edp_name, "%s.edp", edp_filename);
	nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, ESIF_DSP_NAMESPACE, 0, ESIFAUTOLEN);
	key       = EsifData_CreateAs(ESIF_DATA_STRING, edp_name, 0, ESIFAUTOLEN);
	value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

	if (nameSpace == NULL || key == NULL || value == NULL) {
		goto exit;
	}
	esif_build_path(edp_full_path, sizeof(edp_full_path), ESIF_PATHTYPE_DSP, edp_name, NULL);

	if (!esif_ccb_file_exists(edp_full_path) && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
		esif_ccb_strcpy(edp_full_path, edp_name, PATH_STR_LEN);
		IOStream_SetMemory(io_ptr, StoreReadOnly, (BytePtr)value->buf_ptr, value->data_len);
	} else {
		IOStream_SetFile(io_ptr, StoreReadOnly, edp_full_path, (char *)FILE_READ);
	}
	if (IOStream_Open(io_ptr) == 0) {
		struct edp_dir edp_dir;
		size_t r_bytes;

		/* FIND CPC within EDP file */
		r_bytes  = IOStream_Read(io_ptr, &edp_dir, sizeof(struct edp_dir));
		if (!esif_verify_edp(&edp_dir, r_bytes)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid EDP Header: Signature=%4.4s Version=%d\n", (char *)&edp_dir.signature, edp_dir.version);
			goto exit;
		}
		if (edp_dir.cpc_offset > MAX_CPC_SIZE || edp_dir.fpc_offset > MAX_FPC_SIZE || edp_dir.cpc_offset > edp_dir.fpc_offset) {
			cpc_size = MAX_CPC_SIZE + 1;
		}
		else {
			cpc_size = edp_dir.fpc_offset - edp_dir.cpc_offset;
		}
		if (cpc_size > MAX_CPC_SIZE) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: Invalid cpc size: fpc_offset %u cpc_offset %u\n", ESIF_FUNC, edp_dir.fpc_offset, edp_dir.cpc_offset);
			goto exit;
		}
		IOStream_Seek(io_ptr, edp_dir.cpc_offset, SEEK_SET);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: file not found (%s)\n", ESIF_FUNC, edp_full_path);
		goto exit;
	}

	// allocate space for our CPC file contents
	cpc_ptr = (struct esif_lp_cpc *)esif_ccb_malloc(cpc_size);
	if (NULL == cpc_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: malloc failed to allocate %u bytes\n",
						 ESIF_FUNC, cpc_size);
		goto exit;
	}

	// read file contents
	cpc_read = IOStream_Read(io_ptr, cpc_ptr, cpc_size);
	if (cpc_read < cpc_size) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: read short received %u of %u bytes\n",
						 ESIF_FUNC, (int)cpc_read, cpc_size);
		goto exit;
	}

	// check signature to make sure this is a CPC file
	if (cpc_ptr->header.cpc.signature != *(unsigned int *)ESIF_CPC_SIGNATURE) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: signature validation failure not a cpc file\n",
						 ESIF_FUNC);
		goto exit;
	}

	// parse and dump
	byte_ptr = (u8 *)&cpc_ptr->header.cpc.signature;
	if (cpc_ptr->number_of_basic_primitives > MAX_PRIMITIVES ||
		cpc_ptr->number_of_algorithms > MAX_ALGORITHMS ||
		cpc_ptr->number_of_domains > MAX_DOMAINS ||
		cpc_ptr->number_of_events > MAX_EVENTS) {
		goto exit;
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "CPC File Info For %s Size %u:\n\n",
						 edp_filename, cpc_size);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "CPC Source:     %s\n"
						 "CPC Size:       %d\n"
						 "CPC Primitives: %d\n"
						 "CPC Algorithms: %d\n"
						 "CPC Domains:    %d\n"
						 "CPC EventMaps:  %d\n"
						 "CPC Signature:  %c%c%c%c\n"
						 "CPC Version:    %d\n"
						 "CPC Flags:      %08x\n"
						 "DSP Version:    %d\n"
						 "DSP Code:       %s\n"
						 "DSP Content:    v%d.%d\n"
						 "DSP Flags:      %08x\n\n",
						 (IOStream_GetType(io_ptr) == StreamMemory ? "DataVault" : "File"),
						 cpc_ptr->size,
						 cpc_ptr->number_of_basic_primitives,
						 cpc_ptr->number_of_algorithms,
						 cpc_ptr->number_of_domains,
						 cpc_ptr->number_of_events,
						 *byte_ptr, *(byte_ptr + 1), *(byte_ptr + 2), *(byte_ptr + 3),
						 cpc_ptr->header.cpc.version,
						 cpc_ptr->header.cpc.flags,
						 cpc_ptr->header.version,
						 cpc_ptr->header.code,
						 cpc_ptr->header.ver_major,
						 cpc_ptr->header.ver_minor,
						 cpc_ptr->header.flags);
	} else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<cpcinfo>\n"
						 "  <size>%d</size>\n"
						 "  <primitives>%d</primitives>\n"
						 "  <cpcSignature>%c%c%c%c</cpcSignature>\n"
						 "  <cpcVersion>%d</cpcVersion>\n"
						 "  <cpcFlags>%08x</cpcFlags>\n"
						 "  <dspVersion>%d</dspVersion>\n"
						 "  <dspCode>%s</dspCode>\n"
						 "  <dspContentVer>%d.%d</dspContentVer>\n"
						 "  <dspFlags>%08x</dspFlags>\n",
						 cpc_ptr->size,
						 cpc_ptr->number_of_basic_primitives,
						 *byte_ptr, *(byte_ptr + 1), *(byte_ptr + 2), *(byte_ptr + 3),
						 cpc_ptr->header.cpc.version,
						 cpc_ptr->header.cpc.flags,
						 cpc_ptr->header.version,
						 cpc_ptr->header.code,
						 cpc_ptr->header.ver_major, cpc_ptr->header.ver_minor,
						 cpc_ptr->header.flags);
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <primitives>\n");
	}

	primitivePtr = (struct esif_cpc_primitive *)(cpc_ptr + 1);
	for (i = 0; i < cpc_ptr->number_of_basic_primitives; i++) {
		int show = 0;
		char temp_buf[1024];

		if (!strcmp(cpc_pattern, "")) {
			show = 1;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(1024, temp_buf, "%03d.%s.%03d %s actions %d %s\n",
							 primitivePtr->tuple.id,
							 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
							 primitivePtr->tuple.instance,
							 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
							 primitivePtr->number_of_actions,
							 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id));
			// TODO REGEXP Here For Now Simple Pattern Match
			if (0 == show && strstr(temp_buf, cpc_pattern)) {
				show = 1;
			}
			// Show Row?
			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}

			actionPtr = (struct esif_cpc_action *)(primitivePtr + 1);
			for (j = 0; j < primitivePtr->number_of_actions; j++, actionPtr++) {
				if (!strcmp(cpc_pattern, "")) {
					show = 1;
				}
				esif_ccb_sprintf(1024, temp_buf, " %s param_valid %1d:%1d:%1d:%1d:%1d param %x:%x:%x:%x:%x\n",
								 esif_action_type_str(actionPtr->type),
								 actionPtr->param_valid[0],
								 actionPtr->param_valid[1],
								 actionPtr->param_valid[2],
								 actionPtr->param_valid[3],
								 actionPtr->param_valid[4],
								 actionPtr->param[0],
								 actionPtr->param[1],
								 actionPtr->param[2],
								 actionPtr->param[3],
								 actionPtr->param[4]);

				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}
		} else {// FORMAT_XML
			esif_ccb_sprintf(1024, temp_buf,
							 "    <primitive>\n"
							 "      <id>%d</id>\n"
							 "      <primitiveStr>%s</primitiveStr>\n"
							 "      <domain>%s</domain>\n"
							 "      <instance>%d</instance>\n"
							 "      <opcode>%d</opcode>\n"
							 "      <opcodeStr>%s</opcodeStr>\n"
							 "      <actionCount>%d</actionCount>\n"
							 "    </primitive>\n",
							 primitivePtr->tuple.id,
							 esif_primitive_str((enum esif_primitive_type)primitivePtr->tuple.id),
							 esif_primitive_domain_str(primitivePtr->tuple.domain, cpc_domain_str, 8),
							 primitivePtr->tuple.instance,
							 primitivePtr->operation,
							 esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
							 primitivePtr->number_of_actions);
			// TODO REGEXP Here For Now Simple Pattern Match
			if (0 == show && strstr(temp_buf, cpc_pattern)) {
				show = 1;
			}
			// Show Row?
			if (1 == show) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
			}

			actionPtr = (struct esif_cpc_action *)(primitivePtr + 1);
			for (j = 0; j < primitivePtr->number_of_actions; j++, actionPtr++) {
				if (!strcmp(cpc_pattern, "")) {
					show = 1;
				}
				esif_ccb_sprintf(1024, temp_buf,
								 "   <action>\n"
								 "      <type>%d</type>\n"
								 "      <typeStr>%s</typeStr>\n"
								 "      <numValidParam>%d</numValidParam>\n"
								 "      <paramValid>%1d:%1d:%1d:%1d:%1d</paramValid>\n"
								 "      <param0>%x</param0>\n"
								 "      <param1>%x</param1>\n"
								 "      <param2>%x</param2>\n"
								 "      <param3>%x</param3>\n"
								 "      <param4>%x</param4>\n"
								 "   </action>\n",
								 actionPtr->type,
								 esif_action_type_str(actionPtr->type),
								 actionPtr->num_valid_params,
								 actionPtr->param_valid[0],
								 actionPtr->param_valid[1],
								 actionPtr->param_valid[2],
								 actionPtr->param_valid[3],
								 actionPtr->param_valid[4],
								 actionPtr->param[0],
								 actionPtr->param[1],
								 actionPtr->param[2],
								 actionPtr->param[3],
								 actionPtr->param[4]);
				if (0 == show && strstr(temp_buf, cpc_pattern)) {
					show = 1;
				}
				if (1 == show) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", temp_buf);
				}
			}
		}

		// Next Primitive In CPC Set
		primitivePtr = (struct esif_cpc_primitive *)((u8 *)primitivePtr + primitivePtr->size);
	}

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  </primitives>\n");
	}

	if (FORMAT_TEXT == g_format) {
		u8 algo_index   = 0;
		u8 domainIndex = 0;
		u8 event_index  = 0;
		EsifFpcAlgorithmPtr algo_ptr = (EsifFpcAlgorithmPtr)primitivePtr;
		EsifFpcDomainPtr domainPtr  = NULL;
		struct esif_cpc_event *eventPtr    = NULL;

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "ALGORITHMS:\n\n"
						 "Action Description           tempxform tc1      tc2      pxform timexform\n"
						 "------ --------------------- --------- -------- -------- ------ ---------\n");

		for (algo_index = 0; algo_index < cpc_ptr->number_of_algorithms; algo_index++) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%-2d     %-21s %s(%d) %08x %08x %s(%d) %s(%d)\n",
							 algo_ptr->action_type,
							 esif_action_type_str(algo_ptr->action_type),
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->temp_xform)),
							 algo_ptr->temp_xform,
							 algo_ptr->tempC1,
							 algo_ptr->percent_xform,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->power_xform)),
							 algo_ptr->power_xform,
							 esif_algorithm_type_str((enum esif_algorithm_type)(algo_ptr->time_xform)),
							 algo_ptr->time_xform);
			algo_ptr++;
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "DOMAINS:\n\n"
						 "Name     Description          Domain   Priority Capability Type               \n"
						 "-------- -------------------- -------- -------- ---------- -------------------\n");


		domainPtr = (EsifFpcDomainPtr)algo_ptr;
		for (domainIndex = 0; domainIndex < cpc_ptr->number_of_domains; domainIndex++) {
			char qualifier_str[64];

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%-8s %-20s %-8s %08x %08x   %s(%d)\n",
							 domainPtr->descriptor.name,
							 domainPtr->descriptor.description,
							 esif_primitive_domain_str(domainPtr->descriptor.domain, qualifier_str, 64),
							 domainPtr->descriptor.priority,
							 domainPtr->capability_for_domain.capability_flags,
							 esif_domain_type_str(domainPtr->descriptor.domainType), domainPtr->descriptor.domainType);

			// esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", esif_guid_print((esif_guid_t*) &domainPtr->descriptor.guid, qualifier_str));
			domainPtr++;
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "Mapped Events:\n\n"
						 "Alias Event Identification             Group    Event Data           Event Description\n"
						 "----- -------------------------------- -------- -------------------- --------------------------------------\n");

		eventPtr = (struct esif_cpc_event *)domainPtr;
		for (event_index = 0; event_index < cpc_ptr->number_of_events; event_index++) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "%5s ",
							 eventPtr->name);

			for (i = 0; i < 16; i++) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%02x", eventPtr->event_key[i]);
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 " %-8s %-20s %s\n", 
							 ltrim(esif_event_group_enum_str(eventPtr->esif_group), PREFIX_EVENT_GROUP),
							 esif_data_type_str(eventPtr->esif_group_data_type),
							 ltrim(esif_event_type_str(eventPtr->esif_event), PREFIX_EVENT_TYPE));

			eventPtr++;
		}

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</cpcinfo>\n");
	}
exit:
	// Cleanup
	esif_ccb_free(cpc_ptr);
	IOStream_Destroy(io_ptr);
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(key);
	EsifData_Destroy(value);
	return output;
}


// Load
static char *esif_shell_cmd_load(EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char **argv    = shell->argv;
	char *output   = shell->outbuf;
	char *filename = 0;
	char outline[MAX_LINE]={0};
	int outline_len;
	char *ptr;
	int opt       = 1;
	char load_full_path[PATH_STR_LEN]={0};
	int cat       = 0;
	char *checkDV = 0;	// Default Namespace to check ("_dsp", "_cmd", etc), if any
	IOStreamPtr io_ptr = IOStream_Create();

	UNREFERENCED_PARAMETER(output);

	if (argc <= opt || io_ptr == NULL) {
		IOStream_Destroy(io_ptr);
		return NULL;
	}

	// Deduce cat and path options based on command
	if (esif_ccb_stricmp(argv[0], "cat") == 0) {
		cat  = 1;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_CMD, NULL, NULL);
	} 
	else if (esif_ccb_stricmp(argv[0], "cattst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 1;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_DSP, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "load") == 0) {
		cat  = 0;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_CMD, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "loadtst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 0;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_DSP, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "proof") == 0) {
		cat  = 2;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_CMD, NULL, NULL);
	}
	else if (esif_ccb_stricmp(argv[0], "prooftst") == 0) {
		checkDV = ESIF_DSP_NAMESPACE;
		cat     = 2;
		esif_build_path(load_full_path, sizeof(load_full_path), ESIF_PATHTYPE_DSP, NULL, NULL);
	}

	// load file $1 $2 $3 $4 ...
	// Filenameexit
	filename = argv[opt++];

	// Look for Script on disk first then in DataVault (static or file)
	if (checkDV && !esif_ccb_file_exists(filename)) {
		EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, checkDV, 0, ESIFAUTOLEN);
		EsifDataPtr key       = EsifData_CreateAs(ESIF_DATA_STRING, filename, 0, ESIFAUTOLEN);
		EsifDataPtr value     = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		if (nameSpace != NULL && key != NULL && value != NULL && EsifConfigGet(nameSpace, key, value) == ESIF_OK) {
			esif_ccb_strcpy(load_full_path, filename, PATH_STR_LEN);
			IOStream_SetMemory(io_ptr, StoreReadOnly, (BytePtr)value->buf_ptr, value->data_len);
		}
		EsifData_Destroy(nameSpace);
		EsifData_Destroy(key);
		EsifData_Destroy(value);
	}
	if (IOStream_GetType(io_ptr) == StreamNull) {
		esif_ccb_sprintf_concat(PATH_STR_LEN, load_full_path, "%s%s", ESIF_PATH_SEP, filename);
		IOStream_SetFile(io_ptr, StoreReadOnly, load_full_path, "r");
	}
	if (IOStream_Open(io_ptr) == EOK) {
		int run = 1;
		while (run && !g_shell_stopped) {
			if ((IOStream_GetLine(io_ptr, outline, MAX_LINE) == NULL) ||
				(esif_ccb_strlen(outline, MAX_LINE) > sizeof(outline))) {
				break;
			}

			if (0 == cat || 2 == cat) {	/* Load Or Prove */
				// Replace Tokens  ... $1, $2, $3...
				int tok;
				for (tok = 1; tok <= 9 && tok <= argc - opt; tok++) {
					char tokstr[3] = {'$', ((char)tok + '0'), 0};	// "$1".."$9"
					char *replaced = esif_str_replace(outline, tokstr, argv[tok + opt - 1]);
					if (replaced) {
						esif_ccb_strcpy(outline, replaced, MAX_LINE);
						esif_ccb_free(replaced);
					}
				}

				// Replace %dst% with current destination
				if (strstr(outline, "$dst$")) {
					char dst_str[16];
					char *replaced = NULL;
					esif_ccb_sprintf(16, dst_str, "%llu", esif_ccb_handle2llu(g_dst));
					replaced = esif_str_replace(outline, (char *)"$dst$", dst_str);
					if (replaced) {
						esif_ccb_strcpy(outline, replaced, MAX_LINE);
						esif_ccb_free(replaced);
					}
				}
			}

			if (cat) {	// proof or cat NOT Load
				CMD_OUT("%s", outline);
			} else {
				// LOAD
				Bool inQuote = ESIF_FALSE;
				ptr = outline;
				while (*ptr != '\0') {
					if (*ptr == '\"') {
						inQuote = !inQuote;
					}
					if (*ptr == '\r' || *ptr == '\n' || (*ptr == '#' && inQuote)) {
						*ptr = '\0';
					}
					ptr++;
				}

				outline_len = (int)(ptr - outline);
				if (!((esif_ccb_strnicmp(outline, "rem", 3) == 0) && (outline_len > 3) &&
					  ((outline[3] == '\0') || (outline[3] == ' ') || (outline[3] == '\n') || (outline[3] == '\t')))) {
					esif_uf_shell_unlock();
					parse_cmd(outline, ESIF_FALSE, ESIF_TRUE);
					esif_uf_shell_lock();
				}
				output = shell->outbuf = g_outbuf;
				*output = 0;
			}
		}
		IOStream_Close(io_ptr);
	}
	IOStream_Destroy(io_ptr);
	return output;
}


// Log
static char *esif_shell_cmd_log(EsifShellCmdPtr shell)
{
	int argc       = shell->argc;
	char **argv    = shell->argv;
	char *output   = shell->outbuf;
	int arg        = 1;
	char *subcmd   = NULL;
	char *filename = NULL;
	char fullpath[MAX_PATH]={0};
	char *logid    = "shell";
	EsifLogType logtype = ESIF_LOG_SHELL;

	if (argc > 1) {
		subcmd = argv[arg++];
	}

	// Get optional [logid] (use shell log if none specified)
	if (argc >= 3) {
		char *id = argv[arg];
		EsifLogType newtype = EsifLogType_FromString(id);

		// Assume no logid if invalid name
		if (newtype != ESIF_LOG_EVENTLOG || esif_ccb_strnicmp(id, "eventlog", 5)==0) {
			logtype = newtype;
			logid = id;
			arg++;
		}
	}

	// log
	// log list
	if (argc <= 1 || esif_ccb_stricmp(subcmd, "list")==0) {
		EsifLogFile_DisplayList();
	}
	// log scan [pattern]
	else if (esif_ccb_stricmp(subcmd, "scan")==0) {
		esif_ccb_file_enum_t find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
		struct esif_ccb_file ffd = {0};
		char file_path[MAX_PATH] = {0};
		char *log_pattern = "*.log";

		if (argc > arg) {
			log_pattern = argv[arg++];
		}
		esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_LOG, NULL, NULL);

		// Find matching files and return in Text or XML format
		if (g_format==FORMAT_XML) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s", "<logs>\n");
		}
		if ((find_handle = esif_ccb_file_enum_first(file_path, log_pattern, &ffd)) != ESIF_INVALID_FILE_ENUM_HANDLE) {
			do {
				if (esif_ccb_strcmp(ffd.filename, ".")==0 || esif_ccb_strcmp(ffd.filename, "..")==0) {
					continue; // Ignore . and ..
				}
				if (g_format==FORMAT_XML) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "<name>%s</name>\n", ffd.filename);
				}
				else {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", ffd.filename);
				}
			} while (esif_ccb_file_enum_next(find_handle, log_pattern, &ffd));
			esif_ccb_file_enum_close(find_handle);
		}
		if (g_format==FORMAT_XML) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s", "</logs>\n");
		}
	}
	// log close [logid]
	else if (esif_ccb_stricmp(subcmd, "close")==0) {
		if (!EsifLogFile_IsOpen(logtype)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log not open\n", logid);
		}
		else {
			EsifLogFile_Close(logtype);
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log closed\n", logid);
		}
	}
	// log <filename>
	// log open [logid] <filename> [append]
	else if (argc == 2 || (argc > arg && esif_ccb_stricmp(subcmd, "open")==0)) {
		char *fname = NULL;
		int append = ESIF_FALSE;
		if (argc > 2)
			filename = argv[arg++];
		else if (esif_ccb_stricmp(subcmd, "open") == 0)
			return NULL;
		else
			filename = subcmd;

		// Replace %DATETIME% in filename with yyyy-mm-dd-hhmmss
		if (esif_ccb_strstr(filename, "%DATETIME%") != NULL) {
			char datetime[20] = {0};
			time_t now = time(NULL);
			struct tm time = {0};
			if (esif_ccb_localtime(&time, &now) == 0) {
				esif_ccb_sprintf(sizeof(datetime), datetime, "%04d-%02d-%02d-%02d%02d%02d",
					time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
				fname = esif_str_replace(filename, "%DATETIME%", datetime);
				filename = fname;
			}
		}

		if (argc > arg && esif_ccb_stricmp(argv[arg], "append")==0) {
			append = ESIF_TRUE;
		}

		EsifLogFile_Open(logtype, filename, append);
		EsifLogFile_GetFullPath(fullpath, sizeof(fullpath), filename);
		if (!EsifLogFile_IsOpen(logtype)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log: ERROR opening %s\n", logid, fullpath);
		}
		else {
			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log opened: %s\n", logid, fullpath);
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<result>\n"
						 "  <logid>%s</logid>\n"
						 "  <logfilename>%s</logfilename>\n"
						 "  </result>",logid, fullpath);
			}
		}
		esif_ccb_free(fname);
	}
	// log write [logid] "string"
	else if (argc > arg && (esif_ccb_stricmp(subcmd, "write")==0 || esif_ccb_stricmp(subcmd, "msg")==0)) {
		char *msg = argv[arg++];

		if (!EsifLogFile_IsOpen(logtype)) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log not opened\n", logid);
		}
		else {
			EsifLogFile_Write(logtype, "%s\n", msg);
		}
	}
	// log {flush | noflush} [logid]
	else if (esif_ccb_stricmp(subcmd, "flush") == 0 || esif_ccb_stricmp(subcmd, "noflush") == 0) {
		Bool option = (esif_ccb_stricmp(subcmd, "flush") == 0 ? ESIF_TRUE : ESIF_FALSE);
		EsifLogFile_AutoFlush(logtype, option);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s log autoflush = %s\n", logid, (option ? "ON" : "OFF"));
	}
	// unknown subcmd
	else {
		return NULL;
	}
	return output;
}

// nolog
static char *esif_shell_cmd_nolog(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);

	return parse_cmd("log close", ESIF_FALSE, ESIF_FALSE);
}

// Memstats
static char *esif_shell_cmd_memstats(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc *ipc_ptr = NULL;
	u32 i = 0;
	struct esif_command_get_memory_stats *data_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_memory_stats);
	struct esif_ipc_command *command_ptr = NULL;
	u32 reset = 0;

	// Input Validation
	if (argc > 1) {
		if (strcmp(argv[1], "reset") == 0) {
			reset = 1;
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS));
			goto exit;
		}
	}

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_MEMORY_STATS;
	command_ptr->req_data_type   = ESIF_DATA_UINT32;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 4;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	// Command
	*(u32 *)(command_ptr + 1) = reset;
	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// Our data
	data_ptr = (struct esif_command_get_memory_stats *)(command_ptr + 1);
	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nGlobal Memory Stats: \n"
						 "-----------------------\n"
						 "MemAllocs: %u\n"
						 "MemFrees:  %u\n"
						 "MemInuse:  %u\n\n",
						 data_ptr->stats.allocs,
						 data_ptr->stats.frees,
						 data_ptr->stats.allocs - data_ptr->stats.frees);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<memstats>\n"
						 "  <global>\n"
						 "    <memAllocs>%u</memAllocs>\n"
						 "    <memFrees>%u</memFrees>\n"
						 "    <memInuse>%u</memInuse>\n"
						 "    <memPoolObjFrees>%u</memPoolObjFrees>\n"
						 "    <memPoolAllocs>%u</memPoolAllocs>\n"
						 "    <memPoolFrees>%u</memPoolFrees>\n"
						 "    <memPoolInuse>%u</memPoolInuse>\n"
						 "    <memPoolObjAllocs>%u</memPoolObjAllocs>\n"
						 "    <memPoolObjFrees>%u</memPoolObjFrees>\n"
						 "    <memPoolObjInuse>%u</memPoolObjInuse>\n"
						 "  </global>\n",
						 data_ptr->stats.allocs,
						 data_ptr->stats.frees,
						 data_ptr->stats.allocs - data_ptr->stats.frees,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolAllocs,
						 data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolAllocs - data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolObjAllocs,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolObjAllocs - data_ptr->stats.memPoolObjFrees);
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "Memory Pools:\n"
						 "Name                      Tag  Size Allocs       Frees        Inuse        Bytes    \n"
						 "------------------------- ---- ---- ------------ ------------ ------------ ---------\n");
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <mempools>\n");
	}

	for (i = 0; i < ESIF_MEMPOOL_TYPE_MAX; i++) {
		char tag[8] = {0};
		esif_ccb_memcpy(tag, &data_ptr->mempool_stat[i].pool_tag, 4);
		if (0 == data_ptr->mempool_stat[i].pool_tag) {
			continue;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%-25s %s %-4d %-12u %-12u %-12u %-9u\n",
							 data_ptr->mempool_stat[i].name,
							 tag,
							 data_ptr->mempool_stat[i].object_size,
							 data_ptr->mempool_stat[i].alloc_count,
							 data_ptr->mempool_stat[i].free_count,
							 data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count,
							 (data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count) *
							 data_ptr->mempool_stat[i].object_size);
		} else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "    <mempool>\n"
							 "        <name>%s</name>\n"
							 "        <tag>%s</tag>\n"
							 "        <size>%d</size>\n"
							 "        <allocs>%d</allocs>\n"
							 "        <frees>%d</frees>\n"
							 "        <inuse>%d</inuse>\n"
							 "        <bytes>%d</bytes>\n"
							 "    </mempool>\n",
							 data_ptr->mempool_stat[i].name,
							 tag,
							 data_ptr->mempool_stat[i].object_size,
							 data_ptr->mempool_stat[i].alloc_count,
							 data_ptr->mempool_stat[i].free_count,
							 data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count,
							 (data_ptr->mempool_stat[i].alloc_count - data_ptr->mempool_stat[i].free_count) *
							 data_ptr->mempool_stat[i].object_size);
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "\n"
						 "MemPoolObjFrees:  %u\n"
						 "MemPoolAllocs:    %u\n"
						 "MemPoolFrees:     %u\n"
						 "MemPoolInuse:     %u\n"
						 "MemPoolObjAllocs: %u\n"
						 "MemPoolObjFrees:  %u\n"
						 "MemPoolObjInuse:  %u\n\n",
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolAllocs,
						 data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolAllocs - data_ptr->stats.memPoolFrees,
						 data_ptr->stats.memPoolObjAllocs,
						 data_ptr->stats.memPoolObjFrees,
						 data_ptr->stats.memPoolObjAllocs - data_ptr->stats.memPoolObjFrees);
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"  </mempools>\n"
			"</memstats>\n");
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

static char* esif_shell_cmd_addpart(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char** argv = shell->argv;
	char* output = shell->outbuf;
	esif_error_t rc = ESIF_OK;

	EsifParticipantIface newParticipantData;
	esif_guid_t newParticipantClassGuid = ESIF_PARTICIPANT_CONJURE_CLASS_GUID;
	enum esif_participant_enum enumerator = ESIF_PARTICIPANT_ENUM_CONJURE;
	char newParticipantName[ESIF_NAME_LEN] = { 0 };
	char newParticipantDesc[ESIF_DESC_LEN] = { 0 };
	char newParticipantDriverName[ESIF_NAME_LEN] = "N/A";
	char newParticipantDeviceName[ESIF_NAME_LEN] = { 0 }; //HID
	char newParticipantDevicePath[ESIF_PATH_LEN] = "N/A";
	char newParticipantObjectId[ESIF_SCOPE_LEN] = { 0 };  //scope
	const eEsifParticipantOrigin origin = eParticipantOriginUF;
	UInt32 newParticipantAcpiType = 0;
	esif_handle_t newInstance = ESIF_INVALID_HANDLE;
	int guidElementCounter = 0;
	esif_flags_t newParticipantFlags = DYNAMIC_PARTICIPANT_FLAGS;

	// Input Validation
	if ((argc < 5) ||
		(!shell_isstring(argv[1], ESIF_TRUE, ESIF_NAME_LEN)) ||
		(!shell_isstring(argv[2], ESIF_FALSE, ESIF_DESC_LEN)) ||
		(!shell_isstring(argv[3], ESIF_TRUE, ESIF_NAME_LEN)) ||
		(!shell_isnumber(argv[4])) ||
		(argc > 5 && !shell_isnumber(argv[5]))) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Usage: addpart <name> \"Description\" <HID> <ptype> [flags]\n");
		goto exit;
	}

	esif_ccb_strcpy(newParticipantName, argv[1], ESIF_NAME_LEN);
	esif_ccb_strcpy(newParticipantDesc, argv[2], ESIF_DESC_LEN);
	esif_ccb_strcpy(newParticipantDeviceName, argv[3], ESIF_NAME_LEN); //HID
	esif_ccb_sprintf(ESIF_SCOPE_LEN, newParticipantObjectId, "\\_UP_.%s", newParticipantName);

	newParticipantAcpiType = esif_atoi(argv[4]);
	if (argc > 5) {
		newParticipantFlags = esif_atoi(argv[5]);
	}

	for (guidElementCounter = 0; guidElementCounter < ESIF_GUID_LEN; guidElementCounter++) {
		newParticipantData.class_guid[guidElementCounter] = *(newParticipantClassGuid + guidElementCounter);
	}

	newParticipantData.version = DYNAMIC_PARTICIPANT_VERSION;
	newParticipantData.enumerator = enumerator;
	newParticipantData.flags = newParticipantFlags;
	newParticipantData.send_event = NULL;
	newParticipantData.recv_event = NULL;
	newParticipantData.acpi_type = newParticipantAcpiType;
	esif_ccb_strncpy(newParticipantData.name, newParticipantName, ESIF_NAME_LEN);
	esif_ccb_strncpy(newParticipantData.desc, newParticipantDesc, ESIF_DESC_LEN);
	esif_ccb_strncpy(newParticipantData.object_id, newParticipantObjectId, ESIF_SCOPE_LEN);
	esif_ccb_strncpy(newParticipantData.device_path, newParticipantDevicePath, ESIF_PATH_LEN);
	esif_ccb_strncpy(newParticipantData.driver_name, newParticipantDriverName, ESIF_NAME_LEN);
	esif_ccb_strncpy(newParticipantData.device_name, newParticipantDeviceName, ESIF_NAME_LEN);

	if (EsifUpPm_DoesAvailableParticipantExistByName(newParticipantName)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Participant %s already created.\n", newParticipantName);
	}
	else if ((rc = EsifUpPm_RegisterParticipant(origin, &newParticipantData, &newInstance)) == ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Participant %s created.\n", newParticipantName);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error Creating Participant %s (%s)\n", newParticipantName, esif_rc_str(rc));
	}

exit:
	return output;
}


// Add a conjured kernel participant - addpartk
static char *esif_shell_cmd_addpartk(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc_command *cmdPtr = NULL;
	struct esif_command_participant_create *reqDataPtr = NULL;
	struct esif_ipc_event_data_create_participant *dataPtr = NULL;
	UInt32 dataLen = sizeof(*reqDataPtr);
	struct esif_ipc *ipcPtr = NULL;
	esif_guid_t guid = ESIF_PARTICIPANT_CONJURE_CLASS_GUID;
	char name[ESIF_NAME_LEN] = { 0 };
	char desc[ESIF_DESC_LEN] = { 0 };
	char  acpiDevice[ESIF_SCOPE_LEN] = { 0 };
	u32 vid = 0;
	u32 did = 0;
	u32 ptype = 0;

	// Input Validation
	if ((argc < 6) ||
		(!shell_isstring(argv[1], ESIF_TRUE, 5)) ||
		(!shell_isstring(argv[2], ESIF_TRUE, sizeof(name))) ||
		(!shell_isstring(argv[3], ESIF_FALSE, sizeof(desc))) ||
		(!shell_isstring(argv[4], ESIF_TRUE, sizeof(acpiDevice)) && !shell_isnumber(argv[4])) ||
		(!shell_isnumber(argv[5]))) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Usage:\taddpartk PCI <name> <desc> <vid> <did>\n\taddpartk ACPI <name> <desc> <hid> <ptype>\n");
		goto exit;
	}

	esif_ccb_strcpy(name, argv[2], sizeof(name));
	esif_ccb_strcpy(desc, argv[3], sizeof(desc));

	// addpartk PCI <name> <desc> <vid> <did>
	if (!esif_ccb_stricmp(argv[1], "PCI")) {
		vid = esif_atoi(argv[4]);
		did = esif_atoi(argv[5]);
	}
	// addpartk ACPI <name> <desc> <hid> <ptype>
	else if (!esif_ccb_stricmp(argv[1], "ACPI")) {
		esif_ccb_strcpy(acpiDevice, argv[4], sizeof(acpiDevice));
		ptype = esif_atoi(argv[5]);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Unsupported command.\n");
		goto exit;
	}

	ipcPtr = esif_ipc_alloc_command(&cmdPtr, dataLen);

	if ((NULL == ipcPtr) || (NULL == cmdPtr)) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"esif_ipc_alloc_command failed for %u bytes\n",
			dataLen);
		goto exit;
	}

	//
	// Initialize the command portion of the data
	//
	cmdPtr->type = ESIF_COMMAND_TYPE_PARTICIPANT_CREATE;
	cmdPtr->req_data_type = ESIF_DATA_STRUCTURE;
	cmdPtr->req_data_offset = 0;
	cmdPtr->req_data_len = dataLen;
	cmdPtr->rsp_data_type = ESIF_DATA_VOID;
	cmdPtr->rsp_data_offset = 0;
	cmdPtr->rsp_data_len = 0;

	//
	// Initialize the data payload to create the participant
	//
	reqDataPtr = (struct esif_command_participant_create *)(cmdPtr + 1);
	dataPtr = &reqDataPtr->creation_data;

	dataPtr->version = ESIF_PARTICIPANT_VERSION;
	dataPtr->enumerator = ESIF_PARTICIPANT_ENUM_CONJURE;
	esif_ccb_memcpy(dataPtr->class_guid, &guid, sizeof(dataPtr->class_guid));

	esif_ccb_strcpy(dataPtr->name, name, sizeof(dataPtr->name));
	esif_ccb_strcpy(dataPtr->desc, desc, sizeof(dataPtr->desc));

	esif_ccb_strcpy(dataPtr->acpi_device, acpiDevice, sizeof(dataPtr->acpi_device));
	esif_ccb_sprintf(sizeof(dataPtr->acpi_scope), dataPtr->acpi_scope, "\\_LP_.%s", name);
	dataPtr->acpi_type = ptype;

	dataPtr->pci_vendor = vid;
	dataPtr->pci_device = did;

	if (EsifUpPm_DoesAvailableParticipantExistByName(name)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Participant %s already created.\n", name);
	}
	else {
		rc = ipc_execute(ipcPtr);
		if (rc != ESIF_OK || cmdPtr->return_code != ESIF_OK) {
			rc = (rc != ESIF_OK ? rc : cmdPtr->return_code);
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"Failure creating kernel participant %s; err = %s(%d)\n",
				name, esif_rc_str(rc), rc);
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Kernel Participant %s created.\n", name);
		}
	}
exit:
	esif_ipc_free(ipcPtr);
	return output;
}


// Removes a Conjured Kernel Participant - delpartk
static char *esif_shell_cmd_delpartk(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;

	if (argc < 2) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Too few parameters\n");
		goto exit;
	}

	rc = EsifUpPm_DestroyConjuredLfParticipant(argv[1]);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"Failure deleting kernel participant %s; err = %s(%d)\n",
			argv[1], esif_rc_str(rc), rc);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Kernel Participant %s deleted.\n", argv[1]);
	}
exit:
	return output;
}


// Participant
static char *esif_shell_cmd_participantk(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_part_detail *data_ptr = NULL;
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_ipc *ipc_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_part_detail);
	u32 participant_id = 0;

	if (argc < 2) {
		return NULL;
	}
	
	participant_id = esif_atoi(argv[1]);

	ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_PARTICIPANT_DETAIL;
	command_ptr->req_data_type   = ESIF_DATA_UINT32;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 4;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	// ID For Command
	*(u32 *)(command_ptr + 1) = participant_id;
	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// our data
	data_ptr = (struct esif_command_get_part_detail *)(command_ptr + 1);
	if (0 == data_ptr->version) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: participant not available\n", ESIF_FUNC);
		goto exit;
	}

	// Parse
	if (FORMAT_TEXT == g_format) {
		char guid_str[ESIF_GUID_PRINT_SIZE];
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nInstance:          %d\n"
						 "Version:           %d\n"
						 "Enumerator:        %d\n"
						 "Name:              %s\n"
						 "Desc:              %s\n"
						 "Driver Name:       %s\n"
						 "Device Name:       %s\n"
						 "Device Path:       %s\n"
						 "Class:             %s\n"
						 "Flags:             0x%08x\n"
						 "Status:            %s(%d)\n"
						 "Work Timer Period: %d\n\n",
						 data_ptr->id,
						 data_ptr->version,
						 data_ptr->enumerator,
						 data_ptr->name,
						 data_ptr->desc,
						 data_ptr->driver_name,
						 data_ptr->device_name,
						 data_ptr->device_path,
						 esif_guid_print((esif_guid_t *)data_ptr->class_guid, guid_str),
						 data_ptr->flags,
						 esif_lp_state_str((enum esif_lp_state)data_ptr->state),
						 data_ptr->state,
						 data_ptr->timer_period);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "ACPI Attributes\n"
						 "--------------------------------------------------------------------\n"
						 "Device:      %s\n"
						 "Scope:       %s\n"
						 "Unique ID:   %s\n"
						 "Type:        0x%08x\n\n",
						 data_ptr->acpi_device,
						 data_ptr->acpi_scope,
						 data_ptr->acpi_uid,
						 data_ptr->acpi_type);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
						 "PCI Attributes\n"
						 "--------------------------------------------------------------------\n"
						 "Vendor:      0x%08x\n"
						 "Device:      0x%08x\n"
						 "Bus:         0x%02x\n"
						 "Bus Device:  0x%02x\n"
						 "Function:    0x%02x\n"
						 "Revision:    0x%02x\n"
						 "Class:       0x%02x\n"
						 "SubClass:    0x%02x\n"
						 "ProgIF:      0x%02x\n\n",
						 data_ptr->pci_vendor,
						 data_ptr->pci_device,
						 data_ptr->pci_bus,
						 data_ptr->pci_bus_device,
						 data_ptr->pci_function,
						 data_ptr->pci_revision,
						 data_ptr->pci_class,
						 data_ptr->pci_sub_class,
						 data_ptr->pci_prog_if);

		if (data_ptr->capability > 0) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "Participant Capabilities: 0x%08x                                  \n"
							 "--------------------------------------------------------------------\n",
							 data_ptr->capability);

			if (data_ptr->capability & ESIF_CAPABILITY_ACTIVE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "ACTIVE_CONTROL\t\tHas Fan Features\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CORE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "CORE_CONTROL\t\t\tHas CPU/Core Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_CORE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "CORE_CONTROL\t\t\tHas CPU/Core Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_DISPLAY_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "DISPLAY_CONTROL\t\t\tHas Display Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_PERF_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "PERF_CONTROL\t\tHas Performance Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_POWER_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "POWER_CONTROL\t\tHas RAPL Power Controls\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_POWER_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "POWER_STATUS\t\t\t\tHas RAPL Power Feature\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_TEMP_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "TEMP_STATUS\t\tHas Temp Sensor\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_TEMP_THRESHOLD) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "TEMP_THRESHOLD\t\tHas Temp Thresholds\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_UTIL_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "UTIL_STATUS\t\t\tReports Device Utilization\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_RFPROFILE_STATUS) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "RFPROFILE_STATUS\t\t\tHas RF Profile Status\n");
			}

			if (data_ptr->capability & ESIF_CAPABILITY_RFPROFILE_CONTROL) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "RFPROFILE_CONTROL\t\t\tHas RF Profile Control\n");
			}

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		}

		if (1 == data_ptr->have_dsp) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "Device Support Package(DSP):                  \n"
							 "------------------------------------------------\n"
							 "Code:         %s\n"
							 "Content Ver:  %d.%d\n"
							 "Type:         %s\n\n",
							 data_ptr->dsp_code,
							 data_ptr->dsp_ver_major,
							 data_ptr->dsp_ver_minor,
							 "Compact Primitive Catalog (CPC)");
		}
		
		if (1 == data_ptr->have_cpc) {
			u8 *byte_ptr = (u8 *)&data_ptr->cpc_signature;
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							 "Compact Primitive Catalog(CPC):               \n"
							 "------------------------------------------------\n"
							 "Version:          %d\n"
							 "Signature:        %08X (%c%c%c%c)\n"
							 "Size:             %d\n"
							 "Primitive Count:  %d\n\n",
							 data_ptr->cpc_version,
							 data_ptr->cpc_signature,
							 *byte_ptr, *(byte_ptr + 1), *(byte_ptr + 2), *(byte_ptr + 3),
							 data_ptr->cpc_size,
							 data_ptr->cpc_primitive_count);
		}
	} else {// FORMAT_XML
		char sig_str[10] = {0};
		u16 capability_str_len    = 1024;
		char capability_str[1024] = {0};
		char guid_str[ESIF_GUID_PRINT_SIZE];
		esif_ccb_strcpy(sig_str, "NA", 10);

		if (data_ptr->cpc_signature > 0) {
			u8 *ptr = (u8 *)&data_ptr->cpc_signature;
			esif_ccb_sprintf(10, sig_str, "%c%c%c%c", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3));
		}

		if (data_ptr->capability & ESIF_CAPABILITY_TEMP_STATUS) {
			esif_ccb_sprintf_concat(capability_str_len, capability_str, "TEMP_SENSOR,");
		}
		if (data_ptr->capability & ESIF_CAPABILITY_ACTIVE_CONTROL) {
			esif_ccb_sprintf_concat(capability_str_len, capability_str, "COOLING_DEVICE,");
		}
		if (data_ptr->capability & ESIF_CAPABILITY_POWER_STATUS) {
			esif_ccb_sprintf_concat(capability_str_len, capability_str, "RAPL_DEVICE");
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<participant>\n"
						 "  <id>%d</id>\n"
						 "  <name>%s</name>\n"
						 "  <desc>%s</desc>\n"
						 "  <class>%s</class>\n"
						 "  <version>%d</version>\n"
						 "  <status>%d</status>\n"
						 "  <statusStr>%s</statusStr>\n"
						 "  <timer>%d</timer>\n"
						 "  <capabilities>%08x</capabilities>\n"
						 "  <capabilitiesStr>%s</capabilitiesStr>\n"
						 "  <dspCode>%s</dspCode>\n"
						 "  <dspContentVersion>%d.%d</dspContentVersion>\n"
						 "  <dspType>%s</dspType>\n"
						 "  <cpcVersion>%d</cpcVersion>\n"
						 "  <cpcSignature>0x%08x</cpcSignature>\n"
						 "  <cpcSignatureStr>%s</cpcSignatureStr>\n"
						 "  <cpcSize>%d</cpcSize>\n"
						 "  <cpcPrimitiveCount>%d</cpcPrimitiveCount>\n"
						 "</participant>\n",
						 data_ptr->id,
						 data_ptr->name,
						 data_ptr->desc,
						 esif_guid_print((esif_guid_t *)data_ptr->class_guid, guid_str),
						 data_ptr->version,
						 data_ptr->state,
						 esif_lp_state_str((enum esif_lp_state)data_ptr->state),
						 data_ptr->timer_period,
						 data_ptr->capability,
						 capability_str,
						 data_ptr->dsp_code,
						 data_ptr->dsp_ver_major,
						 data_ptr->dsp_ver_minor,
						 "Compact Primitive Catalog",
						 data_ptr->cpc_version,
						 data_ptr->cpc_signature,
						 sig_str,
						 data_ptr->cpc_size,
						 data_ptr->cpc_primitive_count);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

static char *esif_shell_cmd_paths(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	char targetFilePath[MAX_PATH] = { 0 };
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DV, NULL, 0);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "Datavault path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DPTF, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "DPTF path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DLL, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Policy path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DLL_ALT, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Alternate Policy path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DPTF, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Combined.xsl path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_UI, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "UI path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_DSP, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "DSP path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_LOG, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Log path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_CMD, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Command path:\n %s \n\n", targetFilePath);
	esif_build_path(targetFilePath, sizeof(targetFilePath), ESIF_PATHTYPE_BIN, NULL, 0);
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Bin Files path:\n %s \n\n", targetFilePath);
	return output;
}

// Participant
static char *esif_shell_cmd_participant(EsifShellCmdPtr shell)
{
	int argc             = shell->argc;
	char **argv          = shell->argv;
	char *output         = shell->outbuf;
	esif_handle_t participant_id = ESIF_INVALID_HANDLE;
	EsifUpPtr upPtr     = NULL;
	EsifUpDataPtr metaPtr = NULL;

	if (argc < 2) {
		return NULL;
	}

	// participant <add|create> <parameters>
	if (esif_ccb_stricmp(argv[1], "add") == 0 || esif_ccb_stricmp(argv[1], "create") == 0) {
		esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
		esif_participant_enum_t enumerator = (esif_participant_enum_t)(-1);
		int arg = 2;

		// participant add <name> "desc" <HID> <ptype> [flags]
		if ((esif_ccb_stricmp(argv[1], "add") == 0) && (argc >= 6)) {
			enumerator = ESIF_PARTICIPANT_ENUM_CONJURE;
			rc = ESIF_OK;
		}
		// participant create ACPI <name> "desc" <HID> <ptype>
		// participant create PCI  <name> "desc" <VID> <DID>
		// participant create CONJURE <name> "desc" <HID> <ptype>
		else if ((esif_ccb_stricmp(argv[1], "create") == 0) && (argc >= 7)) {
			enumerator = esif_participant_enum_str2enum(argv[arg]);
			switch (enumerator) {
			case ESIF_PARTICIPANT_ENUM_ACPI:
			case ESIF_PARTICIPANT_ENUM_PCI:
			case ESIF_PARTICIPANT_ENUM_CONJURE:
				rc = ESIF_OK;
				arg++;
				break;
			default:
				break;
			}
		}

		if (rc == ESIF_OK) {
			char keyname[MAX_PATH] = { 0 };
			char partname[ESIF_ACPI_NAME_LEN] = { 0 };

			esif_ccb_strcpy(partname, argv[arg], sizeof(partname));
			esif_ccb_strupr(partname, sizeof(partname));
			esif_ccb_sprintf(sizeof(keyname), keyname, "%s%s", DYNAMIC_PARTICIPANTS_KEYSPACE, partname);

			// See if Participant already exists
			upPtr = EsifUpPm_GetAvailableParticipantByName(partname);
			esif_handle_t upInstance = EsifUp_GetInstance(upPtr);
			EsifUp_PutRef(upPtr);
			upPtr = NULL;
			if (upInstance != ESIF_INVALID_HANDLE || DataBank_KeyExists(DYNAMIC_PARTICIPANTS_DATAVAULT, keyname)) {
				rc = ESIF_E_IO_ALREADY_EXISTS;
			}
			else {

				EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, DYNAMIC_PARTICIPANTS_DATAVAULT, 0, ESIFAUTOLEN);
				EsifDataPtr key = EsifData_CreateAs(ESIF_DATA_STRING, keyname, 0, ESIFAUTOLEN);
				EsifDataPtr value = NULL;

				// Create JSON Object String from Participant Data
				IStringPtr jsonPart = NULL;
				switch (enumerator) {
				case ESIF_PARTICIPANT_ENUM_ACPI:
					jsonPart = CreateJsonFromParticipantData(enumerator, partname, argv[arg + 1], argv[arg + 2], argv[arg + 3], NULL);
					break;
				case ESIF_PARTICIPANT_ENUM_PCI:
					jsonPart = CreateJsonFromParticipantData(enumerator, partname, argv[arg + 1], argv[arg + 2], argv[arg + 3], NULL);
					break;
				case ESIF_PARTICIPANT_ENUM_CONJURE:
					jsonPart = CreateJsonFromParticipantData(enumerator, partname, argv[arg + 1], argv[arg + 2], argv[arg + 3], (arg == 2 && argc > arg + 4 ? argv[arg + 4] : NULL));
					break;
				default:
					rc = ESIF_E_INVALID_REQUEST_TYPE;
					break;
				}

				if (jsonPart) {
					value = EsifData_CreateAs(ESIF_DATA_JSON, esif_ccb_strdup(IString_GetString(jsonPart)), ESIFAUTOLEN, ESIFAUTOLEN);
				}

				// Persist to DataVault
				if (nameSpace == NULL || key == NULL || value == NULL || value->buf_ptr == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					rc = EsifConfigSet(nameSpace, key, ESIF_SERVICE_CONFIG_PERSIST, value);
				}
				EsifData_Destroy(nameSpace);
				EsifData_Destroy(key);
				EsifData_Destroy(value);

				// Create Dynamic Participant from JSON string
				if (rc == ESIF_OK) {
					rc = CreateParticipantFromJson(IString_GetString(jsonPart));
				}
				IString_Destroy(jsonPart);
			}
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		goto exit;
	}

	// participant <delete|destroy> <name>
	if ((esif_ccb_strnicmp(argv[1], "delete", 3) == 0) || (esif_ccb_stricmp(argv[1], "destroy") == 0)) {
		esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
		if (argc >= 3) {
			char partname[ESIF_ACPI_NAME_LEN] = { 0 };
			enum esif_participant_enum enumerator = ESIF_PARTICIPANT_ENUM_INVALID;

			esif_ccb_strcpy(partname, argv[2], sizeof(partname));
			esif_ccb_strupr(partname, sizeof(partname));

			// participant destroy <name> = Destroy Participant only
			if (esif_ccb_stricmp(argv[1], "destroy") == 0) {
				// Synchronously Destroy Participant so name can be reused
				rc = EsifUpPm_DestroyParticipant(partname);
			}
			// participant delete <name> = Delete Persisted Dynamic Participant and Destroy Participant
			else {
				char keyname[MAX_PATH] = { 0 };
				esif_ccb_sprintf(sizeof(keyname), keyname, "%s%s", DYNAMIC_PARTICIPANTS_KEYSPACE, partname);

				EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, DYNAMIC_PARTICIPANTS_DATAVAULT, 0, ESIFAUTOLEN);
				EsifDataPtr key = EsifData_CreateAs(ESIF_DATA_STRING, keyname, 0, ESIFAUTOLEN);

				// Delete from DataVault
				if (nameSpace == NULL || key == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else if (DataBank_KeyExists((StringPtr)nameSpace->buf_ptr, keyname) == ESIF_FALSE) {
					rc = ESIF_E_NOT_FOUND;
				}
				else {
					rc = EsifConfigDelete(nameSpace, key);
				}
				EsifData_Destroy(nameSpace);
				EsifData_Destroy(key);

				//
				// Destroy participant if conjured
				//
				if (ESIF_OK == rc) {
					upPtr = EsifUpPm_GetAvailableParticipantByName(partname);
					if (upPtr != NULL) {
						enumerator = EsifUp_GetEnumerator(upPtr);
						EsifUp_PutRef(upPtr);
						upPtr = NULL;

						if (ESIF_PARTICIPANT_ENUM_CONJURE == enumerator) {
							// Synchronously Destroy Participant so name can be reused
							rc = EsifUpPm_DestroyParticipant(partname);
						}
					}
				}
			}
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		goto exit;
	}

	// Lookup Participant by ID or Name
	if (shell_isnumber(argv[1])) {
		participant_id = (esif_handle_t)esif_atoi64(argv[1]);
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(participant_id);
	}
	else {
		upPtr = EsifUpPm_GetAvailableParticipantByName(argv[1]);
		participant_id = EsifUp_GetInstance(upPtr);
	}

	if (NULL == upPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Participant not available\n");
		goto exit;
	}

	metaPtr = EsifUp_GetMetadata(upPtr);
	if (NULL != metaPtr) {
		// Parse
		if (FORMAT_TEXT == g_format) {
			char guid_str[ESIF_GUID_PRINT_SIZE];
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"\n"
				"Instance:          " ESIF_HANDLE_FMT "\n"
				"Version:           %d\n"
				"Enumerator:        %d\n"
				"Name:              %s\n"
				"Desc:              %s\n"
				"Driver Name:       %s\n"
				"Device Name:       %s\n"
				"Device Path:       %s\n"
				"Class:             %s\n"
				"Ptype:             %s (%d)\n"
				"Flags:             0x%08x\n"
				"Status:            %s(%d)\n",
				esif_ccb_handle2llu(participant_id),
				metaPtr->fVersion,
				metaPtr->fEnumerator,
				metaPtr->fName,
				metaPtr->fDesc,
				metaPtr->fDriverName,
				metaPtr->fDeviceName,
				metaPtr->fDevicePath,
				esif_guid_print((esif_guid_t *)metaPtr->fDriverType, guid_str),
				esif_domain_type_str(metaPtr->fAcpiType),
				metaPtr->fAcpiType,
				metaPtr->fFlags,
				esif_pm_participant_state_str((enum esif_pm_participant_state)1),
				0);

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"\n"
				"ACPI Attributes\n"
				"--------------------------------------------------------------------\n"
				"Device:      %s\n"
				"Scope:       %s\n"
				"Unique ID:   %s\n"
				"Type:        0x%08x\n",
				metaPtr->fAcpiDevice,
				metaPtr->fAcpiScope,
				metaPtr->fAcpiUID,
				metaPtr->fAcpiType);

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"\n"
				"PCI Attributes\n"
				"--------------------------------------------------------------------\n"
				"Vendor:      0x%08x\n"
				"Device:      0x%08x\n"
				"Bus:         0x%02x\n"
				"Bus Device:  0x%02x\n"
				"Function:    0x%02x\n"
				"Revision:    0x%02x\n"
				"Class:       0x%02x\n"
				"SubClass:    0x%02x\n"
				"ProgIF:      0x%02x\n\n",
				metaPtr->fPciVendor,
				metaPtr->fPciDevice,
				metaPtr->fPciBus,
				metaPtr->fPciBusDevice,
				metaPtr->fPciFunction,
				metaPtr->fPciRevision,
				metaPtr->fPciClass,
				metaPtr->fPciSubClass,
				metaPtr->fPciProgIf);
		}
		else {// FORMAT_XML
			char guid_str[ESIF_GUID_PRINT_SIZE];
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"<participant>\n"
				"  <id>%llu</id>\n"
				"  <enum>%d</enum>\n"
				"  <name>%s</name>\n"
				"  <desc>%s</desc>\n"
				"  <class>%s</class>\n"
				"  <version>%d</version>\n"
				"  <status>%d</status>\n"
				"  <statusStr>%s</statusStr>\n"
				"  <acpiDevice>%s</acpiDevice>\n"
				"  <acpiScope>%s</acpiScope>\n"
				"  <acpiUID>%s</acpiUID>\n"
				"  <acpiType>0x%08x</acpiType>\n"
				"  <pciVendor>0x%08x</pciVendor>\n"
				"  <pciDevice>0x%08x</pciDevice>\n"
				"  <pciBus>0x%02x</pciBus>\n"
				"  <pciBusDevice>0x%02x</pciBusDevice>\n"
				"  <pciFunction>0x%02x</pciFunction>\n"
				"  <pciRevision>0x%02x</pciRevision>\n"
				"  <pciClass>0x%02x</pciClass>\n"
				"  <pciSubClass>0x%02x</pciSubClass>\n"
				"  <pciProgIF>0x%02x</pciProgIF>\n"
				"</participant>\n",
				esif_ccb_handle2llu(participant_id),
				metaPtr->fEnumerator,
				metaPtr->fName,
				metaPtr->fDesc,
				esif_guid_print((esif_guid_t *)metaPtr->fDriverType, guid_str),
				metaPtr->fVersion,
				1,
				esif_pm_participant_state_str((enum esif_pm_participant_state)1),
				metaPtr->fAcpiDevice,
				metaPtr->fAcpiScope,
				metaPtr->fAcpiUID,
				metaPtr->fAcpiType,
				metaPtr->fPciVendor,
				metaPtr->fPciDevice,
				metaPtr->fPciBus,
				metaPtr->fPciBusDevice,
				metaPtr->fPciFunction,
				metaPtr->fPciRevision,
				metaPtr->fPciClass,
				metaPtr->fPciSubClass,
				metaPtr->fPciProgIf);
		}
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return output;
}


// RemARK
static char *esif_shell_cmd_rem(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	// Do Nothing
	return NULL;
}


// Repeat
/*static*/ char *esif_shell_cmd_repeat(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	if (argc < 2) {
		return NULL;
	}
	// repeat <count>
	else if (argc == 2) {
		g_repeat = esif_atoi(argv[1]);
		g_repeat = esif_ccb_min(esif_ccb_max(g_repeat, 0), MAX_REPEAT);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "repeat next command count=%d times\n", g_repeat);
	}
	// repeat <count> command args ...
	else {
		size_t buflen = 0;
		int opt = 1;
		int count = 0;
		char *cmd = NULL;
		int repeat = esif_atoi(argv[opt]);
		repeat = esif_ccb_min(esif_ccb_max(repeat, 0), MAX_REPEAT);

		for (opt = 2; opt < argc; opt++) {
			buflen += esif_ccb_strlen(argv[opt], OUT_BUF_LEN) + 3;
		}
		cmd = esif_ccb_malloc(buflen);
		if (cmd == NULL)
			return NULL;

		for (count = 0; (count < repeat) && !g_shell_stopped; count++) {
			esif_ccb_memset(cmd, 0, buflen);
			for (opt = 2; opt < argc; opt++) {
				esif_ccb_sprintf_concat(buflen, cmd, "\"%s\"", argv[opt]);
				if (opt + 1 < argc)
					esif_ccb_strcat(cmd, " ", buflen);
			}
			parse_cmd(cmd, ESIF_FALSE, ESIF_TRUE);
		}
		esif_ccb_free(cmd);

		// Output has already been shown by parse_cmd subfunctions
		*output = '\0';
	}
	return output;
}


// Repeat Delay
/*static*/ char *esif_shell_cmd_repeatdelay(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_repeat_delay = esif_atoi(argv[1]);
	g_repeat_delay = esif_ccb_min(esif_ccb_max(g_repeat_delay, 0), MAX_REPEAT_DELAY);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "repeat delay = %d ms between each repeated command\n", g_repeat_delay);
	return output;
}

// Set Buffer
static char *esif_shell_cmd_setb(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	if (argc < 2) {
		return NULL;
	}

	g_binary_buf_size = esif_atoi(argv[1]);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "binary_buf_size=%d\n", g_binary_buf_size);
	return output;
}


// Set Error level
static char *esif_shell_cmd_seterrorlevel(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Input Validation
	if (argc > 1) {
		if (shell_isnumber(argv[1])) {
			g_errorlevel = esif_atoi(argv[1]);
			esif_ccb_sprintf(OUT_BUF_LEN, output, "seterrorlevel = %d\n", g_errorlevel);
			return output;
		}
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	return output;
}


// Set Primitive
static char *esif_shell_cmd_setp(EsifShellCmdPtr shell)
{
	int argc            = shell->argc;
	char **argv         = shell->argv;
	char *output        = shell->outbuf;
	enum esif_rc rc     = ESIF_OK;
	int opt = 1;
	EsifString desc     = ESIF_NOT_AVAILABLE;
	u32 id = 0;
	u16 qualifier       = EVENT_MGR_DOMAIN_D0;
	char *qualifier_str = NULL;
	u8 instance         = 0;
	char *suffix        = "";
	enum esif_data_type primitiveDataType = ESIF_DATA_AUTO;
	EsifDataPtr dataPtr = NULL;
	char *dataStr = NULL;
	struct esif_data request = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data response = {ESIF_DATA_VOID, NULL, 0};
	struct esif_data *sarPtr = NULL;
	EsifPrimitiveActionSelector actSelector = { 0 };
	char *targetParticipant = NULL;
	esif_handle_t participantId = g_dst;

	if (argc < MIN_PARAMETERS_FOR_SET_PRIMITIVE) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Deduce suffix from command
	if (esif_ccb_stricmp(argv[0], "setp") == 0) {
		suffix = "";
	} else if (esif_ccb_stricmp(argv[0], "setp_t") == 0) {
		suffix = &argv[0][4];
	} else if (esif_ccb_stricmp(argv[0], "setp_pw") == 0) {
		suffix = &argv[0][4];
	} else if (esif_ccb_stricmp(argv[0], "setp_bf") == 0 || esif_ccb_stricmp(argv[0], "setp_bs") == 0) {
		suffix = &argv[0][4];
	} else if (esif_ccb_stricmp(argv[0], "setp_part") == 0) {
		//now first parameter should be participant
		if (argc < MIN_PARAMETERS_FOR_SET_PRIMITIVE + 1) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		suffix = "";
		targetParticipant = argv[opt++];
		rc = esif_shell_get_participant_id(targetParticipant, &participantId);
		if (rc != ESIF_OK) {
			goto exit;
		}
	}

	// Primitive ID or Name
	id = (isdigit(argv[opt][0]) ? esif_atoi(argv[opt++]) : (u32)esif_primitive_type_str2enum(argv[opt++]));

	// Qualifier ID
	qualifier_str = argv[opt++];
	qualifier     = domain_str_to_short(qualifier_str);

	// Instance ID
	instance = (u8)esif_atoi(argv[opt++]);

	// Data
	rc = EsifPrimitiveGetDataType(participantId,
			id,
			qualifier_str,
			instance,
			ESIF_PRIMITIVE_OP_SET,
			&primitiveDataType);
	if (rc != ESIF_OK) {
		if (ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP == rc) {
			rc = ESIF_E_PRIMITIVE_SUR_NOT_FOUND_IN_DSP;
		}
		goto exit;
	}

	dataPtr = EsifData_Create();
	if (dataPtr == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/* Read from a File in BIN directory for setp_bf */
	dataStr = argv[opt++];
	if (esif_ccb_strnicmp(suffix, "_b", 2) == 0) {
		char full_path[MAX_PATH] = { 0 };
		char *filename = dataStr;
		int errnum = 0;
		FILE *fp = NULL;
		union esif_data_variant header = { 0 };
		UInt32 offset = 0;
		char *sep = NULL;

		// setp_bf <id> <domain> <instance> [datatype:]filename = load file and prepend with esif_variant header
		if ((sep = esif_ccb_strstr(filename, ":")) != NULL) {
			*sep++ = 0;
			EsifDataType datatype = esif_data_type_str2enum(filename);
			header.type = ((datatype == ESIF_DATA_VOID && esif_ccb_stricmp(filename, "VOID") != 0) ? ESIF_DATA_BINARY : datatype);
			offset = (UInt32)sizeof(header);
			filename = sep;
		}

		// Read File from specified location
		rc = DataVault_TranslatePath(filename, full_path, sizeof(full_path));
		if (rc == ESIF_OK) {
			if ((fp = esif_ccb_fopen(full_path, FILE_READ, &errnum)) == NULL) {
				rc = ESIF_E_NOT_FOUND;
			}
			else {
				struct stat file_stat = { 0 };
				esif_ccb_stat(full_path, &file_stat);
				dataPtr->buf_ptr = esif_ccb_malloc(offset + file_stat.st_size);
				if (dataPtr->buf_ptr == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else if (file_stat.st_size < 1 || esif_ccb_fread((u8 *)dataPtr->buf_ptr + offset, file_stat.st_size, 1, file_stat.st_size, fp) != (size_t)file_stat.st_size) {
					rc = ESIF_E_IO_ERROR;
				}
				else {
					if (offset) {
						header.integer.value = (u64)file_stat.st_size;
						esif_ccb_memcpy(dataPtr->buf_ptr, &header, offset);
					}
					dataPtr->type = primitiveDataType;
					dataPtr->buf_len = file_stat.st_size + offset;
					dataPtr->data_len = file_stat.st_size + offset;
					rc = ESIF_OK;
				}
				esif_ccb_fclose(fp);
			}
		}
	}
	else {
		rc = EsifData_FromString(dataPtr, dataStr, primitiveDataType);
	}

	if (rc != ESIF_OK) {
		goto exit;
	}
	if (primitiveDataType != ESIF_DATA_VOID) {
		if (NULL == dataPtr->buf_ptr) {
			goto exit;
		}
		/*
		 * We make a copy of the data for the request because the data can get
		 * transformed which will result in nonsense values being displayed after
		 * the primitive is executed
		 */
		request.buf_len = dataPtr->buf_len;
		request.data_len = dataPtr->data_len;
		request.type = dataPtr->type;
		request.buf_ptr = esif_ccb_malloc(request.buf_len);
		if (NULL == request.buf_ptr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		esif_ccb_memcpy(request.buf_ptr, dataPtr->buf_ptr, request.buf_len);
	}


	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s ", esif_primitive_str((enum esif_primitive_type)id));

	// Verify this is a SET
	if (EsifPrimitiveVerifyOpcode(
			participantId,
			id,
			qualifier_str,
			instance,
			ESIF_PRIMITIVE_OP_SET) == ESIF_FALSE) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Primitive (%d) not a SET: %s\n", id, esif_primitive_str(id));
		rc = ESIF_E_INVALID_REQUEST_TYPE;
		goto exit;
	}

	// Optional - Action selector
	if (opt < argc) {
		rc = EsifParseSpecificActionArg(argv[opt++], &actSelector);
		if (rc != ESIF_OK) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Action option invalid\n");
			goto exit;
		}
	}

	sarPtr = EsifCreateSpecificActionRequest(
		id,
		qualifier_str,
		instance,
		&request,
		&response,
		&actSelector);
	if (NULL == sarPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed to create specific action request\n");
		goto exit;
	}

	rc = EsifExecutePrimitive(
		participantId,
		SET_SPECIFIC_ACTION_PRIMITIVE,
		"D0",
		255,
		sarPtr,
		NULL);
	if (ESIF_OK != rc) {
		g_errorlevel = 6;
		goto exit;
	}

	switch (request.type) {
	case ESIF_DATA_UINT32:
		desc = "";
		break;

	case ESIF_DATA_TEMPERATURE:
		desc = "Degrees C";
		break;

	case ESIF_DATA_POWER:
		desc = "MilliWatts";
		break;

	case ESIF_DATA_STRING:
		desc = "ASCII";
		break;

	case ESIF_DATA_BINARY:
		desc = "BINARY";
		break;

	case ESIF_DATA_TABLE:
		desc = "TABLE";
		break;

	case ESIF_DATA_PERCENT:
		desc = "Centi-Percent";
		break;

	case ESIF_DATA_FREQUENCY:
		desc = "Frequency(Hz)";
		break;

	case ESIF_DATA_TIME:
		desc = "Time (ms)";
		break;

	default:
		desc = "";
		break;
	}

	switch (dataPtr->type) {
	case ESIF_DATA_STRING:
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = %s %s\n", esif_primitive_str(
			(enum esif_primitive_type)id), suffix, id, qualifier_str, instance, (char *) dataPtr->buf_ptr, desc);
		break;
	case ESIF_DATA_TEMPERATURE:
		if (dataPtr->buf_len >= sizeof(esif_temp_t)) {
			esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, dataPtr->buf_ptr);

			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = %.1f %s\n", esif_primitive_str(
				(enum esif_primitive_type)id), suffix, id, qualifier_str, instance, (*(Int32 *) dataPtr->buf_ptr) / 10.0, desc);
		}	
		break;
	default:
		if (esif_data_type_sizeof(dataPtr->type) == sizeof(UInt32) && dataPtr->buf_ptr != NULL) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = 0x%08x %u %s\n", esif_primitive_str(
				(enum esif_primitive_type)id), suffix, id, qualifier_str, instance, *(UInt32 *)dataPtr->buf_ptr, *(UInt32 *)dataPtr->buf_ptr, desc);
		}
		else if (esif_data_type_sizeof(dataPtr->type) == sizeof(UInt64) && dataPtr->buf_ptr != NULL) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = 0x%016llx %llu %s\n", esif_primitive_str(
				(enum esif_primitive_type)id), suffix, id, qualifier_str, instance, *(UInt64 *)dataPtr->buf_ptr, *(UInt64 *)dataPtr->buf_ptr, desc);
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s setp%s(%03u,%s,%03d) value = N/A %s Data (%u bytes)\n", esif_primitive_str(
				(enum esif_primitive_type)id), suffix, id, qualifier_str, instance, ltrim(esif_data_type_str(dataPtr->type), PREFIX_DATA_TYPE), dataPtr->data_len);
		}
		break;
	}

exit:
	if (ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP == rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Required primitive %s %03u.D0.255 not found in DSP\n",
			esif_primitive_str((enum esif_primitive_type)SET_SPECIFIC_ACTION_PRIMITIVE), SET_SPECIFIC_ACTION_PRIMITIVE);
	} else if (rc != ESIF_OK) {
		if (ESIF_E_PRIMITIVE_SUR_NOT_FOUND_IN_DSP == rc) {
			rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error code = %s(%d)\n",
			esif_rc_str(rc), rc);
	}
	esif_ccb_free(sarPtr);
	esif_ccb_free(request.buf_ptr);
	EsifData_Destroy(dataPtr);
	return output;
}


// Reset/remove an override value in the DV's
// Syntax: rstp <id> [qualifier] [instance]
//         rstp_part <handle|name> <id> [qualifier] [instance]
//
static char *esif_shell_cmd_reset_override(EsifShellCmdPtr shell)
{
	int argc            = shell->argc;
	char **argv         = shell->argv;
	char *output        = shell->outbuf;
	enum esif_rc rc     = ESIF_OK;
	EsifPrimitiveTupleParameter tuple = {0};
	struct esif_data request = { ESIF_DATA_BINARY, &tuple, sizeof(tuple), sizeof(tuple) };
	int opt = 1;
	u32 id = 0;
	u16 qualifier       = EVENT_MGR_DOMAIN_D0;
	char *qualifier_str = "D0";
	u8 instance         = 255;
	char domainStr[8] = "";
	char *targetParticipant = NULL;
	esif_handle_t participantId = g_dst;
	
	if (argc < 2) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (esif_ccb_stricmp(argv[0], "rstp_part") == 0) {
		//now first parameter should be participant
		if (argc < MIN_PARAMETERS_FOR_GET_PRIMITIVE + 1) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		
		targetParticipant = argv[opt++];
		rc = esif_shell_get_participant_id(targetParticipant, &participantId);
		if (rc != ESIF_OK) {
			goto exit;
		}
	}

	// Primitive ID or Name
	id = (isdigit(argv[opt][0]) ? esif_atoi(argv[opt++]) : (u32)esif_primitive_type_str2enum(argv[opt++]));

	// Qualifier ID
	if (opt < argc) {
		qualifier_str = argv[opt++];
	}
	qualifier = domain_str_to_short(qualifier_str);

	// Instance ID
	if (opt < argc) {
		instance = (u8)esif_atoi(argv[opt++]);
	}

	tuple.id.integer.value = id;
	tuple.domain.integer.value = qualifier;
	tuple.instance.integer.value = instance;

	rc = EsifExecutePrimitive(participantId,
		SET_CONFIG_RESET,
		esif_primitive_domain_str((u16)ESIF_PRIMITIVE_DOMAIN_D0, domainStr, sizeof(domainStr)),
		255,
		&request,
		NULL);
exit:
	if (rc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error code = %s(%d)\n",
			esif_rc_str(rc), rc);
	}
	return output;
}


#define TRACELEVEL_ALL (-1)

// Convert string to trace level
int cmd_trace_parse_level(char *str)
{
	static struct {int level; char *name;} trace_levels[] = {
		{ESIF_TRACELEVEL_FATAL, "FATAL"},
		{ESIF_TRACELEVEL_ERROR, "ERROR"},
		{ESIF_TRACELEVEL_WARN,  "WARN"},
		{ESIF_TRACELEVEL_WARN,  "WARNING"},
		{ESIF_TRACELEVEL_INFO,  "INFO"},
		{ESIF_TRACELEVEL_DEBUG, "DEBUG"},
		{TRACELEVEL_ALL,		"ALL"},
		{0, NULL}
	};
	int result = ESIF_TRACELEVEL_DEFAULT;
	
	if (isdigit(*str)) {
		result = esif_atoi(str);
		result = esif_ccb_min(result, g_traceLevel_max);
	}
	else {
		int j;
		for (j = 0; trace_levels[j].name != NULL; j++) {
			if (esif_ccb_stricmp(trace_levels[j].name, str) == 0) {
				result = trace_levels[j].level;
				break;
			}
		}
	}
	return result;
}

// Convert string(s) to trace module mask
esif_tracemask_t cmd_trace_parse_mask(int argc, char **argv)
{
	esif_tracemask_t bitmask=0;
	int j=0;

	// Convert each argument to route and add it to the bitmask
	for (j=0; j < argc; j++) {
		enum esif_tracemodule module = (enum esif_tracemodule)0;
		esif_tracemask_t thismask=0;
		int invert = 0;
		char *name = argv[j];

		// ~MODULE
		if (*name == '~') {
			name++;
			invert = 1;
		}

		// ALL = All modules
		if (esif_ccb_stricmp(name, "ALL") == 0) {
			thismask = ESIF_TRACEMASK_ALL;
		}
		else {
			module = EsifTraceModule_FromString(name);
			if (module == (enum esif_tracemodule)0 && esif_ccb_stricmp(argv[j], "DEFAULT") != 0) {
				continue;
			}
			thismask = ((esif_tracemask_t)1 << (int)(module));
		}

		if (invert) {
			bitmask = ~thismask;
		}
		else {
			bitmask |= thismask;
		}
	}

	// If no bitmask specified, assume it is a  hex bitmask
	if (bitmask == 0) {
		esif_ccb_sscanf(argv[0], esif_tracemask_fmtx, &bitmask);
	}
	return bitmask;
}

// Convert string(s) to trace route mask
esif_traceroute_t cmd_trace_parse_route(int argc, char **argv)
{
	static struct { esif_traceroute_t bit; char *name; } trace_routes[] = {
		{ ESIF_TRACEROUTE_CONSOLE,	"CON" },
		{ ESIF_TRACEROUTE_CONSOLE,	"CONSOLE" },
		{ ESIF_TRACEROUTE_EVENTLOG,	"EVT" },
		{ ESIF_TRACEROUTE_EVENTLOG,	"EVENTLOG" },
		{ ESIF_TRACEROUTE_DEBUGGER,	"DBG" },
		{ ESIF_TRACEROUTE_DEBUGGER,	"DEBUGGER" },
		{ ESIF_TRACEROUTE_LOGFILE,	"LOG" },
		{ ESIF_TRACEROUTE_CONSOLE|ESIF_TRACEROUTE_EVENTLOG|ESIF_TRACEROUTE_DEBUGGER|ESIF_TRACEROUTE_LOGFILE, "ALL" },
		{ 0, "NONE" },
		{ 0, NULL}
	};
	int j=0;
	int k=0;
	esif_traceroute_t bitmask=0;

	// Convert each argument to route and add it to the bitmask
	for (j=0; j < argc; j++) {
		for (k=0; trace_routes[k].name != NULL; k++) {
			if (esif_ccb_stricmp(trace_routes[k].name, argv[j]) == 0) {
				bitmask |= trace_routes[k].bit;
			}
			else if (argv[j][0] == '~' && esif_ccb_stricmp(trace_routes[k].name, &argv[j][1]) == 0) {
				bitmask = ~(trace_routes[k].bit);
			}
		}
	}

	// If no bitmask specified, assume it is a  hex bitmask
	if (bitmask == 0) {
		unsigned int tmpmask=0;
		esif_ccb_sscanf(argv[0], "%x", &tmpmask);
		bitmask = (esif_traceroute_t)tmpmask;
	}
	return bitmask;
}

// Trace
char *esif_shell_cmd_trace(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char newCmd[MAX_LINE] = {0};
	char *shellbuf = NULL;
	size_t shellbuf_len = 0;

	// trace <level>
	// trace level <level>
	if ((argc == 2 && isdigit(argv[1][0])) || (argc > 2 && esif_ccb_stricmp(argv[1], "level")==0)) {
		UInt32 verbosity = 0;
		EsifData verbosity_data = {ESIF_DATA_UINT32,
								   &verbosity,
								   sizeof(verbosity),
								   sizeof(verbosity)};
		int level = (u8)cmd_trace_parse_level((argc == 2 ? argv[1] : argv[2]));

		// Set new trace level and notify DPTF
		g_traceLevel = esif_ccb_min(g_traceLevel_max, level);
		verbosity = (UInt32)g_traceLevel;
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_LOG_VERBOSITY_CHANGED, &verbosity_data);
		EsifWebSetTraceLevel(g_traceLevel);
		CMD_OUT("\ntrace level %d\n", g_traceLevel);
	}
	// trace module <level> <bitmask>
	// trace module <level> [<module> ...]
	else if (argc > 3 && (esif_ccb_stricmp(argv[1], "module")==0 || esif_ccb_stricmp(argv[1], "mask")==0)) {
		int level = cmd_trace_parse_level(argv[2]);
		esif_tracemask_t bitmask = cmd_trace_parse_mask(argc-3, &argv[3]);

		if (level == TRACELEVEL_ALL) {
			int j;
			for (j=0; j <= g_traceLevel_max; j++)
				g_traceinfo[j].modules = bitmask;
		}
		else
			g_traceinfo[level].modules = bitmask;

		EsifWebSetTraceLevel(g_traceLevel);
	}
	// trace route <level> <bitmask>
	// trace route <level> [<route> ...]
	else if (argc > 3 && esif_ccb_stricmp(argv[1], "route")==0) {
		int level = cmd_trace_parse_level(argv[2]);
		esif_traceroute_t bitmask = cmd_trace_parse_route(argc-3, &argv[3]);

		if (level == TRACELEVEL_ALL) {
			int j;
			for (j=0; j <= g_traceLevel_max; j++)
				g_traceinfo[j].routes = bitmask;
		}
		else
			g_traceinfo[level].routes = bitmask;
	}
	// trace test [level]
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "test")==0) {
		int level=-1;
		if (argc > 2)
			level = cmd_trace_parse_level(argv[2]);
		if (argc==2 || level==ESIF_TRACELEVEL_ERROR)	ESIF_TRACE_ERROR("ERROR Message(1)");
		if (argc==2 || level==ESIF_TRACELEVEL_WARN)		ESIF_TRACE_WARN("WARN Message(2)");
		if (argc==2 || level==ESIF_TRACELEVEL_INFO)		ESIF_TRACE_INFO("INFO Message(3)");
		if (argc==2 || level==ESIF_TRACELEVEL_DEBUG)	ESIF_TRACE_DEBUG("DEBUG Message(4)");
	}
	// trace log [ [open] <file> [append] | [close] | [write] <msg> ]
	else if (argc > 2 && esif_ccb_stricmp(argv[1], "log")==0) {
		char *file = argv[2];

		// trace log close
		if (esif_ccb_stricmp(file, "close")==0) {
			esif_ccb_sprintf(sizeof(newCmd), newCmd, "log close trace");
			return parse_cmd(newCmd, ESIF_FALSE, ESIF_FALSE);
		}
		// trace log write <msg>
		if (argc > 3 && esif_ccb_stricmp(file, "write")==0) {
			char *msg = argv[3];
			size_t msglen = esif_ccb_strlen(msg, OUT_BUF_LEN) + MAX_CTIME_LEN;
			char *timestamp_msg = (char *)esif_ccb_malloc(msglen);
			time_t now=0;
			char timestamp[MAX_CTIME_LEN]={0};

			if (timestamp_msg) {
				time(&now);
				esif_ccb_ctime(timestamp, sizeof(timestamp), &now);
				esif_ccb_sprintf(msglen, timestamp_msg, "\"%.16s%s\"", timestamp + 4, msg);
				esif_ccb_sprintf(sizeof(newCmd), newCmd, "log write trace %s", timestamp_msg);
				esif_ccb_free(timestamp_msg);
			}
			return parse_cmd(newCmd, ESIF_FALSE, ESIF_FALSE);
		}
		// trace log open <file> [append]
		else if (argc > 3 && esif_ccb_stricmp(file, "open")==0) {
			file = argv[3];
		}
		if (argc > 4) {
			esif_ccb_sprintf(sizeof(newCmd), newCmd, "log open trace %s %s", file, argv[4]);
		} else {
			esif_ccb_sprintf(sizeof(newCmd), newCmd, "log open trace %s", file);
		}
		return parse_cmd(newCmd, ESIF_FALSE, ESIF_FALSE);
	}
	// trace nolog
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "nolog")==0) {
		esif_ccb_sprintf(sizeof(newCmd), newCmd, "log close trace %s", argv[3]);
		return parse_cmd(newCmd, ESIF_FALSE, ESIF_FALSE);
	}
#ifdef ESIF_ATTR_MEMTRACE
	// trace leak
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "leak") == 0) {
		// create intentional memory leak for debugging
		u32 *memory_leak = (u32*)esif_ccb_malloc(sizeof(*memory_leak));
		UNREFERENCED_PARAMETER(memory_leak);
	}
	// trace free_leaks
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "free_leaks") == 0) {
		// Memory leaks pointer are freed during reporint to verify pointers
		g_memtrace.free_leaks = ESIF_TRUE;
	}
	// trace allow_leaks
	else if (argc > 1 && esif_ccb_stricmp(argv[1], "allow_leaks") == 0) {
		// Memory leaks pointer are NOT freed during reporint to verify pointers
		g_memtrace.free_leaks = ESIF_FALSE;
	}
#endif

	//////////////////

	// trace
	if (g_format == FORMAT_XML)
	{
		char *targetStr = "NA";
		char *currentTraceLevelStr = "NA";
		esif_shell_sprintf(&shellbuf_len, &shellbuf, "<result>\n");
		int j;
		for (j = 0; j <= g_traceLevel_max; j++) {
			if (g_traceLevel == j)
			{
				currentTraceLevelStr = (char *)"*";
			}
			else
			{
				currentTraceLevelStr = (char *)" ";
			}

			if (g_traceinfo[j].routes) {
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_CONSOLE) {
					targetStr = (char *)"CON";
				}
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_EVENTLOG) {
					targetStr = (char *)"EVT";
				}
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_DEBUGGER) {
					targetStr = (char *)"DBG";
				}
				if (g_traceinfo[j].routes & ESIF_TRACEROUTE_LOGFILE) {
					targetStr = (char *)"LOG";
				}
			}
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
				"    <Level>%d</Level>\n"
				"    <Label>%s %s</Label>\n"
				"    <Modules>0x%08X</Modules>\n"
				"    <Routes>0x%02X</Routes>\n"
				"    <Targets>%s</Targets>\n",
				g_traceinfo[j].level,
				currentTraceLevelStr,
				g_traceinfo[j].label,
				g_traceinfo[j].modules,
				g_traceinfo[j].routes,
				targetStr);
		}
		esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "</result>\n");
	}
	else
	{
		if (argc > 0) {
			int j;
			CMD_OUT("\nLevel\tLabel\tModules    Routes Targets\n");
			CMD_OUT("-----\t-----\t---------- ------ --------------------------------------------\n");
			for (j = 0; j <= g_traceLevel_max; j++) {
				int k;
				CMD_OUT("%s %d\t%s\t0x%08X 0x%02X   ", (g_traceLevel == j ? "*" : " "), g_traceinfo[j].level,
					g_traceinfo[j].label, g_traceinfo[j].modules, g_traceinfo[j].routes);
				if (g_traceinfo[j].routes) {
					if (g_traceinfo[j].routes & ESIF_TRACEROUTE_CONSOLE) {
						CMD_OUT("CON ");
					}
					if (g_traceinfo[j].routes & ESIF_TRACEROUTE_EVENTLOG) {
						CMD_OUT("EVT ");
					}
					if (g_traceinfo[j].routes & ESIF_TRACEROUTE_DEBUGGER) {
						CMD_OUT("DBG ");
					}
					if (g_traceinfo[j].routes & ESIF_TRACEROUTE_LOGFILE) {
						CMD_OUT("LOG%s ", (EsifLogFile_IsOpen(ESIF_LOG_TRACE) ? "" : "(?)"));
					}
				}
				CMD_OUT("<< ");
				if (g_traceinfo[j].modules == ESIF_TRACEMASK_ALL)
					CMD_OUT("*");
				else {
					for (k = 0; k < ESIF_TRACEMASK_MAX; k++) {
						if (g_traceinfo[j].modules & ((esif_tracemask_t)1 << k)) {
							const char *modstr = EsifTraceModule_ToString((enum esif_tracemodule)k);
							if (modstr != NULL)
								CMD_OUT("%s ", modstr);
						}
					}
				}
				CMD_OUT("\n");
			}
			CMD_OUT("\n");
		}
	}

	if (shellbuf != NULL) {
		output = shell->outbuf = esif_shell_resize(shellbuf_len);
		esif_ccb_strcpy(output, shellbuf, OUT_BUF_LEN);
		esif_ccb_free(shellbuf);
	}

	return output;
}


static char *esif_shell_cmd_set_osc(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	struct esif_data_complex_osc osc = {0};
	struct esif_data_complex_osc *ret_osc;
	const u32 data_len = sizeof(struct esif_data_complex_osc);
	u8 *data_ptr = NULL;
	u32 id;
	struct esif_data request = {ESIF_DATA_VOID};
	struct esif_data response = {ESIF_DATA_VOID};
	char guid_buf[ESIF_GUID_PRINT_SIZE];

	// Use Active For NOW
	esif_guid_t policy_guids[] = {
		{0xd6, 0x41, 0xa4, 0x42, 0x6a, 0xae, 0x2b, 0x46, 0xa8, 0x4b, 0x4a, 0x8c, 0xe7, 0x90, 0x27, 0xd3},
		{0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef}
	};

	if (argc < 3) {
		return NULL;
	}

	// guid_index
	id = esif_atoi(argv[1]);

	// Capabiity
	osc.capabilities = esif_atoi(argv[2]);

	if (id > 1) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: please select guid or fail guid ID\n",
						 ESIF_FUNC);
		goto exit;
	}

	data_ptr = (u8 *)esif_ccb_malloc(data_len);
	if (NULL == data_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ccb_malloc failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	request.type      = ESIF_DATA_STRUCTURE;
	request.buf_len   = data_len;
	request.buf_ptr   = (u8 *)data_ptr;
	request.data_len  = data_len;

	response.type     = ESIF_DATA_STRUCTURE;
	response.buf_len  = data_len;
	response.buf_ptr  = (u8 *)data_ptr;
	response.data_len = 0;

	// Command
	esif_ccb_memcpy(&osc.guid, &policy_guids[id], ESIF_GUID_LEN);
	osc.revision = 1;
	osc.count    = 2;
	esif_ccb_memcpy(data_ptr, &osc, data_len);


	/* Display Command Data */
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: guid: %s revision %d argcount = %d capabilities %08x\n",
					 ESIF_FUNC, esif_guid_print(&osc.guid, guid_buf),
					 osc.revision, osc.count, osc.capabilities);

	rc = EsifExecutePrimitive(
			g_dst,
			SET_OPERATING_SYSTEM_CAPABILITIES,
			"D0",
			255,
			&request,
			&response);

	// Get status in esif_data_complex_osc
	ret_osc = (struct esif_data_complex_osc *)(response.buf_ptr);

	if (ESIF_OK != rc) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s: set_osc error code = %s(%d), status code = %d\n",
						 ESIF_FUNC,
						 esif_rc_str(rc), rc,
						 ret_osc->status);
		g_errorlevel = 6;
		goto exit;
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s: set_osc returned status %u\n",
					 ESIF_FUNC, ret_osc->status);

exit:

	if (response.buf_ptr != data_ptr) {
		esif_ccb_free(response.buf_ptr);
	}
	esif_ccb_free(data_ptr);
	return output;
}

// Test
static char *esif_shell_cmd_test(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_participants *data_ptr = NULL;
	int start_id = 0;
	int stop_id  = 0;
	int i;
	char *buf    = 0;

	if (argc < 2) {
		return NULL;
	}
	buf = argv[1];

	if (!strncmp(buf, "all", 3)) {
		start_id = 0;
	} else {
		start_id = esif_atoi(buf);
		if (start_id > MAX_START_ID) {
			return NULL;
		}
		stop_id  = start_id + 1;
	}

	{
		const u32 data_len = sizeof(struct esif_command_get_participants);
		struct esif_ipc_command *command_ptr = NULL;

		struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
		if (NULL == ipc_ptr || NULL == command_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
							 ESIF_FUNC, data_len);
			goto exit;
		}

		command_ptr->type = ESIF_COMMAND_TYPE_GET_PARTICIPANTS;
		command_ptr->req_data_type   = ESIF_DATA_VOID;
		command_ptr->req_data_offset = 0;
		command_ptr->req_data_len    = 0;
		command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
		command_ptr->rsp_data_offset = 0;
		command_ptr->rsp_data_len    = data_len;

		ipc_execute(ipc_ptr);

		if (ESIF_OK != ipc_ptr->return_code) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
							 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
			goto exit;
		}

		if (ESIF_OK != command_ptr->return_code) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
							 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
			goto exit;
		}

		// Our Data - List Of Participants
		data_ptr = (struct esif_command_get_participants *)(command_ptr + 1);

		if (start_id != stop_id - 1) {
			stop_id = data_ptr->count;
		}

		g_errorlevel = 0;
		// Loop thorugh from start to finish participants


		for (i = start_id; i < stop_id; i++) {
				#define COMMAND_LEN (32 + 1)
			char command[COMMAND_LEN];
			if (data_ptr->participant_info[i].state == ESIF_PM_PARTICIPANT_STATE_CREATED) {
				esif_ccb_sprintf(COMMAND_LEN, command, "dst %d", i);
				parse_cmd(command, ESIF_FALSE, ESIF_TRUE);

				esif_ccb_sprintf(COMMAND_LEN, command, "loadtst %s.tst", data_ptr->participant_info[i].dsp_code);
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Run Test: %s DUT %d\n", command, g_dst);
				parse_cmd(command, ESIF_FALSE, ESIF_TRUE);

				if (g_soe && g_errorlevel != 0) {
					break;
				}
			}
		}
exit:
		if (NULL != ipc_ptr) {
			esif_ipc_free(ipc_ptr);
		}
	}
	return output;
}


// Timestamp Mode
static char *esif_shell_cmd_timestamp(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *ts     = 0;

	if (argc < 2) {
		return NULL;
	}
	ts = argv[1];

	if (!strcmp(ts, "on")) {
		g_timestamp = 1;
	} else {
		g_timestamp = 0;
	}
	esif_ccb_sprintf(OUT_BUF_LEN, output, "timestamp=%d\n", g_timestamp);
	return output;
}


///////////////////////////////////////////////////////////////////////////////
// COMMANDS
///////////////////////////////////////////////////////////////////////////////

// Debug show
static char *esif_shell_cmd_debugshow(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_debug_module_level *data_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_debug_module_level);
	struct esif_ipc_command *command_ptr = NULL;
	u32 i = 0;
	char *state = NULL;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_DEBUG_MODULE_LEVEL;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// our data
	data_ptr = (struct esif_command_get_debug_module_level *)(command_ptr + 1);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "TraceLevel = %d\nModules = 0x%08X\n", data_ptr->tracelevel, data_ptr->modules);

	// Once for each BIT
	for (i = 0; i < 32; i++) {
		u32 bit = 1;
		bit = bit << i;
		if (data_ptr->modules & 0x1) {
			state = (char *)"ENABLED";
		} else {
			state = (char *)"DISABLED";
		}

		data_ptr->modules = data_ptr->modules >> 1;
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Module: %3s(%02u) Bit: 0x%08X State: %8s Level: 0x%08X\n",
						 esif_debug_mod_str((enum esif_debug_mod)i), i, bit, state, data_ptr->levels[i]);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


// About
static char *esif_shell_cmd_about(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	if (g_format == FORMAT_TEXT) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\n"
						 "ESIF - Eco-System Independent Framework\n"
						 "Copyright (c) 2013-2020 Intel Corporation All Rights Reserved\n"
						 "\n"
						 "esif_uf - ESIF Upper Framework (UF) R3\n"
						 "Version:  %s\n"
						 "OS: %s\n"
						 "\n"
						 "ESIF Framework Modules:\n"
						 "ID Module      Description               Version \n"
						 "-- ----------- ------------------------- ----------\n"
						 "1  esif_shell  ESIF Command Interface    %s\n"
						 "\n"
						 "esif_lf - ESIF Lower Framework (LF) R0\n"
						 "ESIF Kernel Driver Information:\n"
						 "Version: %s\n"
						 "\n",
						 ESIF_UF_VERSION,
						 g_os,
						 g_esif_shell_version,
						 g_esif_kernel_version

						 );
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<about>\n"
						 "  <ufVersion>%s</ufVersion>\n"
						 "  <lfVersion>%s</lfVersion>\n"
						 "  <osType>%s</osType>\n"
						 "  <shellVersion>%s</shellVersion>\n"
						 "</about>\n",
						 ESIF_UF_VERSION,
						 g_esif_kernel_version,
						 g_os,
						 g_esif_shell_version);
	}

	return output;
}

// Exit
static char *esif_shell_cmd_exit(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Ignore commands from REST API
	if (g_isRest) {
		return NULL;
	}

	g_disconnectClient = ESIF_TRUE;
#ifndef ESIF_ATTR_OS_WINDOWS
	g_quit = ESIF_TRUE;
#endif

	esif_ccb_sprintf(OUT_BUF_LEN, output, "Exiting...\n");
	return output;
}


static char *esif_shell_cmd_tableobject(EsifShellCmdPtr shell)
{
	int opt = 1;
	esif_handle_t targetParticipantId = ESIF_INVALID_HANDLE;
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *action = NULL;
	char *targetTable = NULL;
	char *targetParticipantName = NULL;
	char *targetDomain = NULL;
	char *targetData = NULL;
	char *dataSource = NULL;
	char *dataMember = NULL;
	size_t targetDataLen = 0;
	TableObject tableObject = {0};
	EsifDataPtr namespacePtr = NULL;
	EsifDataPtr pathPtr = NULL;
	EsifDataPtr responsePtr = NULL;
	eEsifError rc = ESIF_OK;
	enum tableMode mode = GET;

	/* 
	minimum 5 parameters; 
	format: tableobject <action> <tablename> <participant> <domain> [data]
	if the table expects a revision, the input string should be
	<revision number>:<data>, with the revision number occupying a
	char length of REVISION_INDICATOR_LENGTH 
	*/

	if (argc < 5) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Too few parameters \n");
		goto exit;
	}
	
	action = argv[opt++];
	targetTable = argv[opt++];
	targetParticipantName = argv[opt++];
	/* Try to convert targetParticipantId into a number. If this fails because
	it is a name instead of an ID, the result here will be 0.*/
	targetParticipantId = (esif_handle_t)esif_atoi64(targetParticipantName);
	targetDomain = argv[opt++];

	/* Optionally allow the participant name to be used. If we got a 0 for ID,
	it could mean that we were given a name instead. To verify that, check if the
	raw argument we received is "0". If it is not "0", it means that it is a name.
	Note: This will fail on the case where we pass in "0x0" as the raw argument. */ 
	if (targetParticipantId == 0 && esif_ccb_strcmp(targetParticipantName, "0")) {
		EsifUpPtr upPtr = NULL;

		upPtr = EsifUpPm_GetAvailableParticipantByName(targetParticipantName);

		if (upPtr != NULL) {
			targetParticipantId = EsifUp_GetInstance(upPtr);
			EsifUp_PutRef(upPtr);
		}

	}

	/* determine if there is an alternative datasource other than the primitive*/
	if ((esif_ccb_stricmp(action, "get") == 0 && argc > 5) || (esif_ccb_stricmp(action, "set") == 0 && argc > 6)) {
		dataSource = argv[opt++];
		dataMember = argv[opt++];
	}

	/* determine whether we are intending to write data */
	if (esif_ccb_stricmp(action, "set") == 0) {
		if (argc < opt + 1) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Too few parameters \n");
			goto exit;
		}
		targetData = argv[opt++];
		targetDataLen = esif_ccb_strlen(targetData, g_cmdlen);
		mode = SET;
	}
	
	TableObject_Construct(&tableObject, targetTable, targetDomain, dataSource, dataMember, targetData, targetDataLen, targetParticipantId, mode);
	
	rc = TableObject_LoadAttributes(&tableObject); /* properties such as table type (binary/virtual/datavault) */
	rc = TableObject_LoadData(&tableObject); /* determines the version, and in the case of GET will apply binary data */
	rc = TableObject_LoadSchema(&tableObject); /* get fields for table (dependant on version) */

	if (rc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Unable to load schema \n");
		goto exit;
	}
	
	// tableobject get <tablename> <participant> <domain>
	if (esif_ccb_stricmp(action, "get") == 0) {
		rc = TableObject_LoadXML(&tableObject, ESIF_TEMP_DECIK);
		if(ESIF_OK != rc) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Primitive failed.\n");
			goto exit;
		}
		if (tableObject.dataXML != NULL) {
			esif_ccb_strcpy(output, tableObject.dataXML, esif_ccb_strlen(tableObject.dataXML, OUT_BUF_LEN));
		}
	}
	// tableobject set <tablename> <participant> <domain> "comma,delimited,columns!bang,delimited,rows"
	else if (esif_ccb_stricmp(action, "set") == 0) {
		rc = TableObject_Save(&tableObject);
		
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Unable to save table \n");
			goto exit;
		}	
	}
	// tableobject delete <tablename> <participant> <domain>
	else if (esif_ccb_stricmp(action, "delete") == 0  && argc >= opt) {
		if (argc < opt) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Too few parameters \n");
			goto exit;
		}
		
		rc = TableObject_Delete(&tableObject);
		
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Unable to delete table \n");
			goto exit;
		}	
	}

exit:
	TableObject_Destroy(&tableObject);
	EsifData_Destroy(namespacePtr);
	EsifData_Destroy(pathPtr);
	EsifData_Destroy(responsePtr);
	return output;
}


// Get Buffer Size
static char *esif_shell_cmd_getb(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "binary_buf_size=%d\n", g_binary_buf_size);
	return output;
}


// Get Error Level
static char *esif_shell_cmd_geterrorlevel(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (g_errorlevel <= 0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "geterrorlevel = %s(%d)\n", esif_rc_str((enum esif_rc)-(g_errorlevel)), g_errorlevel);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "geterrorlevel = %s(%d)\n", esif_rc_str((enum esif_rc)(g_errorlevel)), g_errorlevel);
	}
	return output;
}


// Command Info
static char *esif_shell_cmd_info(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	enum esif_rc rc = ESIF_OK;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	UNREFERENCED_PARAMETER(rc);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "ESIF_UF GMIN Build Version = %s\n", ESIF_UF_VERSION);
#else
	const u32 data_len = sizeof(struct esif_command_get_kernel_info);
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_command_get_kernel_info *data_ptr = NULL;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_KERNEL_INFO;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	if (((rc = ipc_execute(ipc_ptr)) != ESIF_OK) && (rc != ESIF_E_NO_LOWER_FRAMEWORK)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "ipc execute error = %s(%d)\n", esif_rc_str(rc), rc);
		goto exit;
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// Our Data
	data_ptr = (struct esif_command_get_kernel_info *)(command_ptr + 1);

	if (FORMAT_TEXT == g_format) {
		// ** Do not change this message format unless you also update extract_kernel_version() **
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Kernel Version = %s\n", data_ptr->ver_str);
	} 
	else {// FORMAT_XML
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "<info>\n"
						 "  <kernelVersion>%s</kernelVersion>\n"
						 "</info>\n", data_ptr->ver_str);
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
#endif
	return output;
}


// Backwards compatibility for calling from ipc_connect()
char *esif_cmd_info(char *output)
{
	char *argv[] = {"info", 0};
	EsifShellCmd shell;
	shell.argc   = sizeof(argv) / sizeof(char *);
	shell.argv   = argv;
	shell.outbuf = output;
	return esif_shell_cmd_info(&shell);
}


// Participants
static char *esif_shell_cmd_participantsk(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct esif_command_get_participants *data_ptr = NULL;
	const u32 data_len = sizeof(struct esif_command_get_participants);
	struct esif_ipc_command *command_ptr = NULL;
	u32 i     = 0;
	u32 count = 0;
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: esif_ipc_alloc_command failed for %u bytes\n",
						 ESIF_FUNC, data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_PARTICIPANTS;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: ipc error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s: command error code = %s(%d)\n",
						 ESIF_FUNC, esif_rc_str(command_ptr->return_code), command_ptr->return_code);
		goto exit;
	}

	// out data
	data_ptr = (struct esif_command_get_participants *)(command_ptr + 1);
	count    = data_ptr->count;

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
						 "\nKERNEL PARTICIPANTS:\n\n"
						 "ID   Name     Description                     Enum Ver Status       Active DSP     \n"
						 "---- -------- ------------------------------- ---- --- ------------ -----------------\n");
	} else {// XML
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<participants>\n");
	}


	for (i = 0; i < count; i++) {
		char *enumStr  = (char *)"NA";
		char *state_str = esif_lp_state_str((enum esif_lp_state)data_ptr->participant_info[i].state);

		switch (data_ptr->participant_info[i].bus_enum) {
		case 0:
			enumStr = (char *)"ACPI";
			break;

		case 1:
			enumStr = (char *)"PCI";
			break;

		case 2:
			enumStr = (char *)"PLAT";
			break;

		case 3:
			enumStr = (char *)"CNJR";
			break;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"  %-2d %-8s %-31s %-5s %d  %-12s",
				data_ptr->participant_info[i].id,
				data_ptr->participant_info[i].name,
				data_ptr->participant_info[i].desc,
				enumStr,
				data_ptr->participant_info[i].version,
				state_str);

			if (data_ptr->participant_info[i].state == ESIF_LP_STATE_REGISTERED) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					" %s(%d.%d)\n",
					data_ptr->participant_info[i].dsp_code,
					data_ptr->participant_info[i].dsp_ver_major,
					data_ptr->participant_info[i].dsp_ver_minor);
			} else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
			}
		} else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"<participant>\n"
				"  <id>%d</id>\n"
				"  <name>%s</name>\n"
				"  <desc>%s</desc>\n"
				"  <enum>%d</enum>\n"
				"  <enumStr>%s</enumStr>\n"
				"  <version>%d</version>\n"
				"  <state>%d</state>\n"
				"  <stateStr>%s</stateStr>\n"
				"  <dspCode>%s</dspCode>\n"
				"  <dspVerMajor>%d</dspVerMajor>\n"
				"  <dspVerMinor>%d</dspVerMinor>\n"
				"</participant>\n",
				data_ptr->participant_info[i].id,
				data_ptr->participant_info[i].name,
				data_ptr->participant_info[i].desc,
				data_ptr->participant_info[i].bus_enum,
				enumStr,
				data_ptr->participant_info[i].version,
				data_ptr->participant_info[i].state,
				state_str,
				data_ptr->participant_info[i].dsp_code,
				data_ptr->participant_info[i].dsp_ver_major,
				data_ptr->participant_info[i].dsp_ver_minor);
		}
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	} else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</participants>\n");
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

void strip_illegal_chars(char *srcStringPtr, const char *illegalCharsPtr)
{
	char *resultStringPtr = srcStringPtr;
	
	if (srcStringPtr == NULL || illegalCharsPtr == NULL) {
		return;
	}

	for (resultStringPtr = srcStringPtr; *srcStringPtr != '\0'; srcStringPtr++) {
		if (!esif_ccb_strchr(illegalCharsPtr,*srcStringPtr)) {
			*resultStringPtr++ = *srcStringPtr;
		}
	}
	*resultStringPtr = '\0';
}

// Participants
static char *esif_shell_cmd_participants(EsifShellCmdPtr shell)
{
	eEsifError iterRc = ESIF_OK;
	eEsifError rc = ESIF_OK;
	UfPmIterator upIter = {0};
	EsifUpPtr upPtr = NULL;
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	char *attribute = "";
	u8 domainIndex	= 0;
	EsifDspPtr dspPtr = NULL;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;
	UInt8 lpId = 0;
	EsifUpDataPtr metaPtr = NULL;
	EsifUpDomainPtr domainPtr = NULL;
	UpDomainIterator udIter = { 0 };
	char *shellbuf = NULL;
	size_t shellbuf_len = 0;
	u32 i = 0;
	const char line[] = "--------------------------------";
	size_t max_desc = sizeof(line) - 1;

	// Qualifier
	if (argc > 1) {
		attribute = argv[1];
		if (!strcmp(attribute, "acpi")) {
			max_desc = 12;
		}
		if (!strcmp(attribute, "reload")) {
			CreateDynamicParticipants();
			*output = 0;
			return output;
		}
		if (!strcmp(attribute, "unload")) {
			DestroyDynamicParticipants();
			*output = 0;
			return output;
		}
	}

	if (g_format == FORMAT_XML) {
		esif_shell_sprintf(&shellbuf_len, &shellbuf, "<result>\n");

		iterRc = EsifUpPm_InitIterator(&upIter);
		if (iterRc == ESIF_OK) {
			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}

		while (ESIF_OK == iterRc) {
			char desc[ESIF_DESC_LEN + 3] = "";	/* desc... */
			char *enumStr = "NA";
			char instanceStr[8 + 1]     = "NA";
			u8 domainCount   = 1;
			char *cleanParticipantNamePtr = NULL;
			char *cleanParticipantMetaDesc = NULL;
			char *cleanParticipantDesc = NULL;
			char *cleanParticipantACPIScope = NULL;
			char *cleanParticipantACPIDevice = NULL;
			char *cleanParticipantACPIUID = NULL;
			
			dspPtr = EsifUp_GetDsp(upPtr);
			metaPtr = EsifUp_GetMetadata(upPtr);

			if ((NULL == dspPtr) || (NULL == metaPtr)) {
				iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
				continue;
			}

			domainCount = (UInt8)dspPtr->get_domain_count(dspPtr);

			/* Truncate Description If Large */
			if (ESIF_SHELL_STRLEN(metaPtr->fDesc) > max_desc) {
				esif_ccb_strcpy(desc, metaPtr->fDesc, max_desc - 3);
				esif_ccb_sprintf_concat(ESIF_DESC_LEN, desc, "...");
			} else {
				esif_ccb_strcpy(desc, metaPtr->fDesc, max_desc);
			}

			enumStr = esif_participant_enum_shortname(metaPtr->fEnumerator);

			participantId = EsifUp_GetInstance(upPtr);

			lpId = EsifUp_GetLpInstance(upPtr);
			if (lpId != ESIF_INSTANCE_INVALID) {
				esif_ccb_sprintf(8, instanceStr, "%-2d", lpId);
			}
			
			//create sanitized versions suitable for well-formed xml
			cleanParticipantNamePtr = esif_ccb_strdup(metaPtr->fName);
			if (cleanParticipantNamePtr == NULL) {
				rc = ESIF_E_NO_MEMORY;
				esif_shell_sprintf(&shellbuf_len, &shellbuf, "NO MEMORY\n");
				goto domain_exit;
			}
			cleanParticipantMetaDesc = esif_ccb_strdup(metaPtr->fDesc);
			if (cleanParticipantMetaDesc == NULL) {
				rc = ESIF_E_NO_MEMORY;
				esif_shell_sprintf(&shellbuf_len, &shellbuf, "NO MEMORY\n");
				goto domain_exit;
			}
			cleanParticipantDesc = esif_ccb_strdup(desc);
			if (cleanParticipantDesc == NULL) {
				rc = ESIF_E_NO_MEMORY;
				esif_shell_sprintf(&shellbuf_len, &shellbuf, "NO MEMORY\n");
				goto domain_exit;
			}
			cleanParticipantACPIScope = esif_ccb_strdup(metaPtr->fAcpiScope);
			if (cleanParticipantACPIScope == NULL) {
				rc = ESIF_E_NO_MEMORY;
				esif_shell_sprintf(&shellbuf_len, &shellbuf, "NO MEMORY\n");
				goto domain_exit;
			}
			cleanParticipantACPIDevice = esif_ccb_strdup(metaPtr->fAcpiDevice);
			if (cleanParticipantACPIDevice == NULL) {
				rc = ESIF_E_NO_MEMORY;
				esif_shell_sprintf(&shellbuf_len, &shellbuf, "NO MEMORY\n");
				goto domain_exit;
			}
			cleanParticipantACPIUID = esif_ccb_strdup(metaPtr->fAcpiUID);
			if (cleanParticipantACPIUID == NULL) {
				rc = ESIF_E_NO_MEMORY;
				esif_shell_sprintf(&shellbuf_len, &shellbuf, "NO MEMORY\n");
				goto domain_exit;
			}

			strip_illegal_chars(cleanParticipantNamePtr, g_illegalXmlChars);
			strip_illegal_chars(cleanParticipantMetaDesc, g_illegalXmlChars);
			strip_illegal_chars(cleanParticipantDesc, g_illegalXmlChars);
			strip_illegal_chars(cleanParticipantACPIScope, g_illegalXmlChars);
			strip_illegal_chars(cleanParticipantACPIDevice, g_illegalXmlChars);
			strip_illegal_chars(cleanParticipantACPIUID, g_illegalXmlChars);

			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
				"  <participant>\n"
				"    <UpId>%llu</UpId>\n"
				"    <LpId>%s</LpId>\n"
				"    <name>%s</name>\n"
				"    <shortDesc>%s</shortDesc>\n"
				"    <desc>%s</desc>\n"
				"    <enum>%u</enum>\n"
				"    <enumStr>%s</enumStr>\n"
				"    <version>%u</version>\n"
				"    <activeDsp>%s</activeDsp>\n"
				"    <acpiHID>%s</acpiHID>\n"
				"    <acpiUID>%s</acpiUID>\n"
				"    <acpiScope>%s</acpiScope>\n"
				"    <acpiType>%u</acpiType>\n"
				"    <domainCount>%u</domainCount>\n",
				esif_ccb_handle2llu(participantId),
				instanceStr,
				cleanParticipantNamePtr,
				cleanParticipantMetaDesc,
				cleanParticipantDesc,
				metaPtr->fEnumerator,
				enumStr,
				metaPtr->fVersion,
				dspPtr->code_ptr,
				cleanParticipantACPIDevice,
				cleanParticipantACPIUID,
				cleanParticipantACPIScope,
				metaPtr->fAcpiType,
				domainCount);

			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
				"    <domains>\n");
			

			iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
			if (ESIF_OK != iterRc)
				goto domain_exit;

			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			while (ESIF_OK == iterRc) {
				char guid_buf[ESIF_GUID_PRINT_SIZE];

				if (NULL == domainPtr) {
					iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
					continue;
				}

				esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
					"      <domain>\n"
					"        <id>%d</id>\n"
					"        <version>1</version>\n"
					"        <name>%s</name>\n"
					"        <guid>%s</guid>\n"
					"        <type>%d</type>\n"
					"        <capability>0x%x</capability>\n",
					domainIndex, domainPtr->domainName,
					esif_guid_print((esif_guid_t *) domainPtr->domainGuid, guid_buf),
					domainPtr->domainType,
					domainPtr->capability_for_domain.capability_flags);

				esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
					"        <capability_masks>");
				for (i = 0; i < MAX_CAPABILITY_MASK; i++) {
					if (i != 0) {
						esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, ",");
					}
					esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
						"0x%x",
						domainPtr->capability_for_domain.capability_mask[i]);
				}
				esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
					"</capability_masks>\n"
					"      </domain>\n");

				iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			}

domain_exit:

			esif_ccb_free(cleanParticipantNamePtr);
			esif_ccb_free(cleanParticipantMetaDesc);
			esif_ccb_free(cleanParticipantDesc);
			esif_ccb_free(cleanParticipantACPIScope);
			esif_ccb_free(cleanParticipantACPIDevice);
			esif_ccb_free(cleanParticipantACPIUID);

			if (rc != ESIF_OK) {
				goto exit;
			}
			
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf,
				"    </domains>\n"
				"  </participant>\n");		

			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}

		esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "</result>\n");
		goto exit;
	}

	/* Create appropriate header */
	if (!strcmp(attribute, "acpi")) {
		esif_shell_sprintf(&shellbuf_len, &shellbuf,
			"\n"
			"ALL PARTICIPANTS: ACPI INFORMATION\n"
			"\n"
			"Name  %-*s Enum Ver HID        SCOPE                          UID  Type UPID       LPID \n"
			"----- %-.*s ---- --- ---------- ------------------------------ ---- ---- ---------- ----\n"
			, (int)max_desc, "Description"
			, (int)max_desc, line);
	} else {
		esif_shell_sprintf(&shellbuf_len, &shellbuf,
			"\n"
			"ALL PARTICIPANTS:\n"
			"\n"
			"Name  %-*s Enum Ver Active DSP DC UPID       LPID \n"
			"----- %-.*s ---- --- ---------- -- ---------- ----\n"
			, (int)max_desc, "Description"
			, (int)max_desc, line);
	}

	iterRc = EsifUpPm_InitIterator(&upIter);
	if (iterRc == ESIF_OK) {
		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}

	while (ESIF_OK == iterRc) {
		char desc[ESIF_DESC_LEN + 3] = "";	/* desc... */
		char *enumStr = "NA";
		char instanceStr[8 + 1]     = "NA";
		u8 domainCount   = 1;

		dspPtr = EsifUp_GetDsp(upPtr);
		metaPtr = EsifUp_GetMetadata(upPtr);

		if ((NULL == dspPtr) || (NULL == metaPtr)) {
			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
			continue;
		}

		domainCount = (UInt8)dspPtr->get_domain_count(dspPtr);

		/* Truncate Description If Large */
		if (ESIF_SHELL_STRLEN(metaPtr->fDesc) > max_desc) {
			esif_ccb_strcpy(desc, metaPtr->fDesc, max_desc - 3);
			esif_ccb_sprintf_concat(ESIF_DESC_LEN, desc, "...");
		} else {
			esif_ccb_strcpy(desc, metaPtr->fDesc, max_desc);
		}

		enumStr = esif_participant_enum_shortname(metaPtr->fEnumerator);

		participantId = EsifUp_GetInstance(upPtr);

		lpId = EsifUp_GetLpInstance(upPtr);
		if (lpId != ESIF_INSTANCE_INVALID) {
			esif_ccb_sprintf(8, instanceStr, "%2d", lpId);
		}

		esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "%-5s %-*s %-5s%3d ",
			metaPtr->fName,
			max_desc,
			desc,
			enumStr,
			metaPtr->fVersion);

		if (!strcmp(attribute, "acpi")) {
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "%-10s %-30s %-4s %4d ",
				metaPtr->fAcpiDevice,
				metaPtr->fAcpiScope,
				metaPtr->fAcpiUID,
				metaPtr->fAcpiType);
		} else {
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "%-10s %-2d ",
				dspPtr->code_ptr,
				domainCount);
		}

		if ((UInt64)participantId < (((UInt64)(1)) << 32)) {
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "0x%08X %-5s\n",
				participantId,
				instanceStr);
		}
		else {
			esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, ESIF_HANDLE_FMT " %-5s\n",
				esif_ccb_handle2llu(participantId),
				instanceStr);
		}

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
	esif_shell_sprintf_concat(&shellbuf_len, &shellbuf, "\n");

	if (iterRc != ESIF_E_ITERATION_DONE) {
		esif_shell_sprintf(&shellbuf_len, &shellbuf, "Error getting participant information\n");
		goto exit;
	}

exit:
	EsifUp_PutRef(upPtr);
	if (shellbuf != NULL) {
		output = shell->outbuf = esif_shell_resize(shellbuf_len);
		esif_ccb_strcpy(output, shellbuf, OUT_BUF_LEN);
		esif_ccb_free(shellbuf);
	}
	return output;
}


// Quit
static char *esif_shell_cmd_quit(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Ignore commands from REST API
	if (g_isRest) {
		return NULL;
	}

	g_disconnectClient = ESIF_TRUE;
#ifndef ESIF_ATTR_OS_WINDOWS
	g_quit = ESIF_TRUE;
#endif

	esif_ccb_sprintf(OUT_BUF_LEN, output, "quit\n");
	return output;
}


// Timer Start
static char *esif_shell_cmd_timerstart(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	esif_ccb_get_time(&g_timer);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "timerstart\n");
	return output;
}


// Timer Stop
static char *esif_shell_cmd_timerstop(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	struct timeval stop = {0};
	struct timeval result;
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	if (g_timer.tv_sec == 0 && g_timer.tv_usec == 0) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "timer not active\n");
		goto exit;
	}

	esif_ccb_get_time(&stop);
	esif_ccb_sprintf(OUT_BUF_LEN, output, "timerstop\n");
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Start time: %06lu.%06lu\n", 
					 g_timer.tv_sec,
					 g_timer.tv_usec);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Stop time: %06lu.%06lu\n", 
					 stop.tv_sec,
					 stop.tv_usec);

	timeval_subtract(&result, &stop, &g_timer);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Time: %06lu.%06lu (%lu seconds + %lu ms + %lu usec)\n", 
					 result.tv_sec,
					 result.tv_usec,
					 result.tv_sec,
					 result.tv_usec / 1000,
					 result.tv_usec % 1000);

	// Reset For Next Time
	g_timer.tv_sec  = 0;
	g_timer.tv_usec = 0;
exit:
	return output;
}


// Conjure Participant
static char *esif_shell_cmd_conjure(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName;
	EsifCnjPtr a_conjure_ptr = NULL;
	u8 i = 0;

	/* Parse The Library Name */
	if (argc < 2) {
		return NULL;
	}

	libName = argv[1];

	/* Check to see if the participant is already loaded only one instance per participant lib allowed */
	if (NULL != esif_uf_conjure_get_instance_from_name(libName)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Conjure Library %s already started.\n", libName);
		goto exit;
	}

	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		if (NULL == g_cnjMgr.fEnrtries[i].fLibNamePtr) {
			break;
		}
	}


	if (ESIF_MAX_CONJURES == i) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Maximum Conjures Reached %u.\n", i);
		goto exit;
	} else {
		g_cnjMgr.fEntryCount++;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Conjure Sandbox %u Selected.\n", i);
	}

	a_conjure_ptr = &g_cnjMgr.fEnrtries[i];
	a_conjure_ptr->fLibNamePtr = (esif_string)esif_ccb_strdup(libName);

	rc = EsifConjureStart(a_conjure_ptr);
	if (ESIF_OK != rc) {
		/* Cleanup */
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Start Conjure Library: %s [%s (%d)]\n", libName, esif_rc_str(rc), rc);
		esif_ccb_free(a_conjure_ptr->fLibNamePtr);
		esif_ccb_memset(a_conjure_ptr, 0, sizeof(*a_conjure_ptr));
	} else {
		/* Proceed */
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Started Conjure Library: %s Instance %u Max %u Running %u\n\n",
						 libName, i, ESIF_MAX_CONJURES, g_cnjMgr.fEntryCount);
	}

exit:
	return output;
}


static char *esif_shell_cmd_unconjure(EsifShellCmdPtr shell)
{
	int argc        = shell->argc;
	char **argv     = shell->argv;
	char *output    = shell->outbuf;
	enum esif_rc rc = ESIF_OK;
	char *libName  = 0;
	EsifCnjPtr a_conjure_ptr = NULL;

	if (argc < 2) {
		return NULL;
	}

	/* Parse The Library Name */
	libName = argv[1];

	a_conjure_ptr = esif_uf_conjure_get_instance_from_name(libName);
	if (NULL == a_conjure_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Find Conjure Library: %s.\n", libName);
		goto exit;
	}

	rc = EsifConjureStop(a_conjure_ptr);
	if (ESIF_OK != rc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed To Stop Conjure Library: %s.\n", libName);
	} else {
		/* Proceed */
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Stopped Conjure Library: %s\n\n", libName);
		g_cnjMgr.fEntryCount--;
	}

exit:

	return output;
}

static char *esif_shell_cmd_status(EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char **argv   = shell->argv;
	char *output  = shell->outbuf;
	eEsifError rc = ESIF_OK;
	eEsifError iterRc = ESIF_OK;
	char tableName[TABLE_OBJECT_MAX_NAME_LEN] = { 0 };
	UfPmIterator upIter = {0};
	EsifUpPtr upPtr = NULL;
	esif_handle_t participantId;
	enum esif_temperature_type tempXformType = ESIF_TEMP_C;

	//
	// Get the table type
	//
	if (argc >= 2) {
		esif_ccb_sprintf(TABLE_OBJECT_MAX_NAME_LEN, tableName, "%s", argv[1]);
	}
	else {
		esif_ccb_sprintf(TABLE_OBJECT_MAX_NAME_LEN, tableName, "%s", "status");
	}

	//
	// Get the temperature conversion type
	//
	if (argc >= 3 && (esif_ccb_strlen(argv[2], MAX_PATH) > 1)) {
		if (*argv[2] == '-') {
			switch (*((UInt16*)&argv[2][1])) {
			case 'km':
				tempXformType = ESIF_TEMP_MILLIK;
				break;
			case 'kd':
				tempXformType = ESIF_TEMP_DECIK;
				break;
			case '\0k':
				tempXformType = ESIF_TEMP_K;
				break;
			case 'cm':
				tempXformType = ESIF_TEMP_MILLIC;
				break;
			case 'cd':
				tempXformType = ESIF_TEMP_DECIC;
				break;
			case '\0c':
				tempXformType = ESIF_TEMP_C;
				break;
			default:
				break;
			}	
		}
	}
	
	esif_ccb_sprintf(OUT_BUF_LEN, output, "update:<status>\n");

	iterRc = EsifUpPm_InitIterator(&upIter);
	if (iterRc == ESIF_OK) {
		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}

	while (iterRc == ESIF_OK) {
		EsifUpDomainPtr domainPtr = NULL;
		UpDomainIterator udIter = { 0 };
		EsifDspPtr dspPtr = NULL;

		participantId = EsifUp_GetInstance(upPtr);

		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"<stat>\n"
			"  <id>%llu</id>\n"
			"  <name>%s</name>\n",
			esif_ccb_handle2llu(participantId),
			EsifUp_GetName(upPtr));

		dspPtr = EsifUp_GetDsp(upPtr);
		if (dspPtr != NULL) {
			iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
			if (ESIF_OK != iterRc)
				goto exit;

			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			
			while (ESIF_OK == iterRc) {
				TableObject tableObject = { 0 };
				if (NULL == domainPtr) {
					iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
					continue;
				}

				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  <domain>\n    <name>%s</name>\n", domainPtr->domainName);

				TableObject_Construct(
					&tableObject,
					tableName, 
					domainPtr->domainStr, 
					NULL,
					NULL,
					NULL,
					0,
					participantId,
					GET);
				rc = TableObject_LoadSchema(&tableObject);

				if (rc == ESIF_OK) {
					rc = TableObject_LoadXML(&tableObject, tempXformType);
					if (rc == ESIF_OK) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, tableObject.dataXML);
					}
				}	
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "  </domain>\n");
				TableObject_Destroy(&tableObject);

				iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			}

			if (iterRc != ESIF_E_ITERATION_DONE) {
				EsifUp_PutRef(upPtr);
			}
		}
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</stat>\n");

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</status>\n");
exit:
	EsifUp_PutRef(upPtr);	
	return output;
}


static char *esif_shell_cmd_domains(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	UInt32 domainIndex = 0;
	int i = 0;
	esif_handle_t participantDst = g_dst;
	EsifUpPtr upPtr = NULL;
	eEsifError iterRc = ESIF_OK;
	EsifUpDomainPtr domainPtr = NULL;
	UpDomainIterator udIter = { 0 };

	// Lookup Participant by ID or Name
	if (argc < 2) {
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantDst);
	}
	else if ((upPtr = EsifUpPm_GetAvailableParticipantByName(argv[1])) != NULL) {
		participantDst = EsifUp_GetInstance(upPtr);
	} else if (shell_isnumber(argv[1])) {
		participantDst = (esif_handle_t)esif_atoi64(argv[1]);
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantDst);
	}

	if (NULL == upPtr) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Unable to get participant, current dst may be invalid\n");
		goto exit;
	}

	iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
	if (ESIF_OK != iterRc)
		goto exit;

	iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);

	if (FORMAT_XML == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "<domains>\n");
	}
	else {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
			"\n"
			"DOMAINS:\n"
			"ID Name     Qual PRI Capability Device Type\n"
			"-- -------- ---- --- ---------- ---------------------------------\n");
	}

	while (ESIF_OK == iterRc) {		
		if (NULL == domainPtr) {
			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			continue;
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"%02d %-8s %-4s %d   0x%08x %s(%d)\n",
				domainIndex,
				domainPtr->domainName,
				domainPtr->domainStr,
				domainPtr->domainPriority,
				domainPtr->capability_for_domain.capability_flags,
				esif_domain_type_str((enum esif_domain_type)domainPtr->domainType),
				domainPtr->domainType);

			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		}
		else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
				"  <domain>\n"
				"    <domainIndex>%d</domainIndex>\n"
				"    <domainName>%s</domainName>\n"
				"    <domainQual>%s</domainQual>\n"
				"    <domainPriority>%d</domainPriority>\n"
				"    <capabilityFlags>0x%x</capabilityFlags>\n"
				"    <domainTypeString>%s</domainTypeString>\n"
				"    <domainType>%d</domainType>\n"
				"    <capabilities>\n",
				domainIndex,
				domainPtr->domainName,
				domainPtr->domainStr,
				domainPtr->domainPriority,
				domainPtr->capability_for_domain.capability_flags,
				esif_domain_type_str((enum esif_domain_type)domainPtr->domainType),
				domainPtr->domainType);
		}

		for (i = 0; i < 32; i++) {	/* TODO:  Limit to actual enum size */
			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s(%d): %02X\n",
					esif_capability_type_str((enum esif_capability_type)i), i, domainPtr->capability_for_domain.capability_mask[i]);
			}
			else {// FORMAT_XML
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					"      <capability>\n"
					"        <capabilityString>%s</capabilityString>\n"
					"        <capabilityType>%d</capabilityType>\n"
					"        <capabilityOption>%d</capabilityOption>\n"
					"      </capability>\n",
					esif_capability_type_str((enum esif_capability_type)i),
					i,
					domainPtr->capability_for_domain.capability_mask[i]);
			}
		}
		
		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		}
		else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "    </capabilities>\n  </domain>\n");
		}

		iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
		domainIndex++;
	}

	if (FORMAT_TEXT == g_format) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
	}
	else {// FORMAT_XML
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</domains>\n");
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return output;
}


esif_error_t ESIF_CALLCONV esif_shell_cmd_event_callback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(eventDataPtr);

	if (fpcEventPtr) {
		ESIF_TRACE_INFO("\nReceived event %s(%d) in shell for Part. %u Dom. 0x%02X\n",
			esif_event_type_str(fpcEventPtr->esif_event), fpcEventPtr->esif_event,
			participantId,
			domainId);
	}

	return ESIF_OK;
}


static char *esif_shell_cmd_event(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	int opt = 1;
	eEsifEventType event_type;
	esif_handle_t participant_id = g_dst;
	EsifDataPtr eventDataPtr = NULL;
	char *qualifier_str = NULL;
	eEsifError rc = ESIF_OK;
	Bool enableEvent = ESIF_FALSE;
	Bool disableEvent = ESIF_FALSE;

	//
	// Use EVENT_MGR_DOMAIN_NA for sending events where the domain ignored;
	// but use EVENT_MGR_MATCH_ANY_DOMAIN to register for events where the
	// domain is to always be ignored.
	//
	u16 domain_id = EVENT_MGR_DOMAIN_NA;

	if (argc < 2) {
		return NULL;
	}

	//
	// First check if the command is to enable or disable an event
	//
	if (!esif_ccb_stricmp(argv[opt], "enable")) {
		enableEvent = ESIF_TRUE;
		domain_id = EVENT_MGR_MATCH_ANY_DOMAIN;
		opt++;
	}
	else if (!esif_ccb_stricmp(argv[opt], "disable")) {
		disableEvent = ESIF_TRUE;
		domain_id = EVENT_MGR_MATCH_ANY_DOMAIN;
		opt++;
	}

	// Event Type
	event_type = (eEsifEventType)esif_atoi(argv[opt++]);

	// Optional Participant ID
	if (argc > opt) {
		participant_id = (esif_handle_t)esif_atoi64(argv[opt++]);
	}

	// Optional Domain ID
	if (argc > opt) {
		qualifier_str = argv[opt++];
		domain_id     = domain_str_to_short(qualifier_str);
	}

	//optional event data
	if (argc > opt) {
		eventDataPtr = EsifData_Create();
		if (eventDataPtr == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		rc = EsifData_FromString(eventDataPtr, argv[opt++], ESIF_DATA_AUTO);
		if (rc != ESIF_OK || eventDataPtr->buf_ptr == NULL) {
			goto exit;
		}
	}

	if (enableEvent) {
		rc = EsifEventMgr_RegisterEventByType(event_type, participant_id, domain_id, esif_shell_cmd_event_callback, 0);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "\nENABLED");
	}
	else if (disableEvent) {
		rc = EsifEventMgr_UnregisterEventByType(event_type, participant_id, domain_id, esif_shell_cmd_event_callback, 0);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "\nDISALBED");
	}
	else {
		rc = EsifEventMgr_SignalUnfilteredEvent(participant_id, domain_id, event_type, eventDataPtr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "\nSEND");
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
		" EVENT %s(%d) for PARTICIPANT " ESIF_HANDLE_FMT " DOMAIN 0x%02X\n",
		esif_event_type_str(event_type), event_type, esif_ccb_handle2llu(participant_id),
		domain_id);
exit:
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error: RC = %s(%d)\n",
			esif_rc_str(rc),
			rc);
	}
	EsifData_Destroy(eventDataPtr);

	return output;
}


// Kernel Participant Extensions: eventkpe
static char *esif_shell_cmd_eventkpe(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc_command *command_ptr = NULL;
	struct esif_command_send_kpe_event *req_data_ptr = NULL;
	u32 data_len = sizeof(*req_data_ptr);
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	enum esif_event_type event_type = 0;
	u32 instance = 0;
	u8 event_data_present = ESIF_FALSE;
	u32 event_data = 0;

	if ((NULL == ipc_ptr) || (NULL == command_ptr)) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"esif_ipc_alloc_command failed for %u bytes\n",
			data_len);
		goto exit;
	}

	if (argc < 3) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"Must specify the event type and KPE instance\n");
		goto exit;
	}

	req_data_ptr = (struct esif_command_send_kpe_event *)(command_ptr + 1);

	event_type = esif_atoi(argv[1]);
	instance = esif_atoi(argv[2]);

	if (argc > 3) {
		event_data_present = ESIF_TRUE;
		event_data = esif_atoi(argv[3]);
	}
	req_data_ptr->event_type = event_type;
	req_data_ptr->instance = instance;
	req_data_ptr->data_present = event_data_present;
	req_data_ptr->data = event_data;

	command_ptr->type = ESIF_COMMAND_TYPE_SEND_KPE_EVENT;
	command_ptr->req_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = data_len;
	command_ptr->rsp_data_type   = ESIF_DATA_VOID;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = 0;

	esif_ccb_sprintf(OUT_BUF_LEN, output,
		"Sending KPE event type %s(%d) to KPE %u ",
		esif_event_type_str(event_type),
		event_type,
		instance);

	if (event_data_present) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, ", data = %u: ", event_data);
	} else {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "(no data): ");	
	}

	ipc_execute(ipc_ptr);

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "RC = %s(%d)\n",
		esif_rc_str(command_ptr->return_code),
		command_ptr->return_code);	

exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}


static char *esif_shell_cmd_help(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	UInt32 capabilityid = 0;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	esif_ccb_sprintf(OUT_BUF_LEN, output, "ESIF CLI Copyright (c) 2013-2020 Intel Corporation All Rights Reserved\n");
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n"
		"Key:  <>-Required parameters\n"
		"      []-Optional parameters\n"
		"       |-Choice of parameters\n"
		"     ...-Repeated parameters\n"
		"\n"
		"GENERAL COMMANDS:\n"
		"help                                     Displays this text\n"
		"quit or exit                             Leave\n"
		"format <xml|text>                        Command Output Format (Default=text)\n"
		"info                                     Get Kernel Version\n"
		"about                                    List ESIF Information\n"
		"capture [-overwrite] [filename]           Write all DPTF settings to .txt file in XML format [overwrite will replace existing file]\n"
		"rem                                      Comment/Remark - ignored\n"
		"repeat <count>                           Repeat Next Command N Times\n"
		"repeat_delay <delay>                     Repeat Delay In ms\n"
		"echo [?] [parameter...]                  Echos Parameters - if ? is used, each\n"
		"                                         parameter is on a separate line\n"
		"memstats [reset]                         Show/Reset Memory Statistics\n"
		"autoexec [command] [...]                 Execute Default Startup Script\n"
		"\n"
		"TEST SCRIPT COMMANDS:\n"
		"load    <filename> [load parameters...]  Load and Execute Command File\n"
		"loadtst <filename> [load parameters...]  Like 'load' but uses DSP DV for file\n"
		"cat     <filename> [load parameters...]  Display Command File\n"
		"proof   <filename> [load parameters...]  Prove Command File Replace Tokens\n"
		"Load parameters replace tokens ($1...$9) in file. $dst$ replaced by file path\n"
		"Use 'proof' to check parameter replacements\n"
		"\n"
		"test <id | all>                          Test By ID or ALL Will Run All Tests\n"
		"soe  <on|off>                            Stop On Error\n"
		"seterrorlevel                            Set / Reset Error Level\n"
		"geterrorlevel                            Get Current Error level\n"
		"timerstart                               Start Interval Timer\n"
		"timerstop                                Stop Interval Timer\n"
		"sleep <ms>                               Sleep for the specified number of ms\n"
		"\n"
		"UI COMMANDS:\n"
		"ui getxslt   [appname]                   Return XSLT Formatting information\n"
		"ui getgroups [appname]                   Get List Of Groups of Left Hand Tabs\n"
		"ui getmodulesingroup <appname> <groupId> Get A List Of Modules For The Group\n"
		"ui getmoduledata <appname> <groupId> <moduleId>\n"
		"                                         Get Data For The App/Group/Module\n"
		"\n"
		"DSP COMMANDS:\n"
		"dsps                                     List all loaded DSPs\n"
		"dspquery [name [vendorid [deviceid [enum [ptype [hid [uid]]]]]]] Query for matching DSP\n"
		"infocpc <filename> [pattern]             Get Dst CPC Information\n"
		"infofpc <filename> [pattern]             Get Dst FPC Information\n"
		"\n"
		"PARTICIPANT COMMANDS:\n"
		"participants                              List Active Participants\n"
		"participantsk                            List Kernel Participants\n"
		"participant create <options>             Create Persisted Dynamic Participant. Options:\n"
		"  CONJURE <name> \"desc\" <hid> <ptype>    Create Persisted Dynamic Upper Framework Participant\n"
		"  ACPI <name> \"desc\" <hid> <ptype>       Create Persisted Dynamic Kernel ACPI Participant\n"
		"  PCI  <name> \"desc\" <vid> <did>         Create Persisted Dynamic Kernel PCI Participant\n"
		"participant delete  NAME                 Delete and Destroy Persisted Dynamic Participant\n"
		"participant destroy NAME                 Destroy Upper Framework Participant\n"
		"participant  <id>                        Get Participant Information\n"
		"participantk <id>                        Get Kernel Participant Information\n"
		"addpart <options>                        Add a new Upper Framework Participant. Options:\n"
		"  <name> \"desc\" <hid> <ptype> [flags]\n"
		"addpartk <options>                       Add a new Kernel Participant. Options: \n"
		"  PCI <name> \"desc\" <vid> <did>\n"
		"  ACPI <name> \"desc\" <hid> <ptype>\n"
		"delpartk <name>                          Removes a Conjured Kernel Participant"
		"dst  <id>                                Set Target Participant By ID\n"
		"dstn <name>                              Set Target Participant By Name\n"
		"domains                                  List Active Domains For Participant\n"
		"\n"
		"PRIMITIVE EXECUTION API:\n"
		"getp <id> [qualifier] [instance] [data=[u32 or string input parameter]] [[~]act_name | [~]act_index]\n"
		"                                         Execute 'GET' Primitive With Automatic\n"
		"                                         Return Type and Size\n"
		"  Where: act_name = Name of Action Type to Use\n"
		"         act_index = Index of the Action to Use\n"
		"         ~ = If Present, Indicates to Exclude that Action/Name\n"
		"         (Default is to try all actions for a primitive until one succeeds)\n"
		"  Note: Action Selector Options not shown in variants below, but may be used\n"
		"\n"
		"getp_u32 <id> [qualifier] [instance]        Execute U32 Get Primitive\n"
		"getp_t   <id> [qualifier] [instance]        Like getp But Converts Temperature\n"
		"getp_part<part> <id> [qualifier] [instance] Like getp But First Parameter is Participant Name or ID\n"
		"getp_pw  <id> [qualifier] [instance]        Like getp But Converts Power To mW\n"
		"getp_s   <id> [qualifier] [instance]        Like getp But Return STRING\n"
		"getp_b   <id> [qualifier] [instance]        Like getp But Return BINARY DATA\n"
		"getp_bd  <id> [qualifier] [instance]        Like getp But Dumps Hex BINARY DATA\n"
		"getp_bf  <id> [qualifier] [instance] [file] Like getp_b But Dumps To File\n"
		"getf_b   <file>                             Like getp_b But Reads From File\n"
		"getf_bd  <file>                             Like getp_bd But Reads From File\n"
		"\n"
		"setp    <id> <qualifier> <instance> <data> [[~]act_name | [~]act_index]\n"
		"                                         Execute 'SET' Primitive\n"
		"  Where: act_name = Name of Action Type to Use\n"
		"         act_index = Index of the Action to Use\n"
		"         ~ = If Present, Indicates to Exclude that Action/Name\n"
		"         (Default is to try all actions for a primitive until on succeeds)\n"
		"  Note: Action Selector Options not shown in variants below, but may be used\n"
		"\n"
		"setp_t  <id> <qualifier> <instance> <temp>  Execute Set as Temperature (C)\n"
		"setp_part<part> <id> <qualifier> <instance> <data> [action] Like setp But First Parameter is Participant Name or ID\n"
		"setp_pw <id> <qualifier> <instance> <power> Execute Set as Power (mW)\n"
		"setp_bf <id> <qualifier> <instance> <file>  Same as setp except Read from File\n"
		"set_osc <id> <capablities>                  Execute ACPI _OSC Command For\n"
		"                                            Listed GUIDs Below:\n"
		"         Active Policy 0 = {0xd6,0x41,0xa4,0x42,0x6a,0xae,0x2b,0x46,\n"
		"                            0xa8,0x4b,0x4a,0x8c,0xe7,0x90,0x27,0xd3}\n"
		"         Fail Case     1 = {0xde,0xad,0xbe,0xef,0xde,0xad,0xbe,0xef,\n"
		"                            0xde,0xad,0xbe,0xef,0xde,0xad,0xbe,0xef}\n"
		"                                            Capabilities per APCI Spec\n"
		"setb <buffer_size>                          Set Binary Buffer Size\n"
		"\n"
		"rstp <id> [qualifier] [instance]            Resets/clears an override\n"
		"\n"
		"rstp_part <part id or name> <id> [qualifier] [instance] Same as rstp but specifies participant \n"
		"\n"
		"getb                                        Get Binary Buffer Size\n"
		"\n"
		"idsp [add | delete <uuid>]                  Display, add or remove a UUID from\n"
		"                                            the IDSP in override.dv\n"
		"\n"
		"CONFIG API:\n"
		"config [list]                               List Open DataVaults\n"
		"config files  [pattern] [...]               List Matching Data Repository Files\n"
		"config open   <@datavault|repo.bin>         Open and Load a DataVault or Repository\n"
		"config close  <@datavault> [...]            Close DataVault(s)\n"
		"config drop   <@datavault|repo.bin> [...]   Drop Closed DataVault\n"
		"config get    [@datavault] [<keyspec>]      Get DataVault Key (or wildcard)\n"
		"config set    [@datavault] <key> <value> [type] [option ...]\n"
		"                                            Set DataVault Key/Value/Type\n"
		"config keys   [@datavault] [<keyspec>]      Enumerate matching Key(s) or wildcard\n"
		"config delete [@datavault] <keyspec>        Delete DataVault Key (or '*')\n"
		"config save   [@datavault] <key> <file>     Save a Key's binary value to a File\n"
		"config exec   [@datavault] <keyspec>        Execute Script(s) in DataVault Key\n"
		"config copy   [@datavault] <keyspec> [...] [@@targetdv] [option ...]\n"
		"                                            Copy (and Replace) keys(s) to another DV\n"
		"config merge  [@datavault] <keyspec> [...] [@@targetdv] [option ...]\n"
		"                                            Merge (No Replace) keys(s) to another DV\n"
		"config export {[@datavault] <keyspec> [...] ...} [@@targetdv] [asl|dv|dvx|repo] [<comment>]\n"
		"                                            Export DV keys to ASL or DV File\n"
		"config payload <@datavault> <file> [<class>] [compress]\n"
		"                                            Manually load a file into a DataVault Payload\n"
		"config rename  <oldname.ext> <newname.ext>  Rename a DataVault Repository File (Rename)\n"
		"config replace <oldname.ext> <newname.ext>  Rename a DataVault Repository File (Replace)\n"
		"config <copyto|appendto> <target> <file(s)> Copy or Append DataVault Repository Files to Target\n"
		"config asl <encode|decode> <input> <output> Encode or Decode DataVault Repository to/from Hex ASL Text format\n"
		"config doc <@datavault> <file.csv> [comment] [label] [append] Export a DV summary to file.csv\n"
		"config gddv set <repo.bin|@@factory>        Set a GDDV override or do a Factory Reset and Restart Apps\n"
		"config gddv reset                           Clear a GDDV override and Restart Apps\n"
		"config gddv backup                          Backup GDDV-related DV Files without Restarting Apps\n"
		"config gddv restore                         Restore GDDV-related DV Files from Backup and Restart Apps\n"
		"config gddv mode                            Display Current GDDV Emulation Mode (Normal/Emulation)\n"
		"config gddv status                          Display Current GDDV Modified Status (Unchanged/Modified)\n"
		"config rekey [@datavault]                   Rekey IETM Participant Keys in GDDV Exports\n"
		"config comment <@datavault|repo.bin> [<comment>] Get or Set DataVault or Repo Comment\n"
		"config version [@datavault] <major.minor>   Set DataVault File Version\n"
		"config upload [overwrite|append] <filename> <base64-data> Upload a file from a base64-encoded string\n"
		"\n"
		"EVENT API:\n"
		"event [enable|disable] <eventType> [participant] [domain]   Enable/Disable/Send a User Mode Event\n" 
		"events [namespec] [appspec]                   Display all events registered in the Event Manager\n"
		"eventkpe <eventType> <index> [u32 data]       Send Kernel Event to KPE\n"
		"                                              index - Index of the KPE based on\n"
		"                                              the order of driversk (0-based)\n"
		"\n"
	#ifndef ESIF_FEAT_OPT_ACTION_SYSFS
		"IPC API:\n"
		"ipcauto                                  Auto Connect/Retry\n"
		"ipccon                                   Force IPC Connect\n"
		"ipcdis                                   Force IPC Disconnection\n"
		"\n"
	#endif
		"APPLICATION MANAGEMENT:\n"
		"apps                                     List all ESIF hosted Applications\n"
		"appstart   <application>                 Start an ESIF Application\n"
		"appstop    <application>                 Stop an ESIF Application\n"
		"appstatus  <application>                 App Status\n"
		"appenable  <application>                 App Enable\n"
		"appabout   <application>                 App About\n"
		"\n"
		"ACTION MANAGEMENT:\n"
		"actions                                  List all DSP Actions\n"
		"actionsk                                 Get Kernel DSP Action Information\n"
		"actionsu                                 Get User-Mode DSP Action Information\n"
		"actionstart <action>                     Start a Loadable DSP Action\n"
		"actionstop  <action>                     Stop a Loadable DSP Action\n"
		"devices	 <action>					  List the devices enumerated and managed by a UPE\n"
		"driversk                                 List Kernel Participant Extensions\n"
		"driverk <id>                             Get Kernel Participant Extension Info\n"
		"upes                                     List User-mode Participant Extensions\n"
		"\n"
#ifdef ESIF_ATTR_OS_WINDOWS
		"tools                                    List all supported tools\n"
		"toolstart <tool name> [parameters...]    Start a supported tool\n"
		"toolstop <tool name>                     Stop a supported tool\n"
		"\n"
#endif
		"CONJURE MANAGEMENT:\n"
		"conjures                                 List loaded ESIF Conjure Libraries\n"
		"conjure   <library name>                 Load Upper Framework Conjure Library\n"
		"unconjure <library name>                 Unload Upper Framework Conjure Library\n"
		"\n"
		"USER-MODE PARTICIPANT DATA LOGGING:\n"
		"participantlog "PARTICIPANTLOG_CMD_START_STR" [all |[PID DID capMask]...]\n"
		"                                        Starts data logging for that particular\n"
		"                                        participant ID,domain and capability ID\n"
		"                                        combination\n"
		"                                        all     - Starts data logging for all\n"
		"                                                  the available participants\n"
		"                                        PID     - Participant ID (e.g 1,2)|\n"
		"                                                  Participant Name(e.g TCHG,\n"
		"                                                  TCPU)\n"
		"                                        DID     - Domain ID(e.g D0,D1)|all\n"
		"                                                  (for all domains)\n"
		"                                        capMask - capability Mask(e.g 0x2100)|\n"
		"                                                  all(for all capabilities)\n"
		"                                        If no arguments are specified for start\n"
		"                                        command, by default logging will start\n"
		"                                        for all the available participants\n"
		"                                        Capability Mask Details:\n"
		);
	for (capabilityid = 0; capabilityid < MAX_CAPABILITY_MASK; capabilityid++) {
		if (!esif_ccb_stricmp(esif_capability_type_str(capabilityid), ESIF_NOT_AVAILABLE)) {
			break;
		}
		else if (esif_ccb_strlen(esif_capability_type_str(capabilityid), MAX_PATH) > sizeof("ESIF_CAPABILITY_TYPE")) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
		"                                        0x%08X -  %s\n",
			(1 << capabilityid),
			esif_capability_type_str(capabilityid) + sizeof("ESIF_CAPABILITY_TYPE")
			);
		}
	}
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
		"participantlog "PARTICIPANTLOG_CMD_SCHEDULE_STR" [delay] [all |[PID DID capMask]...]\n"
		"                                        Start a delayed logging thread ms\n"
		"                                        from now.\n"
		"                                        delay - Time interval in ms. (Default \n"
		"                                        " EXPAND_TOSTR(DEFAULT_SCHEDULE_DELAY_INTERVAL) "ms)\n"
		"                                        Other arguments list descriptions are\n"
		"                                        same as the start command.\n"
		"participantlog "PARTICIPANTLOG_CMD_INTERVAL_STR" [time]          Sets the polling interval for logging\n"
		"                                        to the time specified in ms.\n"
		"                                        time - Polling interval in ms. Supported\n"
		"                                        values: " EXPAND_TOSTR(MIN_LOG_INTERVAL) " to " EXPAND_TOSTR(MAX_LOG_INTERVAL) "ms (Default " EXPAND_TOSTR(DEFAULT_LOG_INTERVAL) "ms)\n"
		"participantlog "PARTICIPANTLOG_CMD_ROUTE_STR" [ALL | [target ... [filename]]] \n"
		"                                        Logs the participant data log to the\n"
		"                                        specified target.\n"
		"                                        If no arguments are specified, data is\n"
		"                                        routed to a file.*\n"
		"                                        If ALL is specified, data is sent to\n"
		"                                        all available targets.\n"
		"                                        The target can be any of the following:\n"
		"                                        CONSOLE, EVENTVIEWER, DEBUGGER, or FILE.\n"
		"                                        If FILE is specified as the target, the\n"
		"                                        next argument, if present, must specify\n"
		"                                        the file name.*\n"
		"                                        *If a filename is not specified, a default\n"
		"                                        file name is used based on the timestamp;\n"
		"                                        e.g, participant_log_2015-11-24-142412.csv.\n"
		"participantlog "PARTICIPANTLOG_CMD_STOP_STR"                     Stops participant data logging if\n"
		"                                        started already\n"
		"\n"										  
		"USER-MODE TRACE LOGGING:\n"
		"trace                             Show User Mode Trace Settings\n"
		"trace level  [level]              View or Set Global Trace Level\n"
		"trace module <level> <module ...> Set Trace Module Bitmask (ALL, NONE, Name)\n"
		"trace route  <level> <route ...>  Set Trace Routing Bitmask (CON, EVT, DBG, LOG)\n"
		"trace log open <filename>         Open Trace Log File\n"
		"trace log close                   Close Trace Log File\n"
		"timestamp <on|off>                Show Execution Timestamps in IPC Trace Data\n"
		"\n"
		"log                               Display Open Log Files\n"
		"log list                          Display Open Log Files\n"
		"log <filename>                    Log To File If Debug On It Will Tee Output\n"
		"log open  [type] <filename>       Open Log File (types: shell(dflt) trace ui)\n"
		"log write [type] <\"Log Msg\">      Write 'Log Msg' to Log File\n"
		"log close [type]                  Close Log File\n"
		"log scan  [pattern]               Display Log Files Match Pattern (dflt=*.log)\n"
		"log <flush|noflush> [type]        Turn AutoFlush On/Off for Log File (dflt=Off)\n"
		"nolog                             Stop Logging To File\n"
		"\n"
		"KERNEL-MODE TRACE LOGGING:\n"
		"debugshow                      Show Debug Status\n"
		"debugset <modules>             Set Kernel Modules To Debug\n"
		"debuglvl <module> <cat_mask>   Set Kernel Debug Category Mask For Module\n"
		"debuglvl <tracelevel>          Set Kernel Trace Level (0..4 Default=ERROR(0))\n"
		"Where <modules> is a hex bit-mask of modules (e.g. 0xfc)\n"
		"      <module> is a module number\n"
		"      <cat_mask> is a hex bit-mask of the module debugging categories to enable\n"
		"\n\n"
		);
	return output;
}


static char *esif_shell_cmd_ui(EsifShellCmdPtr shell)
{
	int argc      = shell->argc;
	char **argv   = shell->argv;
	char *output  = shell->outbuf;
	char *subcmd  = NULL;
	char *appName = "DPTF";
	char statusCommandString[] = "status";
	int paramCount = MIN_PARAMETERS_FOR_APP_STATUS;
	UInt8 groupId   = 0;
	UInt8 moduleId   = 0;
	EsifData statusData = { ESIF_DATA_STRING };
	EsifDataPtr commandDataPtr = NULL;
	eEsifError rc = ESIF_OK;
	eAppStatusCommand command = (eAppStatusCommand)0;
	EsifAppPtr appPtr = NULL;
	char *statusBuffer = NULL;

	if (argc < 2) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	subcmd = argv[1];

	// ui getxslt <appname>
	if (esif_ccb_stricmp(subcmd, "getxslt")==0) {
		command = eAppStatusCommandGetXSLT;
		paramCount = 2;
		if (argc >= 3)
			appName = argv[2];
	}
	// ui getgroups <appname>
	else if (esif_ccb_stricmp(subcmd, "getgroups")==0) {
		command = eAppStatusCommandGetGroups;
		paramCount = 2;
		if (argc >= 3)
			appName = argv[2];
	}
	// ui getmodulesingroup <appname> <gid>
	else if (esif_ccb_stricmp(subcmd, "getmodulesingroup")==0 && argc >= 4) {
		command = eAppStatusCommandGetModulesInGroup;
		paramCount = 3;
		appName = argv[2];
		groupId  = (u8)esif_atoi(argv[3]);
	}
	// ui getmoduledata <appname> <gid> <mid>
	else if (esif_ccb_stricmp(subcmd, "getmoduledata")==0 && argc >= 5) {
		command = eAppStatusCommandGetModuleData;
		paramCount = 4;
		appName = argv[2];
		groupId  = (u8)esif_atoi(argv[3]);
		moduleId  = (u8)esif_atoi(argv[4]);

	}
	// unknown subcmd
	else {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	appPtr = EsifAppMgr_GetAppFromName(appName);
	if (NULL == appPtr) {
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	statusBuffer = esif_ccb_malloc(OUT_BUF_LEN);
	if (statusBuffer == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	statusData.type = ESIF_DATA_STRING;
	statusData.buf_ptr = statusBuffer;
	statusData.buf_len = OUT_BUF_LEN;
	statusData.data_len = 0;

	commandDataPtr = esif_ccb_malloc(paramCount * sizeof(*commandDataPtr));
	if (commandDataPtr == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	commandDataPtr[0].type = ESIF_DATA_STRING;
	commandDataPtr[0].buf_ptr = statusCommandString;
	commandDataPtr[0].buf_len = sizeof(statusCommandString);
	commandDataPtr[0].data_len = sizeof(statusCommandString);

	commandDataPtr[1].type = ESIF_DATA_UINT32;
	commandDataPtr[1].buf_ptr = &command;
	commandDataPtr[1].buf_len = sizeof(command);
	commandDataPtr[1].data_len = sizeof(command);

	if (paramCount > 2) {
		commandDataPtr[2].type = ESIF_DATA_UINT8;
		commandDataPtr[2].buf_ptr = &groupId;
		commandDataPtr[2].buf_len = sizeof(groupId);
		commandDataPtr[2].data_len = sizeof(groupId);
	}
	if (paramCount > 3) {
		commandDataPtr[3].type = ESIF_DATA_UINT8;
		commandDataPtr[3].buf_ptr = &moduleId;
		commandDataPtr[3].buf_len = sizeof(moduleId);
		commandDataPtr[3].data_len = sizeof(moduleId);
	}

	esif_uf_shell_unlock();
	rc = EsifApp_SendCommand(appPtr, paramCount, commandDataPtr, &statusData);
	esif_uf_shell_lock();

	if (ESIF_E_NEED_LARGER_BUFFER == rc) {
		statusBuffer = esif_ccb_realloc(statusBuffer, statusData.data_len);
		if (statusBuffer == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		statusData.buf_ptr = statusBuffer;
		statusData.buf_len = statusData.data_len;

		esif_uf_shell_unlock();
		rc = EsifApp_SendCommand(appPtr, paramCount, commandDataPtr, &statusData);
		esif_uf_shell_lock();

		if (rc == ESIF_OK) {
			output = shell->outbuf = esif_shell_resize(statusData.data_len);
		}
		else {
			output = shell->outbuf = g_outbuf;
		}
	}

exit:
	if (rc == ESIF_OK) {
		esif_ccb_memcpy(output, statusData.buf_ptr, statusData.data_len);
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Failed to get app status: %s \n", esif_rc_str(rc));
	}
	EsifAppMgr_PutRef(appPtr);
	esif_ccb_free(commandDataPtr);
	esif_ccb_free(statusBuffer);

	return output;
}

static char *esif_shell_cmd_web(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	// Ignore commands from REST API
	if (g_isRest) {
		return NULL;
	}

	// web [status]
	if (argc < 2 || esif_ccb_stricmp(argv[1], "status")==0) {
		const char *version = EsifWebVersion();
		esif_ccb_sprintf(OUT_BUF_LEN, output, "web server %s%s%s\n", (EsifWebIsStarted() ? "started" : "stopped"), (*version ? " : Version " : ""), version);
	}
	// web start [unrestricted] [ip.addr] [port]
	// web config [@instance] [ip.addr] [port]
	else if (esif_ccb_stricmp(argv[1], "start")==0 || esif_ccb_stricmp(argv[1], "config") == 0) {
		int arg=2;
		char *ipaddr = NULL;
		u32 port = 0;
		esif_flags_t flags = 0;
		u8 instance = 0;
		int tries = 0;
		Bool restricted = ESIF_FALSE;

		if (argc > arg && argv[arg][0] == '@') {
			instance = (u8)esif_atoi(&argv[arg++][1]);
		}
		if (argc > arg && esif_ccb_strnicmp(argv[arg], "restricted", 4) == 0) {
			restricted = ESIF_TRUE;
			arg++;
		}
		if (argc > arg && esif_ccb_strnicmp(argv[arg], "unrestricted", 5) == 0) {
			flags |= WS_FLAG_NOWHITELIST;
			arg++;
		}
		if (argc > arg && (isalpha(argv[arg][0]) || strchr(argv[arg], '.') != NULL)) {
			ipaddr = argv[arg++];
		}
		if (argc > arg && isdigit(argv[arg][0])) {
			port = esif_atoi(argv[arg++]);
		}

		// Restricted Mode Deprecated
		if (restricted) {
			CMD_OUT("feature deprecated\n");
		}
		// Configure Listener
		else if (esif_ccb_stricmp(argv[1], "config") == 0) {
			if (instance || ipaddr || port || flags) {
				EsifWebSetConfig(instance, ipaddr, port, flags);
				CMD_OUT("web config instance %d[%x] IP=%s port=%d\n", (int)instance, flags, (ipaddr ? ipaddr : "<default>"), port);
			}
			else {
				instance = 0;
				while (EsifWebGetConfig(instance, &ipaddr, &port, &flags)) {
					if (instance == 0 || (ipaddr && ipaddr[0]) || port || flags) {
						CMD_OUT("web config instance %d[%x]: IP=%s port=%d [%s]\n",
							(int)instance,
							flags,
							(ipaddr && ipaddr[0] ? ipaddr : "<default>"),
							port,
							(EsifWebIsStarted() ? "started" : "stopped")
						);
					}
					instance++;
				}
			}
		}
		// Start Web Server if not already started
		else if (!EsifWebIsStarted()) {
			if (!instance || ipaddr || port || flags) {
				EsifWebSetConfig(ESIF_WS_INSTANCE_ALL, ipaddr, port, flags);
			}
			Bool forbidden = ESIF_FALSE;

			// Verify Access Control
			instance = 0;
			while (EsifWebGetConfig(instance, &ipaddr, &port, &flags)) {
				if (instance == 0 || (ipaddr && ipaddr[0]) || port) {
					if (DCfg_Get().opt.GenericUIAccessControl) {
						forbidden = ESIF_TRUE;
					}
				}
				instance++;
			}
			if (!forbidden) {
				EsifWebStart();

				// thread synchronization delay for output
				do {
					esif_ccb_sleep_msec(10);
				} while (++tries < 10 && !EsifWebIsStarted());
			}

			if (!EsifWebIsStarted()) {
				CMD_OUT("web start %s\n", (forbidden ? "disabled" : "failed"));
			}
			else {
				CMD_OUT("web server started\n");
			}
		}
		else {
			CMD_OUT("web server already started\n");
		}
	}
	// web stop
	else if (esif_ccb_stricmp(argv[1], "stop")==0) {
		if (EsifWebIsStarted()) {
			EsifWebStop();
		}
		else {
			CMD_OUT("web server not started\n");
		}
	}
	// web load
	else if (esif_ccb_stricmp(argv[1], "load") == 0) {
		EsifWebLoad();
	}
	// web unload
	else if (esif_ccb_stricmp(argv[1], "unload") == 0) {
		EsifWebUnload();
	}
	return output;
}

// Returns the Legacy Primary Participant Keyname from DPTF DataVault, if any
static esif_error_t GetLegacyPrimaryKeyname(char *buf_ptr, size_t buf_len)
{
	esif_error_t rc = ESIF_E_NO_MEMORY;
	char keyspec[] = "/participants/*.D0/idsp";
	EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, "DPTF", 0, ESIFAUTOLEN);
	EsifDataPtr keyname = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
	EsifDataPtr value = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	EsifConfigFindContext context = NULL;

	if (nameSpace && keyname && value && ((rc = EsifConfigFindFirst(nameSpace, keyname, value, &context)) == ESIF_OK)) {
		// Find matching keys for /participants/XXXX.D0/idsp and return any XXXX other than IETM or DPTFZ
		do {
			EsifString thiskey = esif_str_replace((EsifString)keyname->buf_ptr, "/participants/", "");
			char *domain = NULL;
			if (thiskey && ((domain = esif_ccb_strchr(thiskey, '.')) != NULL)) {
				*domain = 0;
				if (esif_ccb_stricmp(thiskey, "IETM") != 0 && esif_ccb_stricmp(thiskey, "DPTFZ") != 0) {
					esif_ccb_strcpy(buf_ptr, thiskey, buf_len);
					esif_ccb_free(thiskey);
					break;
				}
			}
			EsifData_Set(keyname, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
			esif_ccb_free(thiskey);
		} while ((rc = EsifConfigFindNext(nameSpace, keyname, value, &context)) == ESIF_OK);
		EsifConfigFindClose(&context);
	}
	EsifData_Destroy(nameSpace);
	EsifData_Destroy(keyname);
	EsifData_Destroy(value);
	return rc;
}

static char *esif_shell_cmd_config(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *subcmd = NULL;
	char *namesp = DataBank_GetDefault();
	Bool explicit_namesp = ESIF_FALSE;
	int  explicit_opt = 0;
	int  opt = 1;
	eEsifError   rc = ESIF_OK;
	EsifDataPtr  data_nspace= NULL;
	EsifDataPtr  data_key	= NULL;
	EsifDataPtr  data_value = NULL;
	EsifDataPtr  data_targetdv = NULL;
	EsifDataType data_type = ESIF_DATA_AUTO;

	if (argc > 1) {
		subcmd = argv[opt++];
	}

	// Get Optional DataVault Parameter
	// config <subcmd> [@datavault] ...
	if (argc > 2) {
		if (argv[2][0] == '@') {
			explicit_namesp = ESIF_TRUE;
			explicit_opt = opt;
			namesp = &argv[opt++][1];
		}
	}

	// config [list]
	if (argc < 2 || esif_ccb_stricmp(subcmd, "list")==0) {
		char *line = "----------------------------------------------------------------";
		DataBankPtr DbMgr = DataBank_GetMgr();
		UInt32 j=0;
		CMD_OUT("\nName            %s%-*.*s Keys  Version Flags    Store       Size %-*s%s\n"
				  "----------------%s%-*.*s ----- ------- -------- ------ --------- %-32.32s%s%s\n",
				(argc > 1 ? " " : ""), (argc > 1 ? 16 : 0), (argc > 1 ? 16 : 0), "Segment",
				(argc > 1 ? 32 : 7), "Comment", (argc > 1 ? " Hash" : ""),
				(argc > 1 ? " " : ""), (argc > 1 ? 16 : 0), (argc > 1 ? 16 : 0), line,
				line, (argc > 1 ? " " : ""), (argc > 1 ? line : "")
			);

		esif_ccb_read_lock(&DbMgr->lock);
		for (j = 0; j < DbMgr->size; j++) {
			if (DbMgr->elements && DbMgr->elements[j] && DbMgr->elements[j]->name[0]) {
				char version_str[MAX_PATH];
				DataVaultPtr DB = DbMgr->elements[j];
				char hashstr[(sizeof(DB->digest.hash) * 2) + 1] = { 0 };

				DataVault_GetRef(DB);
				esif_ccb_read_lock(&DB->lock);
				esif_string isdefault = (esif_ccb_stricmp(DB->name, DataBank_GetDefault()) == 0 ? "+" : "");
				esif_string container = (DB->dataclass == ESIFDV_PAYLOAD_CLASS_REPO ? "Repo" : IOStream_GetType(DB->stream) == StreamMemory ? (IOStream_GetStore(DB->stream) == StoreStatic ? "Static" : "Memory") : IOStream_GetType(DB->stream) == StreamFile ? "File" : "Cache");

				esif_ccb_sprintf(sizeof(version_str), version_str, "%d.%d.%d",
					ESIFHDR_GET_MAJOR(DB->version),
					ESIFHDR_GET_MINOR(DB->version),
					ESIFHDR_GET_REVISION(DB->version));

				esif_sha256_tostring(&DB->digest, hashstr, sizeof(hashstr));

				config_flags_str_t flagstr = config_flags_str(DB->flags);
				CMD_OUT("%-16s%s%-*.*s%-1s%5d %-8s%s %-6s%10u %-*.*s%s%s\n",
					DB->name,
					(argc > 1 ? " " : ""), (argc > 1 ? 16 : 0),	(argc > 1 ? 16 : 0),
					(DB->segmentid[0] ? DB->segmentid : DB->name),
					isdefault,
					DataCache_GetCount(DB->cache),
					version_str,
					(StringPtr)&flagstr,
					container,
					(UInt32)(DB->digest.digest_bits / 8),
					(argc > 1 ? 32 : 0), (argc > 1 ? 32 : esif_ccb_strlen(DB->comment, sizeof(DB->comment))),
					DB->comment,
					(argc > 1 ? " " : ""), (argc > 1 ? hashstr : "")
					);
				esif_ccb_read_unlock(&DB->lock);
				DataVault_PutRef(DB);
			}
		}
		esif_ccb_read_unlock(&DbMgr->lock);

		CMD_OUT("\n");
	}
	// config get [@datavault] [<keyspec>] [type]
	else if (esif_ccb_stricmp(subcmd, "get")==0) {
		char *keyspec = (argc > opt ? argv[opt++] : "*");
		char *type = (argc > opt ? argv[opt++] : NULL);
		char *maxlen = (argc > opt ? argv[opt++] : NULL);
		if (type != NULL && maxlen == NULL && (isdigit(*type) || *type == '-')) {
			maxlen = type;
			type = NULL;
		}
		UInt32 max_string = (maxlen ? esif_atoi(maxlen) : 0);

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		data_value  = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		if (data_nspace==NULL || data_key==NULL || data_value==NULL) 
			goto exit;

		// Enumerate matching keys if keyspec contains "*" or "?" [unless STRING output type specified]
		if (esif_ccb_strpbrk(keyspec, "*?") != NULL && esif_ccb_stricmp((type ? type : ""), "STRING") != 0) {
			EsifConfigFindContext context = NULL;
			Bool display_hash = ESIF_FALSE;

			if (type && esif_ccb_strnicmp(type, "SHA", 3) == 0) {
				display_hash = ESIF_TRUE;
				max_string = SHA256_STRING_BYTES;
			}
			if ((rc = EsifConfigFindFirst(data_nspace, data_key, NULL, &context)) == ESIF_OK) {
				CMD_OUT("\nType          Length Flags    Key                                      %s\n"
					"----------- -------- -------- ---------------------------------------- ----------------------------------------------------------------\n",
					(display_hash ? "Hash": "Value")
				);
				do {
					esif_flags_t flags = 0;
					if ((rc = EsifConfigGetItem(data_nspace, data_key, data_value, &flags)) == ESIF_OK) {
						char *valuestr = NULL;
						size_t valuelen = 0;
						size_t j = 0;
						if (display_hash) {
							valuelen = SHA256_STRING_BYTES;
							valuestr = esif_ccb_malloc(valuelen);
							if (valuestr && data_value->buf_ptr) {
								if (type && esif_ccb_stricmp(type, "SHA1") == 0) {
									esif_sha1_t digest = { 0 };
									esif_sha1_init(&digest);
									esif_sha1_update(&digest, data_value->buf_ptr, data_value->data_len);
									esif_sha1_finish(&digest);
									esif_sha1_tostring(&digest, valuestr, valuelen);
								}
								else {
									esif_sha256_t digest = { 0 };
									esif_sha256_init(&digest);
									esif_sha256_update(&digest, data_value->buf_ptr, data_value->data_len);
									esif_sha256_finish(&digest);
									esif_sha256_tostring(&digest, valuestr, valuelen);
								}
							}
						}
						else {
							valuestr = EsifData_ToStringMax(data_value, (max_string ? max_string : 64));
							valuelen = esif_ccb_strlen(valuestr, MAX_STRINGLEN) + 1;
							for (j = 0; valuestr && j < valuelen && valuestr[j]; j++) {
								if (valuestr[j] == '\r' && valuestr[j + 1] == '\n') {
									esif_ccb_memmove(&valuestr[j], &valuestr[j + 1], esif_ccb_strlen(&valuestr[j + 1], valuelen - j) + 1);
									valuelen--;
								}
								if (valuestr[j] == '\n')
									valuestr[j] = '|';
							}
						}

						config_flags_str_t flagstr = config_flags_str(flags);
						CMD_DEBUG("%-11s %8d %-8.8s %-40.40s %.*s\n"
							,
							ltrim(esif_data_type_str(data_value->type), PREFIX_DATA_TYPE),
							data_value->data_len,
							(StringPtr)&flagstr,
							(char*)data_key->buf_ptr,
							(valuelen - 1),
							(valuestr ? valuestr : "")
							);
						esif_ccb_free(valuestr);
					}
					EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
					EsifData_Set(data_value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
				} while ((rc = EsifConfigFindNext(data_nspace, data_key, NULL, &context)) == ESIF_OK);

				CMD_OUT("\n");

				EsifConfigFindClose(&context);
				if (rc == ESIF_E_ITERATION_DONE)
					rc = ESIF_OK;
			}
		}
		// Otherwise get the key/value pair
		else {
			if ((rc = EsifConfigGet(data_nspace, data_key, data_value)) == ESIF_OK) {
				// Convert ESIF Data Type to viewable String
				char *keyvalue = EsifData_ToStringMax(data_value, max_string);
				if (keyvalue) {
					if (type && esif_ccb_stricmp(type, "STRING") != 0) {
						switch (data_value->type) {
						case ESIF_DATA_JSON:
						{
							JsonObjPtr obj = JsonObj_Create();
							StringPtr subkeyValue = NULL;
							if (obj == NULL) {
								rc = ESIF_E_NO_MEMORY;
							}
							else if ((rc = JsonObj_FromString(obj, keyvalue)) == ESIF_OK) {
								subkeyValue = JsonObj_GetValue(obj, type);
								if (subkeyValue) {
									esif_ccb_free(keyvalue);
									keyvalue = esif_ccb_strdup(subkeyValue);
									if (keyvalue == NULL) {
										rc = ESIF_E_NO_MEMORY;
									}
								}
								else {
									rc = ESIF_E_NOT_FOUND;
								}
							}
							JsonObj_Destroy(obj);
							break;
						}
						default:
							break;
						}
					}
					
					if (rc == ESIF_OK) {
						output = shell->outbuf = esif_shell_resize(esif_ccb_strlen(keyvalue, MAX_STRINGLEN) + 1);
						esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", keyvalue);
					}
					else {
						esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
					}
					esif_ccb_free(keyvalue);
				}
			}
		}

		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config set [@datavault] <key> <value> [type] [option ...]
	else if (argc > opt+1 && esif_ccb_stricmp(subcmd, "set")==0) {
		char *keyname = argv[opt++];
		char *keyvalue= argv[opt++];
		char *sep = " && ";
		char *replaced = NULL;
		esif_flags_t options = ESIF_SERVICE_CONFIG_PERSIST; // Implied unless NOPERSIST

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyname, 0, ESIFAUTOLEN);
		data_value  = EsifData_Create();
		if (data_nspace==NULL || data_key==NULL || data_value==NULL) 
			goto exit;

		// [type]
		if (argc > opt) {
			EsifDataType newtype = esif_data_type_str2enum(argv[opt]);
			if (newtype != ESIF_DATA_VOID || esif_ccb_stricmp(argv[opt], "VOID") == 0) {
				opt++;
				data_type = newtype;
			}
		}
		// Use existing data type if key already exists when [type] not specified
		else {
			EsifDataType newtype = DataBank_KeyType(namesp, keyname);
			if (newtype != (EsifDataType)0) {
				data_type = newtype;
			}
		}

		// [option ...]
		while (argc > opt) {
			options = EsifConfigFlags_Set(options, argv[opt++]);
		}

		// Replace " && " with newlines unless type is specified
		if ((data_type == ESIF_DATA_AUTO) && (esif_ccb_strstr(keyvalue, sep) != NULL)) {
			if ((replaced = esif_str_replace(keyvalue, sep, "\n")) != NULL) {
				keyvalue = replaced;
			}
		}

		// Convert Input String to ESIF Data Type
		rc = EsifData_FromString(data_value, keyvalue, data_type);
		esif_ccb_free(replaced);

		// Set Configuration Value
		if (rc == ESIF_OK) {
			rc = EsifConfigSet(data_nspace, data_key, options, data_value);
		}

		// Results
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config keys [@datavault] [<keyspec>] [<0xFLAGS>]
	else if (esif_ccb_stricmp(subcmd, "keys") == 0) {
		char *keyspec = (argc > opt ? argv[opt++] : "*");
		esif_flags_t flagspec = (esif_flags_t)(argc > opt ? esif_atoi(argv[opt++]) : 0);
		EsifConfigFindContext context = NULL;

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		if (data_nspace == NULL || data_key == NULL)
			goto exit;

		// Enumerate Matching Keys 
		if ((rc = EsifConfigFindFirst(data_nspace, data_key, NULL, &context)) == ESIF_OK) {
			do {
				Bool ismatch = (flagspec == 0);
				esif_flags_t flags = 0;
				if (flagspec) {
					DataVaultPtr DB = DataBank_GetDataVault((StringPtr)data_nspace->buf_ptr);
					if (DataVault_KeyExists(DB, (StringPtr)data_key->buf_ptr, NULL, &flags) && FLAGS_TESTALL(flags, flagspec)) {
						ismatch = ESIF_TRUE;
					}
					DataVault_PutRef(DB);
				}
				if (ismatch) {
					esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s\n", (esif_string)data_key->buf_ptr);
				}
				EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			} while ((rc = EsifConfigFindNext(data_nspace, data_key, NULL, &context)) == ESIF_OK);

			EsifConfigFindClose(&context);
			if (rc == ESIF_E_ITERATION_DONE)
				rc = ESIF_OK;
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config copy  [@datavault] <keyspec> [...] @@targetdv [option ...]
	// config merge [@datavault] <keyspec> [...] @@targetdv [option ...]
	else if (argc > opt + 1 && ((esif_ccb_stricmp(subcmd, "copy") == 0) || (esif_ccb_stricmp(subcmd, "merge") == 0))) {
		char *keyspecs = esif_ccb_strdup(argv[opt++]);
		char *targetdv = NULL;
		esif_flags_t options = ESIF_SERVICE_CONFIG_PERSIST;
		Bool replaceKeys = (esif_ccb_stricmp(subcmd, "copy") == 0);
		UInt32 keycount = 0;

		// <keyspec> [...]
		while ((keyspecs != NULL) && (argc > opt) && esif_ccb_strncmp(argv[opt], "@@", 2) != 0) {
			size_t keylen = esif_ccb_strlen(keyspecs, OUT_BUF_LEN) + esif_ccb_strlen(argv[opt], OUT_BUF_LEN) + 2;
			char *old_keyspecs = keyspecs;
			keyspecs = (char *)esif_ccb_realloc(keyspecs, keylen);
			if (keyspecs == NULL) {
				esif_ccb_free(old_keyspecs);
				break;
			}
			esif_ccb_sprintf_concat(keylen, keyspecs, "\t%s", argv[opt++]);
		}

		if (keyspecs == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if ((argc > opt) && esif_ccb_strncmp(argv[opt], "@@", 2) == 0) {
			targetdv = &argv[opt++][2];
		}

		if (namesp && keyspecs && targetdv) {
			data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
			data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspecs, 0, ESIFAUTOLEN);
			data_targetdv = EsifData_CreateAs(ESIF_DATA_STRING, targetdv, 0, ESIFAUTOLEN);
		}
		if (data_nspace == NULL || data_key == NULL || data_targetdv == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}

		// [option ...]
		while (rc == ESIF_OK && argc > opt) {
			options = EsifConfigFlags_Set(options, argv[opt++]);
		}

		// Copy or Merge Data
		if (rc == ESIF_OK) {
			rc = EsifConfigCopy(data_nspace, data_targetdv, data_key, options, replaceKeys, &keycount);
		}

		// Results
		if (rc == ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%d Keys %s from %s.dv to %s.dv\n", keycount, (replaceKeys ? "Copied" : "Merged"), namesp, targetdv);
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
		esif_ccb_free(keyspecs);
	}
	// config delete [@datavault] <keyspec>
	else if (argc > opt && esif_ccb_stricmp(subcmd, "delete")==0) {
		char *keyspec = argv[opt++];
		DataVaultPtr DB = DataBank_GetDataVault(namesp);
		UInt32 keycount = 0;

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);

		// Delete Configuration Value or all values if key = "*"
		if (data_nspace != NULL && data_key != NULL) {
			keycount = DataVault_GetKeyCount(DB);
			rc = EsifConfigDelete(data_nspace, data_key);
		}

		// Results
		if (rc == ESIF_OK) {
			keycount = keycount - DataVault_GetKeyCount(DB);
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%d Keys Deleted from %s.dv\n", keycount, namesp);
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
		DataVault_PutRef(DB);
	}
	// config save [@datavault] <key> <file>
	else if (argc > opt + 1 && esif_ccb_stricmp(subcmd, "save") == 0) {
		char *keyname = argv[opt++];
		char *fname = argv[opt++];

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyname, 0, ESIFAUTOLEN);
		data_value = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
		if (data_nspace == NULL || data_key == NULL || data_value == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else if (shell_isprohibited(fname)) {
			rc = ESIF_E_IO_INVALID_NAME;
		}

		// Save Key Value (if found) to file in bin directory
		if (rc == ESIF_OK && ((rc = EsifConfigGet(data_nspace, data_key, data_value)) == ESIF_OK) && data_value->buf_ptr != NULL) {
			char export_file[MAX_PATH] = { 0 };
			int errnum = 0;
			FILE *fp = NULL;

			esif_build_path(export_file, sizeof(export_file), ESIF_PATHTYPE_BIN, fname, NULL);
			fp = esif_ccb_fopen(export_file, "wb", &errnum);
			if (fp == NULL) {
				rc = ESIF_E_IO_OPEN_FAILED;
			}
			else {
				u32 data_len = data_value->data_len;
				// Trim NUL terminator from strings before saving to file
				if ((data_value->type == ESIF_DATA_STRING || data_value->type == ESIF_DATA_JSON || data_value->type == ESIF_DATA_XML)
					&& (data_len > 0) && ((StringPtr)data_value->buf_ptr)[data_len - 1] == '\0') {
					data_len--;
				}
				if (esif_ccb_fwrite(data_value->buf_ptr, 1, data_len, fp) != data_len) {
					rc = ESIF_E_IO_ERROR;
				}
				else {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%u bytes saved to %s\n", data_len, export_file);
				}
				esif_ccb_fclose(fp);
			}
		}
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config exec [@datavault] <keyspec>
	else if (argc > opt && esif_ccb_stricmp(subcmd, "exec")==0) {
		char *keyspec = argv[opt++];

		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key    = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		data_value  = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

		if (data_nspace != NULL && data_key != NULL && data_value != NULL) {
			EsifConfigFindContext context = NULL;

			// Enumerate and Execute Matching Keys 
			if ((rc = EsifConfigFindFirst(data_nspace, data_key, data_value, &context)) == ESIF_OK) {
				do {
					esif_uf_shell_unlock();
					parse_cmd((char *)data_value->buf_ptr, ESIF_FALSE, ESIF_TRUE);
					esif_uf_shell_lock();

					output = shell->outbuf = g_outbuf;
					*output = '\0';
					EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
					EsifData_Set(data_value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
				} while ((rc = EsifConfigFindNext(data_nspace, data_key, data_value, &context)) == ESIF_OK);

				EsifConfigFindClose(&context);
				if (rc == ESIF_E_ITERATION_DONE)
					rc = ESIF_OK;
			}
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config export {[@datavault] <keyspec> [...] ...} @@targetdv [asl|dv|dvx|repo] ["comment"] [targetname]
	else if (argc > opt && esif_ccb_stricmp(subcmd, "export") == 0) {
		char *outformat = "repo";
		char *sourcedv = namesp;
		char *targetdv = "gddv";
		char *targetname = targetdv;
		char *targetrepo = NULL;
		char *comment = NULL;
		char targetdvtemp[ESIFDV_NAME_LEN + 1] = { 0 };
		UInt32 keycount = 0;
		char repofile[ESIF_NAME_LEN] = { 0 };
		char export_file[MAX_PATH] = { 0 };
		int optstart = opt;
		Bool isRest = g_isRest;
		char cmdline[MAX_LINE] = { 0 };
		int dvcount = 0;

		rc = ESIF_OK;

		// Skip to ... @@targetdv [asl|dv|dvx|repo] [<comment>] [targetname]
		for (opt = optstart; argc > opt; opt++) {
			if (esif_ccb_strncmp(argv[opt], "@@", 2) == 0) {
				targetdv = &argv[opt++][2];
				targetname = targetdv;

				// [asl|dv|dvx|repo]
				if (argc > opt) {
					outformat = argv[opt++];
				}
				// [<comment>]
				if (argc > opt) {
					comment = argv[opt++];
				}
				// [targetname]
				if (argc > opt) {
					targetname = argv[opt++];
				}
				// Validate Filenames
				if (shell_isprohibited(targetdv) || shell_isprohibited(targetname)) {
					rc = ESIF_E_IO_INVALID_NAME;
				}
				break;
			}
		}
		
		// Export each [@datavault] <keyspec> [...] to temporary $$datavault.dv
		opt = optstart;
		while ((rc == ESIF_OK) && (argc > opt) && (esif_ccb_strncmp(argv[opt], "@@", 2) != 0)) {
			// Only first [@datavault] is optional
			if (*argv[opt] == '@') {
				sourcedv = &argv[opt++][1];
			}
			char *keyspecs = esif_ccb_strdup(argv[opt++]);
			while ((rc == ESIF_OK) && (keyspecs != NULL) && (argc > opt) && (*argv[opt] != '@')) {
				size_t keylen = esif_ccb_strlen(keyspecs, OUT_BUF_LEN) + esif_ccb_strlen(argv[opt], OUT_BUF_LEN) + 2;
				char *new_keyspecs = (char *)esif_ccb_realloc(keyspecs, keylen);
				if (new_keyspecs == NULL) {
					esif_ccb_free(keyspecs);
					keyspecs = NULL;
					break;
				}
				keyspecs = new_keyspecs;
				esif_ccb_sprintf_concat(keylen, keyspecs, "\t%s", argv[opt++]);
			}

			if (keyspecs == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				// If exporting to an Embedded Repo, replace "targetdv" with "$$datavault" and embed in "targetdv.dvx"
				if (esif_ccb_stricmp(outformat, "repo") == 0) {
					esif_ccb_sprintf(sizeof(targetdvtemp), targetdvtemp, "%s%s", ESIFDV_TEMP_PREFIX, sourcedv);
					if (dvcount == 0) {
						targetrepo = targetdv;
					}
					targetdv = targetdvtemp;
				}

				// Step 1: Copy Data to targetdv.dv and Close targetdv.dv, unless targetdv.dv already exists
				DataVaultPtr DB = DataBank_GetDataVault(targetdv);
				if (DB != NULL) {
					rc = ESIF_E_IO_ALREADY_EXISTS;
				}
				DataVault_PutRef(DB);

				if (rc == ESIF_OK) {
					if (esif_ccb_stricmp(outformat, "repo") == 0) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config open @%s", targetdv);
						parse_cmd(cmdline, isRest, ESIF_TRUE);
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config id @%s %s", targetdv, sourcedv);
						parse_cmd(cmdline, isRest, ESIF_TRUE);
					}

					esif_ccb_sprintf(sizeof(cmdline), cmdline, "config copy @%s %s @@%s", sourcedv, keyspecs, targetdv);
					parse_cmd(cmdline, isRest, ESIF_TRUE);

					esif_ccb_sprintf(sizeof(cmdline), cmdline, "config rekey @%s", targetdv);
					parse_cmd(cmdline, isRest, ESIF_TRUE);

					DB = DataBank_GetDataVault(targetdv);
					keycount += DataVault_GetKeyCount(DB);
					DataVault_PutRef(DB);

					if (esif_ccb_stricmp(outformat, "repo") == 0 || esif_ccb_stricmp(outformat, "asl") == 0) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config close @%s", targetdv);
						parse_cmd(cmdline, isRest, ESIF_TRUE);
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config doc @%s %s.csv", targetdv, targetrepo);
						if (comment) {
							esif_ccb_sprintf_concat(sizeof(cmdline), cmdline, " \"%s\" %s%s", comment, sourcedv, (dvcount > 0 ? " append" : ""));
						}
						parse_cmd(cmdline, isRest, ESIF_TRUE);
					}
					if (esif_ccb_stricmp(outformat, "repo") == 0) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config append %s%s.bin %s.dv", ESIFDV_EXPORT_PREFIX, targetrepo, targetdv);
						parse_cmd(cmdline, isRest, ESIF_TRUE);
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config drop @%s", targetdv);
						parse_cmd(cmdline, isRest, ESIF_TRUE);
					}
				}
			}
			esif_ccb_free(keyspecs);
			dvcount++;
		}

		// Delete existing export file, if any
		if (rc == ESIF_OK) {
			if (esif_ccb_strnicmp(outformat, "dv", 2) == 0) {
				esif_build_path(export_file, sizeof(export_file), ESIF_PATHTYPE_DV, targetdv, (esif_ccb_stricmp(outformat, "dv") == 0 ? ESIFDV_FILEEXT : ESIFDV_REPOEXT));
			}
			else {
				esif_build_path(export_file, sizeof(export_file), ESIF_PATHTYPE_LOG, targetname, ".asl");
			}
			if (esif_ccb_file_exists(export_file) && esif_ccb_unlink(export_file) != EOK) {
				rc = ESIF_E_IO_DELETE_FAILED;
			}
		}

		// Step 2: Convert targetdv.dv to targetdv.asl and targetdv.csv and delete targetdv.dv
		if (rc == ESIF_OK) {
			// asl = Export to ASL File for compiling into BIOS and CSV for Documentation
			if (esif_ccb_stricmp(outformat, "asl") == 0) {
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config asl encode %s.dv $log/%s.asl", targetdv, targetname);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config replace %s.dv %s.bin", targetdv, targetname);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(repofile), repofile, "%s.bin", targetname);
			}
			// dv  = Single-Segment DataVault Repository (Copy into Cache)
			else if (esif_ccb_stricmp(outformat, "dv") == 0) {
				// Leave existing targetdv.dv in place and do nothing
			}
			// dvx = Single-Segment DataVault Repository (Merge into Cache)
			else if (esif_ccb_stricmp(outformat, "dvx") == 0) {
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config replace %s%s %s%s", targetdv, ESIFDV_FILEEXT, targetdv, ESIFDV_REPOEXT);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
			}
			// repo = Single-Segment Repository with Single-Segment DV Repo Embedded as a REPO Payload
			else if (esif_ccb_stricmp(outformat, "repo") == 0 && targetrepo != NULL) {
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config close @%s", targetrepo);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config open @%s", targetrepo);
				if (comment) {
					esif_ccb_sprintf_concat(sizeof(cmdline), cmdline, " \"%s\"", comment);
				}
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config payload @%s $dv/%s%s.bin REPO compress", targetrepo, ESIFDV_EXPORT_PREFIX, targetrepo);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config close @%s", targetrepo);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config drop %s%s.bin %s.bin", ESIFDV_EXPORT_PREFIX, targetrepo, targetrepo);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config replace %s.dvx %s.bin", targetrepo, targetrepo);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config asl encode %s.bin $log/%s.asl", targetrepo, targetname);
				parse_cmd(cmdline, isRest, ESIF_TRUE);
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "config open @%s", targetrepo);
				if (comment) {
					esif_ccb_sprintf_concat(sizeof(cmdline), cmdline, " \"%s\"", comment);
					parse_cmd(cmdline, isRest, ESIF_TRUE);
				}
				esif_ccb_sprintf(sizeof(repofile), repofile, "%s.bin", targetrepo);
			}

			// Verify export file created
			if (esif_ccb_file_exists(export_file) == ESIF_FALSE) {
				rc = ESIF_E_IO_OPEN_FAILED;
			}
		}

		if (rc == ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%d keys in %d vaults exported to %s\n", keycount, dvcount, export_file);
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config gddv <set|reset|backup|restore|mode>
	else if (argc > opt && esif_ccb_stricmp(subcmd, "gddv") == 0) {
		char *filename = NULL;
		char cmdline[MAX_PATH] = { 0 };
		EsifString loadedApps[ESIF_MAX_APPS] = { 0 };
		AppMgrIterator appIter = { 0 };
		eEsifError appIterRc = AppMgr_InitIterator(&appIter);
		size_t idx = 0;
		struct {
			StringPtr name;	// DV name
			Bool isExported;// DV may contain Exported data
			Bool isLoaded;	// DV currently loaded?
		} gddvDVs[] = {
			{ "dptf",		ESIF_TRUE,	ESIF_FALSE },
			{ "override",	ESIF_TRUE,	ESIF_FALSE },
			{ "platform",	ESIF_TRUE,	ESIF_FALSE },
			{ "gddv",		ESIF_FALSE,	ESIF_FALSE },
		};

		// Get list of loaded apps so we can stop/restart them after setting/resetting GDDV override
		if (appIterRc == ESIF_OK) {
			EsifAppPtr appPtr = NULL;
			while ((appIterRc = AppMgr_GetNextApp(&appIter, &appPtr)) == ESIF_OK) {
				if (EsifApp_IsRestartable(appPtr)) {
					loadedApps[idx++] = EsifApp_CopyAppFullName(appPtr);
				}
			}
		}

		// Get list of loaded DataVaults loaded from GDDV so we can close/reload them when setting/resetting GDDV override
		for (idx = 0; idx < ESIF_ARRAY_LEN(gddvDVs); idx++) {
			DataVaultPtr DB = DataBank_GetDataVault(gddvDVs[idx].name);
			if (DB != NULL) {
				gddvDVs[idx].isLoaded = ESIF_TRUE;
				DataVault_PutRef(DB);
			}
		}

		// config gddv set <filename|@@factory>
		// config gddv backup
		if (esif_ccb_stricmp(argv[opt], "set") == 0 || esif_ccb_stricmp(argv[opt], "backup") == 0) {
			// Options
			const StringPtr factoryReset = "@@factory";
			char logicalFilename[MAX_PATH] = { 0 };
			Bool doBackup = ESIF_FALSE;
			StringPtr optcmd = argv[opt++];
			rc = ESIF_OK;

			if (esif_ccb_stricmp(optcmd, "backup") == 0) {
				doBackup = ESIF_TRUE;
			}
			else if (argc > opt) {
				filename = argv[opt++];
				// Assume $dv/file.bin if no path specified
				if (esif_ccb_stricmp(filename, factoryReset) != 0 && *filename != '$') {
					esif_ccb_sprintf(sizeof(logicalFilename), logicalFilename, "$dv/%s", filename);
					filename = logicalFilename;
				}
				if (shell_isprohibited(filename)) {
					filename = NULL;
					rc = ESIF_E_IO_INVALID_NAME;
				}
			}
			else {
				rc = ESIF_E_PARAMETER_IS_NULL;
			}

			// Verify Repo Filename exists or this is a Factory Reset
			if ((filename != NULL) && (esif_ccb_stricmp(filename, factoryReset) != 0)) {
				char repopath[MAX_PATH] = { 0 };
				rc = DataVault_TranslatePath(filename, repopath, sizeof(repopath));
				if ((rc == ESIF_OK) && (esif_ccb_file_exists(repopath) == ESIF_FALSE)) {
					rc = ESIF_E_NOT_FOUND;
				}
				// Verify that filename is a valid Data Repository file
				else if (rc == ESIF_OK) {
					DataRepoPtr repo = DataRepo_Create();
					if (repo == NULL) {
						rc = ESIF_E_NO_MEMORY;
					}
					else if (IOStream_OpenFile(repo->stream, StoreReadWrite, repopath, "rb") != EOK) {
						rc = ESIF_E_IO_OPEN_FAILED;
					}
					else {
						DataRepoInfo info = { 0 };
						rc = DataRepo_GetInfo(repo, &info);
					}
					DataRepo_Destroy(repo);
				}
			}

			// Valid Repo Filename exists or Factory Reset or Backup Only = Set GDDV Override (if any) and Reload GDDV
			if ((rc == ESIF_OK) && (filename || doBackup)) {
				// Stop all Apps and Destroy Dynamic Participants
				if (filename) {
					for (idx = 0; idx < ESIF_MAX_APPS && loadedApps[idx] != NULL; idx++) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "appstop %s", loadedApps[idx]);
						parse_cmd(cmdline, g_isRest, ESIF_TRUE);
					}
					DestroyDynamicParticipants();
				}

				// Close and Drop or Backup all GDDV-related DataVaults
				for (idx = 0; idx < ESIF_ARRAY_LEN(gddvDVs); idx++) {
					if (filename) {
						if (gddvDVs[idx].isLoaded) {
							esif_ccb_sprintf(sizeof(cmdline), cmdline, "config close @%s", gddvDVs[idx].name);
							parse_cmd(cmdline, g_isRest, ESIF_TRUE);
						}
						if (gddvDVs[idx].isExported) {
							esif_ccb_sprintf(sizeof(cmdline), cmdline, "config drop @%s", gddvDVs[idx].name);
							parse_cmd(cmdline, g_isRest, ESIF_TRUE);
						}
					}
					// Backup-only Option: Lock each DataVault (if open) and make a Backup without closing or opening the cache
					else if (doBackup && gddvDVs[idx].isExported) {
						char dvpath[MAX_PATH] = { 0 };
						esif_build_path(dvpath, sizeof(dvpath), ESIF_PATHTYPE_DV, gddvDVs[idx].name, ESIFDV_FILEEXT);

						if (esif_ccb_file_exists(dvpath)) {
							esif_ccb_sprintf(sizeof(cmdline), cmdline, "config copyto %s.bak %s" ESIFDV_FILEEXT, gddvDVs[idx].name, gddvDVs[idx].name);

							DataVaultPtr DB = DataBank_GetDataVault(gddvDVs[idx].name);
							if (DB) {
								esif_ccb_write_lock(&DB->lock);
								parse_cmd(cmdline, g_isRest, ESIF_TRUE);
								esif_ccb_write_unlock(&DB->lock);
								DataVault_PutRef(DB);
							}
							else {
								parse_cmd(cmdline, g_isRest, ESIF_TRUE);
							}
						}
					}
				}

				// Set GDDV Override, except for Factory Reset
				if (filename && esif_ccb_stricmp(filename, factoryReset) != 0) {
					esif_ccb_sprintf(sizeof(cmdline), cmdline, "setp_bf %d D0 255 \"binary:%s\"", SET_CONFIG_DATAVAULT, filename);
					parse_cmd(cmdline, g_isRest, ESIF_TRUE);
				}

				// Reload GDDV from override.dv or BIOS if Factory Reset
				if (filename) {
					esif_ccb_sprintf(sizeof(cmdline), cmdline, "getp %d", GET_CONFIG);
					parse_cmd(cmdline, g_isRest, ESIF_TRUE);

					// Create New Dynamic Participants
					CreateDynamicParticipants();

					// Restart all Apps
					for (idx = 0; idx < ESIF_MAX_APPS && loadedApps[idx] != NULL; idx++) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "appstart %s", loadedApps[idx]);
						parse_cmd(cmdline, g_isRest, ESIF_TRUE);
					}
				}
				*output = 0;
				rc = ESIF_OK;
			}
		}
		// config gddv reset
		// config gddv restore
		// config gddv mode
		else if (esif_ccb_stricmp(argv[opt], "reset") == 0 || esif_ccb_stricmp(argv[opt], "restore") == 0 || esif_ccb_stricmp(argv[opt], "mode") == 0) {
			// Verify that Override Exists
			esif_handle_t participantId = 0;
			EsifData request = { ESIF_DATA_VOID, NULL, 0, 0 };
			EsifData response = { ESIF_DATA_BINARY, NULL, ESIF_DATA_ALLOCATE, 0 };
			EsifPrimitiveActionSelector actSelector = { ESIF_PRIM_ACT_SEL_FLAG_TYPE_VALID, 0, ESIF_ACTION_CONFIG };
			EsifDataPtr sarPtr = NULL;
			StringPtr optcmd = argv[opt++];

			// Create Specific Action Request to read GDDV object using CONFIG Action(s) only
			sarPtr = EsifCreateSpecificActionRequest(
				GET_CONFIG_DATAVAULT_SUR,
				"D0",
				255,
				&request,
				&response,
				&actSelector);

			if (sarPtr == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				// Load GDDV Object using CONFIG Action(s) only, if any
				rc = EsifExecutePrimitive(
					participantId,
					SET_SPECIFIC_ACTION_PRIMITIVE,
					"D0",
					255,
					sarPtr,
					&response
				);

				// We don't actually need the GDDV data, we only need to know whether a GDDV override exists or not,
				// so if we are told that we need a larger buffer, use that as confirmation that the override exists.
				if (rc == ESIF_E_NEED_LARGER_BUFFER) {
					rc = ESIF_OK;
				}
			}

			// GDDV Override Found - Reset Override and Reload GDDV
			if ((rc == ESIF_OK && esif_ccb_stricmp(optcmd, "reset") == 0) || esif_ccb_stricmp(optcmd, "restore") == 0) {
				// Options
				Bool doRestore = ESIF_FALSE;
				if (esif_ccb_stricmp(optcmd, "restore") == 0) {
					doRestore = ESIF_TRUE;
				}

				// Stop all Apps and Destroy all Dynamic Participants
				for (idx = 0; idx < ESIF_MAX_APPS && loadedApps[idx] != NULL; idx++) {
					esif_ccb_sprintf(sizeof(cmdline), cmdline, "appstop %s", loadedApps[idx]);
					parse_cmd(cmdline, g_isRest, ESIF_TRUE);
				}
				DestroyDynamicParticipants();

				// Delete GDDV Override, if any, if not doing a Restore
				if (doRestore == ESIF_FALSE && rc == ESIF_OK) {
					esif_ccb_sprintf(sizeof(cmdline), cmdline, "rstp %d D0 255", SET_CONFIG_DATAVAULT);
					parse_cmd(cmdline, g_isRest, ESIF_TRUE);
				}

				// Close, Restore, and Reopen all GDDV-related DataVaults
				for (idx = 0; idx < ESIF_ARRAY_LEN(gddvDVs); idx++) {
					if (gddvDVs[idx].isLoaded) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config close @%s", gddvDVs[idx].name);
						parse_cmd(cmdline, g_isRest, ESIF_TRUE);
					}
					if (doRestore && gddvDVs[idx].isExported) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config drop @%s", gddvDVs[idx].name);
						parse_cmd(cmdline, g_isRest, ESIF_TRUE);
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config replace %s.bak %s" ESIFDV_FILEEXT, gddvDVs[idx].name, gddvDVs[idx].name);
						parse_cmd(cmdline, g_isRest, ESIF_TRUE);
					}
					if (gddvDVs[idx].isExported) {
						esif_ccb_sprintf(sizeof(cmdline), cmdline, "config open @%s", gddvDVs[idx].name);
						parse_cmd(cmdline, g_isRest, ESIF_TRUE);
					}
				}

				// Reload GDDV object from BIOS (if any exists)
				esif_ccb_sprintf(sizeof(cmdline), cmdline, "getp %d", GET_CONFIG);
				parse_cmd(cmdline, g_isRest, ESIF_TRUE);

				// Create New Dynamic Participants
				CreateDynamicParticipants();

				// Restart all Apps
				for (idx = 0; idx < ESIF_MAX_APPS && loadedApps[idx] != NULL; idx++) {
					esif_ccb_sprintf(sizeof(cmdline), cmdline, "appstart %s", loadedApps[idx]);
					parse_cmd(cmdline, g_isRest, ESIF_TRUE);
				}
				*output = 0;
				rc = ESIF_OK;
			}
			// Assume Emulation mode if there is a GDDV override
			else if (esif_ccb_stricmp(optcmd, "mode") == 0) {
				StringPtr mode = "Normal";

				// Reload GDDV Object using CONFIG Action(s) to check for Emulation mode
				EsifData reloadResponse = { ESIF_DATA_BINARY, NULL, ESIF_DATA_ALLOCATE, 0 };
				if (EsifExecutePrimitive(
					participantId,
					SET_SPECIFIC_ACTION_PRIMITIVE,
					"D0",
					255,
					sarPtr,
					&reloadResponse) == ESIF_OK) {
					mode = "Emulation";
				}
				esif_ccb_free(reloadResponse.buf_ptr);
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", mode);
				rc = ESIF_OK;
			}
			esif_ccb_free(response.buf_ptr);
			esif_ccb_free(sarPtr);
		}
		// config gddv status
		else if (esif_ccb_stricmp(argv[opt], "status") == 0) {
			StringPtr GddvOverrideKey = "/participants/IETM.D0/gddv";
			StringPtr keyspec = "/*";
			Bool ismodified = ESIF_FALSE;
			
			rc = ESIF_OK;
			data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);

			// Check every Exported GDDV-related DataVault for Persisted changes
			for (idx = 0; !ismodified && idx < ESIF_ARRAY_LEN(gddvDVs); idx++) {
				if (gddvDVs[idx].isExported && gddvDVs[idx].isLoaded) {
					data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, gddvDVs[idx].name, 0, ESIFAUTOLEN);
					if (data_nspace && data_key) {
						EsifConfigFindContext context = NULL;

						// GDDV Status is Dirty if any Persisted keys exist starting with "/" except for GDDV override
						if ((rc = EsifConfigFindFirst(data_nspace, data_key, NULL, &context)) == ESIF_OK) {
							do {
								esif_flags_t flags = 0;
								DataVaultPtr DB = DataBank_GetDataVault((StringPtr)data_nspace->buf_ptr);
								if (esif_ccb_stricmp((StringPtr)data_key->buf_ptr, GddvOverrideKey) != 0 &&
									DataVault_KeyExists(DB, (StringPtr)data_key->buf_ptr, NULL, &flags) &&
									FLAGS_TEST(flags, ESIF_SERVICE_CONFIG_PERSIST)) {
									ismodified = ESIF_TRUE;
								}
								DataVault_PutRef(DB);
								EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
							} while (!ismodified && (rc = EsifConfigFindNext(data_nspace, data_key, NULL, &context)) == ESIF_OK);

							EsifConfigFindClose(&context);
						}
						if (rc == ESIF_E_ITERATION_DONE || rc == ESIF_E_NOT_FOUND) {
							rc = ESIF_OK;
						}
						EsifData_Destroy(data_nspace);
						data_nspace = NULL;
					}
				}
			}
			if (rc == ESIF_OK) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", (ismodified ? "Modified" : "Unchanged"));
			}
		}
		else {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
		}

		for (idx = 0; idx < ESIF_MAX_APPS && loadedApps[idx] != NULL; idx++) {
			esif_ccb_free(loadedApps[idx++]);
		}

		if (!output[0]) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config asl <encode|decode> <infile> <outfile>
	else if (esif_ccb_stricmp(subcmd, "asl") == 0 && argc > opt + 2) {
		StringPtr action = argv[opt++];
		StringPtr infile = argv[opt++];
		StringPtr outfile = argv[opt++];
		StringPtr outext = esif_ccb_strrchr(outfile, '.');
		Bool isencode = (esif_ccb_stricmp(action, "encode") == 0);

		rc = ESIF_E_NOT_SUPPORTED;

		// Filenames must not contain relative paths and output file cannot contain .dv or .dvx extensions
		if (!isencode && esif_ccb_stricmp(action, "decode") != 0) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}
		else if (!shell_isprohibited(infile) && !shell_isprohibited(outfile) &&
			(!outext || ((esif_ccb_stricmp(outext, ESIFDV_FILEEXT) != 0) && (esif_ccb_stricmp(outext, ESIFDV_REPOEXT) != 0)))) {

			char inlogical[MAX_PATH] = { 0 };
			char outlogical[MAX_PATH] = { 0 };
			char inpath[MAX_PATH] = { 0 };
			char outpath[MAX_PATH] = { 0 };
			StringPtr inmode = "rb";
			StringPtr outmode = (isencode ? "w" : "wb");
			FILE *infp = NULL;
			FILE *outfp = NULL;

			// Assume $dv/<filename> if no path specified
			if (*infile != '$') {
				esif_ccb_sprintf(sizeof(inlogical), inlogical, "$dv/%s", infile);
				infile = inlogical;
			}
			if (*outfile != '$') {
				esif_ccb_sprintf(sizeof(outlogical), outlogical, "$dv/%s", outfile);
				outfile = outlogical;
			}

			// Open Input/Output Files and Encode/Decode Bytes to Hex Strings ("0xFF")
			if (((rc = DataVault_TranslatePath(infile, inpath, sizeof(inpath))) == ESIF_OK) &&
				((rc = DataVault_TranslatePath(outfile, outpath, sizeof(outpath))) == ESIF_OK)) {

				struct stat st = { 0 };
				const size_t bufsize = 4096;
				UInt8 *filebuf = esif_ccb_malloc(bufsize);

				if (filebuf == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else if ((esif_ccb_stat(inpath, &st) == 0) &&
					((isencode && st.st_size > MAX_FILE_DECODED_LEN) || (!isencode && st.st_size > MAX_FILE_ENCODED_LEN))) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
				}
				else if ((infp = esif_ccb_fopen(inpath, inmode, NULL)) == NULL) {
					rc = ESIF_E_NOT_FOUND;
				}
				else if ((outfp = esif_ccb_fopen(outpath, outmode, NULL)) == NULL) {
					rc = ESIF_E_IO_OPEN_FAILED;
				}
				else {
					const size_t linewrap = 16;
					size_t total_bytes_read = 0;
					size_t total_bytes_encoded = 0;
					size_t bytes_read = 0;
					size_t bytes_remaining = 0;

					rc = ESIF_OK;
					while ((bytes_read = esif_ccb_fread(filebuf + bytes_remaining, bufsize - bytes_remaining, 1, bufsize - bytes_remaining, infp)) > 0) {
						if (isencode) {
							// Only encode DataVault files
							UInt16 RepoSignature = 0x1FE5; // [E5] [1F]
							if (total_bytes_read == 0 && (bytes_read < sizeof(RepoSignature) || RepoSignature != *(UInt16 *)filebuf)) {
								rc = ESIF_E_NOT_SUPPORTED;
							}

							// Encode each byte as "0xFF" with a comma after each byte (except last) and a newline every 16 bytes
							for (size_t j = 0; rc == ESIF_OK && j < bytes_read; j++) {
								char hexbuf[8] = { 0 };
								size_t towrite = 0;
								if (total_bytes_encoded) {
									hexbuf[towrite++] = ',';
								}
								if (total_bytes_encoded && (total_bytes_encoded % linewrap) == 0) {
									hexbuf[towrite++] = '\n';
								}
								hexbuf[towrite++] = '0';
								hexbuf[towrite++] = 'x';
								hexbuf[towrite++] = ((filebuf[j] & 0xF0) >> 4) + (((filebuf[j] & 0xF0) >> 4) >= 0x0A ? 'A' - 0x0A : '0');
								hexbuf[towrite++] = (filebuf[j] & 0x0F) + ((filebuf[j] & 0x0F) >= 0x0A ? 'A' - 0x0A : '0');

								size_t written = esif_ccb_fwrite(hexbuf, 1, towrite, outfp);
								if (written != towrite) {
									rc = ESIF_E_IO_ERROR;
									break;
								}
								total_bytes_encoded++;
							}
						}
						else {
							// Decode each "0xFF" hex string as a binary byte, ignoring whitespace and commas
							bytes_read += bytes_remaining;
							bytes_remaining = 0;
							for (size_t j = 0; rc == ESIF_OK && j < bytes_read; j++) {
								if (j + 4 > bytes_read) {
									bytes_remaining = bytes_read - j;
									esif_ccb_memmove(filebuf, &filebuf[j], bytes_remaining);
									break;
								}
								else if (filebuf[j] == '0' && filebuf[j + 1] == 'x' && isxdigit(filebuf[j + 2]) && isxdigit(filebuf[j + 3])) {
									UInt8 outbyte = (UInt8)(
										((isdigit(filebuf[j + 2]) ? filebuf[j + 2] - '0' : toupper(filebuf[j + 2]) - 'A' + 10) << 4) |
										(isdigit(filebuf[j + 3]) ? filebuf[j + 3] - '0' : toupper(filebuf[j + 3]) - 'A' + 10)
										);
									if (esif_ccb_fwrite(&outbyte, 1, sizeof(outbyte), outfp) != sizeof(outbyte)) {
										rc = ESIF_E_IO_ERROR;
										break;
									}
									j += 3;
								}
								else if (esif_ccb_strchr("\n\r\t ,", (const char)filebuf[j]) == NULL) {
									rc = ESIF_E_IO_ERROR;
								}
							}
						}
						if (rc != ESIF_OK) {
							break;
						}
						total_bytes_read += bytes_read - bytes_remaining;
					}

					// Sanity Check
					if (rc == ESIF_OK) {
						if ((isencode && (total_bytes_read < (size_t)st.st_size || total_bytes_encoded < total_bytes_read)) ||
							(!isencode && (total_bytes_read < (size_t)st.st_size))) {
							rc = ESIF_E_IO_ERROR;
						}
					}
				}

				if (infp) {
					esif_ccb_fclose(infp);
				}
				if (outfp) {
					esif_ccb_fclose(outfp);
					if (rc != ESIF_OK) {
						IGNORE_RESULT(esif_ccb_unlink(outpath));
					}
				}
				esif_ccb_free(filebuf);
			}
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config doc <@datavault> <file.csv> [comment] [label] [append]
	else if (esif_ccb_strnicmp(subcmd, "doc", 3) == 0 && argc > opt) {
		char dv_path[MAX_PATH] = { 0 };
		char doc_path[MAX_PATH] = { 0 };
		FILE *fpout = NULL;
		StringPtr doc_file = argv[opt++];
		StringPtr comment = NULL;
		char label[ESIFDV_NAME_LEN+1] = "GDDV";
		StringPtr open_mode = "w";
		DataVaultPtr DB = NULL;
		EsifConfigFindContext context = NULL;
		int item = 0;
		esif_string keyspec = "*";

		// [comment]
		if (argc > opt) {
			comment = argv[opt++];
		}

		// [label]
		if (argc > opt) {
			esif_ccb_strcpy(label, argv[opt++], sizeof(label));
			esif_ccb_strupr(label, sizeof(label));
		}
		// [append]
		if (argc > opt) {
			if (esif_ccb_stricmp(argv[opt++], "append") == 0) {
				open_mode = "a";
			}
		}
		esif_build_path(dv_path, sizeof(dv_path), ESIF_PATHTYPE_DV, namesp, ESIFDV_FILEEXT);
		esif_build_path(doc_path, sizeof(doc_path), ESIF_PATHTYPE_LOG, doc_file, NULL);

		// Validate Filenames
		if (shell_isprohibited(doc_file)) {
			rc = ESIF_E_IO_INVALID_NAME;
		}

		// Can only export Closed Datavaults
		if ((rc == ESIF_OK) && (DB = DataBank_GetDataVault(namesp)) != NULL) {
			rc = ESIF_E_IO_ALREADY_EXISTS;
			DataVault_PutRef(DB);
			DB = NULL;
		}
		if (rc == ESIF_OK) {
			rc = DataBank_ImportDataVault(namesp);
			DB = DataBank_GetDataVault(namesp);
			if (comment == NULL && DB != NULL && DB->comment[0]) {
				comment = DB->comment;
			}
		}
		if ((rc == ESIF_OK) && (fpout = esif_ccb_fopen(doc_path, open_mode, NULL)) == NULL) {
			rc = ESIF_E_IO_OPEN_FAILED;
		}

		EsifData_Destroy(data_nspace);
		EsifData_Destroy(data_key);
		EsifData_Destroy(data_value);
		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
		data_value = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);

		// Export @datavault to file.csv
		if (rc == ESIF_OK && data_nspace != NULL && data_key != NULL && data_value != NULL) {
			if (*open_mode == 'w') {
				esif_ccb_fprintf(fpout, "AppVersion,Description\n");
				esif_ccb_fprintf(fpout, "%s,%s\n\n", ESIF_UF_VERSION, (comment ? comment : ""));
			}
			esif_ccb_fprintf(fpout, "%s\n", label);
			if ((rc = EsifConfigFindFirst(data_nspace, data_key, NULL, &context)) == ESIF_OK) {
				do {
					esif_flags_t flags = 0;
					if ((rc = EsifConfigGetItem(data_nspace, data_key, data_value, &flags)) == ESIF_OK) {
						char *valuestr = EsifData_ToString(data_value);
						if (valuestr) {
							char *ch = NULL;
							while ((ch = esif_ccb_strpbrk(valuestr, "\r\n")) != NULL) {
								*ch = ' ';
							}
						}
						item++;
						esif_ccb_fprintf(fpout,
							"%s,%s,%u,%s\n",
							(char*)data_key->buf_ptr,
							ltrim(esif_data_type_str(data_value->type), PREFIX_DATA_TYPE),
							data_value->data_len,
							(valuestr ? valuestr : "")
						);
						esif_ccb_free(valuestr);
					}
					EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
					EsifData_Set(data_value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
				} while ((rc = EsifConfigFindNext(data_nspace, data_key, NULL, &context)) == ESIF_OK);

				EsifConfigFindClose(&context);
				if (rc == ESIF_E_ITERATION_DONE)
					rc = ESIF_OK;
			}
			esif_ccb_fprintf(fpout, "\n");
		}
		if (fpout) {
			esif_ccb_fclose(fpout);
			fpout = NULL;
		}
		if (DB) {
			DataVault_PutRef(DB);
			DataBank_CloseDataVault(namesp);
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config version [@datavault] <major>[.<minor>[.<revision>]]
	else if (esif_ccb_stricmp(subcmd, "version") == 0 && argc > opt) {
		StringPtr version = argv[opt++];
		StringPtr dot = NULL;
		UInt32 major = esif_atoi(version);
		UInt32 minor = ((dot = esif_ccb_strchr(version, '.')) != NULL ? esif_atoi(++dot) : 0);
		UInt32 revision = (dot && (dot = esif_ccb_strchr(dot, '.')) != NULL ? esif_atoi(++dot) : 0);

		DataVaultPtr DB = DataBank_GetDataVault(namesp);
		if (DB != NULL) {
			if (major > 0) {
				esif_ccb_write_lock(&DB->lock);
				DB->version = ESIFHDR_VERSION(major, minor, revision);
				esif_ccb_write_unlock(&DB->lock);
				data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
				rc = EsifConfigSet(data_nspace, NULL, 0, NULL);
			}
			DataVault_PutRef(DB);
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config comment <@datavault|repo.bin> [<comment>]
	else if (esif_ccb_stricmp(subcmd, "comment") == 0) {
		if (explicit_namesp) { // @datavault
			DataVaultPtr DB = DataBank_GetDataVault(namesp);
			if (DB != NULL) {
				if (argc > opt) {
					esif_ccb_write_lock(&DB->lock);
					esif_ccb_strncpy(DB->comment, argv[opt++], sizeof(DB->comment));
					esif_ccb_write_unlock(&DB->lock);
					data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
					rc = EsifConfigSet(data_nspace, NULL, 0, NULL);
				}
				else {
					esif_ccb_read_lock(&DB->lock);
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", DB->comment);
					esif_ccb_read_unlock(&DB->lock);
					if (FORMAT_XML == g_format) {
						strip_illegal_chars(output, g_illegalXmlChars);
					}
				}
				DataVault_PutRef(DB);
			}
			else {
				rc = ESIF_E_NOT_FOUND;
			}
		}
		else if (argc > opt) {	// repo.bin
			StringPtr reponame = argv[opt++];
			if (shell_isprohibited(reponame)) {
				rc = ESIF_E_NOT_SUPPORTED;
			}
			else {
				StringPtr comment = (argc > opt ? argv[opt++] : NULL);
				StringPtr filemode = (comment ? "rb+" : "rb");
				DataRepoPtr repo = DataRepo_Create();
				char repopath[MAX_PATH] = { 0 };
				esif_build_path(repopath, sizeof(repopath), ESIF_PATHTYPE_DV, reponame, NULL);
				if (repo && IOStream_OpenFile(repo->stream, StoreReadWrite, repopath, filemode) == EOK) {
					if (comment) {
						rc = DataRepo_SetComment(repo, comment);
					}
					else {
						DataRepoInfo info = { 0 };
						if (DataRepo_GetInfo(repo, &info) == ESIF_OK) {
							esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", info.comment);
						}
					}
				}
				else {
					rc = ESIF_E_NOT_FOUND;
				}
				DataRepo_Destroy(repo);
			}
		}
		else {
			rc = ESIF_E_PARAMETER_IS_NULL;
		}
		if (rc != ESIF_OK || output[0] == '\0') {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config hash <@datavault|repo.bin>
	else if (esif_ccb_stricmp(subcmd, "hash") == 0) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		if (explicit_namesp) {
			DataVaultPtr DB = DataBank_GetDataVault(namesp);
			if (DB != NULL) {
				char hashstr[SHA256_STRING_BYTES] = { 0 };
				esif_ccb_read_lock(&DB->lock);
				esif_sha256_tostring(&DB->digest, hashstr, sizeof(hashstr));
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", hashstr);
				esif_ccb_read_unlock(&DB->lock);
				if (FORMAT_XML == g_format) {
					strip_illegal_chars(output, g_illegalXmlChars);
				}
				DataVault_PutRef(DB);
				rc = ESIF_OK;
			}
			else {
				rc = ESIF_E_NOT_FOUND;
			}
		}
		else if (argc > opt) {
			char repopath[MAX_PATH] = { 0 };
			StringPtr reponame = argv[opt++];
			DataRepoPtr repo = DataRepo_Create();
			DataRepoInfo info = { 0 };
			esif_build_path(repopath, sizeof(repopath), ESIF_PATHTYPE_DV, reponame, NULL);
			if (repo && IOStream_OpenFile(repo->stream, StoreReadOnly, repopath, "rb") == EOK) {
				rc = DataRepo_GetInfo(repo, &info);
				esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", info.payload_hash);
			}
			else {
				rc = ESIF_E_NOT_FOUND;
			}
			DataRepo_Destroy(repo);
		}
		if (rc != ESIF_OK || output[0] == '\0') {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config segmentid <@datavault|repo.bin> [<segmentid>]
	else if (esif_ccb_stricmp(subcmd, "id") == 0 || esif_ccb_strnicmp(subcmd, "segmentid", 7) == 0) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		if (explicit_namesp) {
			DataVaultPtr DB = DataBank_GetDataVault(namesp);
			if (DB != NULL) {
				if (argc > opt) {
					esif_ccb_write_lock(&DB->lock);
					esif_ccb_strncpy(DB->segmentid, argv[opt++], sizeof(DB->segmentid));
					esif_ccb_write_unlock(&DB->lock);
					data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
					rc = EsifConfigSet(data_nspace, NULL, 0, NULL);
				}
				else {
					esif_ccb_read_lock(&DB->lock);
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", (DB->segmentid[0] ? DB->segmentid : DB->name));
					esif_ccb_read_unlock(&DB->lock);
					rc = ESIF_OK;
				}
				DataVault_PutRef(DB);
			}
			else {
				rc = ESIF_E_NOT_FOUND;
			}
		}
		else if (argc > opt) {
			char repopath[MAX_PATH] = { 0 };
			StringPtr reponame = argv[opt++];
			DataRepoPtr repo = DataRepo_Create();
			DataRepoInfo info = { 0 };
			esif_build_path(repopath, sizeof(repopath), ESIF_PATHTYPE_DV, reponame, NULL);
			if (repo && IOStream_OpenFile(repo->stream, StoreReadOnly, repopath, "rb") == EOK) {
				rc = DataRepo_GetInfo(repo, &info);
				if (rc == ESIF_OK) {
					esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", info.segmentid);
				}
			}
			else {
				rc = ESIF_E_NOT_FOUND;
			}
			DataRepo_Destroy(repo);
		}
		if (rc != ESIF_OK || *output == '\0') {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config write <@datavault>
	else if (esif_ccb_stricmp(subcmd, "write") == 0 && explicit_namesp) {
		data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
		rc = EsifConfigSet(data_nspace, NULL, 0, NULL);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config payload <@datavault> [<payload-file>] [<payload-class>] [compress]
	else if (esif_ccb_stricmp(subcmd, "payload") == 0 && explicit_namesp) {
		Bool compress_payload = ESIF_FALSE;

		// [<payload-file>] [<payload-class] [compress]
		if (argc > opt) {
			StringPtr payload_file = argv[opt++];
			UInt32 payload_class = ESIFDV_PAYLOAD_CLASS_REPO;
			IOStreamPtr repo_stream = IOStream_Create();

			// [<payload-class>] = ('KEYS', 'REPO', etc)
			if (argc > opt) {
				char classname[sizeof(UInt32) + 1] = { 0 };
				esif_ccb_strcpy(classname, argv[opt++], sizeof(classname));
				esif_ccb_strupr(classname, sizeof(classname));
				payload_class = *((UInt32 *)classname);
			}

			// [compress]
			if (argc > opt && esif_ccb_strnicmp(argv[opt], "compress", 4) == 0) {
				compress_payload = ESIF_TRUE;
				opt++;
			}

			if (repo_stream == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				// [<payload-file>] = { <<file.ext | file.ext }
				char repopath[MAX_PATH] = { 0 };
				if (esif_ccb_strncmp(payload_file, "<<", 2) == 0) {
					payload_file += 2;
				}
				rc = DataVault_TranslatePath(payload_file, repopath, sizeof(repopath));

				// Create a File Stream that can be Copied to a DataVault Payload
				if (rc == ESIF_OK) {
					if (IOStream_SetFile(repo_stream, StoreReadOnly, repopath, "rb") != EOK) {
						rc = ESIF_E_NO_MEMORY;
					}
					else if (rc == ESIF_OK) {
						DataVaultPtr DB = DataBank_OpenDataVault(namesp);
						if (DB == NULL) {
							rc = ESIF_E_NO_MEMORY;
						}
						else {
							rc = DataVault_SetPayload(DB, payload_class, repo_stream, compress_payload);
						}
						DataVault_PutRef(DB);
					}
				}
			}
			IOStream_Destroy(repo_stream);
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
		// No <payload-file> = Flush Persisted Key/Value Pairs as KEYS Payload
		else {
			data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
			rc = EsifConfigSet(data_nspace, NULL, 0, NULL);
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	
	// config <files|info|scan> [filespec] [...]
	else if (esif_ccb_strnicmp(subcmd, "files", 4) == 0 || esif_ccb_strnicmp(subcmd, "info", 4) == 0 || esif_ccb_stricmp(subcmd, "scan") == 0 || esif_ccb_strnicmp(subcmd, "repo", 4) == 0) {
		esif_ccb_file_enum_t find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
		struct esif_ccb_file ffd = {0};
		char file_path[MAX_PATH] = {0};
		char full_name[MAX_PATH + MAX_PATH + 1] = {0};
		char *default_filespecs[] = { "*.dv", "*.dvx" };
		char **filespec_argv = default_filespecs;
		int filespec_argc = sizeof(default_filespecs) / sizeof(default_filespecs[0]);
		int filespec_opt = 0;
		Bool details = (esif_ccb_stricmp(subcmd, "scan") != 0);
		Bool infoall = (esif_ccb_strnicmp(subcmd, "info", 4) == 0);

		if (argc > opt) {
			filespec_argv = &argv[opt];
			filespec_argc = argc - opt;
			filespec_opt = 0;
		}
		esif_build_path(file_path, sizeof(file_path), ESIF_PATHTYPE_DV, NULL, NULL);

		if (FORMAT_XML == g_format) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "<repos>\n");
		}
		else {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"Repo Name                Size Modified            %sComment%s\n"
				"-------------------- -------- ------------------- %s--------------------------------%s\n",
				(details ? "Ver   Type Flags    Segment  Payload  " : ""),
				(infoall ? "                          Hash" : ""),
				(details ? "----- ---- -------- -------- -------- " : ""),
				(infoall ? " -------------------------------" : "")
			);
		}

		while (filespec_argc > filespec_opt) {
			char this_folder[MAX_PATH] = { 0 };
			char *this_filespec = filespec_argv[filespec_opt++];
			char *sep = NULL;

			// Support Scanning Subdirectories
			esif_ccb_strcpy(this_folder, file_path, sizeof(this_folder));
			if (((sep = esif_ccb_strrchr(this_filespec, '\\')) != NULL) || ((sep = esif_ccb_strrchr(this_filespec, '/')) != NULL)) {
				esif_ccb_sprintf_concat(sizeof(this_folder), this_folder, "%s%.*s",
					ESIF_PATH_SEP, 
					(int)(size_t)(sep - this_filespec),
					this_filespec);
				this_filespec = ++sep;
			}
			
			// Validate Filenames
			if (shell_isprohibited(this_folder) || shell_isprohibited_filespec(this_filespec)) {
				rc = ESIF_E_IO_INVALID_NAME;
			}
			else if ((find_handle = esif_ccb_file_enum_first(this_folder, this_filespec, &ffd)) != ESIF_INVALID_FILE_ENUM_HANDLE) {
				do {
					if (esif_ccb_strcmp(ffd.filename, ".") == 0 || shell_isprohibited(ffd.filename)) {
						continue; // Ignore . and .. and other prohibited names
					}

					// Get and Display File Details
					char timestamp[MAX_CTIME_LEN] = { 0 };
					char versionstr[ESIF_VERSION_LEN] = { 0 };
					char typestr[sizeof(UInt32) + 1] = { 0 };
					char reposizestr[12] = { 0 };
					char psizestr[12] = { 0 };
					config_flags_str_t flagstr = { 0 };

					esif_ccb_sprintf(sizeof(full_name), full_name, "%s%s%s", this_folder, ESIF_PATH_SEP, ffd.filename);

					// Read Repo Header MetaData, if any
					DataRepoPtr repo = DataRepo_Create();
					DataRepoInfo info = { 0 };
					if (repo && IOStream_OpenFile(repo->stream, StoreReadOnly, full_name, "rb") == EOK) {
						DataRepo_GetInfo(repo, &info);
						esif_ccb_sprintf(sizeof(reposizestr), reposizestr, "%zu", info.repo_size);
					}
					DataRepo_Destroy(repo);

					if (info.version) {
						esif_ccb_sprintf(sizeof(versionstr), versionstr, "%d.%d.%d", ESIFHDR_GET_MAJOR(info.version), ESIFHDR_GET_MINOR(info.version), ESIFHDR_GET_REVISION(info.version));
						esif_ccb_sprintf(sizeof(typestr), typestr, "%4.4s", (char*)&info.payload_class);
						esif_ccb_sprintf(sizeof(psizestr), psizestr, "%u", info.payload_size);
						flagstr = config_flags_str(info.flags);
					}

					if (FORMAT_XML == g_format) {
						struct tm mtime = { 0 };
						if (info.modified && esif_ccb_localtime(&mtime, &info.modified) == 0) {
							esif_ccb_sprintf(sizeof(timestamp), timestamp, "%04d-%02d-%02dT%02d:%02d:%02d",
								mtime.tm_year + 1900, mtime.tm_mon + 1, mtime.tm_mday, mtime.tm_hour, mtime.tm_min, mtime.tm_sec);
						}
						strip_illegal_chars(ffd.filename, g_illegalXmlChars);

						esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							" <repo>\n"
							"  <name>%s</name>\n"
							"  <size>%s</size>\n"
							"  <modified>%s</modified>\n"
							"  <version>%s</version>\n"
							"  <flags>%s</flags>\n"
							"  <comment>%s</comment>\n"
							"  <segmentid>%s</segmentid>\n"
							"  <payloadType>%s</payloadType>\n"
							"  <payloadSize>%s</payloadSize>\n"
							"  <payloadHash>%s</payloadHash>\n"
							" </repo>\n",
							ffd.filename,
							reposizestr,
							timestamp,
							versionstr,
							(StringPtr)&flagstr,
							info.comment,
							info.segmentid,
							typestr,
							psizestr,
							info.payload_hash
						);
					}
					else {
						struct tm mtime = { 0 };
						if (info.modified && esif_ccb_localtime(&mtime, &info.modified) == 0) {
							esif_ccb_sprintf(sizeof(timestamp), timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
									mtime.tm_year + 1900, mtime.tm_mon + 1, mtime.tm_mday, mtime.tm_hour, mtime.tm_min, mtime.tm_sec);
						}
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							"%-20s %8s %-19.19s",
							ffd.filename,
							reposizestr,
							timestamp
						);
						if (details) {
							esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
								" %-5s %-4s %-8s %-8s %8s",
								versionstr,
								typestr,
								(StringPtr)&flagstr,
								info.segmentid,
								psizestr
							);
						}
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
							" %-*s%s",
							(infoall ? 32 : esif_ccb_strlen(info.comment, sizeof(info.comment))),
							info.comment,
							(infoall ? "" : "\n")
						);
						if (infoall) {
							esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
								" %s\n",
								info.payload_hash
							);
						}
					}

				} while (esif_ccb_file_enum_next(find_handle, this_filespec, &ffd));
				esif_ccb_file_enum_close(find_handle);
			}
		}

		if (FORMAT_XML == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</repos>\n");
		}
		else {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		}

		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
		}
	}
	// config open <@datavault [comment] | reponame.ext>
	else if (esif_ccb_stricmp(subcmd, "open") == 0) {
		StringPtr reponame = NULL;

		// config open <reponame.ext>
		if (argc > opt && !explicit_namesp) {
			// Create a Repo Object for the given filename and Load it into DataVault Cache(s)
			reponame = argv[opt++];

			// Validate Filenames
			if (shell_isprohibited(reponame)) {
				rc = ESIF_E_IO_INVALID_NAME;
			}
			else {
				DataRepoPtr repo = DataRepo_CreateAs(StreamFile, StoreReadWrite, reponame);
				if (repo == NULL) {
					rc = ESIF_E_NOT_FOUND;
				}
				else {
					ESIF_TRACE_INFO("Loading REPO %s\n", reponame);
					rc = DataRepo_LoadSegments(repo);
					DataRepo_Destroy(repo);
				}
			}
		}
		
		// config open <@datavault> [comment]
		if (explicit_namesp) {
			DataVaultPtr DB = DataBank_GetDataVault(namesp);
			if (DB != NULL) {
				rc = ESIF_E_IO_ALREADY_EXISTS;
			}
			else {
				StringPtr comment = (argc > opt ? argv[opt++] : NULL);
				rc = DataBank_ImportDataVault(namesp);
				DB = DataBank_GetDataVault(namesp);

				if (rc == ESIF_OK && DB != NULL && comment != NULL) {
					esif_ccb_write_lock(&DB->lock);
					esif_ccb_strcpy(DB->comment, comment, sizeof(DB->comment));
					esif_ccb_write_unlock(&DB->lock);
				}
				else if (rc != ESIF_OK && DB != NULL) {
					DataBank_CloseDataVault(namesp);
				}
			}
			DataVault_PutRef(DB);
		}
		
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config close <@datavault> [...]
	else if (esif_ccb_stricmp(subcmd, "close")==0 && explicit_namesp) {
		int closed = 0;
		
		rc = ESIF_E_INVALID_ARGUMENT_COUNT;
		opt = explicit_opt;
		while (argc > opt) {
			StringPtr dvname = argv[opt++];
			if (dvname[0] == '@') {
				dvname++;
				DataVaultPtr DB = DataBank_GetDataVault(dvname);
				if (DB == NULL) {
					rc = ESIF_E_NOT_FOUND;
				}
				else if (DB->flags & ESIF_SERVICE_CONFIG_STATIC) {
					rc = ESIF_E_NOT_SUPPORTED;
				}
				else {
					DataBank_CloseDataVault(dvname);
					rc = ESIF_OK;
					closed++;
				}
				DataVault_PutRef(DB);
			}
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s (%d closed)\n", esif_rc_str(rc), closed);
	}
	// config drop <@datavault | repo.bin> [...]
	else if (esif_ccb_stricmp(subcmd, "drop")==0) {
		int dropped = 0;

		rc = ESIF_E_INVALID_ARGUMENT_COUNT;
		if (explicit_namesp) {
			opt = explicit_opt;
		}

		while (argc > opt) {
			char fullpath[MAX_PATH] = { 0 };
			StringPtr argname = argv[opt++];
			StringPtr dvname = NULL;
			if (argname[0] == '@') {
				dvname = argname + 1;
				esif_build_path(fullpath, sizeof(fullpath), ESIF_PATHTYPE_DV, dvname, ESIFDV_FILEEXT);
			}
			else if (shell_isprohibited(argname)) {
				rc = ESIF_E_IO_INVALID_NAME;
			}
			else {
				esif_build_path(fullpath, sizeof(fullpath), ESIF_PATHTYPE_DV, argname, NULL);
			}

			if (fullpath[0]) {
				if (dvname && DataBank_GetDataVault(dvname) != NULL) {
					rc = ESIF_E_NOT_SUPPORTED;
				}
				else if (esif_ccb_unlink(fullpath) != 0) {
					rc = ESIF_E_NOT_FOUND;
				}
				else {
					rc = ESIF_OK;
					dropped++;
				}
			}
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s (%d dropped)\n", esif_rc_str(rc), dropped);
	}
	// config <copyto|appendto> <target.ext> <file1.ext> [...]
	else if ((esif_ccb_stricmp(subcmd, "copyto") == 0 || esif_ccb_strnicmp(subcmd, "append", 6) == 0 || esif_ccb_stricmp(subcmd, "concat") == 0) && !explicit_namesp && argc > opt + 1) {
		char *open_mode = "wb";
		char target_path[MAX_PATH] = { 0 };
		char source_path[MAX_PATH] = { 0 };
		StringPtr target_name = argv[opt++];
		StringPtr source_name = NULL;
		FILE *targetfp = NULL;
		FILE *sourcefp = NULL;
		size_t buf_len = (4 * 1024);
		BytePtr buffer = NULL;
		size_t bytes_read = 0;
		size_t total_bytes = 0;
		int filecount = 0;
		int errnum = 0;

		// Append file?
		if (esif_ccb_strnicmp(subcmd, "append", 6) == 0) {
			open_mode = "ab";
		}

		// Open target file first, then copy every other file to it
		esif_build_path(target_path, sizeof(target_path), ESIF_PATHTYPE_DV, target_name, NULL);
		if (*target_name == '.' || shell_isprohibited(target_name)) {
			rc = ESIF_E_IO_INVALID_NAME;
		}
		else if ((targetfp = esif_ccb_fopen(target_path, open_mode, &errnum)) == NULL) {
			rc = ESIF_E_IO_OPEN_FAILED;
		}
		else {
			if ((buffer = esif_ccb_malloc(buf_len)) == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			// <file1.ext> [...]
			while ((rc == ESIF_OK) && (argc > opt)) {
				source_name = argv[opt++];
				esif_build_path(source_path, sizeof(source_path), ESIF_PATHTYPE_DV, source_name, NULL);

				// Validate Filenames
				if (shell_isprohibited(source_name)) {
					rc = ESIF_E_IO_INVALID_NAME;
					break;
				}
				else if ((sourcefp = esif_ccb_fopen(source_path, "rb", &errnum)) == NULL) {
					rc = ESIF_E_IO_OPEN_FAILED;
					break;
				}
				while ((bytes_read = esif_ccb_fread(buffer, buf_len, sizeof(Byte), buf_len, sourcefp)) > 0) {
					if (esif_ccb_fwrite(buffer, sizeof(Byte), bytes_read, targetfp) < bytes_read) {
						rc = ESIF_E_IO_ERROR;
						break;
					}
					total_bytes += bytes_read;
				}
				filecount++;
				esif_ccb_fclose(sourcefp);
			}

			esif_ccb_free(buffer);
			esif_ccb_fclose(targetfp);
			if (rc != ESIF_OK) {
				IGNORE_RESULT(esif_ccb_unlink(target_path));
			}
		}

		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s [%d] (%d files, %llu total bytes)\n", esif_rc_str(rc), errnum, filecount, (UInt64)total_bytes);
	}
	// config rename <oldname.ext> <newname.ext>
	else if ((esif_ccb_stricmp(subcmd, "rename") == 0 || esif_ccb_stricmp(subcmd, "replace") == 0) && !explicit_namesp && argc > opt + 1) {
		char oldpath[MAX_PATH] = { 0 };
		char newpath[MAX_PATH] = { 0 };
		StringPtr oldfile = argv[opt++];
		StringPtr newfile = argv[opt++];
		Bool replaceTarget = (esif_ccb_stricmp(subcmd, "replace") == 0);
		int errnum = 0;

		// Validate Filenames
		if (shell_isprohibited(oldfile) || shell_isprohibited(newfile)) {
			rc = ESIF_E_IO_INVALID_NAME;
		}
		else {
			esif_build_path(oldpath, sizeof(oldpath), ESIF_PATHTYPE_DV, oldfile, NULL);
			esif_build_path(newpath, sizeof(newpath), ESIF_PATHTYPE_DV, newfile, NULL);

			if (replaceTarget && esif_ccb_file_exists(newpath)) {
				if (esif_ccb_unlink(newpath) != EOK) {
					errnum = errno;
					rc = ESIF_E_IO_DELETE_FAILED;
				}
			}
			if (rc == ESIF_OK && esif_ccb_rename(oldpath, newpath) != 0) {
				errnum = errno;
				rc = (errnum == ENOENT ? ESIF_E_NOT_FOUND : errnum == EEXIST ? ESIF_E_IO_ALREADY_EXISTS : ESIF_E_IO_ERROR);
			}
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s [%d]\n", esif_rc_str(rc), errnum);
	}
	// config select <@datavault>
	else if (esif_ccb_stricmp(subcmd, "select") == 0 && explicit_namesp) {
		if (DataBank_GetDataVault(namesp) == NULL) {
			rc = ESIF_E_NOT_FOUND;
		}
		else {
			DataBank_SetDefault(namesp);
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config rekey [@datavault]
	else if (esif_ccb_stricmp(subcmd, "rekey") == 0) {
		EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByInstance(ESIF_HANDLE_PRIMARY_PARTICIPANT);
		EsifString primaryParticipants[] = { EsifUp_GetName(upPtr), "DPTFZ", NULL, NULL };

		// Look for Legacy IETM Keys other than current IETM device and DPTFZ
		char legacyKey[ESIF_NAME_LEN] = { 0 };
		if (GetLegacyPrimaryKeyname(legacyKey, sizeof(legacyKey)) == ESIF_OK) {
			for (int j = 0; j < ESIF_ARRAY_LEN(primaryParticipants) - 1; j++) {
				if (primaryParticipants[j] == NULL) {
					primaryParticipants[j] = legacyKey;
					break;
				}
			}
		}

		// For each non-IETM Primary Participant, merge all keys with IETM key
		for (size_t j = 0; primaryParticipants[j] != NULL; j++) {
			if (esif_ccb_stricmp(primaryParticipants[j], ESIF_PARTICIPANT_DPTF_NAME) == 0) {
				continue;
			}
			char keyspec[MAX_PATH] = { 0 };
			esif_ccb_sprintf(sizeof(keyspec), keyspec, "/participants/%s.D0/*", primaryParticipants[j]);

			EsifDataPtr nameSpace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
			EsifDataPtr keyname = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			EsifDataPtr newkeyname = EsifData_CreateAs(ESIF_DATA_STRING, NULL, 0, ESIFAUTOLEN);
			EsifDataPtr value = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
			EsifConfigFindContext context = NULL;

			if (nameSpace == NULL || keyname == NULL || newkeyname == NULL || value == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else if ((rc = EsifConfigFindFirst(nameSpace, keyname, value, &context)) == ESIF_OK) {
				char nameTag[ESIF_NAME_LEN] = { 0 };
				char newTag[ESIF_NAME_LEN] = { 0 };
				esif_ccb_sprintf(sizeof(nameTag), nameTag, "/%s.D0/", primaryParticipants[j]);
				esif_ccb_sprintf(sizeof(newTag), newTag, "/%s.D0/", ESIF_PARTICIPANT_DPTF_NAME);
				do {
					// merge /participants/XXXX.D0/yyyy with /participants/IETM.D0/yyyy
					EsifString newkey = esif_str_replace((EsifString)keyname->buf_ptr, nameTag, newTag);
					if (newkey != NULL && DataBank_KeyExists(namesp, newkey) == ESIF_FALSE) {
						EsifData_Set(newkeyname, ESIF_DATA_STRING, newkey, 0, ESIFAUTOLEN);
						rc = EsifConfigSet(nameSpace, newkeyname, ESIF_SERVICE_CONFIG_PERSIST, value);
					}
					// delete /participants/XXXX.D0/yyyy
					if (rc == ESIF_OK) {
						rc = EsifConfigDelete(nameSpace, keyname);
					}
					EsifData_Set(keyname, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
					EsifData_Set(newkeyname, ESIF_DATA_STRING, NULL, 0, ESIFAUTOLEN);
					EsifData_Set(value, ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
					esif_ccb_free(newkey);
				} while ((rc == ESIF_OK) && ((rc = EsifConfigFindNext(nameSpace, keyname, value, &context)) == ESIF_OK));

				EsifConfigFindClose(&context);
				if (rc == ESIF_E_ITERATION_DONE) {
					rc = ESIF_OK;
				}
			}
			EsifData_Destroy(nameSpace);
			EsifData_Destroy(keyname);
			EsifData_Destroy(newkeyname);
			EsifData_Destroy(value);
		}
		EsifUp_PutRef(upPtr);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	// config upload [overwrite|append] <filename> <base64-encoded-data>
	else if (esif_ccb_stricmp(subcmd, "upload") == 0 && argc > opt + 1) {
		esif_string fileopt = NULL;
		esif_string filename = NULL;
		esif_string encoded_buf = NULL;

		rc = ESIF_E_NOT_SUPPORTED;

		// [overwrite|append]
		if (esif_ccb_stricmp(argv[opt], "overwrite") == 0) {
			fileopt = "wb";
			opt++;
		}
		else if (esif_ccb_stricmp(argv[opt], "append") == 0) {
			fileopt = "ab";
			opt++;
		}

		// <filename> <base64-encoded-data>
		if (argc > opt + 1) {
			filename = argv[opt++];
			encoded_buf = argv[opt++];
		}

		size_t encoded_len = esif_ccb_strlen(encoded_buf, MAX_STRINGLEN);
		size_t filename_len = esif_ccb_strlen(filename, MAX_PATH);
		size_t dvext_len = esif_ccb_strlen(ESIFDV_FILEEXT, MAX_PATH);
		size_t repoext_len = esif_ccb_strlen(ESIFDV_REPOEXT, MAX_PATH);
		
		// Filename must be valid, not a relative path, and not have a .dv or .dvx suffix
		if (!shell_isprohibited(filename) 
				&& (esif_ccb_strpbrk(filename, "/\\:") == NULL)
				&& (filename_len > esif_ccb_max(repoext_len, dvext_len))
				&& (esif_ccb_stricmp(&filename[filename_len - dvext_len], ESIFDV_FILEEXT) != 0)
				&& (esif_ccb_stricmp(&filename[filename_len - repoext_len], ESIFDV_REPOEXT) != 0)
			) {
			FILE *outfile = NULL;
			UInt8 *decoded_buf = NULL;
			size_t decoded_len = 0;
			char fullpath[MAX_PATH] = { 0 };
			char *fileext = (esif_ccb_strchr(filename, '.') ? NULL : ".bin");
			char *encoded_buf_dyn = NULL;

			esif_build_path(fullpath, sizeof(fullpath), ESIF_PATHTYPE_DV, filename, fileext);

			// Allow Base64-Encoded Data to be loaded from a known local file location like $dv/file.name
			if (esif_ccb_strncmp(encoded_buf, "<<$", 3) == 0 || *encoded_buf == '$') {
				if (esif_ccb_strncmp(encoded_buf, "<<", 2) == 0) {
					encoded_buf += 2;
				}

				char base64path[MAX_PATH] = { 0 };
				if ((rc = DataVault_TranslatePath(encoded_buf, base64path, sizeof(base64path))) == ESIF_OK) {
					FILE *base64fp = NULL;
					struct stat st;
					encoded_buf = NULL;

					if (esif_ccb_stat(base64path, &st) != 0) {
						rc = ESIF_E_NOT_FOUND;
					}
					else if (st.st_size < 1 || st.st_size > MAX_BASE64_ENCODED_LEN) {
						rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					}
					else if ((encoded_buf_dyn = esif_ccb_malloc((size_t)st.st_size)) == NULL) {
						rc = ESIF_E_NO_MEMORY;
					}
					else if ((base64fp = esif_ccb_fopen(base64path, "rb", NULL)) == NULL) {
						rc = ESIF_E_IO_OPEN_FAILED;
					}
					else {
						encoded_len = (size_t)st.st_size;
						encoded_buf = encoded_buf_dyn;
						if (esif_ccb_fread(encoded_buf, encoded_len, 1, encoded_len, base64fp) != encoded_len) {
							rc = ESIF_E_IO_ERROR;
						}
						esif_ccb_fclose(base64fp);
					}
				}
			}

			// Cannot overwrite existing file unless [overwrite|append] specified
			if (fileopt == NULL && esif_ccb_file_exists(fullpath)) {
				rc = ESIF_E_IO_ALREADY_EXISTS;
			}
			else if (encoded_buf) {
				fileopt = (fileopt ? fileopt : "wb");
				outfile = esif_ccb_fopen(fullpath, fileopt, NULL);
				if (outfile == NULL) {
					rc = ESIF_E_IO_OPEN_FAILED;
				}
				else if (((rc = esif_base64_decode(NULL, &decoded_len, NULL, encoded_len)) == ESIF_E_NEED_LARGER_BUFFER) && (decoded_len > 0)) {
					// Allocate a result buffer and Decode Base64-Encoded Data
					decoded_buf = esif_ccb_malloc(decoded_len);
					if (decoded_buf == NULL) {
						rc = ESIF_E_NO_MEMORY;
					}
					else {
						rc = esif_base64_decode(decoded_buf, &decoded_len, encoded_buf, encoded_len);
					}
				}
			}

			// Write Base64-Decoded Data
			if (outfile) {
				if (rc == ESIF_OK) {
					if (esif_ccb_fwrite(decoded_buf, 1, decoded_len, outfile) != decoded_len) {
						rc = ESIF_E_IO_ERROR;
					}
				}
				esif_ccb_fclose(outfile);
				if (rc != ESIF_OK) {
					esif_ccb_unlink(fullpath);
				}
			}
			esif_ccb_free(encoded_buf_dyn);
			esif_ccb_free(decoded_buf);
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(rc));
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(ESIF_E_INVALID_ARGUMENT_COUNT));
	}

exit:
	// Cleanup
	EsifData_Destroy(data_nspace);
	EsifData_Destroy(data_key);
	EsifData_Destroy(data_value);
	EsifData_Destroy(data_targetdv);
	return output;
}


// Kernel Participant Extensions: driversk, driverk
static char *esif_shell_cmd_driversk(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	struct esif_ipc_command *command_ptr = NULL;
	u32 data_len = sizeof(struct esif_command_get_drivers);
	struct esif_ipc *ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);
	struct esif_command_get_drivers *data_ptr = NULL;
	struct esif_driver_info *drv_ptr = NULL;
	u32 count = 0;
	u32 i = 0;
	u8 get_driver = ESIF_FALSE;
	u32 driver_id = 0;
	char guid_str[ESIF_GUID_PRINT_SIZE] = { 0 };

	if (argc > 1) {
		get_driver = ESIF_TRUE;
		driver_id = esif_atoi(argv[1]);
	}

	if (NULL == ipc_ptr || NULL == command_ptr) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"esif_ipc_alloc_command failed for %u bytes\n",
			data_len);
		goto exit;
	}

	command_ptr->type = ESIF_COMMAND_TYPE_GET_DRIVERS;
	command_ptr->req_data_type   = ESIF_DATA_VOID;
	command_ptr->req_data_offset = 0;
	command_ptr->req_data_len    = 0;
	command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
	command_ptr->rsp_data_offset = 0;
	command_ptr->rsp_data_len    = data_len;

	if (get_driver) {
		command_ptr->req_data_type = ESIF_DATA_UINT32;
		command_ptr->req_data_len = (u32) sizeof(u32);
		*(u32 *)(command_ptr + 1) = driver_id;
	}

	ipc_execute(ipc_ptr);

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"IPC error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}

	/*
	 * If the buffer is too small, reallocate based on the number of drivers that are present and retry
	 */
	if (ESIF_E_NEED_LARGER_BUFFER == command_ptr->return_code) {
		data_ptr = (struct esif_command_get_drivers *)(command_ptr + 1);
		count = data_ptr->available_count;
		if (0 == count) {
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"Invalid driver count (0) returned for ESIF_E_NEED_LARGER_BUFFER\n");
			goto exit;	
		}
		data_len = sizeof(struct esif_command_get_drivers) + ((count - 1) * sizeof(*data_ptr->driver_info));

		esif_ipc_free(ipc_ptr);

		ipc_ptr = esif_ipc_alloc_command(&command_ptr, data_len);

		if (NULL == ipc_ptr || NULL == command_ptr) {
			esif_ccb_sprintf(OUT_BUF_LEN,
				output,
				"esif_ipc_alloc_command failed for %u bytes\n",
				data_len);
			goto exit;
		}

		command_ptr->type = ESIF_COMMAND_TYPE_GET_DRIVERS;
		command_ptr->req_data_type   = ESIF_DATA_VOID;
		command_ptr->req_data_offset = 0;
		command_ptr->req_data_len    = 0;
		command_ptr->rsp_data_type   = ESIF_DATA_STRUCTURE;
		command_ptr->rsp_data_offset = 0;
		command_ptr->rsp_data_len    = data_len;

		if (get_driver) {
			command_ptr->req_data_type = ESIF_DATA_UINT32;
			command_ptr->req_data_len = sizeof(u32);
			*(u32 *)(command_ptr + 1) = driver_id;
		}

		ipc_execute(ipc_ptr);
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN, output,
			"ipc error code = %s(%d)\n",
			esif_rc_str(ipc_ptr->return_code),
			ipc_ptr->return_code);
		goto exit;
	}

	if (ESIF_OK != command_ptr->return_code) {
		esif_ccb_sprintf(OUT_BUF_LEN,
			output,
			"Command error code = %s(%d)\n",
			 esif_rc_str(command_ptr->return_code),
			 command_ptr->return_code);
		goto exit;
	}

	// out data
	data_ptr = (struct esif_command_get_drivers *)(command_ptr + 1);
	count    = data_ptr->returned_count;

	// driverk <id>
	if (get_driver) {
		if (count < 1) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "driver not available\n");
		}
		else if (FORMAT_TEXT == g_format) {
			drv_ptr = &data_ptr->driver_info[0];
			esif_ccb_sprintf(
				OUT_BUF_LEN,
				output,
				"\n"
				"Instance:    %d\n"
				"Action Type: %s (%d)\n"
				"Version:     %d\n"
				"GUID:        %s\n"
				"Flags:       0x%08X\n"
				"Name:        %s\n"
				"Description: %s\n"
				"Driver Name: %s\n"
				"Device Name: %s\n"
				"\n"
				,
				driver_id,
				esif_action_type_str(drv_ptr->action_type), drv_ptr->action_type,
				(u32)drv_ptr->version,
				esif_guid_print(&drv_ptr->class_guid, guid_str),
				drv_ptr->flags,
				drv_ptr->name,
				drv_ptr->desc,
				drv_ptr->driver_name,
				drv_ptr->device_name
				);
		}
		else {
			drv_ptr = &data_ptr->driver_info[0];
			esif_ccb_sprintf(OUT_BUF_LEN, output,
				"<driver>\n"
				"  <instance>%d</instance>"
				"  <action_type>%d</action_type>\n"
				"  <action_type_str>%s</action_type_str>\n"
				"  <version>%d</version>\n"
				"  <guid>%s</guid>\n"
				"  <flags>0x%08X</flags>\n"
				"  <name>%s</name>\n"
				"  <desc>%s</desc>\n"
				"  <driver_name>%s</driver_name>\n"
				"  <device_name>%s</device_name>\n"
				"</driver>\n"
				,
				driver_id,
				drv_ptr->action_type,
				esif_action_type_str(drv_ptr->action_type),
				(u32)drv_ptr->version,
				esif_guid_print(&drv_ptr->class_guid, guid_str),
				drv_ptr->flags,
				drv_ptr->name,
				drv_ptr->desc,
				drv_ptr->driver_name,
				drv_ptr->device_name
				);
		}
	}
	// driversk
	else {
		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf(
				OUT_BUF_LEN,
				output,
				"\nKERNEL PARTICIPANT EXTENSION (KPE) DRIVERS:\n\n"
				"ActionType Name                 Description          Device           Ver  Flags\n"
				"---------- -------------------- -------------------- ---------------- ---- ----------\n");
		}
		else {// XML
			esif_ccb_sprintf(OUT_BUF_LEN, output, "<driversk>\n");
		}

		for (i = 0; i < count; i++) {
			drv_ptr = &data_ptr->driver_info[i];

			if (FORMAT_TEXT == g_format) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN,
					output,
					"%-10.10s %-20s %-20s %-16s %-4d 0x%08X\n",
					ltrim(esif_action_type_str(drv_ptr->action_type), PREFIX_ACTION_TYPE),
					drv_ptr->name,
					drv_ptr->desc,
					drv_ptr->device_name,
					drv_ptr->version,
					drv_ptr->flags
					);
			}
			else {// FORMAT_XML
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output,
					"<driver>\n"
					"  <action_type>%d</action_type>\n"
					"  <action_type_str>%s</action_type_str>\n"
					"  <name>%s</name>\n"
					"  <desc>%s</desc>\n"
					"  <device_name>%s</device_name>\n"
					"  <version>%d</version>\n"
					"  <flags>0x%08X</flags>\n"
					"  <guid>%s</guid>\n"
					"</driver>\n",
					drv_ptr->action_type,
					esif_action_type_str(drv_ptr->action_type),
					drv_ptr->name,
					drv_ptr->desc,
					drv_ptr->device_name,
					drv_ptr->version,
					drv_ptr->flags,
					esif_guid_print(&drv_ptr->class_guid, guid_str)
					);
			}
		}

		if (FORMAT_TEXT == g_format) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "\n");
		}
		else {// FORMAT_XML
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</driversk>\n");
		}
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return output;
}

static char *esif_shell_cmd_shell(EsifShellCmdPtr shell)
{
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;

	// Verify Access Control
	if (DCfg_Get().opt.ShellAccessControl) {
		g_shell_enabled = 0;
		esif_uf_os_shell_disable();
		CMD_OUT("shell access disabled\n");
		return output;
	}

	// shell
	if (argc < 2) {
		CMD_OUT("shell %s\n", (g_shell_enabled ? "enabled" : "disabled"));
	}
	// shell enable
	else if (esif_ccb_stricmp(argv[1], "enable")==0) {
		g_shell_enabled = 1;
		esif_uf_os_shell_enable();
		CMD_OUT("shell enabled\n");
	}
	// shell disable
	else if (esif_ccb_stricmp(argv[1], "disable")==0) {
		g_shell_enabled = 0;
		esif_uf_os_shell_disable();
		CMD_OUT("shell disabled\n");
	}
	return output;
}

static char *esif_shell_cmd_sleep(EsifShellCmdPtr shell)
{
	   int argc     = shell->argc;
	   char **argv  = shell->argv;
	   char *output = shell->outbuf;

	   // sleep <ms>
	   if (argc < 1 || !shell_isnumber(argv[1])) {
		   esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS));
	   }
	   else {
			  UInt32 ms = esif_atoi(argv[1]);
			  ms = (ms > MAX_SLEEP_MS ? MAX_SLEEP_MS : ms);
			  esif_ccb_sleep_msec(ms);
	   }
	   return output;
}



// Run External Shell Command
static void esif_shell_exec_cmdshell(char *line)
{
	esif_ccb_system(line);
}


// Helper for esif_shell_cmd_idsp
typedef struct IdspEntry_s {
	union esif_data_variant variant;
	esif_guid_t uuid;
} IdspEntry, *IdspEntryPtr;


// Helper for esif_shell_cmd_idsp
static char *concat_uuid_list(
	char *outputPtr,
	IdspEntryPtr entryPtr,
	int numEntries
	)
{
	u8 mangledUuid[ESIF_GUID_LEN] = { 0 };
	char uuidStr[ESIF_GUID_PRINT_SIZE] = { 0 };
	int i = 0;

	ESIF_ASSERT(outputPtr != NULL);
	ESIF_ASSERT(entryPtr != NULL);

	for (i = 0; i < numEntries; i++) {
		esif_ccb_memcpy(&mangledUuid, (u8 *)entryPtr->uuid, ESIF_GUID_LEN);
		esif_guid_mangle(&mangledUuid);
		esif_guid_print((esif_guid_t *)(mangledUuid), uuidStr);
		esif_ccb_sprintf_concat(OUT_BUF_LEN, outputPtr, "%s\n", uuidStr);

		entryPtr++;
	}

	return outputPtr;
}


// Helper for esif_shell_cmd_idsp
static IdspEntryPtr find_uuid_in_idsp_entries(
	esif_guid_t *uuidPtr,
	IdspEntryPtr entryPtr,
	int numEntries
	)
{
	int i = 0;

	ESIF_ASSERT(uuidPtr != NULL);
	ESIF_ASSERT(entryPtr != NULL);

	for (i = 0; i < numEntries; i++) {
		if (memcmp(entryPtr->uuid, (char *)uuidPtr, ESIF_GUID_LEN) == 0) {
			goto exit;
		}
		entryPtr++;
	}
	entryPtr = NULL;
exit:
	return entryPtr;
}


static char *esif_shell_cmd_idsp(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc     = shell->argc;
	char **argv  = shell->argv;
	char *output = shell->outbuf;
	char *subcmd = NULL;
	int  opt = 1;
	char* uuidStr = NULL;
	esif_guid_t uuidBuffer = {0};
	esif_guid_t nullUuid = {0};
	UInt16 guidShorts[32] = {0};
	char notAllowed = '\0';
	int uuidFieldsRead = 0;
	EsifDataPtr reqPtr = NULL;
	EsifDataPtr rspPtr = NULL;
	UInt32 numEntries = 0;
	UInt32 newEntryIndex = 0;
	IdspEntryPtr matchingEntryPtr = NULL;
	IdspEntryPtr nullEntryPtr = NULL;
	IdspEntryPtr endPtr = NULL;
	IdspEntryPtr curEntryPtr = NULL;
	IdspEntryPtr nextEntryPtr = NULL;
	IdspEntryPtr newIdspPtr = NULL;
	int newIdspSize = 0;
	
	// Get the current IDSP
	rspPtr = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	if (NULL == rspPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to allocate memory\n");
		goto exit;
	}
	rc = EsifExecutePrimitive(
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		GET_SUPPORTED_POLICIES,
		"D0",
		255,
		NULL,
		rspPtr);
	if (rc != ESIF_OK) {
		rspPtr->data_len = 0; /* Defensive, should already be 0 */
	}

	numEntries = (int)(rspPtr->data_len / sizeof(*curEntryPtr));

	// Parse command arguments

	// idsp
	if (1 == argc) {
		if (numEntries != 0) {
			concat_uuid_list(output, (IdspEntryPtr)rspPtr->buf_ptr, numEntries);
		} else {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to retrieve or no IDSP present\n");
		}
		goto exit;
	}

	// idsp subcmd (Missing UUID)
	if (2 == argc) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: UUID must be specified\n");
		goto exit;
	}

	// idsp subcmd UUID
	subcmd = argv[opt++];
	uuidStr = argv[opt++];

	// Validate the UUID is correctly formatted
	uuidFieldsRead = esif_ccb_sscanf(uuidStr, "%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx-%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%1hx%c", 
		&guidShorts[0], &guidShorts[1], &guidShorts[2], &guidShorts[3],
		&guidShorts[4], &guidShorts[5], &guidShorts[6], &guidShorts[7],
		&guidShorts[8], &guidShorts[9], &guidShorts[10], &guidShorts[11],
		&guidShorts[12], &guidShorts[13], &guidShorts[14], &guidShorts[15],
		&guidShorts[16], &guidShorts[17], &guidShorts[18], &guidShorts[19],
		&guidShorts[20], &guidShorts[21], &guidShorts[22], &guidShorts[23],
		&guidShorts[24], &guidShorts[25], &guidShorts[26], &guidShorts[27],
		&guidShorts[28], &guidShorts[29], &guidShorts[30], &guidShorts[31],
		SCANFBUF(&notAllowed, 1));

	if (uuidFieldsRead != (ESIF_GUID_LEN * 2)) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: UUID must be in the format xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\n");
		goto exit;		
	}

	uuidFieldsRead = esif_ccb_sscanf(uuidStr, "%2hx%2hx%2hx%2hx-%2hx%2hx-%2hx%2hx-%2hx%2hx-%2hx%2hx%2hx%2hx%2hx%2hx", 
		&guidShorts[0], &guidShorts[1], &guidShorts[2], &guidShorts[3],
		&guidShorts[4], &guidShorts[5], &guidShorts[6], &guidShorts[7],
		&guidShorts[8], &guidShorts[9], &guidShorts[10], &guidShorts[11],
		&guidShorts[12], &guidShorts[13], &guidShorts[14], &guidShorts[15]);
	esif_copy_shorts_to_bytes((UInt8 *)&uuidBuffer, guidShorts, ESIF_GUID_LEN);

	//
	// Put into mangled format, same as format from BIOS and how it is stored in override
	//
	esif_guid_mangle(&uuidBuffer);

	matchingEntryPtr = find_uuid_in_idsp_entries(&uuidBuffer, (IdspEntryPtr)rspPtr->buf_ptr, numEntries);

	// idsp add UUID
	if (esif_ccb_stricmp(subcmd, "add") == 0) {
		if (matchingEntryPtr != NULL) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Specified UUID already present\n");
			goto exit;
		}

		// See if there is a NULL entry we can replace
		nullEntryPtr = find_uuid_in_idsp_entries(&nullUuid, (IdspEntryPtr)rspPtr->buf_ptr, numEntries);

		newIdspSize = rspPtr->data_len;
		newEntryIndex = numEntries;
		if (NULL == nullEntryPtr) {
			newIdspSize += sizeof(*newIdspPtr);
		} else {
			newEntryIndex = (UInt32)(nullEntryPtr - ((IdspEntryPtr)rspPtr->buf_ptr));
		}

		newIdspPtr = (IdspEntryPtr)esif_ccb_malloc(newIdspSize);
		if (newIdspPtr == NULL) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to write new IDSP\n");
			goto exit;
		}

		// Copy the current UUIDs to the new IDSP buffer
		esif_ccb_memcpy((char *)newIdspPtr, rspPtr->buf_ptr, rspPtr->data_len);

		// Set up the new entry at the end or replace the first "null" entry
		curEntryPtr = newIdspPtr + newEntryIndex;
		curEntryPtr->variant.type = ESIF_DATA_BINARY;
		curEntryPtr->variant.string.length = ESIF_GUID_LEN;
		curEntryPtr->variant.string.reserved = 0;
		esif_ccb_memcpy(curEntryPtr->uuid, uuidBuffer, ESIF_GUID_LEN);

		// Write the new IDSP to the datavault
		reqPtr = EsifData_CreateAs(ESIF_DATA_BINARY, newIdspPtr, newIdspSize, newIdspSize);
		rc = EsifExecutePrimitive(
			ESIF_HANDLE_PRIMARY_PARTICIPANT,
			SET_SUPPORTED_POLICIES,
			"D0",
			255,
			reqPtr,
			NULL);
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to write new IDSP\n");
			goto exit;
		}
	}

	// idsp delete UUID
	else if (esif_ccb_stricmp(subcmd, "delete") == 0) {
		if (NULL == matchingEntryPtr) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Specified UUID not found\n");
			goto exit;
		}

		// Move UUIDs up to cover the UUID being deleted
		endPtr = ((IdspEntryPtr)rspPtr->buf_ptr) + numEntries;
		curEntryPtr = matchingEntryPtr;
		nextEntryPtr = curEntryPtr + 1;
		while (nextEntryPtr < endPtr) {
			esif_ccb_memcpy(curEntryPtr->uuid, nextEntryPtr->uuid, sizeof(curEntryPtr->uuid));
			curEntryPtr++;
			nextEntryPtr++;
		}

		// Write the new IDSP to the datavault
		reqPtr = EsifData_CreateAs(ESIF_DATA_BINARY, rspPtr->buf_ptr, 0, rspPtr->data_len - sizeof(*curEntryPtr));
		rc = EsifExecutePrimitive(
			ESIF_HANDLE_PRIMARY_PARTICIPANT,
			SET_SUPPORTED_POLICIES,
			"D0",
			255,
			reqPtr,
			NULL);
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to write new IDSP\n");
			goto exit;
		}
	}

	// Unknown subcommand
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unknown option specified\n");
		goto exit;
	}
exit:
	EsifData_Destroy(reqPtr);
	EsifData_Destroy(rspPtr);
	return output;
}


static char *esif_shell_cmd_get_available_devices(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	EsifDataPtr reqPtr = NULL;
	EsifDataPtr rspPtr = NULL;
	int opt = 1;
	int numRequiredParameters = 2;
	int numDevices = 0;
	struct loadable_action_devices *deviceListPtr = NULL;
	struct loadable_action_device *devicePtr = NULL;
	UInt32 *deviceCodePtr = NULL;
	char deviceCodeStr[5] = { 0 };

	if (argc < numRequiredParameters) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Too few parameters. \n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	reqPtr = EsifData_Create();
	if (reqPtr == NULL) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	
	esif_ccb_strcpy(deviceCodeStr, argv[opt], sizeof(deviceCodeStr));
	deviceCodePtr = (UInt32 *)(void *)deviceCodeStr;

	reqPtr->type = ESIF_DATA_UINT32;
	reqPtr->buf_ptr = deviceCodePtr;
	reqPtr->buf_len = sizeof(*deviceCodePtr);

	rspPtr = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	if (NULL == rspPtr) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error: Unable to allocate memory\n");
		goto exit;
	}
	rc = EsifExecutePrimitive(
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		GET_AVAILABLE_DEVICES,
		"D0",
		255,
		reqPtr,
		rspPtr);

	if (rc != ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Error Executing Primitive: %s \n", esif_rc_str(rc));
		goto exit;
	}

	if (rspPtr->data_len < sizeof(*deviceListPtr)) {
		rc = ESIF_E_UNSPECIFIED;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Incomplete data structure. \n");
		goto exit;
	}
	
	deviceListPtr = (struct loadable_action_devices *)rspPtr->buf_ptr;
	numDevices = deviceListPtr->numDevicesLoaded;
	if (rspPtr->data_len < ((UInt32)(SIZE_OF(struct loadable_action_devices, deviceData) * (numDevices)) + OFFSET_OF(struct loadable_action_devices, deviceData))) { 
		rc = ESIF_E_UNSPECIFIED;
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Incomplete data structure. \n");
		goto exit;
	}

	if (g_format == FORMAT_XML) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "<Devices>\n");
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Devices: \n");
	}
	
	devicePtr = deviceListPtr->deviceData;
	for (int i = 0; i < numDevices; i++) {
		if (g_format == FORMAT_XML) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "<Device>\n");
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "<Name>%s</Name>\n", devicePtr->deviceName);
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "<Description>%s</Description>\n", devicePtr->deviceDescription);
			for (int n = 0; n < LOADABLE_ACTION_DEVICE_DATA_COUNT; n++) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "<NumericData%d>%d</NumericData%d>\n", n, devicePtr->deviceNumericData[n], n);
			}
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</Device>\n");
		}
		else {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Device Name: %s\n", devicePtr->deviceName);
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Description: %s\n", devicePtr->deviceDescription);
			for (int n = 0; n < LOADABLE_ACTION_DEVICE_DATA_COUNT; n++) {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Numeric Data %d: %d\n", n, devicePtr->deviceNumericData[n]);
			}
		}
		devicePtr++;
	}
	if (g_format == FORMAT_XML) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "</Devices>\n");
	}
	
exit:
	esif_ccb_free(reqPtr);
	EsifData_Destroy(rspPtr);
	return output;
}

//
// Helper functions for esif_shell_cmd_capture. Writes XML formatted info to file.
//

// Writes version information to file in XML format.
static eEsifError write_about_xml(FILE *fp)
{
	eEsifError rc = ESIF_OK;
	char fullpath[MAX_PATH] = { 0 };
	char *currentGddvDesc = "N/A";
	char *currentCacheDesc = "N/A";

	esif_build_path(fullpath, sizeof(fullpath), ESIF_PATHTYPE_DV, "override", ESIFDV_FILEEXT);
	
	// Get Current GDDV Description from GDDV DataCache
	DataVaultPtr DV = DataBank_GetDataVault("gddv");
	if ( DV != NULL) {
		esif_ccb_read_lock(&DV->lock);
		if (DV->comment[0]) {
			currentCacheDesc = DV->comment;
		}
		esif_ccb_read_unlock(&DV->lock);
		DataVault_PutRef(DV);
	}

	// Get Base GDDV Description from GDDV object
	EsifData response = { ESIF_DATA_STRING, NULL, ESIF_DATA_ALLOCATE, 0 };
	rc = EsifExecutePrimitive(
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		GET_CONFIG_DATAVAULT_DESCRIPTION,
		"D0",
		255,
		NULL,
		&response);
	if (rc == ESIF_OK && response.buf_ptr && response.data_len) {
		currentGddvDesc = (char *)response.buf_ptr;
	}

	if (esif_ccb_fprintf(fp,
		"<about>\n"
		"  <ufVersion>%s</ufVersion>\n"
		"  <lfVersion>%s</lfVersion>\n"
		"  <osType>%s</osType>\n"
		"  <shellVersion>%s</shellVersion>\n"
		"  <overrideDvExistsOnDisk>%s</overrideDvExistsOnDisk>\n"
		"  <currentGddvDescription>%s</currentGddvDescription>\n"
		"  <currentCacheDescription>%s</currentCacheDescription>\n"
		"</about>\n\n",
		ESIF_UF_VERSION,
		g_esif_kernel_version,
		g_os,
		g_esif_shell_version,
		(esif_ccb_file_exists(fullpath) ? "true" : "false"),
		currentGddvDesc,
		currentCacheDesc) <= 0) {
		rc = ESIF_E_IO_ERROR;
	}
	esif_ccb_free(response.buf_ptr);
	return rc;
}

// Writes UUID list to file in XML format. Helper function for write_idsp_data_xml.
static eEsifError write_uuid_list_xml(
	FILE *fp,
	IdspEntryPtr entryPtr,
	int numEntries
)
{
	eEsifError rc = ESIF_OK;
	int i = 0;
	u8 mangledUuid[ESIF_GUID_LEN] = { 0 };
	char uuidStr[ESIF_GUID_PRINT_SIZE] = { 0 };

	ESIF_ASSERT(entryPtr != NULL);

	// Write to file
	if (esif_ccb_fprintf(fp, "<idsp>\n") <= 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	for (i = 0; i < numEntries; i++) {
		esif_ccb_memcpy(&mangledUuid, (u8 *)entryPtr->uuid, ESIF_GUID_LEN);
		esif_guid_mangle(&mangledUuid);
		esif_guid_print((esif_guid_t *)(mangledUuid), uuidStr);
		if (esif_ccb_fprintf(fp, "  <fld>%s</fld>\n", uuidStr) <= 0) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		entryPtr++;
	}
	if (esif_ccb_fprintf(fp, "</idsp>\n\n") <= 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

exit:
	return rc;
}

// Writes IDSP info to file in XML format.
static eEsifError write_idsp_data_xml(FILE *fp)
{
	eEsifError rc = ESIF_OK;
	EsifDataPtr rspPtr = NULL;
	UInt32 numEntries = 0;
	IdspEntryPtr curEntryPtr = NULL;

	// Get the current IDSP
	rspPtr = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	if (NULL == rspPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	rc = EsifExecutePrimitive(
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		GET_SUPPORTED_POLICIES,
		"D0",
		255,
		NULL,
		rspPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}
	numEntries = (int)(rspPtr->data_len / sizeof(*curEntryPtr));
	if (numEntries == 0) {
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	// Write list to file
	rc = write_uuid_list_xml(fp, (IdspEntryPtr)rspPtr->buf_ptr, numEntries);

exit:
	EsifData_Destroy(rspPtr);
	return rc;
}

int is_empty_result_set(char *string) 
{
	return esif_ccb_strcmp(string, "<result>\n  <tableRow>\n  </tableRow>\n</result>\n");
}

int is_end_result_set(char *string) 
{
	return esif_ccb_strcmp(string, "</result>");
}

void addTab(
	size_t *string_length, 
	char **string, 
	int tabCount
)
{
	int i = 0;

	for (i = 0; i < tabCount; i++) {
		esif_shell_sprintf_concat(string_length, string, "\t");
	}
}

char* findNthInstance(
	char *string,
	int n,
	char sep
)
{
	int i;
	for (i = 0; i < n; i++) {
		if ((string = esif_ccb_strchr(string, sep)) != NULL) {
			string += 1;
		}
		else {
			goto exit;
		}
	}
exit:
	return string;
}

// Writes an instance of tableobject data to file in XML format. This takes a 
// tableobject code, participant ID and a domain ID and outputs the table to 
// the file. Ex: info for "tableobject get trippoints 0 d0" would be written to
// the file.
void write_instance_tableobject_get_xml(
	char *tableobjectCode,
	esif_handle_t participantId,
	char *domainID,
	FILE *fp,
	int tabCount,
	TableObject *tableObject,
	char *dataSource,
	char *dataMember,
	int *counter
)
{
	eEsifError rc = ESIF_OK;
	char *localCxt = NULL;
	char *temp = NULL;
	char *tableObjectXML = NULL;
	size_t tableObjectXML_length = 0;

	// Load table
	if (tableObject->name == NULL) {
		TableObject_Construct(tableObject, tableobjectCode, domainID, dataSource, dataMember, NULL, 0, participantId, GET);
		if ((rc = TableObject_LoadAttributes(tableObject)) != ESIF_OK) {
			goto exit;
		}
	}
	if (((rc = TableObject_LoadData(tableObject)) != ESIF_OK) ||
		((rc = TableObject_LoadSchema(tableObject)) != ESIF_OK)) {
		goto exit;
	}

	// Write table info to file
	rc = TableObject_LoadXML(tableObject, ESIF_TEMP_C);
	if ((rc == ESIF_OK) &&
		(tableObject->dataXML != NULL) &&
		(is_empty_result_set(tableObject->dataXML) != 0)) {
		if ((esif_ccb_strcmp(tableobjectCode, "gpss") == 0) && ((temp = esif_ccb_strtok(tableObject->dataXML, "\n", &localCxt)) != NULL)) {
			if (esif_ccb_strcmp(temp, "<row>") == 0) {
				(*counter) += 1;
			}
			while ((temp = esif_ccb_strtok(NULL, "\n", &localCxt)) != NULL) {
				if (esif_ccb_strcmp(temp, "<row>") == 0) {
					(*counter) += 1;
				}
			}
		} else {
			if ((temp = esif_ccb_strtok(tableObject->dataXML, "\n", &localCxt)) != NULL) {
				addTab(&tableObjectXML_length, &tableObjectXML, tabCount);
				esif_shell_sprintf_concat(&tableObjectXML_length, &tableObjectXML, "%s\n", temp);
				addTab(&tableObjectXML_length, &tableObjectXML, tabCount);
			}
			while ((temp = esif_ccb_strtok(NULL, "\n", &localCxt)) != NULL) {
				if (is_end_result_set(temp) == 0) {
					esif_shell_sprintf_concat(&tableObjectXML_length, &tableObjectXML, "%s\n", temp);
				}
				else {
					esif_shell_sprintf_concat(&tableObjectXML_length, &tableObjectXML, "%s\n", temp);
					addTab(&tableObjectXML_length, &tableObjectXML, tabCount);
				}
			}
			if (tableObjectXML) {
				esif_ccb_fprintf(fp, "%s", tableObjectXML);
			}
		}
	}

exit:
	TableObject_Destroy(tableObject);
	esif_ccb_free(tableObjectXML);
	return;
}

// Writes an instance of ntt data to file in XML format. This takes a 
// participant ID and a domain ID and outputs the ntt info to the file.
void write_instance_getp_uint32_xml(
	esif_handle_t participantId,
	char *domainID,
	FILE *fp,
	int primitiveID,
	const UInt8 instance,
	int tabCount
)
{
	eEsifError rc = ESIF_OK;
	EsifDataPtr rspPtr = NULL;
	char *getpXML = NULL;
	size_t getpXML_length = 0;

	rspPtr = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	if (NULL == rspPtr) {
		goto exit;
	}

	rc = EsifExecutePrimitive(
		participantId,
		primitiveID,
		domainID,
		instance,
		NULL,
		rspPtr);

	if (rc == ESIF_OK) {
		if (NULL == rspPtr->buf_ptr) {
			goto exit;
		}

		// Our Data
		u32 val = *(u32 *)(rspPtr->buf_ptr);
		addTab(&getpXML_length, &getpXML, tabCount);
		esif_shell_sprintf_concat(&getpXML_length, &getpXML, "<value>%u</value>\n", val);
		addTab(&getpXML_length, &getpXML, tabCount);
		esif_shell_sprintf_concat(&getpXML_length, &getpXML, "<valueDesc> </valueDesc>\n");
		esif_ccb_fprintf(fp, "%s", getpXML);
	}
exit:
	esif_ccb_free(getpXML);
	if (rspPtr != NULL) {
		EsifData_Destroy(rspPtr);
	}
	return;
}

// Writes an instance of ntt data to file in XML format. This takes a 
// participant ID and a domain ID and outputs the ntt info to the file.
void write_instance_ntt_data_xml(
	esif_handle_t participantId,
	char *domainID,
	FILE *fp
)
{
	eEsifError rc = ESIF_OK;
	EsifDataPtr rspPtr = NULL;

	rspPtr = EsifData_CreateAs(ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0);
	if (NULL == rspPtr) {
		goto exit;
	}

	rc = EsifExecutePrimitive(
		participantId,
		GET_NOTIFICATION_TEMP_THRESHOLD,
		domainID,
		255,
		NULL,
		rspPtr);

	if (rc == ESIF_OK) {
		if (NULL == rspPtr->buf_ptr) {
			goto exit;
		}

		// Our Data
		u32 val = *(u32 *)(rspPtr->buf_ptr);
		float temp = 0.0;
		Bool disabled = ESIF_FALSE;

		// Magic number used to disable trip-points...
		if (ESIF_DISABLED_TEMP_VALUE == val) {
			disabled = ESIF_TRUE;
		}

		// Internally we use 1/10ths K, but display in float C
		esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, &val);

		temp = (float)((int)val / 10.0);

		// Write info to file
		if (!disabled) {
			esif_ccb_fprintf(fp,
				"\t\t<ntt>\n"
				"\t\t\t<value>%.1f</value>\n"
				"\t\t\t<valueDesc>%s</valueDesc>\n"
				"\t\t</ntt>\n",
				temp, "Degrees C");
		}
		else {
			esif_ccb_fprintf(fp,
				"\t\t<ntt>\n"
				"\t\t\t<value>DISABLED</value>\n"
				"\t\t\t<valueDesc> </valueDesc>\n"
				"\t\t</ntt>\n");
		}
	}
exit:
	if (rspPtr != NULL) {
		EsifData_Destroy(rspPtr);
	}
	return;
}

//
// The following code will write tableobjectCode data into a file. The functions
// will interpret the input '*' as well as individual inputs and then call the helper
// functions write_instance_trippoints_data_xml and write_instance_ntt_data_xml
// with the interpreted parameters. In these functions, a '*' input represents 
// all participants or all domains for a particular participant.
//

// Writes trippoints data and ntt data to a file.
static eEsifError wd_write_trippoints_data_xml(
	esif_handle_t participantID,
	char *participantName,
	char *domainID,
	FILE *fp,
	EsifUpPtr upPtr,
	EsifUpDomainPtr domainPtr
)
{
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;
	EsifUpDataPtr metaPtr = NULL;
	TableObject tableObject = { 0 };

	if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD]) {
		if ((metaPtr = EsifUp_GetMetadata(upPtr)) != NULL) {
			rc = ESIF_OK;
			if (esif_ccb_fprintf(fp,
				"<participant>\n"
				"\t<name>%s</name>\n"
				"\t<domainID>%s</domainID>\n"
				"\t<description>%s</description>\n"
				"\t<trippoints>\n",
				participantName, domainID, metaPtr->fDesc) <= 0) {
				rc = ESIF_E_IO_ERROR;
				goto exit;
			}
			write_instance_tableobject_get_xml("trippoints", participantID, domainID, fp, 2, &tableObject, NULL, NULL, NULL);
			write_instance_ntt_data_xml(participantID, domainID, fp);
			if (esif_ccb_fprintf(fp, "\t</trippoints>\n</participant>\n\n") <= 0) {
				rc = ESIF_E_IO_ERROR;
			}
		}
	} else {
		rc = ESIF_OK;
	}
exit:
	return rc;
}


// Writes specified default tables to a file. These are limited to default tables vsct and vspt that only
// exist for virtual participants.
static eEsifError wd_write_vsct_vspt_xml(
	char *tableobjectCode,
	esif_handle_t participantID,
	char *participantName,
	char *domainID,
	FILE *fp,
	EsifUpPtr upPtr
)
{
	eEsifError rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	TableObject tableObject = { 0 };

	if ((dspPtr = EsifUp_GetDsp(upPtr)) == NULL) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}
	if (esif_ccb_strcmp(dspPtr->code_ptr, "dpf_virt") == 0) {
		if (esif_ccb_fprintf(fp,
			"<participant>\n"
			"\t<name>%s</name>\n"
			"\t<domainID>%s</domainID>\n",
			participantName, domainID) <= 0) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		if (esif_ccb_fprintf(fp,
			"\t<default_%s>\n",
			tableobjectCode) <= 0) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
		write_instance_tableobject_get_xml(tableobjectCode, participantID, domainID, fp, 2, &tableObject, NULL, NULL, NULL);
		if (esif_ccb_fprintf(fp,
			"\t</default_%s>\n"
			"</participant>\n\n",
			tableobjectCode) <= 0) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
	}
exit:
	return rc;
}


static eEsifError wd_write_participant_list_xml(FILE *fp)
{
	eEsifError rc = ESIF_OK;
	eEsifError iterRc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	UfPmIterator upIter = { 0 };
	EsifString participantName;

	ESIF_ASSERT(fp);

	iterRc = EsifUpPm_InitIterator(&upIter);
	if (ESIF_OK == iterRc) {

		if (esif_ccb_fprintf(fp, "<participants>\n") <= 0) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}

		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		while (ESIF_OK == iterRc) {
			participantName = EsifUp_GetName(upPtr);
			if (participantName) {
				if (esif_ccb_fprintf(fp,
					"\t<participant>\n"
						"\t\t<name>%s</name>\n"
					"\t</participant>\n",
					participantName) <= 0) {
					rc = ESIF_E_IO_ERROR;
					goto exit;
				}
			}
			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}

		if (esif_ccb_fprintf(fp, "</participants>\n\n") <= 0) {
			rc = ESIF_E_IO_ERROR;
			goto exit;
		}
	}
exit:
	return rc;
}



static eEsifError wd_write_cap_header_xml(
	char *participantName,
	char *controlCapName,
	char *domainID,
	FILE *fp
)
{
	eEsifError rc = ESIF_OK;
	if (domainID != NULL) {
		if (esif_ccb_fprintf(fp,
			"<participant>\n"
			"\t<name>%s</name>\n"
			"\t<domainID>%s</domainID>\n"
			"\t<%s>\n",
			participantName, domainID, controlCapName) <= 0) {
			rc = ESIF_E_IO_ERROR;
		}
	} else {
		if (esif_ccb_fprintf(fp,
			"\t</%s>\n"
			"</participant>\n\n",
			controlCapName) <= 0) {
			rc = ESIF_E_IO_ERROR;
		}
	}
	return rc;
}

static eEsifError wd_write_cap_info_xml(
	esif_handle_t participantID,
	char *domainID,
	char *primitiveName,
	FILE *fp,
	enum esif_primitive_type primitiveID,
	enum esif_domain_type domainType
)
{
	eEsifError rc = ESIF_OK;
	char *getpXML = NULL;
	size_t getpXML_length = 0;

	if (esif_ccb_fprintf(fp,
		"\t\t<%s>\n",
		primitiveName) <= 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	if (primitiveID == 0) {
		TableObject tableObject = { 0 };
		write_instance_tableobject_get_xml(primitiveName, participantID, domainID, fp, 3, &tableObject, NULL, NULL, NULL);
	} else {
		if ((domainType == 0) && (esif_ccb_strcmp(primitiveName, "pppc") == 0)) {
			addTab(&getpXML_length, &getpXML, 3);
			esif_shell_sprintf_concat(&getpXML_length, &getpXML, "<value>0</value>\n");
			addTab(&getpXML_length, &getpXML, 3);
			esif_shell_sprintf_concat(&getpXML_length, &getpXML, "<valueDesc> </valueDesc>\n");
			esif_ccb_fprintf(fp, "%s", getpXML);
		} else if ((domainType == 0) && (esif_ccb_strcmp(primitiveName, "gpss") == 0)) {
			TableObject tableObject = { 0 };
			int counter = 0;
			write_instance_tableobject_get_xml(primitiveName, participantID, domainID, fp, 3, &tableObject, NULL, NULL, &counter);
			addTab(&getpXML_length, &getpXML, 3);
			esif_shell_sprintf_concat(&getpXML_length, &getpXML, "<value>%d</value>\n", counter);
			addTab(&getpXML_length, &getpXML, 3);
			esif_shell_sprintf_concat(&getpXML_length, &getpXML, "<valueDesc> </valueDesc>\n");
			esif_ccb_fprintf(fp, "%s", getpXML);
		} else {
			write_instance_getp_uint32_xml(participantID, domainID, fp, primitiveID, domainType, 3);
		}
	}

	if (esif_ccb_fprintf(fp,
		"\t\t</%s>\n",
		primitiveName) <= 0) {
		rc = ESIF_E_IO_ERROR;
	}
exit:
	esif_ccb_free(getpXML);
	return rc;
}

// Translates domain input and then calls the appropriate function based on
// tableobjectCode.
static eEsifError wd_translate_domain_input(
	char *tableobjectCode,
	esif_handle_t participantID,
	char *participantName,
	char *domainID,
	FILE *fp,
	EsifUpPtr upPtr
)
{
	eEsifError rc = ESIF_OK;
	eEsifError iterRc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	UpDomainIterator udIter = { 0 };
	EsifUpDomainPtr domainPtr = NULL;

	if ((esif_ccb_strcmp(domainID, "D0") != 0) &&
		(esif_ccb_strcmp(domainID, "D1") != 0) &&
		(esif_ccb_strcmp(domainID, "D2") != 0) &&
		(esif_ccb_strcmp(domainID, "*") != 0)) {
		rc = ESIF_E_INVALID_DOMAIN_ID;
		goto exit;
	}
	dspPtr = EsifUp_GetDsp(upPtr);
	if (dspPtr != NULL) {
		iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
		if (iterRc != ESIF_OK )
			goto exit;
		iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
		while (iterRc == ESIF_OK) {
			if ((domainPtr != NULL) && ((esif_ccb_strcmp(domainPtr->domainStr, domainID) == 0) || (esif_ccb_strcmp(domainID, "*") == 0))) {
				if (esif_ccb_strcmp(tableobjectCode, "trippoints") == 0) {
					rc = wd_write_trippoints_data_xml(participantID, participantName, domainPtr->domainStr, fp, upPtr, domainPtr);
				} else if ((esif_ccb_strcmp(tableobjectCode, "vsct") == 0) || (esif_ccb_strcmp(tableobjectCode, "vspt") == 0)) {
					rc = wd_write_vsct_vspt_xml(tableobjectCode, participantID, participantName, domainPtr->domainStr, fp, upPtr);
				} else if (esif_ccb_strcmp(tableobjectCode, "power_cc") == 0) {
					if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_POWER_CONTROL]) {
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, domainPtr->domainStr, fp);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "ppcc", fp, 0, 0);
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, NULL, fp);
					}
				} else if (esif_ccb_strcmp(tableobjectCode, "performance_cc") == 0) {
					if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_PERF_CONTROL]) {
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, domainPtr->domainStr, fp);
						if ((domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_PERF_CONTROL] == 1) || 
							(domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_PERF_CONTROL] == 4)) {
							rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "ppdl", fp, GET_PERF_PSTATE_DEPTH_LIMIT, ESIF_DOMAIN_TYPE_ALL);
							rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "pppc", fp, GET_PARTICIPANT_PERF_PRESENT_CAPABILITY, ESIF_DOMAIN_TYPE_ALL);
						} else if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_PERF_CONTROL] == 2) {
							rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "_pdl", fp, GET_PROC_PERF_PSTATE_DEPTH_LIMIT, ESIF_DOMAIN_TYPE_ALL);
							rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "_ppc", fp, GET_PROC_PERF_PRESENT_CAPABILITY, ESIF_DOMAIN_TYPE_ALL);
						} else if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_PERF_CONTROL] == 3) {
							rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "gpss", fp, GET_PERF_SUPPORT_STATES, 0);
							rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "pppc", fp, GET_PARTICIPANT_PERF_PRESENT_CAPABILITY, 0);
						}
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, NULL, fp);
					}
				} else if (esif_ccb_strcmp(tableobjectCode, "display_cc") == 0) {
					if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL]) {
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, domainPtr->domainStr, fp);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "dddl", fp, GET_DISPLAY_DEPTH_LIMIT, ESIF_DOMAIN_TYPE_ALL);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "ddpc", fp, GET_DISPLAY_CAPABILITY, ESIF_DOMAIN_TYPE_ALL);
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, NULL, fp);
					}
				} else if (esif_ccb_strcmp(tableobjectCode, "core_cc") == 0) {
					if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_CORE_CONTROL]) {
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, domainPtr->domainStr, fp);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "lpc", fp, GET_PROC_LOGICAL_PROCESSOR_COUNT, ESIF_DOMAIN_TYPE_ALL);
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, NULL, fp);
					}
				} else if (esif_ccb_strcmp(tableobjectCode, "rf_profile_cc") == 0) {
					if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL]) {
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, domainPtr->domainStr, fp);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "dcf", fp, GET_RFPROFILE_DEFAULT_CENTER_FREQUENCY, ESIF_DOMAIN_TYPE_ALL);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "cf", fp, GET_RFPROFILE_CENTER_FREQUENCY, ESIF_DOMAIN_TYPE_ALL);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "far", fp, GET_RFPROFILE_FREQUENCY_ADJUST_RESOLUTION, ESIF_DOMAIN_TYPE_ALL);
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, NULL, fp);
					}
				} else if (esif_ccb_strcmp(tableobjectCode, "fan_cc") == 0) {
					if (domainPtr->capability_for_domain.capability_mask[ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL]) {
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, domainPtr->domainStr, fp);
						rc = wd_write_cap_info_xml(participantID, domainPtr->domainStr, "fcdc", fp, 0, 0);
						rc = wd_write_cap_header_xml(participantName, tableobjectCode, NULL, fp);
					}
				} else {
					rc = ESIF_E_NOT_IMPLEMENTED;
				}
			}
			iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
		}
	}
exit:
	return rc;
}

// Translates participantScope and then calls wd_translate_domain_input
// to continue translating user inputted parameters.
static eEsifError wd_translate_participant_input(
	char *tableobjectCode,
	char *participantScope,
	char *domainID,
	FILE *fp
)
{
	eEsifError rc = ESIF_OK;
	eEsifError iterRc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	UfPmIterator upIter = { 0 };
	EsifString participantName;
	esif_handle_t participantID = ESIF_INVALID_HANDLE;

	if (esif_ccb_strcmp(participantScope, "*") == 0) {
		iterRc = EsifUpPm_InitIterator(&upIter);
		if (iterRc == ESIF_OK) {
			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}
		while (iterRc == ESIF_OK) {
			if (rc == ESIF_OK) {
				participantID = EsifUp_GetInstance(upPtr);
				participantName = EsifUp_GetName(upPtr);
				rc = wd_translate_domain_input(tableobjectCode, participantID, participantName, domainID, fp, upPtr);
			}
			iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
		}
	} else {
		if ((upPtr = EsifUpPm_GetAvailableParticipantByName(participantScope)) != NULL) {
			participantID = EsifUp_GetInstance(upPtr);
			rc = wd_translate_domain_input(tableobjectCode, participantID, participantScope, domainID, fp, upPtr);
		} else {
			rc = ESIF_E_INVALID_PARTICIPANT_ID;
		}
	}
	return rc;
}

// Writes specified default table to file in XML format.
static eEsifError write_default_table_info_xml(
	char *tableobjectCode,
	FILE *fp
)
{
	eEsifError rc = ESIF_OK;
	TableObject tableObject = { 0 };

	TableObject_Construct(&tableObject, tableobjectCode, "D0", NULL, NULL, NULL, 0, 0, GET);
	if ((rc = TableObject_LoadAttributes(&tableObject)) != ESIF_OK) {
		goto exit;
	}
	if (esif_ccb_fprintf(fp, "<default_%s>\n", tableobjectCode) <= 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	write_instance_tableobject_get_xml(tableobjectCode, 0, "D0", fp, 1, &tableObject, NULL, NULL, NULL);
	if (esif_ccb_fprintf(fp, "</default_%s>\n\n", tableobjectCode) <= 0) {
		rc = ESIF_E_IO_ERROR;
	}
exit:
	TableObject_Destroy(&tableObject);
	return rc;
}

// Writes specified named table to file in XML format.
void write_named_table_info_xml(
	char *namedTablePath,
	char *acpiCode,
	FILE *fp
)
{
	eEsifError rc = ESIF_OK;
	char *namesp = DataBank_GetDefault();
	char *keyspec = NULL;
	char *tableobjectCode = NULL;
	char *dataMember = NULL;
	char *tableNumber = NULL;
	size_t keyspec_length = 0;
	size_t dataMember_length = 0;
	EsifConfigFindContext context = NULL;
	EsifDataPtr  data_nspace = NULL;
	EsifDataPtr  data_key = NULL;
	TableObject tableObject = { 0 };

	if (esif_ccb_strchr(acpiCode, '_') != NULL) {
		tableobjectCode = acpiCode + 1;
	} else {
		tableobjectCode = acpiCode;
	}

	if (esif_ccb_fprintf(fp, "<named_%s>\n", tableobjectCode) <= 0) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	esif_shell_sprintf(&keyspec_length, &keyspec, "/shared/tables/%s", namedTablePath);
	data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, namesp, 0, ESIFAUTOLEN);
	data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
	if (data_nspace == NULL || data_key == NULL) {
		goto exit;
	}
	if ((rc = EsifConfigFindFirst(data_nspace, data_key, NULL, &context)) == ESIF_OK) {
		do {
			esif_shell_sprintf(&dataMember_length, &dataMember, "%s", (esif_string)data_key->buf_ptr);
			EsifData_Set(data_key, ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
			tableNumber = findNthInstance(dataMember, 4, '/');
			if (tableNumber) {
				if (esif_ccb_fprintf(fp, "\t<%s>\n", tableNumber) <= 0) {
					rc = ESIF_E_IO_ERROR;
					break;
				}
				write_instance_tableobject_get_xml(tableobjectCode, 0, "D0", fp, 2, &tableObject, namesp, dataMember, NULL);
				if (esif_ccb_fprintf(fp, "\t</%s>\n", tableNumber) <= 0) {
					rc = ESIF_E_IO_ERROR;
					break;
				}
			}
		} while ((rc = EsifConfigFindNext(data_nspace, data_key, NULL, &context)) == ESIF_OK);

		EsifConfigFindClose(&context);
		if (rc == ESIF_E_ITERATION_DONE) {
			rc = ESIF_OK;
		}
	}
	if (esif_ccb_fprintf(fp, "</named_%s>\n\n", tableobjectCode) <= 0) {
		rc = ESIF_E_IO_ERROR;
	}

exit:
	esif_ccb_free(keyspec);
	esif_ccb_free(dataMember);
	EsifData_Destroy(data_nspace);
	EsifData_Destroy(data_key);
	return;
}

// DPTF dump
static char *esif_shell_cmd_capture(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	eEsifError rc = ESIF_OK;
	int errnum = 0;
	int argc = shell->argc;
	size_t acpiCode_length = 0;
	char **argv = shell->argv;
	char *filename = NULL;
	char *extension = NULL;
	char *localCxt = NULL;
	char *lCxt = NULL;
	char *tableobjectCode = NULL;
	char *participantScope = NULL;
	char *domainID = NULL;
	char *acpiCode = NULL;
	char *temp = NULL;
	char *sep = ",\n";
	char configFilePath[MAX_PATH] = { 0 };
	char configReadBuf[MAX_FILE_LINE_LENGTH] = { 0 };
	char filePath[MAX_PATH] = { 0 };
	Bool overwriteIfExists = ESIF_FALSE;
	Bool useGivenFileName = ESIF_FALSE;
	FILE *configFp = NULL;
	FILE *fp = NULL;
	int arg = 1;

	/*
	Create filename to write to.
	If filename provided, use filename. Otherwise format filename with
	local date and time (yyyy-mm-dd-hhmmss).
	*/

	// [-overwrite]
	if ((argc > arg) && esif_ccb_strnicmp(argv[arg], "-overwrite", 2) == 0) {
		overwriteIfExists = ESIF_TRUE;
		arg++;
	}

	// [filename]
	if ((argc > arg) && esif_ccb_strchr(argv[arg], '*') == NULL) {
		filename = argv[arg++];
		useGivenFileName = ESIF_TRUE;
	}

	if (!useGivenFileName) {
		char datetime[MAX_FILENAME] = { 0 };
		time_t now = time(NULL);
		struct tm time = { 0 };
		overwriteIfExists = ESIF_TRUE;
		if (esif_ccb_localtime(&time, &now) == 0) {
			esif_ccb_sprintf(sizeof(datetime), datetime, "Dynamic_Tuning_Settings_From_%04d-%02d-%02d-%02d%02d%02d",
				time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
		}
		filename = datetime;
	}
	extension = (esif_ccb_strchr(filename, '.') == NULL ? ".txt" : NULL);

	// Open files (CaptureConfig.txt for reading and filename.txt for writing)
	esif_build_path(configFilePath, sizeof(configFilePath), ESIF_PATHTYPE_UI, "CaptureConfig.txt", NULL);
	configFp = esif_ccb_fopen(configFilePath, "r", &errnum);
	if (configFp == NULL) {
		rc = ESIF_E_IO_OPEN_FAILED;
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Could not open CaptureConfig.txt at location %s.\n", configFilePath);
		goto exit;
	}
	esif_build_path(filePath, sizeof(filePath), ESIF_PATHTYPE_LOG, filename, extension);
	//check for existence
	if (overwriteIfExists == ESIF_FALSE && esif_ccb_file_exists(filePath)) {
		rc = ESIF_E_IO_ALREADY_EXISTS;
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "File: %s already exists. \n", filePath);
		goto exit;
	}
	fp = esif_ccb_fopen(filePath, "w", &errnum);
	if (fp == NULL) {
		rc = ESIF_E_IO_OPEN_FAILED;
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Could not write to configuration file %s at location %s.\n", filename, filePath);
		goto exit;
	}

	// Look up info for every line in CaptureConfig.txt file
	while ((rc == ESIF_OK) && (esif_ccb_fgets(configReadBuf, sizeof(configReadBuf), configFp) != NULL)) {
		// Parse and validate info provided
		if ((tableobjectCode = esif_ccb_strtok(configReadBuf, sep, &localCxt)) == NULL ||
			(participantScope = esif_ccb_strtok(NULL, sep, &localCxt)) == NULL ||
			(domainID = esif_ccb_strtok(NULL, sep, &localCxt)) == NULL ||
			esif_ccb_strtok(NULL, sep, &localCxt) != NULL) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Improper file format, continuing to next line.\n");
		} else if (esif_ccb_strcmp(tableobjectCode, "about") == 0) {
			rc = write_about_xml(fp);
		}
		else if (esif_ccb_strcmp(tableobjectCode, "idsp") == 0) {
			rc = write_idsp_data_xml(fp);
		} else if (esif_ccb_strcmp(tableobjectCode, "participantlist") == 0) {
			rc = wd_write_participant_list_xml(fp);
		}
		else if ((esif_ccb_strcmp(tableobjectCode, "trippoints") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "vsct") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "vspt") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "power_cc") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "performance_cc") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "display_cc") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "core_cc") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "rf_profile_cc") == 0) ||
			(esif_ccb_strcmp(tableobjectCode, "fan_cc") == 0)) {
			domainID = esif_ccb_strupr(domainID, MAX_DOMAIN_ID_LENGTH + 1);
			rc = wd_translate_participant_input(tableobjectCode, participantScope, domainID, fp);
		} else if (((temp = esif_ccb_strchr(tableobjectCode, '/')) != NULL) && (esif_ccb_strcmp(temp, "/*") == 0)) {
			esif_shell_sprintf(&acpiCode_length, &acpiCode, "%s", tableobjectCode);
			esif_ccb_strtok(acpiCode, "/", &lCxt);
			write_named_table_info_xml(tableobjectCode, acpiCode, fp);
		} else {
			rc = write_default_table_info_xml(tableobjectCode, fp);
		}
		
		if (rc == ESIF_E_NOT_IMPLEMENTED) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Improper request for data \"%s\", continuing to next line.\n", tableobjectCode);
			rc = ESIF_OK;
		} else if (rc == ESIF_E_INVALID_PARTICIPANT_ID) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Improper request for participant \"%s\", continuing to next line.\n", participantScope);
			rc = ESIF_OK;
		} else if (rc == ESIF_E_INVALID_DOMAIN_ID) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Improper request for domain \"%s\", continuing to next line.\n", domainID);
			rc = ESIF_OK;
		}
	}

exit:
	esif_ccb_free(acpiCode);
	if (fp != NULL) {
		esif_ccb_fclose(fp);
	}
	if (configFp != NULL) {
		esif_ccb_fclose(configFp);
	}
	if (rc == ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Dynamic Tuning Configuration written to %s\n", filePath);
	} else {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "%s (%d)\n", esif_rc_str(rc), rc);
	}
	return output;
}


#ifdef ESIF_ATTR_OS_WINDOWS

// List the supported tool
static char *esif_shell_cmd_tools(EsifShellCmdPtr shell)
{
	int argc = shell->argc;
	char *output = shell->outbuf;

	if (argc > 1) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "No parameter is required for tools command\n");
		goto exit;
	}

	esif_ccb_sprintf(OUT_BUF_LEN, output, "Supported Tools:\n");
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "1. DPPA - Dynamic Performance & Power Analyzer\n");

exit:
	return output;
}

// Launch the supported tool
static char *esif_shell_cmd_toolstart(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc = shell->argc;
	int toolArgc = 0;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	char *toolName = NULL;
	char **toolArgv = NULL;

	if (argc < 2) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Usage: toolstart <tool name> [parameters...]\n");
		goto exit;
	}

	// tool arguments start from argv[2]
	toolName = argv[1];
	toolArgc = argc - 2;
	toolArgv = &(argv[2]);

	rc = EsifToolStart(toolName, toolArgc, toolArgv);
	if (rc == ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Start %s successfully!\n", toolName);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Fail to start %s, status = %d\n", toolName, rc);
	}

exit:
	return output;
}

// Stop the supported tool
static char *esif_shell_cmd_toolstop(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	char *toolName = NULL;

	if (argc != 2) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Usage: toolstop <tool name>\n");
		goto exit;
	}

	toolName = argv[1];

	rc = EsifToolStop(toolName);
	if (rc == ESIF_OK) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Stop %s successfully!\n", toolName);
	} else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Fail to stop %s, status = %d\n", toolName, rc);
	}

exit:
	return output;
}

#endif


///////////////////////////////////////////////////////////////////////////////
// DEBUG
///////////////////////////////////////////////////////////////////////////////

static void dump_table_hdr(struct esif_table_hdr *tab)
{
	CMD_DEBUG("Binary Dump For Table Header\n\n");
	CMD_DEBUG("binary_data_object[%u] = {\n", (int)sizeof(*tab));

	CMD_DEBUG("    table header = {\n");
	CMD_DEBUG("       revision = %d,\n", tab->revision);
	CMD_DEBUG("       num of rows = %d,\n", tab->rows);
	CMD_DEBUG("       num of cols = %d,\n", tab->cols);
	CMD_DEBUG("}\n");
}


// Binary Data Variant Object
static void dump_binary_object(
	u8 *byte_ptr,
	u32 byte_len
	)
{
	union esif_data_variant *obj = (union esif_data_variant *)byte_ptr;
	int remain_bytes = byte_len, counter = 1024;
	char *str;

	CMD_DEBUG("Binary Dump Up To %d Objects:\n\n", counter);
	CMD_DEBUG("binary_data_object[%u] = {\n", byte_len);

	while (remain_bytes > 0 && counter) {
		remain_bytes -= sizeof(*obj);
		counter--;
		switch (obj->type) {
		case ESIF_DATA_UINT8:
		case ESIF_DATA_UINT16:
		case ESIF_DATA_UINT32:
		case ESIF_DATA_UINT64:
		case ESIF_DATA_INT8:
		case ESIF_DATA_INT16:
		case ESIF_DATA_INT32:
		case ESIF_DATA_INT64:
		case ESIF_DATA_TEMPERATURE:
		case ESIF_DATA_FREQUENCY:
		case ESIF_DATA_PERCENT:
		{
			CMD_DEBUG("    integer = { type = %s(%d) value = %lld (0x%llx) }\n",
					  esif_data_type_str(obj->integer.type),
					  obj->integer.type,
					  obj->integer.value,
					  obj->integer.value);
			obj = (union esif_data_variant *)((u8 *)obj + sizeof(*obj));
			break;
		}

		case ESIF_DATA_STRING:
		{
			str = (char *)((u8 *)obj + sizeof(*obj));

			CMD_DEBUG("    string = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = \"%s\"\n", str);
			CMD_DEBUG("    }\n");
			if ((u32)remain_bytes < obj->string.length) {
				remain_bytes = 0;
				continue;
			}
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant *)((u8 *)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		case ESIF_DATA_UNICODE:
		{
			u32 i = 0;
			str = (char *)((u8 *)obj + sizeof(*obj));
			CMD_DEBUG("    unicode = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = ");

			if ((u32)remain_bytes < obj->string.length) {
				remain_bytes = 0;
				continue;
			}
			for (i = 0; i < obj->string.length; i++) {
				CMD_DEBUG("%02X ", (u8)str[i]);
			}

			CMD_DEBUG("\n    }\n");
			if ((u32)remain_bytes < obj->string.length) {
				remain_bytes = 0;
				continue;
			}
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant *)((u8 *)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		case ESIF_DATA_BINARY:
		{
			u32 i = 0;
			str = (char *)((u8 *)obj + sizeof(*obj));
			CMD_DEBUG("    binary = {\n");
			CMD_DEBUG("       type = %s(%d),\n", esif_data_type_str(obj->string.type), obj->string.type);
			CMD_DEBUG("       length = %d,\n", obj->string.length);
			CMD_DEBUG("       value = ");

			if ((u32)remain_bytes < obj->string.length) {
				remain_bytes = 0;
				continue;
			}
			for (i = 0; i < obj->string.length; i++) {
				CMD_DEBUG("%02X ", (u8)str[i]);
			}

			CMD_DEBUG("\n    }\n");
			remain_bytes -= obj->string.length;
			obj = (union esif_data_variant *)((u8 *)obj + (sizeof(*obj) + obj->string.length));
			break;
		}

		/* No more binary data, so stop here! */
		default:
			counter = 0;
		}
	}
	CMD_DEBUG("}\n");
}


// Binary Data
static void dump_binary_data(
	u8 *byte_ptr,
	u32 byte_len
	)
{
	u32 i = 0;
	CMD_DEBUG("Buffer Dump Up To 256 Bytes Maximum:\n");
	for (i = 0; ((i < byte_len) && (i < 256)); i++) {
		if (0 == i) {
			CMD_DEBUG("\n%04x: ", i);
		}

		if (i > 0 && 0 == (i % 8)) {
			CMD_DEBUG("\n%04x: ", i);
		}
		CMD_DEBUG("%02x", *((u8 *)byte_ptr + i));

		if ('\r' == *((u8 *)byte_ptr + i)) {
			CMD_DEBUG("(CR) ");
		} else if ('\n' == *((u8 *)byte_ptr + i)) {
			CMD_DEBUG("(LF) ");
		} else {
			CMD_DEBUG("(%2c) ", *((u8 *)byte_ptr + i));
		}
	}
	CMD_DEBUG("\n\n");
}


// Execute Shell Command
char *esif_shell_exec_command(
	const char *line,
	size_t buf_len,
	UInt8 isRest,
	UInt8 showOutput
	)
{
	char *out_str = NULL;
	enum output_format last_format = g_format;
	char *cmd_separator = NULL;
	size_t cmdlen=0;
	char multi_cmd_sep[] = " && ";
	size_t multi_cmd_seplen = sizeof(multi_cmd_sep) - 1;
	char *temp_line = NULL;
	char *cmdPtr = NULL;
	char *lineCpy = NULL;
	char *local_context = NULL;

	if (esif_ccb_strlen(line, buf_len) >= (buf_len - 1)) {
		return NULL;
	}

	lineCpy = esif_ccb_strdup(line);
	if (NULL == lineCpy) {
		return NULL;
	}

	esif_uf_shell_lock();

	// Create output buffer if necessary
	if ((g_outbuf == NULL) && ((g_outbuf = esif_ccb_malloc(OUT_BUF_LEN)) == NULL)) {
		goto exit;
	}

	// REST API almost always uses XML results
	g_isRest = isRest;
	if (ESIF_TRUE == isRest) {
		last_format = g_format;
		g_format = FORMAT_XML;
	}

	cmdlen = esif_ccb_strlen(lineCpy, buf_len);
	g_cmdlen = cmdlen;
	do {

		esif_ccb_free(temp_line);
		temp_line = NULL;
		temp_line = esif_ccb_strdup(lineCpy);
		if (NULL == temp_line) {
			out_str = NULL;
			goto display_output;
		}

		// Skip first iteration
		if (cmd_separator != NULL) {
			esif_ccb_strcpy(lineCpy, cmd_separator, cmdlen);
		}

		// Allow multiple commands separated by newlines or " && " (except in quoted strings)
		if ((cmd_separator = strstr(lineCpy, "\n")) != NULL) {
			*cmd_separator++ = 0;
		}
		else if (strstr(lineCpy, multi_cmd_sep) != NULL) {
			char lastquote = 0;
			char *ch = 0;
			for (ch = lineCpy; cmd_separator == NULL && *ch != 0; ch++) {
				if (*ch == lastquote) {
					lastquote = 0;
				}
				else if (*ch == '\"' || *ch == '\'') {
					lastquote = *ch;
				}
				else if (!lastquote && esif_ccb_strncmp(ch, multi_cmd_sep, multi_cmd_seplen) == 0) {
					*ch = 0;
					cmd_separator = ch + multi_cmd_seplen;
				}
			}
		}

		g_errorlevel = 0;

		// Run External Command Shell. Disabled in UMDF, shell, & Daemon mode and REST API
		if (g_cmdshell_enabled && !g_isRest && lineCpy[0]=='!') {
			esif_shell_exec_cmdshell(lineCpy + 1);
			out_str = NULL;
			goto display_output;
		}

		cmdPtr = esif_ccb_strtok(temp_line, ESIF_SHELL_STRTOK_SEP, &local_context);
		if (cmdPtr == NULL || *cmdPtr == '#') {
			out_str = NULL;
			goto display_output;
		}
		g_outbuf[0] = 0;

		out_str = esif_shell_exec_dispatch(lineCpy, g_outbuf);

display_output:
		if (showOutput) {
			if (NULL == out_str) {
				CMD_OUT("%s", "");
			}
			if ((NULL != out_str) && (out_str[0] != '\0')) {
				CMD_OUT("%s", out_str);
			}
		}
	} while (cmdPtr != NULL && cmd_separator != NULL);

	if (ESIF_TRUE == isRest) {
		g_format = last_format;
	}
	g_isRest = 0;

exit:
	esif_ccb_free(lineCpy);
	esif_ccb_free(temp_line);
	esif_uf_shell_unlock();
	return out_str;
}


// Parse Command By Subsystem
char *parse_cmd(
	const char *line,
	UInt8 isRest,
	UInt8 showOutput
	)
{
	return esif_shell_exec_command(line, MAX_LINE, isRest, showOutput);
}

// Count the number of arguments
static int count_cmd_args(
	const char *command
	)
{
	int argc = 0;
	char *temp_cmd = NULL;
	char *cmd = NULL;
	char *local_context = NULL;

	cmd = esif_ccb_strdup(command);
	if (NULL == cmd) {
		goto exit;
	}

	temp_cmd = esif_ccb_strtok(cmd, ESIF_SHELL_STRTOK_SEP, &local_context);
	while (temp_cmd != NULL)
	{
		argc++;
		if (*temp_cmd == ';') {	// break on comment
			break;
		}
		temp_cmd = esif_ccb_strtok(NULL, ESIF_SHELL_STRTOK_SEP, &local_context);
	}

exit:
	esif_ccb_free(cmd);
	return argc;
}


//////////////////////////////////////////////////////////////////////////////
// Command Interpreter

// Shell Wrapper
typedef void (*VoidFunc)();
typedef char *(*ArgvFunc)(EsifShellCmd *);

typedef enum FuncType_t {
	fnArgv,		// Use Command Line argc/argv Parser
} FuncType;
typedef struct EsifShellMap_t {
	char *cmd;
	FuncType type;
	VoidFunc func;
} EsifShellMap;

#ifndef ESIF_FEAT_OPT_ACTION_SYSFS

// Shell Command Wrapper
char *esif_shell_cmd_ipc_autoconnect(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_autoconnect(30); // 30 second timeout
	return NULL;
}


// Shell Command Wrapper
char *esif_shell_cmd_ipc_connect(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_connect();
	return NULL;
}

// Shell Command Wrapper
char *esif_shell_cmd_ipc_disconnect(EsifShellCmdPtr shell)
{
	UNREFERENCED_PARAMETER(shell);
	ipc_disconnect();
	return NULL;
}

#endif

// Shell Command Mapping. Keep this array sorted alphabetically to facilitate Binary Searches
static EsifShellMap ShellCommands[] = {
	{"about",                fnArgv, (VoidFunc)esif_shell_cmd_about               },
	{"actions",              fnArgv, (VoidFunc)esif_shell_cmd_actions             },
	{"actionsk",             fnArgv, (VoidFunc)esif_shell_cmd_actionsk            },
	{"actionstart",          fnArgv, (VoidFunc)esif_shell_cmd_actionstart         },
	{"actionstop",           fnArgv, (VoidFunc)esif_shell_cmd_actionstop          },
	{"actionsu",             fnArgv, (VoidFunc)esif_shell_cmd_actionsu            },
	{"addpart",              fnArgv, (VoidFunc)esif_shell_cmd_addpart             },
	{"addpartk",             fnArgv, (VoidFunc)esif_shell_cmd_addpartk            },
	{"app",                  fnArgv, (VoidFunc)esif_shell_cmd_app                 },
	{"apps",                 fnArgv, (VoidFunc)esif_shell_cmd_apps                },
	{"appstart",             fnArgv, (VoidFunc)esif_shell_cmd_appstart            },
	{"appstop",              fnArgv, (VoidFunc)esif_shell_cmd_appstop             },
	{"autoexec",             fnArgv, (VoidFunc)esif_shell_cmd_autoexec            },
	{"capture",              fnArgv, (VoidFunc)esif_shell_cmd_capture			  },
	{"cat",                  fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"cattst",               fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"config",               fnArgv, (VoidFunc)esif_shell_cmd_config              },
	{"conjure",              fnArgv, (VoidFunc)esif_shell_cmd_conjure             },
	{"conjures",             fnArgv, (VoidFunc)esif_shell_cmd_conjures            },
	{"debuglvl",             fnArgv, (VoidFunc)esif_shell_cmd_debuglvl            },
	{"debugset",             fnArgv, (VoidFunc)esif_shell_cmd_debugset            },
	{"debugshow",            fnArgv, (VoidFunc)esif_shell_cmd_debugshow           },
	{"delpartk",             fnArgv, (VoidFunc)esif_shell_cmd_delpartk            },
	{"devices",				 fnArgv, (VoidFunc)esif_shell_cmd_get_available_devices},
	{"domains",              fnArgv, (VoidFunc)esif_shell_cmd_domains             },
	{"driverk",              fnArgv, (VoidFunc)esif_shell_cmd_driversk            },
	{"driversk",             fnArgv, (VoidFunc)esif_shell_cmd_driversk            },
	{"dspquery",			 fnArgv, (VoidFunc)esif_shell_cmd_dspquery			  },
	{"dsps",                 fnArgv, (VoidFunc)esif_shell_cmd_dsps                },
	{"dst",                  fnArgv, (VoidFunc)esif_shell_cmd_dst                 },
	{"dstn",                 fnArgv, (VoidFunc)esif_shell_cmd_dstn                },
	{"dv",                   fnArgv, (VoidFunc)esif_shell_cmd_config              },
	{"echo",                 fnArgv, (VoidFunc)esif_shell_cmd_echo                },
	{"event",                fnArgv, (VoidFunc)esif_shell_cmd_event               },
	{"eventkpe",             fnArgv, (VoidFunc)esif_shell_cmd_eventkpe            },
	{"events",			     fnArgv, (VoidFunc)esif_shell_cmd_events              },
	{"exit",                 fnArgv, (VoidFunc)esif_shell_cmd_exit                },
	{"format",               fnArgv, (VoidFunc)esif_shell_cmd_format              },
	{"getb",                 fnArgv, (VoidFunc)esif_shell_cmd_getb                },
	{"geterrorlevel",        fnArgv, (VoidFunc)esif_shell_cmd_geterrorlevel       },
	{"getf_b",               fnArgv, (VoidFunc)esif_shell_cmd_getf                },
	{"getf_bd",              fnArgv, (VoidFunc)esif_shell_cmd_getf                },
	{"getp",                 fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst)"
	{"getp_b",               fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as binary"
	{"getp_bd",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },
	{"getp_bf",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },
	{"getp_bs",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },
	{"getp_part",            fnArgv, (VoidFunc)esif_shell_cmd_getp                },
	{"getp_pw",              fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as power"
	{"getp_s",               fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as string"
	{"getp_t",               fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as temperature"
	{"getp_u32",             fnArgv, (VoidFunc)esif_shell_cmd_getp                },// Alias for "get primitive(id,qual,inst) as uint32"
	{"help",                 fnArgv, (VoidFunc)esif_shell_cmd_help                },
	{"idsp",                 fnArgv, (VoidFunc)esif_shell_cmd_idsp                },
	{"info",                 fnArgv, (VoidFunc)esif_shell_cmd_info                },
	{"infocpc",              fnArgv, (VoidFunc)esif_shell_cmd_infocpc             },
	{"infofpc",              fnArgv, (VoidFunc)esif_shell_cmd_infofpc             },
#ifndef ESIF_FEAT_OPT_ACTION_SYSFS
	{"ipcauto",              fnArgv, (VoidFunc)esif_shell_cmd_ipc_autoconnect     },
	{"ipccon",               fnArgv, (VoidFunc)esif_shell_cmd_ipc_connect         },
	{"ipcdis",               fnArgv, (VoidFunc)esif_shell_cmd_ipc_disconnect      },
#endif
	{"load",                 fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"loadtst",              fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"log",                  fnArgv, (VoidFunc)esif_shell_cmd_log                 },
	{"memstats",             fnArgv, (VoidFunc)esif_shell_cmd_memstats            },
	{"nolog",                fnArgv, (VoidFunc)esif_shell_cmd_nolog               },
	{"part",                 fnArgv, (VoidFunc)esif_shell_cmd_participant         },
	{"participant",          fnArgv, (VoidFunc)esif_shell_cmd_participant         },	
	{"participantk",         fnArgv, (VoidFunc)esif_shell_cmd_participantk        },
	{"participantlog",       fnArgv, (VoidFunc)EsifShellCmd_ParticipantLog        },
	{"participants",         fnArgv, (VoidFunc)esif_shell_cmd_participants        },
	{"participantsk",        fnArgv, (VoidFunc)esif_shell_cmd_participantsk       },
	{"partk",                fnArgv, (VoidFunc)esif_shell_cmd_participantk        },
	{"parts",                fnArgv, (VoidFunc)esif_shell_cmd_participants        },
	{"partsk",               fnArgv, (VoidFunc)esif_shell_cmd_participantsk       },
	{"paths",				 fnArgv, (VoidFunc) esif_shell_cmd_paths },
	{"proof",                fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"prooftst",             fnArgv, (VoidFunc)esif_shell_cmd_load                },
	{"quit",                 fnArgv, (VoidFunc)esif_shell_cmd_quit                },
	{"rem",                  fnArgv, (VoidFunc)esif_shell_cmd_rem                 },
	{"repeat",               fnArgv, (VoidFunc)esif_shell_cmd_repeat              },
	{"repeat_delay",         fnArgv, (VoidFunc)esif_shell_cmd_repeatdelay         },
	{"rstp",	             fnArgv, (VoidFunc)esif_shell_cmd_reset_override      },
	{"rstp_part",	         fnArgv, (VoidFunc)esif_shell_cmd_reset_override },
	{"set_osc",              fnArgv, (VoidFunc)esif_shell_cmd_set_osc             },
	{"setb",                 fnArgv, (VoidFunc)esif_shell_cmd_setb                },
	{"seterrorlevel",        fnArgv, (VoidFunc)esif_shell_cmd_seterrorlevel       },
	{"setp",                 fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_bf",              fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_bs",              fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_part",            fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_pw",              fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"setp_t",               fnArgv, (VoidFunc)esif_shell_cmd_setp                },
	{"shell",                fnArgv, (VoidFunc)esif_shell_cmd_shell               },
	{"sleep",                fnArgv, (VoidFunc)esif_shell_cmd_sleep               },
	{"soe",                  fnArgv, (VoidFunc)esif_shell_cmd_soe                 },
	{"status",               fnArgv, (VoidFunc)esif_shell_cmd_status              },
	{"tableobject",          fnArgv, (VoidFunc)esif_shell_cmd_tableobject         },
	{"test",                 fnArgv, (VoidFunc)esif_shell_cmd_test                },	
	{"thermalapi",           fnArgv, (VoidFunc)EsifShellCmdThermalApi             },
	{"timerstart",           fnArgv, (VoidFunc)esif_shell_cmd_timerstart          },
	{"timerstop",            fnArgv, (VoidFunc)esif_shell_cmd_timerstop           },
	{"timestamp",            fnArgv, (VoidFunc)esif_shell_cmd_timestamp           },
#ifdef ESIF_ATTR_OS_WINDOWS
	{"tools",                fnArgv, (VoidFunc)esif_shell_cmd_tools               },
	{"toolstart",            fnArgv, (VoidFunc)esif_shell_cmd_toolstart           },
	{"toolstop",             fnArgv, (VoidFunc)esif_shell_cmd_toolstop            },
#endif
	{"trace",                fnArgv, (VoidFunc)esif_shell_cmd_trace               },
	{"ufpoll",				 fnArgv, (VoidFunc)esif_shell_cmd_ufpoll			  },
	{"ui",                   fnArgv, (VoidFunc)esif_shell_cmd_ui                  },// formerly ui_getxslt, ui_getgroups, ui_getmodulesingroup, ui_getmoduledata
	{"unconjure",            fnArgv, (VoidFunc)esif_shell_cmd_unconjure           },
	{"upes",                 fnArgv, (VoidFunc)esif_shell_cmd_upes                },
	{"web",                  fnArgv, (VoidFunc)esif_shell_cmd_web                 },
};

// Public ESIF argc/argv Command Dispatcher
eEsifError esif_shell_dispatch(
	int argc,
	char **argv,
	char **output_ptr
)
{
	static int items = sizeof(ShellCommands) / sizeof(EsifShellMap);
	int start = 0, end = items - 1, node = items / 2;
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;

	if (argc > 0 && argv != NULL && output_ptr != NULL) {
		char *shell_cmd = argv[0];
		EsifShellCmd shell = { 0 };

		rc = ESIF_E_NOT_IMPLEMENTED;

		// Ignore comments
		if (*shell_cmd == '#' || *shell_cmd == ';') {
			start = end;
			rc = ESIF_OK;
		}

		// Do a Binary Search
		while (start <= end) {
			int comp = esif_ccb_stricmp(shell_cmd, ShellCommands[node].cmd);
			if (comp == 0) {
				switch (ShellCommands[node].type) {

				// Command Line argc/argv Parser Support only
				case fnArgv:
					shell.argc   = argc;
					shell.argv   = argv;
					shell.outbuf = *output_ptr;
					*output_ptr = (*(ArgvFunc)(ShellCommands[node].func))(&shell);
					rc = ESIF_OK;
					break;

				default:
					rc = ESIF_E_INVALID_REQUEST_TYPE;
					break;
				}
				break;
			}
			else if (comp > 0) {
				start = node + 1;
			}
			else {
				end = node - 1;
			}
			node = (end - start) / 2 + start;
		}
	}

	if (rc != ESIF_OK && output_ptr != NULL) {
		esif_ccb_sprintf(OUT_BUF_LEN, *output_ptr, "%s (%d)\n", esif_rc_str(rc), rc);
	}
	return rc;
}

// Public ESIF Command line Dispatcher
eEsifError esif_shell_dispatch_cmd(
	const char *line,
	char **output_ptr
)
{
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;
	char *cmd = NULL;
	char *token = NULL;
	char *ctx = NULL;
	int args = 0;
	int argc = 0;
	char **argv = NULL;

	if (line != NULL && output_ptr != NULL) {
		// Convert string to argc/argv array and pass to dispatcher
		rc = ESIF_OK;
		args = count_cmd_args(line);
		if (args > 0) {
			cmd = esif_ccb_strdup(line);
			argv = (char **)esif_ccb_malloc((size_t)args * sizeof(char*));

			if (cmd == NULL || argv == NULL) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}

			token = esif_ccb_strtok(cmd, ESIF_SHELL_STRTOK_SEP, &ctx);
			while (token != NULL && argc < args) {
				if (*token == ';') { // comment
					break;
				}
				argv[argc++] = token;
				token = esif_ccb_strtok(NULL, ESIF_SHELL_STRTOK_SEP, &ctx);
			}

			if (argc > 0) {
				*output_ptr[0] = 0;
				rc = esif_shell_dispatch(argc, argv, output_ptr);

				// If Shell Command not found, allow appname aliases for "appname ..." to "app cmd appname ..."
				if (rc == ESIF_E_NOT_IMPLEMENTED) {
					EsifAppPtr appPtr = EsifAppMgr_GetAppFromName(argv[0]);
					if (appPtr && esif_ccb_stricmp(argv[0], appPtr->fAppNamePtr) == 0) {
						char **new_argv = (char **)esif_ccb_realloc(argv, sizeof(char*) * (argc + 2));
						if (new_argv) {
							argc += 2;
							argv = new_argv;
							esif_ccb_memmove(&argv[2], &argv[0], (sizeof(char *) * (argc - 2)));
							argv[0] = "app";
							argv[1] = "cmd";
							*output_ptr[0] = 0;
							rc = esif_shell_dispatch(argc, argv, output_ptr);
						}
					}
					EsifAppMgr_PutRef(appPtr);
				}
			}
		}
	}

exit:
	if (rc != ESIF_OK && output_ptr != NULL) {
		esif_ccb_sprintf(OUT_BUF_LEN, *output_ptr, "%s (%d)\n", esif_rc_str(rc), rc);
	}
	esif_ccb_free(cmd);
	esif_ccb_free(argv);
	return rc;
}

// ESIF Command Dispatcher
static char *esif_shell_exec_dispatch(
	const char *line,
	char *output
	)
{
	char *outbuf = output;
	esif_shell_dispatch_cmd(line, &outbuf);
	return outbuf;
}

// Execute Shell Command, with repeats if necessary
enum esif_rc esif_shell_execute(const char *command)
{
	enum esif_rc rc = ESIF_OK;
	char *cmdCpy = NULL;
	char *pTemp = NULL;
	char previous = 0;
	int count = 0;

	ESIF_ASSERT(command != NULL);
	
	if (esif_ccb_strlen(command, MAX_LINE) >= (MAX_LINE - 1)) {
		CMD_OUT("Allowed command length (%d) exceeded\n", (MAX_LINE - 1));
		return ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}

	cmdCpy = esif_ccb_strdup(command);
	if (NULL == cmdCpy) {
		return ESIF_E_NO_MEMORY;
	}

	esif_ccb_event_reset(&g_shellStopEvent);
	if (g_shell_stopped) {
		rc = ESIF_E_IFACE_DISABLED;
		goto exit;
	}

	pTemp = cmdCpy;
	Bool inQuote = ESIF_FALSE;
	while (*pTemp != '\0') {
		if (*pTemp == '\"') {
			inQuote = !inQuote;
		}
		if (*pTemp == '\r' || *pTemp == '\n' || (*pTemp == '#' && !inQuote)) {
			previous = *pTemp;
			*pTemp = '\0';
			break;
		}
		pTemp++;
	}

	if ((1 == g_repeat) || esif_ccb_strnicmp(cmdCpy, "repeat", 6) == 0) {
		parse_cmd(cmdCpy, ESIF_FALSE, ESIF_TRUE);
	}
	else {
		for (count = 0; (count < g_repeat) && !g_shell_stopped; count++) {
			parse_cmd(cmdCpy, ESIF_FALSE, ESIF_TRUE);
			if (g_soe && g_errorlevel != 0) {
				rc = g_errorlevel;
				break;
			}

			if (g_repeat_delay && (count + 1 < g_repeat)) {
				esif_ccb_sleep_msec(g_repeat_delay);
			}
		}
		g_repeat = 1;
	}
exit:
	esif_ccb_free(cmdCpy);
	esif_ccb_event_set(&g_shellStopEvent);
	return rc;
}
