/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "esif_uf.h"
#include "esif_uf_ccb_system.h"
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
#define MAX_SYSFS_STRING (4 * 1024)
#define MAX_SYSFS_PATH 256
#define ERROR_VALUE 255
#define MAX_ESIF_TABLES 2
#define BINARY_TABLE_SIZE OUT_BUF_LEN
#define MAX_GUID_STR_LEN 40 // Assuming XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format
#define MAX_STR_LINE_LEN 64
#define MAX_FMT_STR_LEN 15 // "%<Int32>s"
#define MAX_ACTION_HT_SIZE 30
#define MAX_SYSFS_POLL_STRING 50
#define MAX_ACX_ENTRIES 10
#define MAX_SYSFS_PSTATES 0x7FFFFFFFFFFFFFFE
#define MAX_SYSFS_PERF_STATES 0x7FFFFFFFFFFFFFFE
#define MIN_HYSTERESIS_MILLIC 1000
#define MAX_HYSTERESIS_MILLIC 10000
#define EPSILON_CONVERT_PERC 0.00001 // For rounding out errors in floating point calculations
#define INVALID_64BIT_UINTEGER 0xFFFFFFFFFFFFFFFFU

#define MAX_ACPI_SCOPE_LEN ESIF_SCOPE_LEN
#define ACPI_THERMAL_IOR_TYPE 's'
#define TRT_LEN _IOR(ACPI_THERMAL_IOR_TYPE, 1, u64)
#define ART_LEN _IOR(ACPI_THERMAL_IOR_TYPE, 2, u64)
#define TRT_COUNT _IOR(ACPI_THERMAL_IOR_TYPE, 3, u64)
#define ART_COUNT _IOR(ACPI_THERMAL_IOR_TYPE, 4, u64)
#define GET_TRT	_IOR(ACPI_THERMAL_IOR_TYPE, 5, u64)
#define GET_ART	_IOR(ACPI_THERMAL_IOR_TYPE, 6, u64)

#define MIN_INT64	((Int64) 0x8000000000000000)
#define MAX_INT64	((Int64) 0x7FFFFFFFFFFFFFFF)

#define ACPI_DPTF		"INT3400:00"
#define ACPI_CPU		"INT3401:00"
#define SYSFS_PCI		"/sys/bus/pci/devices"
#define SYSFS_PLATFORM		"/sys/bus/platform/devices"
#define SYSFS_PSTATE_PATH	"/sys/devices/system/cpu/intel_pstate/"
#define SYSFS_THERMAL		"/sys/class/thermal"

static const char *CPU_location[] = {"0000:00:04.0", "0000:00:0b.0", "0000:00:00.1", NULL};

static int *cpufreq; //Array storing Intercative Governor Cpu Frequencies

static int number_of_cores;

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

struct rfkill_event {
	u32 idx;
	u8 type;
	u8 operation;
	u8 soft, hard;
};

enum rfkill_type {
	RFKILL_TYPE_ALL = 0,
	RFKILL_TYPE_WLAN,
	RFKILL_TYPE_BLUETOOTH,
	RFKILL_TYPE_UWB,
	RFKILL_TYPE_WWAN,
	RFKILL_TYPE_GPS,
	RFKILL_TYPE_FM,
	NUM_RFKILL_TYPES,
};

enum rfkill_operation {
	RFKILL_OP_ADD = 0,
	RFKILL_OP_DEL,
	RFKILL_OP_CHANGE,
	RFKILL_OP_CHANGE_ALL,
};


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
	ESIF_SYSFS_GET_DISPLAY_BRIGHTNESS = 'SBDG',
	ESIF_SYSFS_SET_CPU_PSTATE = 'SPCS',
	ESIF_SYSFS_SET_WWAN_PSTATE = 'SPWS',
	ESIF_SYSFS_SET_OSC = 'CSOS',
	ESIF_SYSFS_SET_FAN_LEVEL = 'ELFS',
	ESIF_SYSFS_SET_BRIGHTNESS_LEVEL = 'ELBS'
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

struct tzPolicy {
	char policy[MAX_SYSFS_PATH];
};
#pragma pack(pop)

static struct tzPolicy* tzPolicies = NULL;

