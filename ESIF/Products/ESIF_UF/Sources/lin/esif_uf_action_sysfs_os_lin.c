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
/* ability to compile out this entire page without ESIF_FEAT_OPT_ACTION_SYSFS */
#ifdef ESIF_FEAT_OPT_ACTION_SYSFS

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_tableobject.h"
#include "esif_hash_table.h"
#include "esif_participant.h"
#include "esif_sdk_fan.h"

#define MIN_PERF_PERCENTAGE 0
#define MAX_SEARCH_STRING 50
#define MAX_PARAM_STRING (MAX_SEARCH_STRING + MAX_SEARCH_STRING + 1)
#define MAX_IDX_HOLDER 10
#define MAX_NODE_IDX 15
#define MAX_SYSFS_STRING 4 * 1024
#define MAX_SYSFS_PATH 256
#define ERROR_VALUE 255
#define MAX_ESIF_TABLES 2
#define BINARY_TABLE_SIZE OUT_BUF_LEN
#define MAX_GUID_STR_LEN 40 // Assuming XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXX format
#define MAX_ACTION_HT_SIZE 30
#define MAX_SYSFS_POLL_STRING 50
#define MAX_ACX_ENTRIES 10
#define EPSILON_FAN_PERC 0.01 // For rounding out errors in floating point calculations

#define MAX_ACPI_SCOPE_LEN ESIF_SCOPE_LEN
#define ACPI_THERMAL_IOR_TYPE 's'
#define TRT_LEN _IOR(ACPI_THERMAL_IOR_TYPE, 1, u64)
#define ART_LEN _IOR(ACPI_THERMAL_IOR_TYPE, 2, u64)
#define TRT_COUNT _IOR(ACPI_THERMAL_IOR_TYPE, 3, u64)
#define ART_COUNT _IOR(ACPI_THERMAL_IOR_TYPE, 4, u64)
#define GET_TRT	_IOR(ACPI_THERMAL_IOR_TYPE, 5, u64)
#define GET_ART	_IOR(ACPI_THERMAL_IOR_TYPE, 6, u64)

#define CORE			"0000:00:04.0"
#define ATOM			"0000:00:0b.0"
#define ACPI_DPTF		"INT3400:00"
#define ACPI_CPU		"INT3401:00"
#define SYSFS_PCI		"/sys/bus/pci/devices"
#define SYSFS_PLATFORM		"/sys/bus/platform/devices"
#define SYSFS_PSTATE_PATH	"/sys/devices/system/cpu/intel_pstate/"

struct trt_table {
	char trt_source_device[8]; /* ACPI single name */
	char trt_target_device[8]; /* ACPI single name */
	u64 trt_influence;
	u64 trt_sample_period;
	u64 trt_reserved[4];
};

struct art_table {
	char art_source_device[8]; /* ACPI 4 char name */
	char art_target_device[8]; /* ACPI 4 char name */
	u64 art_weight;
	u64 art_ac0_max_level;
	u64 art_ac1_max_level;
	u64 art_ac2_max_level;
	u64 art_ac3_max_level;
	u64 art_ac4_max_level;
	u64 art_ac5_max_level;
	u64 art_ac6_max_level;
	u64 art_ac7_max_level;
	u64 art_ac8_max_level;
	u64 art_ac9_max_level;
};
/* END THERMAL */

enum esif_sysfs_command {
	ESIF_SYSFS_DIRECT_PATH = 'PTCD',
	ESIF_SYSFS_DIRECT_ENUM = 'ETCD',
	ESIF_SYSFS_ALT_PATH = 'PTLA',
	ESIF_SYSFS_DIRECT_QUERY = 'QTCD',
	ESIF_SYSFS_ALT_QUERY = 'QTLA',
	ESIF_SYSFS_CALC = 'CLAC',
	ESIF_SYSFS_ATTRIBUTE = 'RRTA',
	ESIF_SYSFS_DIRECT_QUERY_ENUM = 'EQCD',
	ESIF_SYSFS_BINARY_TABLE = 'LBTB'
};

enum esif_sysfs_param {
	ESIF_SYSFS_GET_SOC_RAPL = 'ARSG',
	ESIF_SYSFS_GET_CPU_PDL = 'DPCG',
	ESIF_SYSFS_GET_SOC_PL1 = 'LPSG',
	ESIF_SYSFS_GET_SOC_TEMP = 'ETSG',
	ESIF_SYSFS_GET_FAN_INFO = 'FIFG',
	ESIF_SYSFS_GET_FAN_PERF_STATES = 'SPFG',
	ESIF_SYSFS_GET_FAN_STATUS = 'TSFG',
	ESIF_SYSFS_SET_CPU_PSTATE = 'SPCS',
	ESIF_SYSFS_SET_OSC = 'CSOS',
	ESIF_SYSFS_SET_FAN_LEVEL = 'ELFS'
};


enum esif_thermal_rel_type {
	ART = 0,
	TRT
};

#pragma pack(push, 1)
struct sysfsActionHashKey {
	struct esif_primitive_tuple primitiveTuple;
	u8 participantId;
};
#pragma pack(pop)

static int sysfs_set_u64(char *path, char *filename, u64 val);
static int sysfs_set_string(char *path, char *filename, char *val);
static int sysfs_get_string(char *path, char *filename, char *str);
static int sysfs_get_string_multiline(char *path, char *filename, char *str);
static int sysfs_get_u64(char *path, char *filename, u64 *p_u64);
static int sysfs_get_u64_direct(int fd, u64 *p_u64);
static int replace_str(char *str, char *old, char *new, char *rpl_buff, int rpl_buff_len);
static int get_key_value_pair_from_str(const char *str, char *key, char *value);
static enum esif_rc get_thermal_rel_str(enum esif_thermal_rel_type type, char *table_str);
static void get_full_scope_str(char *orig, char *new);
static void replace_cpu_id(char *str);
static u64 GetPowerLimit0MaxUw(void);
static enum esif_rc get_supported_policies(char *table_str, int idspNum, char *sysfs_str);
static enum esif_rc get_rapl_power_control_capabilities(char *table_str, esif_guid_t *target_guid);
static enum esif_rc get_proc_perf_support_states(char *table_str);
static enum esif_rc get_participant_current_control_capabilities(char *table_str, char *participant_path);
static enum esif_rc get_perf_support_states(char *table_str, char *participant_path);
static eEsifError get_participant_scope(char *acpi_name, char *acpi_scope);
static int SetActionContext(struct sysfsActionHashKey *keyPtr, EsifString devicePathName, EsifString deviceNodeName);
static struct esif_ht *actionHashTablePtr = NULL;
static char sys_long_string_val[MAX_SYSFS_STRING];
static eEsifError SetFanLevel(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr);
static eEsifError GetFanInfo(EsifDataPtr responsePtr);
static eEsifError GetFanPerfStates(EsifDataPtr responsePtr);
static eEsifError GetFanStatus(EsifDataPtr responsePtr);

/*
 * Handle ESIF Action "Get" Request
 */
