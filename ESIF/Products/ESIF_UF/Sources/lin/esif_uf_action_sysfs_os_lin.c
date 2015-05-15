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
	ESIF_SYSFS_BINARY_TABLE = 'LBTB',
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
static void* get_full_scope_str(char *orig, char *new);
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

/*
 * Handle ESIF Action "Get" Request
 */
static eEsifError ESIF_CALLCONV ActionSysfsGet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	EsifUpDomainPtr domainPtr = NULL;
	eEsifError esifStatus = ESIF_E_ACTION_NOT_IMPLEMENTED;
	EsifString command = NULL;
	EsifString parm1 = NULL;
	EsifString parm2 = NULL;
	EsifString parm3 = NULL;
	EsifString parm4 = NULL;
	EsifString devicePathPtr = NULL;

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
	int calc_type = 0;
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
	char cur_path[MAX_SYSFS_PATH] = { 0 };
	char pcipath[] = "/sys/bus/pci/devices";
	char pcinode[] = "0000:00:%id%.0";
	char table_str[BINARY_TABLE_SIZE];
	TableObject tableObject = {0};
	struct sysfsActionHashKey key = {0};
	size_t actionContext = 0;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	rc = EsifActionGetParams(actionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto error_exit;
	}
	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifActionCreateTokenReplacedParamString(params[i].buf_ptr, upPtr, primitivePtr);
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
		goto error_exit;
	}
	
	if (responsePtr->buf_len < sizeof(u32)) {
		goto error_exit;
	}
	
	//verify valid buf length for response

	command = (EsifString) params[0].buf_ptr;
	parm1 = (EsifString) params[1].buf_ptr;
	parm2 = (EsifString) params[2].buf_ptr;
	parm3 = (EsifString) params[3].buf_ptr;
	parm4 = (EsifString) params[4].buf_ptr;
	devicePathPtr = (EsifString) upPtr->fMetadata.fDevicePath;
	
	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);

	// Assemble hash table key to look for existing file pointer to sysfs node
	key.participantId = upPtr->fInstance;
	key.primitiveTuple = primitivePtr->tuple;
	actionContext = (size_t) esif_ht_get_item(actionHashTablePtr, (u8 *)&key, sizeof(key));

	// actionContext is not a pointer but instead the file descriptor. It cannot possibly be 0 because 0 is reserved for stdout
	// So if we do get 0 it would translate to NULL pointer which means that the key is not found in the hash table
	if (actionContext) {
		if (0 == sysfs_get_u64_direct((int) actionContext, &sysval)) {
			tripval = sysval;
			*(u32 *) responsePtr->buf_ptr = (u32) tripval;
			esifStatus = ESIF_OK;
			//printf("CONTEXT FOUND AT: context:%p, value: %d, skipping scan.\n",actionContext, (u32)tripval);
			return esifStatus;
		} else
			goto error_exit;
	}

	//printf("NO CONTEXT AVAILABLE for participant %d, domain %d, action %d, SCANNING.\n", upPtr->fInstance, primitivePtr->tuple.domain, primitivePtr->tuple.id);
	sysopt = *(enum esif_sysfs_command *) command;

	switch (sysopt) {
	case ESIF_SYSFS_DIRECT_PATH:
		if (sysfs_get_u64(devicePathPtr, parm1, &sysval) < 0) {
			goto error_exit;
		}
		if (SetActionContext(&key, devicePathPtr, parm1)) {
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
				goto error_exit;
			}
			if (sysfs_get_u64(devicePathPtr, cur_node_name, &sysval) > -1 && sysval > 0) {
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
			goto error_exit;
		}
		break;
	case ESIF_SYSFS_ALT_PATH:
		if (sysfs_get_u64(parm1, parm2, &sysval) < 0) {
			goto error_exit;
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
				goto error_exit;
			}
			if (replace_str(parm3, "%i%", idx_holder, alt_node_name, MAX_SYSFS_PATH) > 0) {
				goto error_exit;
			}
			if (sysfs_get_string(devicePathPtr, cur_node_name, sysvalstring) > -1) {
				if (esif_ccb_stricmp(parm2, sysvalstring) == 0) {
					if (sysfs_get_u64(devicePathPtr, alt_node_name, &sysval) > -1 && sysval > 0 && node_idx >= min_idx) {
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
			return ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
		}
		*(u32 *) responsePtr->buf_ptr = (u32) tripval;
		break;
	case ESIF_SYSFS_ALT_QUERY:
		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			candidate_found = ESIF_FALSE;
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				goto error_exit;
			}

			// Only need to locate key/value pair once
			if (0 == *srchnm) {
				if (get_key_value_pair_from_str(parm2, srchnm, srchval)) {
					goto error_exit;
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
						if (sysfs_get_u64(cur_node_name, node_name_ptr, &sysval) > -1 && sysval > 0) {
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
        		return ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT;
		}
		*(u32 *) responsePtr->buf_ptr = (u32) sysval;
		break;
	case ESIF_SYSFS_CALC:
		calc_type = esif_atoi(parm3);
		switch (calc_type) {
			case 0: /* rapl */
				
				if (domainPtr->lastPowerTime == 0) {
					esif_ccb_get_time(&starttm);
					domainPtr->lastPowerTime = (u64)(starttm.tv_sec * 1000000) + starttm.tv_usec;
				}
					
				esif_ccb_get_time(&endtm);
				elapsed_tm = (((endtm.tv_sec * 1000000) + endtm.tv_usec) - (domainPtr->lastPowerTime)) / 1000000.0;
				
				if (sysfs_get_u64(parm1, parm2, &sysval) < 0) {
					goto error_exit;
				}
				
				if (elapsed_tm > 0) {
					ret_val = (u64) ((sysval - domainPtr->lastPower) / elapsed_tm);
					domainPtr->lastPowerTime = (u64)(endtm.tv_sec * 1000000) + endtm.tv_usec;
				}
				domainPtr->lastPower = sysval;
				
				*(u32 *) responsePtr->buf_ptr = (u32) ret_val;
				break;
			case 1: /* pdl */
				if (sysfs_get_u64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < 0) {
					goto error_exit;
				}
				/* Sysfs returns total states - we want the max state, so subtract one */
				if (pdl_val != 0) {
					pdl_val -= 1;
				}
				*(u32 *) responsePtr->buf_ptr = (u32) pdl_val;
				break;
			case 2: /* power limit */
				/* should return the lessor between PL1 max and current power limit*/

				pl1max_val = GetPowerLimit0MaxUw();
				if (0 == pl1max_val)
					goto error_exit;
				if (sysfs_get_u64("/sys/class/powercap/intel-rapl:0", "constraint_0_power_limit_uw", &sysval) < 0) {
					goto error_exit;
				}
				cur_power_limit = (sysval < pl1max_val) ? sysval : pl1max_val;
				
				*(u32 *) responsePtr->buf_ptr = (u32) cur_power_limit;
				break;
			case 3: /* Package or Graphics Core GET_TEMPERATURE */
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
					goto error_exit;
				}
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
			goto error_exit;
		}

		cur_item_count = 0;
		target_item_count = esif_atoi(srchval);
		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				goto error_exit;
			}
			if (replace_str(parm3, "%i%", idx_holder, alt_node_name, MAX_SYSFS_PATH) > 0) {
				goto error_exit;
			}
			if (sysfs_get_string(devicePathPtr, cur_node_name, sysvalstring) > -1) {
				if (esif_ccb_stricmp(srchnm, sysvalstring) == 0) {
					if (cur_item_count == target_item_count) {
						if (sysfs_get_u64(devicePathPtr, alt_node_name, &sysval) < 0) {
							goto error_exit;
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
			return ESIF_E_PARAMETER_IS_NULL;
		}
		esif_ccb_memset(table_str, 0, BINARY_TABLE_SIZE);
		/*	domain str and participant id are not relevant in this
			case since we use device paths */
		TableObject_Construct(&tableObject, parm1, "D0", 0);
		rc = TableObject_LoadSchema(&tableObject);
		if (rc != ESIF_OK) {
			TableObject_Destroy(&tableObject);
			return rc;
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
			rc = get_rapl_power_control_capabilities(table_str,(esif_guid_t *)upPtr->fMetadata.fDriverType);
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
			return ESIF_E_PARAMETER_IS_NULL;
		}
		
		if (ESIF_OK != rc) {
			TableObject_Destroy(&tableObject);
			return rc;
		}
	
		tableObject.dataText = esif_ccb_strdup(table_str);
		rc = TableObject_Convert(&tableObject);
		if (ESIF_OK != rc) {
			TableObject_Destroy(&tableObject);
			return rc;
		}
		esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, tableObject.binaryData, tableObject.binaryDataSize);
		responsePtr->type = ESIF_DATA_BINARY;
		responsePtr->data_len = tableObject.binaryDataSize;
		
		//esif_ccb_free(table_str);
		TableObject_Destroy(&tableObject);
		if (rc != ESIF_OK) {
			return ESIF_E_PRIMITIVE_ACTION_FAILURE;
		}
		break;
	default:
		return ESIF_E_OPCODE_NOT_IMPLEMENTED;
	}

	esifStatus = ESIF_OK;
	return esifStatus;

error_exit:
	*(u32 *) responsePtr->buf_ptr = ERROR_VALUE;
	return ESIF_E_PRIMITIVE_ACTION_FAILURE;
}


/*
 * Handle ESIF Action "Set" Request
 */
static eEsifError ESIF_CALLCONV ActionSysfsSet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError esifStatus = ESIF_E_ACTION_NOT_IMPLEMENTED;
	EsifString command = NULL;
	EsifString parm1 = NULL;
	EsifString parm2 = NULL;
	EsifString parm3 = NULL;
	EsifString parm4 = NULL;
	EsifString devicePathPtr = NULL;
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
	int calc_type;
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

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(responsePtr);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);

	rc = EsifActionGetParams(actionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto error_exit;
	}

	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifActionCreateTokenReplacedParamString(params[i].buf_ptr, upPtr, primitivePtr);
		if (replacedStr != NULL) {
			params[i].buf_ptr = replacedStr;
			replacedStrs[i] = replacedStr;
		}
	}
	if (requestPtr == NULL || requestPtr->buf_ptr == NULL || params[0].buf_ptr == NULL) {
		goto error_exit;
	}
	
	if (requestPtr->buf_len < sizeof(u32)) {
		goto error_exit;
	}

	command = (EsifString) params[0].buf_ptr;
	parm1 = (EsifString) params[1].buf_ptr;
	parm2 = (EsifString) params[2].buf_ptr;
	parm3 = (EsifString) params[3].buf_ptr;
	parm4 = (EsifString) params[4].buf_ptr;
	devicePathPtr = (EsifString) upPtr->fMetadata.fDevicePath;


	sysopt = *(enum esif_sysfs_command *) command;
	switch (sysopt) {
	case ESIF_SYSFS_DIRECT_PATH:
		if (sysfs_set_u64(devicePathPtr, parm1, *(u32 *) requestPtr->buf_ptr) < 0) {
			goto error_exit;
		}
		break;
	case ESIF_SYSFS_ALT_PATH:
		sysval = (u64)*(u32 *)requestPtr->buf_ptr;
		if (0 == esif_ccb_strcmp(parm2, "constraint_0_power_limit_uw")) {
			// Primitve SET_RAPL_POWER_LIMIT, need to check PL1 upper limit before set
			// Note that in Linux drivers the name "power limit 0" is used to denote PL1
			u64 powerLimit0Max = GetPowerLimit0MaxUw();
			if (0 == powerLimit0Max)
				goto error_exit;
			sysval = (sysval < powerLimit0Max) ? sysval : powerLimit0Max;
		}

		if (sysfs_set_u64(parm1, parm2, sysval) < 0) {
			goto error_exit;
		}
		break;
	case ESIF_SYSFS_ALT_QUERY:
		candidate_found = 0;
		if (get_key_value_pair_from_str(parm2, srchnm, srchval)) {
			goto error_exit;
		}

		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				goto error_exit;
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
			goto error_exit;
		}
		break;
	case ESIF_SYSFS_CALC:
		calc_type = esif_atoi(parm3);
		switch(calc_type) {
			case 0:  /* pstate to perc */
				if (sysfs_get_u64(SYSFS_PSTATE_PATH, "num_pstates", &pdl_val) < 0) {
					goto error_exit;
				}
				target_perc = ((1.00 / (double) pdl_val) * ((u32) pdl_val - *(u32 *) requestPtr->buf_ptr)) * 100.0;
				if (target_perc > 100) {
					target_perc = 100;
				}
				else if (target_perc < MIN_PERF_PERCENTAGE) {
					target_perc = MIN_PERF_PERCENTAGE;
				}
				if (sysfs_set_u64(parm1, parm2, (int) target_perc) < 0) {
					goto error_exit;
				}
				// Logic to turn on or off turbo
				if (0 == sysfs_get_u64(SYSFS_PSTATE_PATH, "turbo_pct", &turbo_perc)) {
					// Sys FS node exposes turbo frequency range, we need to convert it to turbo target
					turbo_perc = 100 - turbo_perc;
					if (target_perc < turbo_perc) {
						if (sysfs_set_u64(SYSFS_PSTATE_PATH, "no_turbo",1) < 0) {
							goto error_exit;
						}
					}
					else {
						if (sysfs_set_u64(SYSFS_PSTATE_PATH, "no_turbo",0) < 0) {
							goto error_exit;
						}
					}
				}
				break;
			case 1:  /* osc */
				// Check validity of _OSC request
				if (ESIF_DATA_STRUCTURE != requestPtr->type ||
					sizeof(struct esif_data_complex_osc) != requestPtr->buf_len) {
					goto error_exit;
				}
				struct esif_data_complex_osc *oscPtr = (struct esif_data_complex_osc *)requestPtr->buf_ptr;
				oscPtr->status = 0x2;	// Pre-initialize the return byte to _OSC failure

				for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
					esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name, "/sys/class/thermal/thermal_zone%d", node_idx);
					if (sysfs_get_string(cur_node_name, "type", sysvalstring) > -1) {
						if (esif_ccb_stricmp("INT3400", sysvalstring) == 0) {
							if (oscPtr->capabilities) {	// DPTF to take over thermal control
								if (sysfs_set_string(cur_node_name, "mode", "disabled") < 0) {
									goto error_exit;
								}
							} else {
								if (sysfs_set_string(cur_node_name, "mode", "enabled") < 0) {
									goto error_exit;
								}
							}
							// Need to output an IDSP GUID to current_uuid sysfs node. It does not matter what what GUID is as long as there is one.
							if (sysfs_get_string("/sys/devices/platform/INT3400:00/uuids/", "available_uuids", sys_long_string_val) < 0) {
								goto error_exit;
							}
							if (sysfs_set_string("/sys/devices/platform/INT3400:00/uuids/", "current_uuid", sys_long_string_val) < 0) {
								goto error_exit;
							}
							
						}
					}
					if (sysfs_get_string(cur_node_name, "policy", sysvalstring) > -1) {
						if (sysfs_set_string(cur_node_name, "policy", "user_space") < 0) {
							goto error_exit;
						}
					}
				}
				// Set success return status byte
				oscPtr->status = 0;
				break;
				
			default:
				break;
		}
		break;
	default:
		return ESIF_E_OPCODE_NOT_IMPLEMENTED;
		break;
	}

	esifStatus = ESIF_OK;
	return esifStatus;