static int sysfs_set_int64(char *path, char *filename, Int64 val);
static int sysfs_set_string(const char *path, const char *filename, char *val);
static int sysfs_get_string(const char *path, const char *filename, char *str);
static int sysfs_get_string_multiline(const char *path, const char *filename, char *str);
static int sysfs_get_int64(const char *path, const char *filename, Int64 *p64);
static int sysfs_get_int64_direct(int fd, Int64 *p64);
static int replace_str(char *str, char *old, char *new, char *rpl_buff, int rpl_buff_len);
static int get_key_value_pair_from_str(const char *str, char *key, char *value);
static enum esif_rc get_thermal_rel_str(enum esif_thermal_rel_type type, char *table_str);
static void get_full_scope_str(char *orig, char *new);
static void replace_cpu_id(char *str);
static u64 GetCpuFreqPdl(void);
static void GetNumberOfCpuCores();
static enum esif_rc get_supported_policies(char *table_str, int idspNum, char *sysfs_str);
static enum esif_rc get_rapl_power_control_capabilities(char *table_str, esif_guid_t *target_guid);
static enum esif_rc get_proc_perf_support_states(char *table_str);
static enum esif_rc get_participant_current_control_capabilities(char *table_str, char *participant_path);
static enum esif_rc get_perf_support_states(char *table_str, char *participant_path);
static enum esif_rc get_supported_brightness_levels(char *table_str, char *participant_path);
static eEsifError get_participant_scope(char *acpi_name, char *acpi_scope);
static int SetActionContext(struct sysfsActionHashKey *keyPtr, EsifString devicePathName, EsifString deviceNodeName);
static struct esif_ht *actionHashTablePtr = NULL;
static char sys_long_string_val[MAX_SYSFS_STRING];
static eEsifError SetFanLevel(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr);
static eEsifError SetBrightnessLevel(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr);
static eEsifError GetFanInfo(EsifDataPtr responsePtr);
static eEsifError GetFanPerfStates(EsifDataPtr responsePtr);
static eEsifError GetFanStatus(EsifDataPtr responsePtr, const EsifString devicePathPtr);
static eEsifError GetDisplayBrightness(char *path, EsifDataPtr responsePtr);
static eEsifError SetOsc(EsifUpPtr upPtr, const EsifDataPtr requestPtr);
static eEsifError ResetThermalZonePolicyToDefault();
static eEsifError SetThermalZonePolicy();
static eEsifError SetIntelPState(u64 val);
static eEsifError ValidateOutput(char *devicePathPtr, char *nodeName, u64 val);