static eEsifError ESIF_CALLCONV ActionSysfsGet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	EsifUpDomainPtr domainPtr = NULL;
	EsifString command = NULL;
	EsifString parm1 = NULL;
	EsifString parm2 = NULL;
	EsifString parm3 = NULL;
	EsifString parm4 = NULL;
	EsifString devicePathPtr = NULL;
	EsifString deviceAltPathPtr = NULL;
	EsifString deviceFullPathPtr = NULL;
	EsifString deviceTargetPathPtr = NULL;
	char *pathTok = NULL;

	eEsifError rc = ESIF_OK;
	EsifData params[5] = {0};
	EsifString replacedStrs[5] = {0};
	EsifString replacedStr = NULL;
	UInt8 i = 0;
	u64 sysval = 0;
	u64 tripval = 0;
	int node_idx = 0;
	int node2_idx = 0;
	int max_node_idx = MAX_NODE_IDX;
	enum esif_sysfs_command sysopt = 0;
	int cur_item_count = 0;
	int target_item_count = 0;
	enum esif_sysfs_param calc_type = 0;
	u64 pdl_val = 0;
	u64 pl1max_val = 0;
	u64 cur_power_limit = 0;
	int min_idx = 0;
	int candidate_found = 0;
	char srchnm[MAX_SEARCH_STRING] = { 0 };
	char srchval[MAX_SEARCH_STRING]= { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	char *node_holder = 0;
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char alt_node_name[MAX_SYSFS_PATH]= { 0 };
	char idx_holder[MAX_IDX_HOLDER] = { 0 };
	static struct timeval starttm = { 0 };
	struct timeval endtm = { 0 };
	double elapsed_tm = 0;
	u64 ret_val = 0;
	int domain_idx0 = 0;	// DTS 0
	int domain_idx1 = 0;	// DTS 1
	int temp_val0 = 0;
	int temp_val1 = 0;
	int pathAccessReturn = 0;
	char cur_path[MAX_SYSFS_PATH] = { 0 };
	char pcipath[] = "/sys/bus/pci/devices";
	char pcinode[] = "0000:00:%id%.0";
	char table_str[BINARY_TABLE_SIZE];
	TableObject tableObject = {0};
	struct sysfsActionHashKey key = {0};
	size_t actionContext = 0;
	EsifUpDataPtr metaPtr = NULL;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	rc = EsifFpcAction_GetParams(fpcActionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		ESIF_TRACE_WARN("Failed to get action parameters. Error code: %d .\n",rc);
		goto exit;
	}
	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifUp_CreateTokenReplacedParamString(upPtr, primitivePtr, params[i].buf_ptr);
		if (replacedStr != NULL) {
			params[i].buf_ptr = replacedStr;
			replacedStrs[i] = replacedStr;
		}
	}
	ESIF_ASSERT(NULL != params[0].buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == params[0].type);
	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	if (responsePtr == NULL || responsePtr->buf_ptr == NULL || params[0].buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("Failed to get response buffer.\n");
		goto exit;
	}

	if (responsePtr->buf_len < sizeof(u32)) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		ESIF_TRACE_WARN("Response buffer too small. \n");
		goto exit;
	}

	//verify valid buf length for response

	command = (EsifString) params[0].buf_ptr;
	parm1 = (EsifString) params[1].buf_ptr;
	parm2 = (EsifString) params[2].buf_ptr;
	parm3 = (EsifString) params[3].buf_ptr;
	parm4 = (EsifString) params[4].buf_ptr;

	metaPtr = EsifUp_GetMetadata(upPtr);
	if (NULL == metaPtr) {
		rc = ESIF_E_UNSPECIFIED;
		ESIF_TRACE_WARN("Failed to get metadata.\n");
		goto exit;
	}
	deviceFullPathPtr = (EsifString)metaPtr->fDevicePath;
	if (deviceFullPathPtr != NULL) {
		devicePathPtr = esif_ccb_strtok(deviceFullPathPtr, "|", &pathTok);
		deviceAltPathPtr = esif_ccb_strtok(NULL, "|", &pathTok);
	}

	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);

	// Assemble hash table key to look for existing file pointer to sysfs node
	key.participantId = EsifUp_GetInstance(upPtr);
	key.primitiveTuple = primitivePtr->tuple;
	actionContext = (size_t) esif_ht_get_item(actionHashTablePtr, (u8 *)&key, sizeof(key));

	// actionContext is not a pointer but instead the file descriptor. It cannot possibly be 0 because 0 is reserved for stdout
	// So if we do get 0 it would translate to NULL pointer which means that the key is not found in the hash table
	if (actionContext) {
		if (sysfs_get_u64_direct((int) actionContext, &sysval) > 0) {
			tripval = sysval;
			*(u32 *) responsePtr->buf_ptr = (u32) tripval;
			goto exit;
		} else {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_WARN("Failed to get action context .\n");
			goto exit;
		}
	}

	sysopt = *(enum esif_sysfs_command *) command;

	switch (sysopt) {
	case ESIF_SYSFS_DIRECT_PATH:
		if (0 == esif_ccb_strcmp(parm2, "alt")) {
			deviceTargetPathPtr = deviceAltPathPtr;
		}
		else {
			deviceTargetPathPtr = devicePathPtr;
		}
		pathAccessReturn = sysfs_get_u64(devicePathPtr, parm1, &sysval);
		if (pathAccessReturn < 1) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_WARN("Failed to get value from path: %s/%s . Error: %d \n",devicePathPtr,parm1,pathAccessReturn);
			goto exit;
		}
		if (SetActionContext(&key, deviceTargetPathPtr, parm1)) {
			ESIF_TRACE_WARN("Fail to save context for participant %d, primitive %d, domain %d, instance %d\n",
			key.participantId, key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
		}
		tripval = sysval;
		*(u32 *) responsePtr->buf_ptr = (u32) tripval;
		break;
	case ESIF_SYSFS_DIRECT_ENUM:
		candidate_found = 0;
		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			if (sysfs_get_u64(devicePathPtr, cur_node_name, &sysval) > 0 && sysval > 0) {
				candidate_found = 1;
				if (SetActionContext(&key, devicePathPtr, cur_node_name)) {
					ESIF_TRACE_WARN("Fail to save context for participant %d, primitive %d, domain %d, instance %d\n",
					key.participantId, key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
				}
				*(u32 *) responsePtr->buf_ptr = (u32) sysval;
				break;
			}
		}
		if (candidate_found < 1) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		break;
	case ESIF_SYSFS_ALT_PATH:
		if (sysfs_get_u64(parm1, parm2, &sysval) < 1) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		if (SetActionContext(&key, parm1, parm2)) {
			ESIF_TRACE_WARN("Fail to save context for participant %d, primitive %d, domain %d, instance %d\n",
			key.participantId, key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
		}
		*(u32 *) responsePtr->buf_ptr = (u32) sysval;
		break;
	case ESIF_SYSFS_DIRECT_QUERY:
		min_idx = 0;
		if(parm4) {
			min_idx = esif_atoi(parm4);
		}
		sysval = 0;
		tripval = 0;
		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);

			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			if (replace_str(parm3, "%i%", idx_holder, alt_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			if (sysfs_get_string(devicePathPtr, cur_node_name, sysvalstring) > -1) {
				if (esif_ccb_stricmp(parm2, sysvalstring) == 0) {
					if (sysfs_get_u64(devicePathPtr, alt_node_name, &sysval) > 0 && sysval > 0 && node_idx >= min_idx) {
						if (SetActionContext(&key, devicePathPtr, alt_node_name)) {
							ESIF_TRACE_WARN("Fail to save context for participant %d, primitive %d, domain %d, instance %d\n",
							key.participantId, key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
						}
						tripval = sysval;
						break;
					}
				}
			}
		}
		// If temperature thresholds are not defined (0), return error instead of 0
		// so that DPTF does not shut down or hibernate the system
		if (0 == sysval) {
			*(u32 *) responsePtr->buf_ptr = ERROR_VALUE;
			rc = ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
			goto exit;
		}
		*(u32 *) responsePtr->buf_ptr = (u32) tripval;
		break;
	case ESIF_SYSFS_ALT_QUERY:
		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			candidate_found = ESIF_FALSE;
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}

			// Only need to locate key/value pair once
			if (0 == *srchnm) {
				if (get_key_value_pair_from_str(parm2, srchnm, srchval)) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
			}

			if (sysfs_get_string(cur_node_name, srchnm, sysvalstring) > -1) {
				if (esif_ccb_stricmp(srchval, sysvalstring) == 0) {
					for (node2_idx = 0; node2_idx < max_node_idx; node2_idx++) {
						char *node_name_ptr = parm3;
						esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node2_idx);
						if (replace_str(parm3, "%i%", idx_holder, alt_node_name, MAX_SYSFS_PATH) == 0) {
							// replacement is successful, set node name pointer to alt
							node_name_ptr = alt_node_name;
						}
						if (sysfs_get_u64(cur_node_name, node_name_ptr, &sysval) > 0 && sysval > 0) {
							if (SetActionContext(&key, cur_node_name, node_name_ptr)) {
								ESIF_TRACE_WARN("Fail to save context for participant %d, primitive %d, domain %d, instance %d\n",
								key.participantId, key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
							}
							candidate_found = ESIF_TRUE;
							break;
						}
					}
					if (candidate_found)
						break;
				}
			}
		}
		// If temperature thresholds are not defined (0), return error intead of 0
		// so that DPTF does not shut down or hibernate the system
		if ((!candidate_found) || (0 == sysval)) {
			*(u32 *) responsePtr->buf_ptr = ERROR_VALUE;
			rc = ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
			goto exit;
		}
		*(u32 *) responsePtr->buf_ptr = (u32) sysval;
		break;
	case ESIF_SYSFS_CALC:
		calc_type = *(enum esif_sysfs_param *) parm3;
		switch (calc_type) {
			case ESIF_SYSFS_GET_SOC_RAPL: /* rapl */
				if (domainPtr->lastPowerTime == 0) {
					esif_ccb_get_time(&starttm);
					domainPtr->lastPowerTime = (u64)(starttm.tv_sec * 1000000) + starttm.tv_usec;
				}

				esif_ccb_get_time(&endtm);
				elapsed_tm = (((endtm.tv_sec * 1000000) + endtm.tv_usec) - (domainPtr->lastPowerTime)) / 1000000.0;

				if (sysfs_get_u64(parm1, parm2, &sysval) < 1) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}

				if (elapsed_tm > 0) {
					ret_val = (u64) ((sysval - domainPtr->lastPower) / elapsed_tm);
					domainPtr->lastPowerTime = (u64)(endtm.tv_sec * 1000000) + endtm.tv_usec;
				}
				domainPtr->lastPower = sysval;

				*(u32 *) responsePtr->buf_ptr = (u32) ret_val;
				break;
			case ESIF_SYSFS_GET_CPU_PDL: /* pdl */
				if (sysfs_get_u64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < 1) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				/* Sysfs returns total states - we want the max state, so subtract one */
				if (pdl_val != 0) {
					pdl_val -= 1;
				}
				*(u32 *) responsePtr->buf_ptr = (u32) pdl_val;
				break;
			case ESIF_SYSFS_GET_SOC_PL1: /* power limit */
				/* should return the lessor between PL1 max and current power limit*/

				pl1max_val = GetPowerLimit0MaxUw();
				if (0 == pl1max_val) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}

				if (sysfs_get_u64("/sys/class/powercap/intel-rapl:0", "constraint_0_power_limit_uw", &sysval) < 1) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				cur_power_limit = (sysval < pl1max_val) ? sysval : pl1max_val;

				*(u32 *) responsePtr->buf_ptr = (u32) cur_power_limit;
				break;
			case ESIF_SYSFS_GET_SOC_TEMP: /* Package or Graphics Core GET_TEMPERATURE */
				domain_idx0 = esif_atoi(parm1);
				domain_idx1 = esif_atoi(parm2);
				EsifPrimitiveTuple temp0Tuple = {GET_TEMPERATURE, upPtr->domains[domain_idx0].domain, 255};
				EsifPrimitiveTuple temp1Tuple = {GET_TEMPERATURE, upPtr->domains[domain_idx1].domain, 255};

				if (EsifUp_ExecutePrimitive(upPtr, &temp0Tuple, requestPtr, responsePtr)) {
					temp_val0 = -1;
				} else {
					temp_val0 = *((u32 *)responsePtr->buf_ptr);
				}

				if (EsifUp_ExecutePrimitive(upPtr, &temp1Tuple, requestPtr, responsePtr)) {
					temp_val1 = -1;
				} else {
					temp_val1 = *((u32 *) responsePtr->buf_ptr);
				}

				// Even if there is only one sub-domain that has a valid temperature, we will take it
				if ((temp_val0 >= 0) || (temp_val1 >=0)) {
					*(u32 *) responsePtr->buf_ptr = esif_ccb_max(temp_val0, temp_val1) * 1000;
				} else {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				break;

			case ESIF_SYSFS_GET_FAN_INFO:
				rc = GetFanInfo(responsePtr);
				break;

			case ESIF_SYSFS_GET_FAN_PERF_STATES:
				rc = GetFanPerfStates(responsePtr);
				break;

			case ESIF_SYSFS_GET_FAN_STATUS:
				rc = GetFanStatus(responsePtr);
				break;

			default:
				break;
		}
		break;
	case ESIF_SYSFS_ATTRIBUTE:
		esif_ccb_sprintf(MAX_SYSFS_PATH, (char *) responsePtr->buf_ptr, "%s", devicePathPtr);
		break;
	case ESIF_SYSFS_DIRECT_QUERY_ENUM:
		if (get_key_value_pair_from_str(parm2, srchnm, srchval)) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}

		cur_item_count = 0;
		target_item_count = esif_atoi(srchval);
		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			if (replace_str(parm3, "%i%", idx_holder, alt_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			if (sysfs_get_string(devicePathPtr, cur_node_name, sysvalstring) > -1) {
				if (esif_ccb_stricmp(srchnm, sysvalstring) == 0) {
					if (cur_item_count == target_item_count) {
						if (sysfs_get_u64(devicePathPtr, alt_node_name, &sysval) < 1) {
							rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
							goto exit;
						}
						if (SetActionContext(&key, devicePathPtr, alt_node_name)) {
							ESIF_TRACE_WARN("Fail to save context for participant %d, primitive %d, domain %d, instance %d\n",
							key.participantId, key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
						}
						break;
					}
					cur_item_count++;
				}
			}
		}
		*(u32 *) responsePtr->buf_ptr = (u32) sysval;
		break;
	case ESIF_SYSFS_BINARY_TABLE:
		//required
		if (!parm1) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		esif_ccb_memset(table_str, 0, BINARY_TABLE_SIZE);
		/*	domain str and participant id are not relevant in this
			case since we use device paths */
		TableObject_Construct(&tableObject, parm1, "D0", NULL, NULL, NULL, 0, GET);
		rc = TableObject_LoadSchema(&tableObject);
		if (rc != ESIF_OK) {
			TableObject_Destroy(&tableObject);
			goto exit;
		}

		if (esif_ccb_stricmp("art", parm1) == 0) {
			rc = get_thermal_rel_str(ART, table_str);
		}
		else if (esif_ccb_stricmp("trt", parm1) == 0) {
			rc = get_thermal_rel_str(TRT, table_str);
		}
		else if (esif_ccb_stricmp("idsp", parm1) == 0) {
			int lineNum = sysfs_get_string_multiline("/sys/devices/platform/INT3400:00/uuids/", "available_uuids", sys_long_string_val);
			int i;
			FLAGS_CLEAR(tableObject.options, TABLEOPT_ALLOW_SELF_DEFINE);
			for (i = 0; i < lineNum; i++) {
				tableObject.fields[i].dataType = ESIF_DATA_BINARY;
			}
			rc = get_supported_policies(table_str, lineNum, sys_long_string_val);
		}
		else if (esif_ccb_stricmp("ppcc", parm1) == 0) {
			rc = get_rapl_power_control_capabilities(table_str,(esif_guid_t *)metaPtr->fDriverType);
		}
		else if (esif_ccb_stricmp("pccc", parm1) == 0) {
			rc = get_participant_current_control_capabilities(table_str, devicePathPtr);
		}
		else if (esif_ccb_stricmp("pss", parm1) == 0) {
			rc = get_proc_perf_support_states(table_str);
		}
		else if (esif_ccb_stricmp("ppss", parm1) == 0) {
			rc = get_perf_support_states(table_str, devicePathPtr);
		}
		else if (esif_ccb_stricmp("fsl", parm1) == 0) {
			/* to come
			rc = get_thermal_rel_str(TRT, table_str);
			*/
		}
		else if (esif_ccb_stricmp("fst", parm1) == 0) {
			/* to come
			rc = get_thermal_rel_str(TRT, table_str);
			*/
		}
		else if (esif_ccb_stricmp("fps", parm1) == 0) {
			/* to come
			rc = get_thermal_rel_str(TRT, table_str);
			*/
		}
		else {
			TableObject_Destroy(&tableObject);
			rc = ESIF_E_PARAMETER_IS_NULL;
		}

		if (ESIF_OK != rc) {
			TableObject_Destroy(&tableObject);
			goto exit;
		}

		tableObject.dataText = esif_ccb_strdup(table_str);
		rc = TableObject_Convert(&tableObject);
		if (ESIF_OK != rc) {
			TableObject_Destroy(&tableObject);
			goto exit;
		}
		esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, tableObject.binaryData, tableObject.binaryDataSize);
		responsePtr->type = ESIF_DATA_BINARY;
		responsePtr->data_len = tableObject.binaryDataSize;

		//esif_ccb_free(table_str);
		TableObject_Destroy(&tableObject);
		break;
	default:
		rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
	}