error_exit:
	return ESIF_E_PRIMITIVE_ACTION_FAILURE;
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
	if (0 == sysfs_get_u64(fullPath, "power_limit_0_max_uw", &sysVal))
		return sysVal;

	// Next try Core PCI
	esif_ccb_sprintf(sizeof(fullPath), fullPath, "%s/%s/power_limits", SYSFS_PCI, CORE);
	if (0 == sysfs_get_u64(fullPath, "power_limit_0_max_uw", &sysVal))
		return sysVal;

	// Finally try Atom ACPI
	esif_ccb_sprintf(sizeof(fullPath), fullPath, "%s/%s/power_limits", SYSFS_PLATFORM, ACPI_CPU);
	if (0 == sysfs_get_u64(fullPath, "power_limit_0_max_uw", &sysVal))
		return sysVal;

	return 0;	// 0 indicates "not found"
}

// acpi_thermal_rel driver only returns the leaf node strings,
// DPTF expects full scope, so  prepend source/target stringS
// with \_UP.CNJR scope namE
static void* get_full_scope_str(char *orig, char *new)
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
	char *p = NULL;

	/* return if target string doesn't exist in haystack */
	p = esif_ccb_strstr(str, orig);
	if (p == NULL) {
		return 1;
	}
	
	
	/*create new version of the original string in the buffer that contains everything before the target str */
	esif_ccb_strncpy(rpl_buff, str, (p - str) + 1);
	rpl_buff[(p - str) + 1] = '\0';

	/* add the replace str, then continue with the original buffer val */
	esif_ccb_sprintf_concat(rpl_buff_len, rpl_buff, "%s%s", new, p + strlen(orig));
	return 0;

}