#ifdef ESIF_ATTR_OS_ANDROID
static void NotifyJhs(EsifUpPtr upPtr, const EsifDataPtr requestPtr);
#endif

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
	int min_idx = 0;
	int candidate_found = 0;
	char srchnm[MAX_SEARCH_STRING] = { 0 };
	char srchval[MAX_SEARCH_STRING]= { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
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
	devicePathPtr = esif_ccb_strtok(deviceFullPathPtr, "|", &pathTok);
	deviceAltPathPtr = esif_ccb_strtok(NULL, "|", &pathTok);

	// Assemble hash table key to look for existing file pointer to sysfs node
	key.participantId = EsifUp_GetInstance(upPtr);
	key.primitiveTuple = primitivePtr->tuple;
	actionContext = (size_t) esif_ht_get_item(actionHashTablePtr, (u8 *)&key, sizeof(key));

	// actionContext is not a pointer but instead the file descriptor. It cannot possibly be 0 because 0 is reserved for stdout
	// So if we do get 0 it would translate to NULL pointer which means that the key is not found in the hash table
	if (actionContext) {
		if (sysfs_get_int64_direct((int) actionContext, &sysval) > 0) {
			tripval = sysval;
			*(u32 *) responsePtr->buf_ptr = (u32) tripval;
			goto exit;
		} else {
			ESIF_TRACE_WARN("Failed to get action context, attempting to read from sysfs.\n");
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
		pathAccessReturn = sysfs_get_int64(devicePathPtr, parm1, &sysval);
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
			if (sysfs_get_int64(devicePathPtr, cur_node_name, &sysval) > 0 && sysval > 0) {
				if (ESIF_OK != ValidateOutput(devicePathPtr, cur_node_name, sysval))
					continue;

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
		if (sysfs_get_int64(parm1, parm2, &sysval) < 1) {
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
					if (sysfs_get_int64(devicePathPtr, alt_node_name, &sysval) > 0 && sysval > 0 && node_idx >= min_idx) {
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
						if (sysfs_get_int64(cur_node_name, node_name_ptr, &sysval) > 0 && sysval > 0) {
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
				domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);
				if (NULL == domainPtr) {
					rc = ESIF_E_INVALID_DOMAIN_ID;
					goto exit;
				}
				if (domainPtr->lastPowerTime == 0) {
					esif_ccb_get_time(&starttm);
					domainPtr->lastPowerTime = (u64)(starttm.tv_sec * 1000000) + starttm.tv_usec;
				}

				esif_ccb_get_time(&endtm);
				elapsed_tm = (((endtm.tv_sec * 1000000) + endtm.tv_usec) - (domainPtr->lastPowerTime)) / 1000000.0;

				if (sysfs_get_int64(parm1, parm2, &sysval) < 1) {
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
				if (sysfs_get_string(SYSFS_PSTATE_PATH, "num_pstates", sysvalstring) > -1) {
					if ((sysfs_get_int64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < 1) || (pdl_val > MAX_SYSFS_PSTATES)) {
						rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
						goto exit;
					}
					/* Sysfs returns total states - we want the max state, so subtract one */
					pdl_val -= 1;	
				}
				else {
					pdl_val = GetCpuFreqPdl();
				}
				
				*(u32 *) responsePtr->buf_ptr = (u32) pdl_val;
				break;
			case ESIF_SYSFS_GET_SOC_PL1: /* power limit */
				if (sysfs_get_int64("/sys/class/powercap/intel-rapl:0", "constraint_0_power_limit_uw", &sysval) < 1) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}

				*(u32 *) responsePtr->buf_ptr = (u32) sysval;
				break;
			case ESIF_SYSFS_GET_SOC_TEMP: /* Package or Graphics Core GET_TEMPERATURE */
				domain_idx0 = esif_atoi(parm1);
				domain_idx1 = esif_atoi(parm2);
				EsifUpDomainPtr Domain0 = EsifUp_GetDomainByIndex(upPtr, domain_idx0);
				EsifUpDomainPtr Domain1 = EsifUp_GetDomainByIndex(upPtr, domain_idx1);
				if (Domain0 == NULL || Domain1 == NULL) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					goto exit;
				}
				EsifPrimitiveTuple temp0Tuple = {GET_TEMPERATURE, Domain0->domain, 255};
				EsifPrimitiveTuple temp1Tuple = {GET_TEMPERATURE, Domain1->domain, 255};

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
					temp_val0 = esif_ccb_max(temp_val0, temp_val1);
					esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_MILLIC, &temp_val0);
					*(u32 *)responsePtr->buf_ptr = temp_val0;
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
				rc = GetFanStatus(responsePtr, devicePathPtr);
				break;
			case ESIF_SYSFS_GET_DISPLAY_BRIGHTNESS:
				rc = GetDisplayBrightness(parm1,responsePtr);
				break;

			default:
				break;
		}
		break;
	case ESIF_SYSFS_ATTRIBUTE:
		esif_ccb_sprintf(MAX_SYSFS_PATH, (char *) responsePtr->buf_ptr, "%s", devicePathPtr);
		break;
	case ESIF_SYSFS_DIRECT_QUERY_ENUM:
		/* This is a search loop, so default to failure */
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;

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
						if (sysfs_get_int64(devicePathPtr, alt_node_name, &sysval) < 1) {
							rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
							goto exit;
						}
						else {
							rc = ESIF_OK;
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
		rc = TableObject_LoadAttributes(&tableObject); /* properties such as table type (binary/virtual/datavault) and revision */
		if (rc != ESIF_OK) {
			TableObject_Destroy(&tableObject);
			goto exit;
		}
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
		else if (esif_ccb_stricmp("bcl", parm1) == 0) {
			rc = get_supported_brightness_levels(table_str, devicePathPtr);
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
		if (ESIF_OK == rc && tableObject.binaryDataSize > TABLE_OBJECT_MAX_BINARY_LEN) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}
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
	enum esif_sysfs_param calc_type = 0;
	char sys_long_string_val[MAX_SYSFS_STRING];
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char alt_node_name[MAX_SYSFS_PATH] = { 0 };
	char idx_holder[MAX_IDX_HOLDER] = { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	int node_idx = 0;
	int candidate_found = 0;
	int max_node_idx = MAX_NODE_IDX;
	u64 sysval = 0;
	u64 pdl_val = 0;
	EsifUpDataPtr metaPtr = NULL;
	int rfkill_fd = 0;
	struct rfkill_event event = {0};
	int core = 0;

	UNREFERENCED_PARAMETER(actCtx);

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
	devicePathPtr = esif_ccb_strtok(deviceFullPathPtr, "|", &pathTok);
	deviceAltPathPtr = esif_ccb_strtok(NULL, "|", &pathTok);

	sysopt = *(enum esif_sysfs_command *) command;
	switch (sysopt) {
	case ESIF_SYSFS_DIRECT_PATH:
		if (0 == esif_ccb_strcmp(parm2, "alt")) {
			deviceTargetPathPtr = deviceAltPathPtr;
		}
		else {
			deviceTargetPathPtr = devicePathPtr;
		}
		if (deviceTargetPathPtr == NULL) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		if (sysfs_set_int64(deviceTargetPathPtr, parm1, *(Int32 *) requestPtr->buf_ptr) < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		break;
	case ESIF_SYSFS_ALT_PATH:
		sysval = (u64)*(u32 *)requestPtr->buf_ptr;
		if (sysfs_set_int64(parm1, parm2, sysval) < 0) {
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
					if (sysfs_set_int64(cur_node_name, parm3, *(u32 *) requestPtr->buf_ptr) == 0) {
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
				sysval = *(u32 *) requestPtr->buf_ptr;
				
				if (sysfs_get_string(SYSFS_PSTATE_PATH, "num_pstates", sysvalstring) > -1) {
					rc = SetIntelPState(sysval);
				}
				else {
					pdl_val = GetCpuFreqPdl();
					if ((sysval <= pdl_val) && (cpufreq != NULL)) {
						for(core = 0; core < (number_of_cores+1); core++) {
							char cpuString[MAX_SYSFS_PATH] = {0};
							esif_ccb_sprintf(MAX_SYSFS_PATH, cpuString,"/sys/devices/system/cpu/cpu%d/cpufreq", core);
							if (sysfs_set_int64(cpuString, "scaling_max_freq", cpufreq[sysval]) < 0) {
								rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
							}	
						}
					}
					else {
						rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					}
				}
				
				if (rc != ESIF_OK) {
					goto exit;					
				}
				break;
			case ESIF_SYSFS_SET_WWAN_PSTATE:
				sysval = *(u32 *) requestPtr->buf_ptr;
				if (sysval > 0) {
					/* normalize for now */
					sysval = 1;
				}
				event.type = RFKILL_TYPE_WWAN;
				event.operation = RFKILL_OP_CHANGE_ALL;
				event.soft = sysval;
				event.hard = 0;

				if((rfkill_fd = open("/dev/rfkill", O_RDWR)) < 0){
				   rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				   goto exit;
				}

				if(write(rfkill_fd, &event, sizeof(event)) < 0){
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				}

				close(rfkill_fd);
				break;
			case ESIF_SYSFS_SET_OSC:  /* osc */
				rc = SetOsc(upPtr, requestPtr);
				break;

			case ESIF_SYSFS_SET_FAN_LEVEL:
				rc = SetFanLevel(upPtr, requestPtr, devicePathPtr);
				break;
			case ESIF_SYSFS_SET_BRIGHTNESS_LEVEL:
				rc = SetBrightnessLevel(upPtr, requestPtr, parm1);
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

static eEsifError SetIntelPState(u64 sysval)
{
	u64 pdl_val = 0;
	double target_perc = 0.0;
	u64 turbo_perc = 0;
	eEsifError rc = ESIF_OK;
	
	if ((sysfs_get_int64(SYSFS_PSTATE_PATH, "num_pstates", &pdl_val) < 1) || (sysval >= pdl_val)) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}
	target_perc = ((1.00 / (double) pdl_val) * ((u32) pdl_val - sysval)) * 100.0;
	if (target_perc > 100) {
		target_perc = 100;
	}
	else if (target_perc < MIN_PERF_PERCENTAGE) {
		target_perc = MIN_PERF_PERCENTAGE;
	}
	if (sysfs_set_int64("/sys/devices/system/cpu/intel_pstate","max_perf_pct", (int) target_perc) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}
	// Logic to turn on or off turbo
	if (sysfs_get_int64(SYSFS_PSTATE_PATH, "turbo_pct", &turbo_perc) > 0) {
		// Sys FS node exposes turbo frequency range, we need to convert it to turbo target
		turbo_perc = 100 - turbo_perc;
		if (target_perc < turbo_perc) {
			if (sysfs_set_int64(SYSFS_PSTATE_PATH, "no_turbo",1) < 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
		}
		else {
			if (sysfs_set_int64(SYSFS_PSTATE_PATH, "no_turbo",0) < 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
		}
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

static u64 GetCpuFreqPdl(void)
{
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	char *token;
	char *next_token;
	const char delimiter[2] = " ";
	u64 pdl_val = -1;
	if (sysfs_get_string_multiline("/sys/devices/system/cpu/cpu0/cpufreq", "scaling_available_frequencies", sysvalstring) > -1) {
		token = esif_ccb_strtok(sysvalstring,delimiter,&next_token);
		while ((token != NULL) && (isdigit(*token))) {
			token = esif_ccb_strtok(NULL,delimiter,&next_token);
			pdl_val++;
		}
	}
	return pdl_val;
}

static void GetNumberOfCpuCores()
{
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	char *token;
	char *next_token;
	const char delimiter[2] = "-";
	if (sysfs_get_string_multiline("/sys/devices/system/cpu/cpu0/topology", "core_siblings_list", sysvalstring) > -1) {
		token = esif_ccb_strtok(sysvalstring,delimiter,&next_token);
		while ((token != NULL) && (isdigit(*token))) {
			number_of_cores = esif_atoi(token);
			token = esif_ccb_strtok(NULL,delimiter,&next_token);
		}
	}
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
	esif_ccb_sprintf_concat(rpl_buff_len, rpl_buff, "%s%s", new, p + esif_ccb_strlen(orig, MAX_SYSFS_PATH));

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

static int sysfs_get_string(const char *path, const char *filename, char *str)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		goto exit;
	}

	// Use dynamic format width specifier to avoid scanf buffer overflow
	char fmt[MAX_FMT_STR_LEN] = { 0 };
	esif_ccb_sprintf(sizeof(fmt), fmt, "%%%ds", (int)MAX_SYSFS_PATH - 1);
	rc = esif_ccb_fscanf(fd, fmt, SCANFBUF(str, MAX_SYSFS_PATH));
	esif_ccb_fclose(fd);

exit:
	return rc;
}

static int sysfs_get_string_multiline(const char *path, const char *filename, char *str)
{
	FILE *fp = NULL;
	int rc = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	char lineStr[MAX_STR_LINE_LEN + 1] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fp = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		goto exit;
	}

	*str = 0; // Initialize first character to ensure that concat works properly
	while (esif_ccb_fgets(lineStr, MAX_STR_LINE_LEN, fp)) {
		esif_ccb_sprintf_concat(MAX_SYSFS_STRING, str, "%s\n", lineStr);
		rc++;
	}

	esif_ccb_fclose(fp);

exit:
	return rc;
}

static int sysfs_get_int64(const char *path, const char *filename, Int64 *p64)
{
	FILE *fd = NULL;
	int rc = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		goto exit;
	}
	rc = esif_ccb_fscanf(fd, "%lld", p64);
	esif_ccb_fclose(fd);

exit:
	// Klocwork bounds check. Should depend on context
	if (rc > 0 && (*p64 < MIN_INT64 || *p64 > MAX_INT64)) {
		rc = 0;
	}
	return rc;
}

static int sysfs_get_int64_direct(int fd, Int64 *p64)
{
	int rc = 0;
	char buf[MAX_SYSFS_STRING] = {0};

	lseek(fd, 0, SEEK_SET);
	if (read(fd, buf, MAX_SYSFS_STRING) > 0) {
		rc = esif_ccb_sscanf(buf, "%lld", p64);
		if (ESIF_OK != rc) {
			ESIF_TRACE_WARN("Failed to get file scan. Error code: %d .\n",rc);
		}
	}
	else {
		ESIF_TRACE_WARN("Error on context file read: %s\n", strerror(errno));
	}

	return rc;
}

static int sysfs_set_int64(char *path, char *filename, Int64 val)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "w", NULL)) == NULL) {
		goto exit;
	}

	esif_ccb_fprintf(fd, "%lld", val);
	esif_ccb_fclose(fd);
	rc = 0;

exit:
	return rc;
}

static int sysfs_set_string(const char *path, const char *filename, char *val)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "w", NULL)) == NULL) {
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
	char guidStr[MAX_GUID_STR_LEN + 1] = {0};
	int i = 0;

	for (i = 0; i < idspNum; i++) {
		char scanguid_fmt[MAX_FMT_STR_LEN] = { 0 };
		esif_ccb_sprintf(sizeof(scanguid_fmt), scanguid_fmt, "%%%ds", (int)sizeof(guidStr) - 1);
		esif_ccb_sscanf(scanPtr, scanguid_fmt, SCANFBUF(guidStr, sizeof(guidStr)));
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

	if ((sysfs_get_int64(participant_path, "max_state", &pdl_val) < 1) || (pdl_val > MAX_SYSFS_PERF_STATES)){
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	esif_ccb_sprintf(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu,mA,%llu!",pcounter,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
	for (pcounter = 1; pcounter <= pdl_val; pcounter++) {
		esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu,mA,%llu!",pcounter,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
	}

exit:
	return rc;
}

static enum esif_rc get_supported_brightness_levels(char *table_str, char *participant_path)
{	int max_value = 100;
	int increment_value = 5;
	int default_brightness = 100;
	int pcounter = 0;
	eEsifError rc = ESIF_OK;

	esif_ccb_sprintf(BINARY_TABLE_SIZE, table_str, "%d!%d!",default_brightness,default_brightness);
	for (pcounter = 5;pcounter <= max_value;pcounter += increment_value) {
		esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d!",pcounter);
	}

	return rc;
}

static enum esif_rc get_proc_perf_support_states(char *table_str)
{
	u64 pdl_val = 0;
	u64 placeholder_val =0;
	int pcounter = 0;
	eEsifError rc = ESIF_OK;
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	char *token;
	char *next_token;
	const char s[2] = " ";
	int counter = 0;
	
	if (sysfs_get_string(SYSFS_PSTATE_PATH, "num_pstates", sysvalstring) > -1) {
		if ((sysfs_get_int64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < 1) || (pdl_val > MAX_SYSFS_PSTATES)) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
	}
	else {
		pdl_val = GetCpuFreqPdl();
		cpufreq = (int*)esif_ccb_malloc(sizeof(int) * pdl_val);
		if (cpufreq == NULL) {
			ESIF_TRACE_ERROR("Unable to allocate cpufreq\n");
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		if (sysfs_get_string_multiline("/sys/devices/system/cpu/cpu0/cpufreq", "scaling_available_frequencies", sysvalstring) > -1) {
			token = esif_ccb_strtok(sysvalstring,s,&next_token);
			while ((token != NULL) && (isdigit(*token))) {
				cpufreq[counter] = esif_atoi(token);
				esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu!",(cpufreq	[counter]/1000),placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
				token = esif_ccb_strtok(NULL,s,&next_token);
				counter++;	
			}
		}
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
	int n = 0;
	int cpu_loc_counter = 0;
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
		while (n-- && !candidate_found) {
			cpu_loc_counter = 0;
			while (CPU_location[cpu_loc_counter] != NULL && !candidate_found) {
				if (esif_ccb_strstr(namelist[n]->d_name, CPU_location[cpu_loc_counter]) != NULL) {
					esif_ccb_sprintf(MAX_SYSFS_PATH, node, "%s", CPU_location[cpu_loc_counter]);
				candidate_found = 1;
			}
				cpu_loc_counter++;
			}
			if (!candidate_found) {
				if (esif_ccb_strstr(namelist[n]->d_name, ACPI_CPU) != NULL) {
				esif_ccb_sprintf(MAX_SYSFS_PATH, node, "%s", ACPI_CPU);
				candidate_found = 1;
				}
			}
			free(namelist[n]); // Use NATIVE free since allocated by scandir
		}
		free(namelist); // Use NATIVE free since allocated by scandir
	}
	closedir(dir);

	if (candidate_found == 0) {
		goto exit;
	}
	/****************************/


	esif_ccb_sprintf(MAX_SYSFS_PATH, cur_path, "%s/%s/power_limits", path, node);
	if (sysfs_get_int64(cur_path, "power_limit_0_min_uw", &sysval) < 1) {
		goto exit;
	}
	pl1Min = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_0_max_uw",  &sysval) < 1) {
		goto exit;
	}
	pl1Max = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_0_step_uw", &sysval) < 1) {
		goto exit;
	}
	step1 = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_0_tmin_us", &sysval) < 1) {
		goto exit;
	}
	time1Min = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_0_tmax_us", &sysval) < 1) {
		goto exit;
	}
	time1Max = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_1_min_uw",  &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Min = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_1_max_uw",  &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Max = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_1_step_uw", &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	step2 = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_1_tmin_us", &sysval) < 1) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	time2Min = sysval;
	if (sysfs_get_int64(cur_path, "power_limit_1_tmax_us", &sysval) < 1) {
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

	table = (unsigned char *) esif_ccb_malloc(len);
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
	esif_ccb_free(table);
	if (file >= 0)
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
		esif_ccb_sprintf(MAX_ACPI_SCOPE_LEN, acpi_scope, "%s", (acpi_name ? acpi_name : ESIF_NOT_AVAILABLE));
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

	if (sysfs_get_int64(devicePathPtr, "max_state", &pdlVal) < 1) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail get participant %d's fan max_state\n", upPtr->fInstance);
		goto exit;
	}

	target_perc = ((double) *(u32 *) requestPtr->buf_ptr) / 100.0;
	curVal = (target_perc + EPSILON_CONVERT_PERC) * pdlVal; // Avoid rounding errors
	if (sysfs_set_int64(devicePathPtr, "cur_state", curVal) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail set participant %d's fan cur_state\n", upPtr->fInstance);
		goto exit;
	}

	ESIF_TRACE_DEBUG("Set upper particiapnt %d's fan speed to %llu\n", upPtr->fInstance, curVal);

exit:
	return rc;
}