exit:
	return rc;
}


/*
 * Handle ESIF Action "Set" Request
 */
static eEsifError ESIF_CALLCONV ActionSysfsSet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr
	)
{
	EsifString command = NULL;
	EsifString parm1 = NULL;
	EsifString parm2 = NULL;
	EsifString parm3 = NULL;
	EsifString parm4 = NULL;
	EsifString devicePathPtr = NULL;
	EsifString deviceAltPathPtr = NULL;
	EsifString deviceFullPathPtr = NULL;
	EsifString deviceTargetPathPtr = NULL;
	char *pathTok = NULL;
	eEsifError rc = ESIF_OK;
	EsifData params[5] = {0};
	EsifString replacedStrs[5] = {0};
	EsifString replacedStr = NULL;
	UInt8 i = 0;
	enum esif_sysfs_command sysopt = 0;

	char srchnm[MAX_SEARCH_STRING] = { 0 };
	char srchval[MAX_SEARCH_STRING] = { 0 };
	char *srch = 0;
	int token_counter = 0;
	enum esif_sysfs_param calc_type = 0;
	char sys_long_string_val[MAX_SYSFS_STRING];
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char alt_node_name[MAX_SYSFS_PATH] = { 0 };
	char idx_holder[MAX_IDX_HOLDER] = { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	int node_idx = 0;
	int node2_idx = 0;
	int candidate_found = 0;
	int max_node_idx = MAX_NODE_IDX;
	double target_perc = 0.0;
	u64 turbo_perc = 0;
	u64 sysval = 0;
	u64 pdl_val = 0;
	char *next_tok = 0;
	EsifUpDataPtr metaPtr = NULL;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(responsePtr);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);

	rc = EsifFpcAction_GetParams(fpcActionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto exit;
	}

	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifUp_CreateTokenReplacedParamString(upPtr, primitivePtr, params[i].buf_ptr);
		if (replacedStr != NULL) {
			params[i].buf_ptr = replacedStr;
			replacedStrs[i] = replacedStr;
		}
	}
	if (requestPtr == NULL || requestPtr->buf_ptr == NULL || params[0].buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (requestPtr->buf_len < sizeof(u32)) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	command = (EsifString) params[0].buf_ptr;
	parm1 = (EsifString) params[1].buf_ptr;
	parm2 = (EsifString) params[2].buf_ptr;
	parm3 = (EsifString) params[3].buf_ptr;
	parm4 = (EsifString) params[4].buf_ptr;

	metaPtr = EsifUp_GetMetadata(upPtr);
	if (NULL == metaPtr) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}
	deviceFullPathPtr = (EsifString)metaPtr->fDevicePath;
	if (deviceFullPathPtr != NULL) {
		devicePathPtr = esif_ccb_strtok(deviceFullPathPtr, "|", &pathTok);
		deviceAltPathPtr = esif_ccb_strtok(NULL, "|", &pathTok);
	}

	sysopt = *(enum esif_sysfs_command *) command;
	switch (sysopt) {
	case ESIF_SYSFS_DIRECT_PATH:
		if (0 == esif_ccb_strcmp(parm2, "alt")) {
			deviceTargetPathPtr = deviceAltPathPtr;
		}
		else {
			deviceTargetPathPtr = devicePathPtr;
		}
		if (sysfs_set_u64(deviceTargetPathPtr, parm1, *(u32 *) requestPtr->buf_ptr) < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		break;
	case ESIF_SYSFS_ALT_PATH:
		sysval = (u64)*(u32 *)requestPtr->buf_ptr;
		if (0 == esif_ccb_strcmp(parm2, "constraint_0_power_limit_uw")) {
			// Primitve SET_RAPL_POWER_LIMIT, need to check PL1 upper limit before set
			// Note that in Linux drivers the name "power limit 0" is used to denote PL1
			u64 powerLimit0Max = GetPowerLimit0MaxUw();
			if (0 == powerLimit0Max) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			sysval = (sysval < powerLimit0Max) ? sysval : powerLimit0Max;
		}

		if (sysfs_set_u64(parm1, parm2, sysval) < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		break;
	case ESIF_SYSFS_ALT_QUERY:
		candidate_found = 0;
		if (get_key_value_pair_from_str(parm2, srchnm, srchval)) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}

		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			if (sysfs_get_string(cur_node_name, srchnm, sysvalstring) > -1) {
				if (esif_ccb_stricmp(srchval, sysvalstring) == 0) {
					if (sysfs_set_u64(cur_node_name, parm3, *(u32 *) requestPtr->buf_ptr) == 0) {
						candidate_found = 1;
						break;
					}
				}
			}
		}
		if (candidate_found < 1) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		break;
	case ESIF_SYSFS_CALC:
		calc_type = *(enum esif_sysfs_param *) parm3;
		switch(calc_type) {
			case ESIF_SYSFS_SET_CPU_PSTATE:  /* pstate to perc */
				if (sysfs_get_u64(SYSFS_PSTATE_PATH, "num_pstates", &pdl_val) < 1) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				target_perc = ((1.00 / (double) pdl_val) * ((u32) pdl_val - *(u32 *) requestPtr->buf_ptr)) * 100.0;
				if (target_perc > 100) {
					target_perc = 100;
				}
				else if (target_perc < MIN_PERF_PERCENTAGE) {
					target_perc = MIN_PERF_PERCENTAGE;
				}
				if (sysfs_set_u64(parm1, parm2, (int) target_perc) < 0) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				// Logic to turn on or off turbo
				if (sysfs_get_u64(SYSFS_PSTATE_PATH, "turbo_pct", &turbo_perc) > 0) {
					// Sys FS node exposes turbo frequency range, we need to convert it to turbo target
					turbo_perc = 100 - turbo_perc;
					if (target_perc < turbo_perc) {
						if (sysfs_set_u64(SYSFS_PSTATE_PATH, "no_turbo",1) < 0) {
							rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
							goto exit;
						}
					}
					else {
						if (sysfs_set_u64(SYSFS_PSTATE_PATH, "no_turbo",0) < 0) {
							rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
							goto exit;
						}
					}
				}
				break;
			case ESIF_SYSFS_SET_OSC:  /* osc */
				// Check validity of _OSC request
				if (ESIF_DATA_STRUCTURE != requestPtr->type ||
					sizeof(struct esif_data_complex_osc) != requestPtr->buf_len) {
					rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
					goto exit;
				}
				struct esif_data_complex_osc *oscPtr = (struct esif_data_complex_osc *)requestPtr->buf_ptr;
				oscPtr->status = 0x2;	// Pre-initialize the return byte to _OSC failure

				for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
					esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name, "/sys/class/thermal/thermal_zone%d", node_idx);
					if (sysfs_get_string(cur_node_name, "type", sysvalstring) > -1) {
						if (esif_ccb_stricmp("INT3400", sysvalstring) == 0) {
							if (oscPtr->capabilities) {	// DPTF to take over thermal control
								if (sysfs_set_string(cur_node_name, "mode", "disabled") < 0) {
									rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
									goto exit;
								}
							} else {
								if (sysfs_set_string(cur_node_name, "mode", "enabled") < 0) {
									rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
									goto exit;
								}
							}
							// Need to output an IDSP GUID to current_uuid sysfs node. It does not matter what what GUID is as long as there is one.
							if (sysfs_get_string("/sys/devices/platform/INT3400:00/uuids/", "available_uuids", sys_long_string_val) < 0) {
								rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
								goto exit;
							}
							if (sysfs_set_string("/sys/devices/platform/INT3400:00/uuids/", "current_uuid", sys_long_string_val) < 0) {
								rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
								goto exit;
							}

						}
					}
					if (sysfs_get_string(cur_node_name, "policy", sysvalstring) > -1) {
						if (sysfs_set_string(cur_node_name, "policy", "user_space") < 0) {
							rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
							goto exit;
						}
					}
				}
				// Set success return status byte
				oscPtr->status = 0;
				break;

			case ESIF_SYSFS_SET_FAN_LEVEL:
				rc = SetFanLevel(upPtr, requestPtr, devicePathPtr);
				break;

			default:
				break;
		}
		break;
	default:
		rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
		break;
	}