static int get_key_value_pair_from_str(const char *str, char *key, char *value)
{
	char *pch = 0;
	char *next_pch = 0;
	char param_copy[MAX_PARAM_STRING];

	// esif_ccb_strtok() is destructive to parameters (passed in as first argument)
	// Make a copy of str before proceeding
	esif_ccb_strcpy(param_copy, str, MAX_PARAM_STRING);
	
	pch = esif_ccb_strtok(param_copy, "=", &next_pch);
	if (pch)
		esif_ccb_strcpy(key, pch, MAX_SEARCH_STRING);
	else
		return 1;

	pch = esif_ccb_strtok(NULL, "=", &next_pch);
	if (pch)
		esif_ccb_strcpy(value, pch, MAX_SEARCH_STRING);
	else
		return 1;

	return 0;
}

static int sysfs_get_string(char *path, char *filename, char *str)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fd, filepath, "r") != 0) {
		return ret;
	}
	
	ret = esif_ccb_fscanf(fd, "%s", SCANFBUF(str, MAX_SYSFS_PATH));
	esif_ccb_fclose(fd);

	return ret;
}

static int sysfs_get_string_multiline(char *path, char *filename, char *str)
{
	FILE *fp = NULL;
	int ret = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	size_t fileSize = 0;
	char lineStr[MAX_GUID_STR_LEN + 1] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fp, filepath, "r") != 0) {
		return ret;
	}

	*str = 0; // Initialize first character to ensure that concat works properly
	while (esif_ccb_fgets(lineStr, MAX_GUID_STR_LEN, fp)) {
		esif_ccb_sprintf_concat(MAX_SYSFS_STRING, str, "%s\n", lineStr);
		ret++;
	}

	esif_ccb_fclose(fp);

	return ret;
}