static eEsifError SetBrightnessLevel(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr)
{
	eEsifError rc = ESIF_OK;
	u64 bdlVal = 0;
	u64 curVal = 0;
	double target_perc = 0;

	if (sysfs_get_int64(devicePathPtr, "max_brightness", &bdlVal) < 1) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail get participant %d's max_brightness\n", upPtr->fInstance);
		goto exit;
	}

	target_perc = ((double) *(u32 *) requestPtr->buf_ptr) / 100.0;
	curVal = (u64)((target_perc * bdlVal) + EPSILON_CONVERT_PERC); // Avoid rounding errors
	if (sysfs_set_int64(devicePathPtr, "brightness", curVal) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail set participant %d's brightness\n", upPtr->fInstance);
		goto exit;
	}

	ESIF_TRACE_DEBUG("Set upper participant %d's display brightness to %llu\n", upPtr->fInstance, curVal);

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
	int stepSize = (int)(100.0 / (double)MAX_ACX_ENTRIES);

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
	fps.tripPoint.integer.value = INVALID_64BIT_UINTEGER;
	fps.speed.integer.type = ESIF_DATA_UINT64;
	fps.speed.integer.value = INVALID_64BIT_UINTEGER;
	fps.noiseLevel.integer.type = ESIF_DATA_UINT64;
	fps.noiseLevel.integer.value = INVALID_64BIT_UINTEGER;
	fps.power.integer.type = ESIF_DATA_UINT64;
	fps.power.integer.value = INVALID_64BIT_UINTEGER;

	// First copy revision, then multiple fps "packages"
	esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, &revision, sizeof(revision));
	size += sizeof(revision);
	for (i = 0; i < MAX_ACX_ENTRIES; i++) {
		fps.control.integer.value = i * stepSize;
		esif_ccb_memcpy((u8 *) responsePtr->buf_ptr + size, &fps, sizeof(fps));
		size += sizeof(fps);
	}