exit:
	return rc;
}

static void replace_cpu_id(char *str)
{
	char cpu_path[MAX_SYSFS_PATH] = { 0 };
	char *cpu_target_loc = "/sys/devices/pci0000:00/0000:00:04.0/firmware_node";
	char *cpu_target_node = "path";
	sysfs_get_string(cpu_target_loc, cpu_target_node, cpu_path);
	if (esif_ccb_stricmp(str, "B0D4") == 0 ||    // Haswell & BroadwelL
		esif_ccb_stricmp(str, "B0DB") == 0) {    // Cherry TraiL
		esif_ccb_strcpy(str, "TCPU", 5);
	}
}

static u64 GetPowerLimit0MaxUw(void)
{
	char fullPath[MAX_SYSFS_PATH] = { 0 };
	u64 sysVal = 0;

	// Try Atom PCI first
	esif_ccb_sprintf(sizeof(fullPath), fullPath, "%s/%s/power_limits", SYSFS_PCI, ATOM);
	if (sysfs_get_u64(fullPath, "power_limit_0_max_uw", &sysVal) > 0)
		goto exit;

	// Next try Core PCI
	esif_ccb_sprintf(sizeof(fullPath), fullPath, "%s/%s/power_limits", SYSFS_PCI, CORE);
	if (sysfs_get_u64(fullPath, "power_limit_0_max_uw", &sysVal) > 0)
		goto exit;

	// Finally try Atom ACPI
	esif_ccb_sprintf(sizeof(fullPath), fullPath, "%s/%s/power_limits", SYSFS_PLATFORM, ACPI_CPU);
	if (sysfs_get_u64(fullPath, "power_limit_0_max_uw", &sysVal) > 0)
		goto exit;

	sysVal = 0;	// 0 indicates "not found"
exit:
	return sysVal;
}