static int sysfs_get_u64(char *path, char *filename, u64 *p_u64)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);
	
	if (esif_ccb_fopen(&fd, filepath, "r") != 0) {
		return ret;
	}
	ret = esif_ccb_fscanf(fd, "%llu", (u64 *)p_u64);
	
	esif_ccb_fclose(fd);

	return 0;
}

static int sysfs_get_u64_direct(int fd, u64 *p_u64)
{
	char buf[MAX_SYSFS_POLL_STRING] = {0};

	lseek(fd, 0, SEEK_SET);
	if (read(fd, buf, MAX_SYSFS_POLL_STRING) > 0) {
		esif_ccb_sscanf(buf, "%llu", p_u64);
		return 0;
	}

	return -1;
}

static int sysfs_set_u64(char *path, char *filename, u64 val)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fd, filepath, "w") !=0) {
		return ret;
	}

	ret = esif_ccb_fprintf(fd, "%llu", (u64)val);
	esif_ccb_fclose(fd);

	return 0;
}

static int sysfs_set_string(char *path, char *filename, char *val)
{
        FILE *fd = NULL;
        int ret = -1;
        char filepath[MAX_SYSFS_PATH] = { 0 };

        esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

        if (esif_ccb_fopen(&fd, filepath, "w") !=0) {
                return ret;
        }

        ret = esif_ccb_fprintf(fd, "%s", val);
        esif_ccb_fclose(fd);

	return 0;
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
	
	if (sysfs_get_u64(participant_path, "max_state", &pdl_val) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}
	
	esif_ccb_sprintf(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%d,%d,mA,%d!",pcounter,placeholder_val,placeholder_val,placeholder_val,pcounter,pcounter,pcounter);
	for (pcounter = 1;pcounter <= pdl_val;pcounter++) {
		esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%d,%d,mA,%d!",pcounter,placeholder_val,placeholder_val,placeholder_val,pcounter,pcounter,pcounter);
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
	
	if (sysfs_get_u64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
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
		goto error_exit;
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
		goto error_exit;
	}
	/****************************/
	

	esif_ccb_sprintf(MAX_SYSFS_PATH, cur_path, "%s/%s/power_limits", path, node);
	if (sysfs_get_u64(cur_path, "power_limit_0_min_uw", &sysval) < 0) {
		goto error_exit;
	}
	pl1Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_max_uw",  &sysval) < 0) {
		goto error_exit;
	}
	pl1Max = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_step_uw", &sysval) < 0) {
		goto error_exit;
	}
	step1 = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_tmin_us", &sysval) < 0) {
		goto error_exit;
	}
	time1Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_0_tmax_us", &sysval) < 0) {
		goto error_exit;
	}
	time1Max = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_min_uw",  &sysval) < 0) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_max_uw",  &sysval) < 0) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Max = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_step_uw", &sysval) < 0) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	step2 = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_tmin_us", &sysval) < 0) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	time2Min = sysval;
	if (sysfs_get_u64(cur_path, "power_limit_1_tmax_us", &sysval) < 0) {
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
	
	return ESIF_OK;
	
error_exit:
	return ESIF_E_PRIMITIVE_ACTION_FAILURE;
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
		// Special case for ART - need to prepend revision number 2 to table
		len += snprintf(table_str + len, 4, "02:");
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
	
	if (NULL == acpi_name) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}
	target_participant = EsifUpPm_GetAvailableParticipantByName(acpi_name);
	if (NULL == target_participant) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}
	else {
		if (NULL == target_participant->fMetadata.fAcpiScope) {
			rc = ESIF_E_PARTICIPANT_NOT_FOUND;
			goto exit;
		}
		if (esif_ccb_strlen(target_participant->fMetadata.fAcpiScope,MAX_ACPI_SCOPE_LEN) < 1) {
			rc = ESIF_E_PARTICIPANT_NOT_FOUND;
			goto exit;
		}

		esif_ccb_sprintf(MAX_ACPI_SCOPE_LEN, acpi_scope, "%s", target_participant->fMetadata.fAcpiScope);
	}
	
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

/*
 *******************************************************************************
 * Register ACTION with ESIF
 *******************************************************************************
 */
extern EsifActMgr g_actMgr;


static EsifActType g_sysfs = {
	0,
	ESIF_ACTION_SYSFS,
	{ PAD },
	"SYSFS",
	"Sysfs Implementation",
	"ALL",
	"x1.0.0.1",
	{ 0 },
	ESIF_ACTION_IS_NOT_KERNEL_ACTION,
	ESIF_ACTION_IS_NOT_PLUGIN,
	{ PAD },
	ActionSysfsGet,
	ActionSysfsSet,
	NULL,
	NULL
};

enum esif_rc EsifActSysfsInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_sysfs);
		actionHashTablePtr = esif_ht_create(MAX_ACTION_HT_SIZE);
	}
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActSysfsExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
		if (actionHashTablePtr)
			esif_ht_destroy(actionHashTablePtr, ActionContextCleanUp);
	}
	ESIF_TRACE_EXIT_INFO();
}

#endif
/* end compile out entire page without ESIF_FEAT_OPT_SYSFS */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