exit:
	return rc;
}

static eEsifError GetDisplayBrightness(char *path, EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;
	u64 bdlVal = 0;
	u64 curVal = 0;
	double target_perc = 0;

	if (sysfs_get_int64(path, "max_brightness", &bdlVal) < 1) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Failed get participant's max display brightness\n");
		goto exit;
	}

	if (sysfs_get_int64(path, "brightness", &curVal) < 1) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Failed get participant's current brightness\n");
		goto exit;
	}

	target_perc = ((double)curVal / (double)bdlVal) * 100;
	*(u32 *) responsePtr->buf_ptr = (u32) target_perc;

exit:
	return rc;
}

static eEsifError GetFanStatus(EsifDataPtr responsePtr, const EsifString devicePathPtr)
{
	// Since the ACPI object _FST is not exposed to user space, use the value
	// set by SetFanLevel.
	eEsifError rc = ESIF_OK;
	u64 curVal = 0;
	struct EsifDataBinaryFstPackage fst = {0};
	int stepSize = (int)(100.0 / (double)MAX_ACX_ENTRIES);

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
	fst.speed.integer.value = INVALID_64BIT_UINTEGER;

	if (sysfs_get_int64(devicePathPtr, "cur_state", &curVal) < 1) {
		// Do nothing, best effort.
	}
	else {
		fst.control.integer.value = curVal * stepSize;
	}

	esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, &fst, sizeof(fst));