// acpi_thermal_rel driver only returns the leaf node strings,
// DPTF expects full scope, so  prepend source/target stringS
// with \_UP.CNJR scope namE
static void get_full_scope_str(char *orig, char *new)
{
	esif_ccb_strcpy(new, "\\_UP.CNJR.", MAX_ACPI_SCOPE_LEN);
	esif_ccb_strcat(new, orig, MAX_ACPI_SCOPE_LEN); // length is total size of buffeR
}


static int SetActionContext(struct sysfsActionHashKey *keyPtr, EsifString devicePathName, EsifString deviceNodeName)
{
	int fd = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	int ret = 0;

	ESIF_TRACE_ENTRY();
	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", devicePathName, deviceNodeName);
	fd = open(filepath, O_RDONLY);

	if (fd != -1) {
		size_t actionContext = (size_t) fd;
		ret = esif_ht_add_item(actionHashTablePtr, (u8 *) keyPtr, sizeof(struct sysfsActionHashKey), (void *) actionContext);
	}

	return ret;
}

static int replace_str(char *str, char *orig, char *new, char *rpl_buff, int rpl_buff_len)
{
	int rc = 0;
	char *p = NULL;

	/* return if target string doesn't exist in haystack */
	p = esif_ccb_strstr(str, orig);
	if (p == NULL) {
		rc = 1;
		goto exit;
	}

	/*create new version of the original string in the buffer that contains everything before the target str */
	esif_ccb_strncpy(rpl_buff, str, (p - str) + 1);
	rpl_buff[(p - str) + 1] = '\0';

	/* add the replace str, then continue with the original buffer val */
	esif_ccb_sprintf_concat(rpl_buff_len, rpl_buff, "%s%s", new, p + strlen(orig));

exit:
	return rc;

}