exit:
	return rc;
}

static void NotifyJhs(EsifUpPtr upPtr, const EsifDataPtr requestPtr)
{
	EsifUpDomainPtr d0 = EsifUp_GetDomainByIndex(upPtr, 0);
	if (NULL != d0) {
		EsifPrimitiveTuple notifyJhsTuple = {SET_JAVA_HELPER_SERVICE_NOTIFICATION, d0->domain, 255};
		// No need to check return code from EsifUp_ExecutePrimitive() call
		// We are trying to tell JHS that DPTF is up, but JHS itself may not be up
		// If EsifUp_ExecutePrimitive() fails it does not matter
		EsifUp_ExecutePrimitive(upPtr, &notifyJhsTuple, requestPtr, NULL);
	}
}

static eEsifError HandleOscRequest(const struct esif_data_complex_osc *oscPtr, const char *cur_node_name)
{
	eEsifError rc = ESIF_OK;
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	char guidStr[ESIF_GUID_PRINT_SIZE] = { 0 };
	esif_guid_t *guidPtr = (esif_guid_t *) oscPtr->guid;

	// Convert IDSP GUID from binary format to text format because
	// INT3400 driver only takes text format as input
	esif_guid_mangle(guidPtr);
	esif_guid_print(guidPtr, guidStr);

	if (oscPtr->capabilities) {	// DPTF to take over thermal control
		if (sysfs_set_string(cur_node_name, "mode", "enabled") < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
	} else {
		if (sysfs_set_string(cur_node_name, "mode", "disabled") < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
	}

	if (sysfs_set_string("/sys/devices/platform/INT3400:00/uuids/", "current_uuid", guidStr) < 0) {
		ESIF_TRACE_WARN("Failed to set _OSC for GUID: %s\n", guidStr);
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	if (sysfs_get_string(cur_node_name, "policy", sysvalstring) > -1) {
		if (sysfs_set_string(cur_node_name, "policy", "user_space") < 0) {
			ESIF_TRACE_WARN("Failed to change INT3400 driver to user_space mode\n");
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
	}
	ESIF_TRACE_DEBUG("Successfully set _OSC for GUID: %s\n", guidStr);

exit:
	return rc;
}

static eEsifError SetOsc(EsifUpPtr upPtr, const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;

	int node_idx = 0;
	int max_node_idx = MAX_NODE_IDX;
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };

#ifdef ESIF_ATTR_OS_ANDROID
	// Notify JHS that DPTF is up
	NotifyJhs(upPtr, requestPtr);
#endif
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
				rc = HandleOscRequest(oscPtr, cur_node_name);
				if (ESIF_OK != rc) {
					goto exit;
				}
				break;
			}
		}
	}

	// Set success return status byte
	oscPtr->status = 0;

exit:
	return rc;
}