static int get_key_value_pair_from_str(const char *str, char *key, char *value)
{
	int rc = 0;
	char *pch = 0;
	char *next_pch = 0;
	char param_copy[MAX_PARAM_STRING];

	// esif_ccb_strtok() is destructive to parameters (passed in as first argument)
	// Make a copy of str before proceeding
	esif_ccb_strcpy(param_copy, str, MAX_PARAM_STRING);

	pch = esif_ccb_strtok(param_copy, "=", &next_pch);
	if (pch)
		esif_ccb_strcpy(key, pch, MAX_SEARCH_STRING);
	else {
		rc = 1;
		goto exit;
	}

	pch = esif_ccb_strtok(NULL, "=", &next_pch);
	if (pch)
		esif_ccb_strcpy(value, pch, MAX_SEARCH_STRING);
	else
		rc = 1;

exit:
	return rc;
}

static int sysfs_get_string(char *path, char *filename, char *str)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fd, filepath, "r") != 0) {
		goto exit;
	}

	rc = esif_ccb_fscanf(fd, "%s", SCANFBUF(str, MAX_SYSFS_PATH));
	esif_ccb_fclose(fd);

exit:
	return rc;
}

static int sysfs_get_string_multiline(char *path, char *filename, char *str)
{
	FILE *fp = NULL;
	int rc = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	size_t fileSize = 0;
	char lineStr[MAX_GUID_STR_LEN + 1] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fp, filepath, "r") != 0) {
		goto exit;
	}

	*str = 0; // Initialize first character to ensure that concat works properly
	while (esif_ccb_fgets(lineStr, MAX_GUID_STR_LEN, fp)) {
		esif_ccb_sprintf_concat(MAX_SYSFS_STRING, str, "%s\n", lineStr);
		rc++;
	}

	esif_ccb_fclose(fp);

exit:
	return rc;
}


static int sysfs_get_u64(char *path, char *filename, u64 *p_u64)
{
	FILE *fd = NULL;
	int rc = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fd, filepath, "r") != 0) {
		goto exit;
	}
	rc = esif_ccb_fscanf(fd, "%llu", (u64 *)p_u64);
	esif_ccb_fclose(fd);

exit:
	return rc;
}

static int sysfs_get_u64_direct(int fd, u64 *p_u64)
{
	int rc = 0;
	int dummy = 0;
	char buf[MAX_SYSFS_POLL_STRING] = {0};

	lseek(fd, 0, SEEK_SET);
	if (read(fd, buf, MAX_SYSFS_POLL_STRING) > 0) {
		rc = esif_ccb_sscanf(buf, "%llu", p_u64);
	}

	return rc;
}

static int sysfs_set_u64(char *path, char *filename, u64 val)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fd, filepath, "w") !=0) {
		goto exit;
	}

	esif_ccb_fprintf(fd, "%llu", (u64)val);
	esif_ccb_fclose(fd);
	rc = 0;

exit:
	return rc;
}

static int sysfs_set_string(char *path, char *filename, char *val)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fd, filepath, "w") !=0) {
		goto exit;
	}

	esif_ccb_fprintf(fd, "%s", val);
	esif_ccb_fclose(fd);
	rc =0;

exit:
	return rc;
}

static enum esif_rc get_supported_policies(char *table_str, int idspNum, char *sysfs_str)
{
	eEsifError rc = ESIF_OK;
	char *scanPtr = sysfs_str;
	char guidStr[MAX_GUID_STR_LEN] = {0};
	int i = 0;

	for (i = 0; i < idspNum; i++) {
		esif_ccb_sscanf(scanPtr, "%s", SCANFBUF(guidStr, sizeof(guidStr)));
		esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "16,%s!", guidStr);
		while (*(scanPtr++) != '\n');
		while (*scanPtr == '\n') scanPtr++; // Move to the beginning of the next string
	}

	return rc;
}

static enum esif_rc get_participant_current_control_capabilities(char *table_str, char *participant_path)
{
	/* need to implement (not used currently) */
	return ESIF_E_PRIMITIVE_ACTION_FAILURE;
}

static enum esif_rc get_perf_support_states(char *table_str, char *participant_path)
{
	u64 pdl_val = 0;
	u64 placeholder_val =0;
	int pcounter = 0;
	eEsifError rc = ESIF_OK;

	if (sysfs_get_u64(participant_path, "max_state", &pdl_val) < 1) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	esif_ccb_sprintf(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu,mA,%llu!",pcounter,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
	for (pcounter = 1;pcounter <= pdl_val;pcounter++) {
		esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu,mA,%llu!",pcounter,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
	}

exit:
	return rc;
}

static enum esif_rc get_proc_perf_support_states(char *table_str)
{
	u64 pdl_val = 0;
	u64 placeholder_val =0;
	int pcounter = 0;
	eEsifError rc = ESIF_OK;

	if (sysfs_get_u64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < 1) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	/* Sysfs returns total states - we want the max state, so subtract one */
	if (pdl_val != 0) {
		pdl_val -= 1;
	}

	esif_ccb_sprintf(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu!",pcounter,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
	for (pcounter = 1;pcounter <= pdl_val;pcounter++) {
		esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu!",pcounter,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
	}

exit:
	return rc;
}

static enum esif_rc get_rapl_power_control_capabilities(
	char *table_str,
	esif_guid_t *target_guid)
{
	eEsifError rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
	DIR *dir;
	struct dirent **namelist;
	int n;
	u64 pl1Min = 0;
	u64 pl1Max = 0;
	u64 time1Min = 0;
	u64 time1Max = 0;
	u64 step1 = 0;
	u64 pl2Min = 0;
	u64 pl2Max = 0;
	u64 time2Min = 0;
	u64 time2Max = 0;
	u64 step2 = 0;
	u64 sysval = 0;
	u8 guid_compare[ESIF_GUID_LEN] = ESIF_PARTICIPANT_PLAT_CLASS_GUID;
	int candidate_found = 0;
	int guid_different = 0;
	int is_platform = 0;
	int guid_element_counter = 0;
	char cur_path[MAX_SYSFS_PATH] = { 0 };
	char revision[] = "2";
	char path[MAX_SYSFS_PATH] = { 0 };
	char node[MAX_SYSFS_PATH] = { 0 };

	ESIF_ASSERT(NULL != target_guid);

	/* determine which bus the cpu is on */
	guid_different = 0;
	for (guid_element_counter = 0; guid_element_counter < ESIF_GUID_LEN; guid_element_counter++) {
		if (*((u8 *)target_guid + guid_element_counter) != guid_compare[guid_element_counter]) {
			guid_different = 1;
			break;
		}
	}

	if (guid_different == 1) {
		esif_ccb_sprintf(MAX_SYSFS_PATH, path, "/sys/bus/%s/devices",  "pci");
	}
	else {
		esif_ccb_sprintf(MAX_SYSFS_PATH, path, "/sys/bus/%s/devices",  "platform");
	}

	/****************************/
	/* find path */
	dir = opendir(path);
	if (!dir) {
		goto exit;
	}
	n = scandir(path, &namelist, 0, alphasort);
	if (n < 0) {
		//no scan
	}
	else {
		while (n--) {
			if (esif_ccb_strstr(namelist[n]->d_name, CORE) != NULL) {
				esif_ccb_sprintf(MAX_SYSFS_PATH, node, "%s", CORE);
				candidate_found = 1;
			}
			else if (esif_ccb_strstr(namelist[n]->d_name, ATOM) != NULL) {
				esif_ccb_sprintf(MAX_SYSFS_PATH, node, "%s", ATOM);
				candidate_found = 1;
			}
			else if (esif_ccb_strstr(namelist[n]->d_name, ACPI_CPU) != NULL) {
				esif_ccb_sprintf(MAX_SYSFS_PATH, node, "%s", ACPI_CPU);
				candidate_found = 1;
			}
			free(namelist[n]);
		}
		free(namelist);
	}
	closedir(dir);

	if (candidate_found == 0) {
		goto exit;
	}
	/****************************/


	esif_ccb_sprintf(MAX_SYSFS_PATH, cur_path, "%s/%s/power_limits", path, node);
	if (sysfs_get_u64(cur_path, "power_limit_0_min_uw", &sysval) < 1) {
		goto exit;
	}
	pl1Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_max_uw",  &sysval) < 1) {
		goto exit;
	}
	pl1Max = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_step_uw", &sysval) < 1) {
		goto exit;
	}
	step1 = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_tmin_us", &sysval) < 1) {
		goto exit;
	}
	time1Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_tmax_us", &sysval) < 1) {
		goto exit;
	}
	time1Max = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_min_uw",  &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_max_uw",  &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Max = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_step_uw", &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	step2 = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_tmin_us", &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	time2Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_tmax_us", &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	time2Max = sysval;
	pl1Min = (pl1Min > 0) ? pl1Min / 1000 : 0;
	pl1Max = (pl1Max > 0) ? pl1Max / 1000 : 0;
	time1Min = (time1Min > 0) ? time1Min / 100 : 0;
	time1Max = (time1Max > 0) ? time1Max / 100 : 0;
	step1 = (step1 > 0) ? step1 / 1000 : 0;
	pl2Min = (pl2Min > 0) ? pl2Min / 1000 : 0;
	pl2Max = (pl2Max > 0) ? pl2Max / 1000 : 0;
	time2Min = (time2Min > 0) ? time2Min / 100 : 0;
	time2Max = (time2Max > 0) ? time2Max / 100 : 0;
	step2 = (step2 > 0) ? step2 / 1000 : 0;
	esif_ccb_sprintf(BINARY_TABLE_SIZE, table_str, "%s,0,%llu,%llu,%llu,%llu,%llu,1,%llu,%llu,%llu,%llu,%llu",revision,pl1Min,pl1Max,time1Min,time1Max,step1,pl2Min,pl2Max,time2Min,time2Max,step2);
	rc = ESIF_OK;

exit:
	return rc;
}

static enum esif_rc get_thermal_rel_str(enum esif_thermal_rel_type type, char *table_str)
{
	int file = 0;
	u64 count = 0;
	u64 len = 0;
	long acpi_rc = 0;
	unsigned char *table = NULL;
	struct trt_table *trt_entry = NULL;
	struct art_table *art_entry = NULL;
	enum esif_rc rc = ESIF_OK;

	file = open("/dev/acpi_thermal_rel", O_RDONLY);
	if (file < 0) {
		rc = ESIF_E_IO_OPEN_FAILED;
		goto exit;
	}

	if (ART == type)
		acpi_rc = ioctl(file, ART_COUNT, &count);
	else
		acpi_rc = ioctl(file, TRT_COUNT, &count);

	if (acpi_rc) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	if (ART == type)
		acpi_rc = ioctl(file, ART_LEN, &len);
	else
		acpi_rc = ioctl(file, TRT_LEN, &len);

	if (len < 1) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	if (acpi_rc) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	table = (unsigned char *) malloc(len);
	if (!table) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	if (ART == type)
		acpi_rc = ioctl(file, GET_ART, table);
	else
		acpi_rc = ioctl(file, GET_TRT, table);

	if (acpi_rc) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}

	/* Parse the table and dump characters in DPTF format */
	if (ART == type)
		art_entry = (struct art_table *) table;
	else
		trt_entry = (struct trt_table *) table;

	len = 0;
	if (ART == type) {
		// Special case for ART - need to prepend revision number 0 to table
		len += snprintf(table_str + len, 4, "00:");
	}

	while (count) {
		char full_acpi_scope[MAX_ACPI_SCOPE_LEN] = "";

		if (ART == type) {
			// If target device is CPU, make sure that it is named TCPU (TCPU cannot be source in ART)
			//replace_cpu_id(art_entry->target_device);

			// Format ART strings
			get_participant_scope(art_entry->art_source_device, full_acpi_scope);
			len += snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += snprintf(table_str + len, 2, ",");

			get_participant_scope(art_entry->art_target_device, full_acpi_scope);
			len += snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += snprintf(table_str + len, 2, ",");

			len += snprintf(table_str + len, 8, "%llu,", art_entry->art_weight);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac0_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac1_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac2_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac3_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac4_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac5_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac6_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac7_max_level);
			len += snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac8_max_level);
			len += snprintf(table_str + len, 8, "%lld", (long long) art_entry->art_ac9_max_level);
		} else {
			// If device is CPU, make sure that it is named TCPU
			//replace_cpu_id(trt_entry->source_device);
			//replace_cpu_id(trt_entry->target_device);

			// Format TRT strings
			get_participant_scope(trt_entry->trt_source_device, full_acpi_scope);
			len += snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += snprintf(table_str + len, 2, ",");

			get_participant_scope(trt_entry->trt_target_device, full_acpi_scope);
			len += snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += snprintf(table_str + len, 2, ",");

			len += snprintf(table_str + len, 8, "%llu,", trt_entry->trt_influence);
			len += snprintf(table_str + len, 8, "%llu,", trt_entry->trt_sample_period);
			len += snprintf(table_str + len, 8, "%llu,", trt_entry->trt_reserved[0]);
			len += snprintf(table_str + len, 8, "%llu,", trt_entry->trt_reserved[1]);
			len += snprintf(table_str + len, 8, "%llu,", trt_entry->trt_reserved[2]);
			len += snprintf(table_str + len, 8, "%llu", trt_entry->trt_reserved[3]);
		}

		count--;
		if (count) {
			len += snprintf(table_str + len, 2, "!");
			if (ART == type)
				art_entry++;
			else
				trt_entry++;
		}
	}