static eEsifError ValidateOutput(char *devicePathPtr, char *nodeName, u64 val)
{
	eEsifError rc = ESIF_OK;

	if (NULL != esif_ccb_strstr(nodeName, "hyst")) {
		// Output is hysteresis in milli degree C
		if ((val < MIN_HYSTERESIS_MILLIC) || (val > MAX_HYSTERESIS_MILLIC)) {
			ESIF_TRACE_ERROR("Read invalid hysteresis value from %s/%s, discard...\n",
					devicePathPtr, nodeName);
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}
	}

	return rc;
}

/*
 *******************************************************************************
 * Register ACTION with ESIF
 *******************************************************************************
 */

static eEsifError SetThermalZonePolicy()
{
	DIR *dir;
	struct dirent **namelist;
	int n;
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	eEsifError rc = ESIF_E_UNSPECIFIED;

	dir = opendir(SYSFS_THERMAL);
	if (!dir) {
		ESIF_TRACE_DEBUG("No thermal sysfs\n");
	}
	n = scandir(SYSFS_THERMAL, &namelist, 0, alphasort);
	tzPolicies = (struct tzPolicy*)esif_ccb_malloc(sizeof(struct tzPolicy) * n);
	if (tzPolicies == NULL) {
		ESIF_TRACE_ERROR("Unable to allocate tzPolicies\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	if (n < 0) {
		//no scan
	}
	else {
		while (n--) {
			esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name, "%s/%s", SYSFS_THERMAL,namelist[n]->d_name);
			if (sysfs_get_string(cur_node_name, "policy", sysvalstring) > -1) {
				esif_ccb_sprintf(MAX_SYSFS_PATH,tzPolicies[n].policy,"%s",sysvalstring);
				if (0 != esif_ccb_strcmp(sysvalstring, "user_space")) {
					if (sysfs_set_string(cur_node_name, "policy", "user_space") < 0) {
						ESIF_TRACE_WARN("Failed to change thermal zone policy type to user_space mode\n");
					}	
				}
			}
		}
	}
exit:
	return rc;
}

static eEsifError ResetThermalZonePolicy()
{
	DIR *dir;
	struct dirent **namelist;
	int n;
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	eEsifError rc = ESIF_E_UNSPECIFIED;

	if (tzPolicies == NULL) {
		ESIF_TRACE_ERROR("Unable to allocate tzPolicies\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	dir = opendir(SYSFS_THERMAL);
	if (!dir) {
		ESIF_TRACE_DEBUG("No thermal sysfs\n");
	}
	n = scandir(SYSFS_THERMAL, &namelist, 0, alphasort);
	if (n < 0) {
		//no scan
	}
	else {
		while (n--) {
			esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name, "%s/%s", SYSFS_THERMAL,namelist[n]->d_name);
			if (sysfs_get_string(cur_node_name, "policy", sysvalstring) > -1) {
				if (sysfs_set_string(cur_node_name, "policy", tzPolicies[n].policy) < 0) {
					ESIF_TRACE_WARN("Failed to change thermal zone policy type to default\n");
				}
			}
		}
	}
	
	esif_ccb_free(tzPolicies);
exit:
	return rc;
}

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
	SetThermalZonePolicy();
	GetNumberOfCpuCores();
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActSysfsExit()
{
	EsifActMgr_UnregisterAction((EsifActIfacePtr)&g_sysfs);
	if (actionHashTablePtr)
		esif_ht_destroy(actionHashTablePtr, ActionContextCleanUp);
	if(cpufreq)
		esif_ccb_free(cpufreq);
	ResetThermalZonePolicy();
	ESIF_TRACE_EXIT_INFO();
}

#endif
/* end compile out entire page without ESIF_FEAT_OPT_SYSFS */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