exit:
	if (table)
		free(table);
	close(file);

	return rc;
}

static eEsifError get_participant_scope(char *acpi_name, char *acpi_scope)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr target_participant = NULL;
	EsifUpDataPtr metaPtr = NULL;

	if (NULL == acpi_name) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}
	target_participant = EsifUpPm_GetAvailableParticipantByName(acpi_name);
	if (NULL == target_participant) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	metaPtr = EsifUp_GetMetadata(target_participant);
	if (NULL == metaPtr) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	if (NULL == metaPtr->fAcpiScope) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}
	if (esif_ccb_strlen(metaPtr->fAcpiScope,MAX_ACPI_SCOPE_LEN) < 1) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	esif_ccb_sprintf(MAX_ACPI_SCOPE_LEN, acpi_scope, "%s", metaPtr->fAcpiScope);

exit:
	if (target_participant != NULL) {
		EsifUp_PutRef(target_participant);
	}

	if (rc != ESIF_OK) {
		esif_ccb_sprintf(MAX_ACPI_SCOPE_LEN, acpi_scope, "%s", acpi_name);
	}
	return rc;
}

static void ActionContextCleanUp(void *itemPtr)
{
	ESIF_TRACE_ENTRY();

	size_t actionContext = (size_t) itemPtr;
	int fd = (int) actionContext;
	if (fd) close(fd);
}

static eEsifError SetFanLevel(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr)
{
	eEsifError rc = ESIF_OK;
	u64 pdlVal = 0;
	u64 curVal = 0;
	double target_perc = 0;

	if (sysfs_get_u64(devicePathPtr, "max_state", &pdlVal) < 1) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail get participant %d's fan max_state\n", upPtr->fInstance);
		goto exit;
	}

	target_perc = ((double) *(u32 *) requestPtr->buf_ptr) / 100.0;
	curVal = (target_perc + EPSILON_FAN_PERC) * pdlVal; // Avoid rounding errors
	if (sysfs_set_u64(devicePathPtr, "cur_state", curVal) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail set participant %d's fan cur_state\n", upPtr->fInstance);
		goto exit;
	}

	ESIF_TRACE_DEBUG("Set upper particiapnt %d's fan speed to %llu\n", upPtr->fInstance, curVal);

exit:
	return rc;
}

static eEsifError GetFanInfo(EsifDataPtr responsePtr)
{
	// Since the ACPI object _FIF is not exposed to user space, just fake
	// the data. It's mostly a filler since DPTF only cares one field (see below)
	eEsifError rc = ESIF_OK;
	struct EsifDataBinaryFifPackage fif = {0};

	responsePtr->type = ESIF_DATA_BINARY;
	responsePtr->data_len = sizeof(fif);
	if (responsePtr->buf_len < sizeof(fif)) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		ESIF_TRACE_WARN("Need larger buffer to return _FIF data to DPTF\n");
		goto exit;
	}

	fif.revision.integer.type = ESIF_DATA_UINT64;
	fif.hasFineGrainControl.integer.type = ESIF_DATA_UINT64;
	fif.stepSize.integer.type = ESIF_DATA_UINT64;
	fif.supportsLowSpeedNotification.integer.type = ESIF_DATA_UINT64;
	fif.hasFineGrainControl.integer.value = ESIF_TRUE; // Only thing cared by DPTF, set it to true
	esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, &fif, sizeof(fif));

exit:
	return rc;
}

static eEsifError GetFanPerfStates(EsifDataPtr responsePtr)
{
	// Since the ACPI object _FPS is not exposed to user space, just fake
	// the data. It's just a filler and all fields are ignored by DPTF.
	// However, its presence is required otherwise DPTF will abort fan control
	eEsifError rc = ESIF_OK;
	union esif_data_variant revision = {0};
	struct EsifDataBinaryFpsPackage fps = {0};
	int size = 0;
	int totalSize = 0;
	int i = 0;

	totalSize = sizeof(revision);
	totalSize += MAX_ACX_ENTRIES * sizeof(fps);
	responsePtr->type = ESIF_DATA_BINARY;
	responsePtr->data_len = totalSize;
	if (responsePtr->buf_len < totalSize) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		ESIF_TRACE_WARN("Need larger buffer to return _FPS data to DPTF\n");
		goto exit;
	}

	revision.integer.type = ESIF_DATA_UINT64;
	fps.control.integer.type = ESIF_DATA_UINT64;
	fps.tripPoint.integer.type = ESIF_DATA_UINT64;
	fps.speed.integer.type = ESIF_DATA_UINT64;
	fps.noiseLevel.integer.type = ESIF_DATA_UINT64;
	fps.power.integer.type = ESIF_DATA_UINT64;

	// First copy revision, then multiple fps "packages"
	esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, &revision, sizeof(revision));
	size += sizeof(revision);
	for (i = 0; i < MAX_ACX_ENTRIES; i++) {
		esif_ccb_memcpy((u8 *) responsePtr->buf_ptr + size, &fps, sizeof(fps));
		size += sizeof(fps);
	}

exit:
	return rc;
}

static eEsifError GetFanStatus(EsifDataPtr responsePtr)
{
	// Since the ACPI object _FST is not exposed to user space, just fake
	// the data. It's just a filler and all fields are ignored by DPTF.
	// However, its presence is required otherwise DPTF will abort fan control
	eEsifError rc = ESIF_OK;
	struct EsifDataBinaryFstPackage fst = {0};

	responsePtr->type = ESIF_DATA_BINARY;
	responsePtr->data_len = sizeof(fst);
	if (responsePtr->buf_len < sizeof(fst)) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		ESIF_TRACE_WARN("Need larger buffer to return _FST data to DPTF\n");
		goto exit;
	}

	fst.revision.integer.type = ESIF_DATA_UINT64;
	fst.control.integer.type = ESIF_DATA_UINT64;
	fst.speed.integer.type = ESIF_DATA_UINT64;
	esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, &fst, sizeof(fst));

exit:
	return rc;
}

/*
 *******************************************************************************
 * Register ACTION with ESIF
 *******************************************************************************
 */

static EsifActIfaceStatic g_sysfs = {
	eIfaceTypeAction,
	ESIF_ACT_IFACE_VER_STATIC,
	sizeof(g_sysfs),
	ESIF_ACTION_SYSFS,
	ESIF_ACTION_FLAGS_DEFAULT,
	"SYSFS",
	"Sysfs Implementation",
	ESIF_ACTION_VERSION_DEFAULT,
	NULL,
	NULL,
	ActionSysfsGet,
	ActionSysfsSet
};

enum esif_rc EsifActSysfsInit()
{
	EsifActMgr_RegisterAction((EsifActIfacePtr)&g_sysfs);
	actionHashTablePtr = esif_ht_create(MAX_ACTION_HT_SIZE);
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActSysfsExit()
{
	EsifActMgr_UnregisterAction((EsifActIfacePtr)&g_sysfs);
	if (actionHashTablePtr)
		esif_ht_destroy(actionHashTablePtr, ActionContextCleanUp);
	ESIF_TRACE_EXIT_INFO();
}

#endif
/* end compile out entire page without ESIF_FEAT_OPT_SYSFS */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
