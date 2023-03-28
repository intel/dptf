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
#include "esif_uf_sysfs_os_lin.h"
#include "esif_ccb_cpuid.h"
#include <unistd.h>
#include <math.h>

#define ESIF_FIVR_PF_MULT                   128
#define ESIF_XTAL_CLOCK_FREQ_38_4	    38400000 // Hz
#define ESIF_FREQ_ADJ_STEP_15		    150000   // Hz
#define FIVR_CENTER_FREQ_MASK		    0xFF00C7FF
#define FIVR_MSB_SHIFT			    16
#define FIVR_MSB_MASK  			    0xFF
#define FIVR_LSB_SHIFT			    11
#define FIVR_LSB_MASK			    0x7
#define MIN_PERF_PERCENTAGE 0
#define MAX_SEARCH_STRING 50
#define MAX_PARAM_STRING (MAX_SEARCH_STRING + MAX_SEARCH_STRING + 1)
#define MAX_IDX_HOLDER 10
#define MAX_NODE_IDX 15
#define ERROR_VALUE 255
#define MAX_ESIF_TABLES 2
#define BINARY_TABLE_SIZE OUT_BUF_LEN
#define MAX_GUID_STR_LEN 40 // Assuming XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX format
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
#define SYSFS_FILE_RETRIEVAL_SUCCESS 1
#define MILLICELSIUS_PER_CELSIUS (1000)
#define ESIF_FIVR_SSC_MASK 0xFF
#define ESIF_FIVR_SSC_CLK_MASK 0x1
#define ESIF_FIVR_SSC_0_2_RES_MAX_REG_VAL 0x3f
#define ESIF_FIVR_SSC_0_1_RES_MAX_REG_VAL 0x1C
#define ESIF_FIVR_SSC_0_2_RES_MIN_REG_VAL 0x1D
#define ESIF_FIVR_SSC_ENABLE 0x100
#define ESIF_FIVR_SSC_0_1_RES 10
#define ESIF_FIVR_SSC_0_1_RES_MIN_PER 20
#define ESIF_FIVR_SSC_0_2_RES 20
#define ESIF_FIVR_SSC_0_2_RES_MIN_PER 320
#define RAPL_ENERGY_UNIT_BIT_POS 8
#define RAPL_ENERGY_UNIT_NBITS 5
#define ENERGY_UNIT_CONVERSION_FACTOR 1000000
#define GT_FREQ_STEP 100
#define MAX_GFX_PSTATE	64
#define MAX_FAN_PROPERTIES 101
#define MAX_RFI_BUSY_LOOP_COUNT 100

#define MAX_ACPI_SCOPE_LEN ESIF_SCOPE_LEN
#define ACPI_THERMAL_IOR_TYPE 's'
#define TRT_LEN _IOR(ACPI_THERMAL_IOR_TYPE, 1, u64)
#define ART_LEN _IOR(ACPI_THERMAL_IOR_TYPE, 2, u64)
#define TRT_COUNT _IOR(ACPI_THERMAL_IOR_TYPE, 3, u64)
#define ART_COUNT _IOR(ACPI_THERMAL_IOR_TYPE, 4, u64)
#define GET_TRT	_IOR(ACPI_THERMAL_IOR_TYPE, 5, u64)
#define GET_ART	_IOR(ACPI_THERMAL_IOR_TYPE, 6, u64)

#define ACPI_CPU                "INT3401:00"
#define GT_RP0_FREQ_MHZ         "gt_RP0_freq_mhz"
#define GT_RPN_FREQ_MHZ         "gt_RPn_freq_mhz"
#define GT_MIN_FREQ_MHZ         "gt_min_freq_mhz"
#define GT_MAX_FREQ_MHZ         "gt_max_freq_mhz"
#define SYSFS_PCI               "/sys/bus/pci/devices"
#define SYSFS_PLATFORM          "/sys/bus/platform/devices"
#define SYSFS_PSTATE_PATH       "/sys/devices/system/cpu/intel_pstate/"
#define SYSFS_PSTATE_TFN        "/sys/bus/acpi/devices/"
#define SYSFS_FIVR_PATH       	"/sys/bus/pci/devices/0000:00:04.0/fivr"
#define SYSFS_FIVR_NODE       	"rfi_vco_ref_code"
#define SYSFS_FIVR_PCH_PATH     "pch_fivr_switch_frequency"
#define SYSFS_FIVR_PCH_NODE_SET "freq_mhz_high_clock"
#define SYSFS_FIVR_PCH_NODE_GET "fivr_switching_freq_mhz"
#define SYSF_FIVR_PCH_SSC       "ssc_clock_info"
#define SYSF_FIVR_PCH_VER       "fivr_switching_fault_status"
#define SYSFS_THERMAL           "/sys/class/thermal"
#define SYSFS_EPP_PATH          "/sys/bus/cpu/devices/cpu%d/cpufreq"
#define SYSFS_GFX_PATH		"/sys/class/drm/card0/"
#define SYSFS_EPP_NODE          "energy_performance_preference"
#define SYSFS_DATA_VAULT        "data_vault"
#define SYSFS_AVAILABLE_UUIDS   "uuids/available_uuids"
#define SYSFS_CURRENT_UUID      "uuids/current_uuid"
#define SYSFS_OEM_VARIABLE      "odvp"
#define SYSFS_IMOK              "imok"

extern char g_ManagerSysfsPath[MAX_SYSFS_PATH];
extern char g_TCPUSysfsPath[MAX_SYSFS_PATH];

extern SystemCpuIndexTable g_systemCpuIndexTable;
static const char *CPU_location[] = {
	"0000:00:04.0",
	"0000:00:0b.0",
	"0000:00:00.1",
	NULL
};

static int *cpufreq; //Array storing Intercative Governor Cpu Frequencies

static int number_of_cores;

struct trt_table {
	char trt_source_device[8]; /* ACPI single name */
	char trt_target_device[8]; /* ACPI single name */
	u64 trt_influence;
	u64 trt_sample_period;
	u64 trt_reserved[4];
};

struct fan_properties {
	UInt64 control;
	UInt64 tripPoint;
	UInt64 speed;
	UInt64 noiseLevel;
	UInt64 power;
};

struct fan_properties g_fanProperties[MAX_FAN_PROPERTIES];
UInt32 g_isFanFpsSupported = 0xFFFFFFFF;
UInt32 g_isFanFineGrainSupported = 0xFFFFFFFF;
UInt32 g_fanStepSize = 0xFFFFFFFF;

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

struct eppWorkloadMapEntry {
	char workloadType[32];
	UInt32 eppValue;
	UInt32 percentage;
};
struct eppWorkloadMapEntry eppWorkloadMapTable[] = {
	{"none",128,50},
	{"bursty",128,50},
	{"semi-active",128,50},
	{"sustained",128,50}, 
	{"battery_life",179,70},
	{"idle",179,70}
};
// create graphix pstate table entry
typedef struct gfxPstateEntry {
	UInt32 pstate;
	UInt32 freqValue;
}gfxPstateEntry;
gfxPstateEntry g_gfxPstateFreqMapTable[MAX_GFX_PSTATE] = {0};

struct rfkill_event {
	u32 idx;
	u8 type;
	u8 operation;
	u8 soft, hard;
};

enum rfim_dvfs_point {
	DDR_DATA_RATE_P0_POS = 0,
	DDR_DATA_RATE_P1_POS = 10,
	DDR_DATA_RATE_P2_POS = 20,
	DDR_DATA_RATE_P3_POS = 30,
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
	ESIF_SYSFS_GET_CSTATE_RESIDENCY= 'RSCG',
	ESIF_SYSFS_GET_GFX_PSTATE = 'SPGG',
	ESIF_SYSFS_GET_RAPL_ENERGY_UNIT= 'UERG',
	ESIF_SYSFS_GET_RAPL_ENERGY= 'ERSG',
	ESIF_SYSFS_GET_RAPL_TIME_WINDOW = 'WTRG',
	ESIF_SYSFS_GET_PLATFORM_POWER_LIMIT_TIME_WINDOW = 'WTPG',
	ESIF_SYSFS_GET_TCC_OFFSET = 'CCTG',
	ESIF_SYSFS_GET_PLATFORM_MAX_BATTERY_POWER = 'XAMP',
	ESIF_SYSFS_GET_PLATFORM_POWER_SOURCE = 'CRSP',
	ESIF_SYSFS_GET_ADAPTER_POWER_RATING = 'GTRA',
	ESIF_SYSFS_GET_DDR_DVFS_DATA_RATE = 'RDDG',
	ESIF_SYSFS_GET_RFPROFILE_CENTER_FREQUENCY = 'FCRG',
	ESIF_SYSFS_GET_RFPROFILE_MAX_FREQUENCY = 'AMRG',
	ESIF_SYSFS_GET_RFPROFILE_MIN_FREQUENCY = 'IMRG',
	ESIF_SYSFS_GET_RFPROFILE_CENTER_FREQUENCY_PCH = 'PCRG',
	ESIF_SYSFS_GET_CHARGER_TYPE = 'PYTC',
	ESIF_SYSFS_GET_PLATFORM_BATTERY_STEADY_STATE = 'SSBP',
	ESIF_SYSFS_GET_PLATFORM_REST_OF_POWER = 'PORP',
	ESIF_SYSFS_GET_BATTERY_HIGH_FREQUENCY_IMPEDANCE = 'FHBR',
	ESIF_SYSFS_GET_BATTERY_CURRENT_DISCHARGE_CAPABILITY = 'PPMC',
	ESIF_SYSFS_GET_BATTERY_NO_LOAD_VOLTAGE = 'LNBV',
	ESIF_SYSFS_GET_PARTICIPANT_UNIQUE_ID = 'DIU_',
	ESIF_SYSFS_GET_RFPROFILE_SSC = 'FSRG',
	ESIF_SYSFS_GET_RFPROFILE_SSC_PCH = 'PSSG',
	ESIF_SYSFS_GET_BATTERY_MAX_PEAK_CURRENT = 'ppmc',
	ESIF_SYSFS_GET_RFPROFILE_FREQUENCY_ADJUST_RESOLUTION = 'RAFG',
	ESIF_SYSFS_GET_FIVR_VER_PCH = 'PVFG',
	ESIF_SYSFS_SET_CPU_PSTATE = 'SPCS',
	ESIF_SYSFS_SET_GFX_PSTATE = 'SPGS',
	ESIF_SYSFS_SET_DDR_DVFS_RFI_RESTRICTION = 'RRDS',
	ESIF_SYSFS_SET_RFPROFILE_CENTER_FREQUENCY = 'FCRS',
	ESIF_SYSFS_SET_RFPROFILE_CENTER_FREQUENCY_PCH = 'PCRS',
	ESIF_SYSFS_SET_WWAN_PSTATE = 'SPWS',
	ESIF_SYSFS_SET_OSC = 'CSOS',
	ESIF_SYSFS_SET_PBOK = 'KOBP',
	ESIF_SYSFS_SET_FAN_LEVEL = 'ELFS',
	ESIF_SYSFS_SET_BRIGHTNESS_LEVEL = 'ELBS',
	ESIF_SYSFS_SET_RAPL_TIME_WINDOW = 'WTRS',
	ESIF_SYSFS_SET_PLATFORM_POWER_LIMIT_TIME_WINDOW = 'WTPS',
	ESIF_SYSFS_SET_TCC_OFFSET = 'CCTS',
	ESIF_SYSFS_SET_IMOK = 'KOMI',
	ESIF_SYSFS_SET_EPP_WORKLOAD_TYPE = 'TWES',
};

enum esif_thermal_rel_type {
	ART = 0,
	TRT
};

#pragma pack(push, 1)
struct sysfsActionHashKey {
	struct esif_primitive_tuple primitiveTuple;
	esif_handle_t participantId;
	UInt32 actionPriority;
};

struct tzPolicy {
	char policy[MAX_SYSFS_PATH];
};

#pragma pack(pop)

extern thermalZonePtr g_thermalZonePtr;

static struct tzPolicy* tzPolicies = NULL;

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
static enum esif_rc GetGfxPerfSupportStates(EsifDataPtr responsePtr);
static enum esif_rc get_participant_current_control_capabilities(char *table_str, char *participant_path);
static enum esif_rc get_perf_support_states(char *table_str, char *participant_path);
static enum esif_rc get_supported_brightness_levels(char *table_str, char *participant_path);
static eEsifError get_participant_scope(char *acpi_name, char *acpi_scope);
static int SetActionContext(struct sysfsActionHashKey *keyPtr, EsifString devicePathName, EsifString deviceNodeName, Bool isWrite);
static struct esif_ht *actionHashTablePtr = NULL;
static char sys_long_string_val[MAX_SYSFS_STRING];
static eEsifError SetFanLevel(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr);
static eEsifError SetBrightnessLevel(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr);
static eEsifError SetEppWorkloadType(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr, const EsifString fileNamePtr);
static eEsifError GetFanInfo(EsifDataPtr responsePtr, const EsifString devicePathPtr);
static eEsifError GetFanPerfStates(const EsifString acpiDevName, EsifDataPtr responsePtr, const EsifString devicePathPtr);
static eEsifError EnumerateFpsEntries(EsifDataPtr responsePtr, const EsifString fpsPathPtr);
static eEsifError SetDdrRfiRestriction(UInt64 sysval, char *path);
static eEsifError GetDdrDvfsDataRate(EsifDataPtr responsePtr, char *path);
static eEsifError GetRfprofileFreqAdjuRes(EsifDataPtr responsePtr);
static eEsifError GetRfprofileCenterFreq(EsifDataPtr responsePtr, char *path, char *node);
static eEsifError GetFanStatus(EsifDataPtr responsePtr, const EsifString devicePathPtr);
static eEsifError GetDisplayBrightness(char *path, EsifDataPtr responsePtr);
static eEsifError GetCStateResidency(char *path, char *node, UInt32 msrAddr, EsifDataPtr responsePtr);
static eEsifError GetMsrValue(char *path, char *node, UInt32 msrAddr, EsifDataPtr responsePtr);
static UInt64 ExtractBits(UInt64 *number, UInt32 bitPos, UInt32 nBits);
static eEsifError GetRaplEnergyUnit(char *path, char *node, UInt32 msrAddr, EsifDataPtr responsePtr);
static eEsifError GetRaplRawEnergyInUnits(UInt64 energyUjs, UInt32 energyUnit, UInt32 *rawEnergyInUnits);
static eEsifError SetOsc(EsifUpPtr upPtr, const EsifDataPtr requestPtr);
static eEsifError SetFivrCenterFreqCpu(EsifUpPtr upPtr,UInt64 targetFreq);
static eEsifError SetFivrCenterFreqPch(EsifUpPtr upPtr,UInt64 targetFreq, char *path);
static eEsifError SetImOk(EsifUpPtr upPtr, const EsifDataPtr requestPtr);
static eEsifError ResetThermalZonePolicyToDefault();
static eEsifError SetThermalZonePolicy();
static eEsifError SetIntelPState(u64 val);
static eEsifError ValidateOutput(char *devicePathPtr, char *nodeName, u64 val);
static eEsifError GetGddvData(const EsifDataPtr responsePtr);
static eEsifError GetOemVariables(char *table_str);
static void UpdateFineGrainSupportedStatus(const EsifString devicePathPtr);
static void UpdateStepSize(const EsifString devicePathPtr);
static void UpdateFpsSupportedStatus(const EsifString devicePathPtr);


// Get crystal clock freq
static eEsifError GetCrystalClockFrequency(UInt32 *crystalFreq)
{
	static UInt32 crystalClockFrequency = 0;
	eEsifError rc = ESIF_OK;
	esif_ccb_cpuid_t cpuInfo = { 0 };

	// Check if it already obtained and use it
	if ( crystalClockFrequency != 0 ) {
		*(UInt32 *)crystalFreq = crystalClockFrequency;
		goto exit;
	}
	else {
		// First time - query through cpuid
		cpuInfo.leaf = ESIF_CPUID_LEAF_XTAL_CLOCK_FREQ_INFO;
		esif_ccb_cpuid(&cpuInfo);

		crystalClockFrequency = cpuInfo.ecx;
		*(UInt32 *)crystalFreq = crystalClockFrequency;
	}

exit:
	if (crystalClockFrequency != ESIF_XTAL_CLOCK_FREQ_38_4) {
		rc = ESIF_E_NOT_SUPPORTED;
		ESIF_TRACE_WARN("Unsupported crystal clock frequency: %d\n ",cpuInfo.ecx);
	}
    return rc;
}

static eEsifError GetProcessorSignature(UInt32 *cpuSign)
{
	eEsifError rc = ESIF_OK;
	esif_ccb_cpuid_t cpuInfo = { 0 };

	cpuInfo.leaf = ESIF_CPUID_LEAF_PROCESSOR_SIGNATURE;
	esif_ccb_cpuid(&cpuInfo);
	if(cpuInfo.eax == 0)
	{
		rc = ESIF_E_NOT_SUPPORTED;
		ESIF_TRACE_ERROR("Unsupported cpuid: ExtFamily ExtModel PType Family Stepping: %x \n ", cpuInfo.eax);
		goto exit;
	}

	/* masking stepping information from CPUID */
	*(UInt32 *)cpuSign = (cpuInfo.eax & 0xFFFFFFF0);
	ESIF_TRACE_INFO("cpuinfo: ExtFamily ExtModel PType Family: %x \n ", cpuInfo.eax);

exit :
        return rc;
}
/*
 * Inline function: GetUIntFromActionContext
 * -----------------------------------------
 * Read a 32-bit or 64-bit integer from the file that is associated to the context and return this value to the caller.
 * ESIF spawns multiple timer threads to read participant temperatures or performance states periodically.
 * The polling period could be quite frequent - for example, one thread polls the SoC temperature once every second,
 * per each of the 3 available domains. Such repeated open/read/close procedures would cause unnecessary system call
 * overhead. To reduce this call overhead, we open the associated node only once (twice actually) and then store the
 * corresponding file descriptor in a hash table. So on the next read, we can retrieve the file descriptor quickly
 * without going through the costly open/close calls.
 *
 * Here the context is the file descriptor that we have obtained from the hash table. File descriptors are unsigned integers
 * and since 0 is reserved for stdin, we first check if the descriptor is greater than 0. Once this is verified, we then
 * issue a read using this file descriptor.
 *
 * Most such reads return a 32-bit integer, however, there are certain primitives defined in the DSP files that require
 * 64-bit return values, among those, are the PC2 to PC10 primitives and the TSC primitive. We use the size of the
 * response buffer to tell what type of read it is, then cast the read value to the type of the return value.
 *
 * Most cached read will do a direct read from the sysfs node, for example, a temperature value. However for MSR reads, we
 * use the pread system call to read from a particular offset passed by the caller. The sysopt is used to distinguish
 * if the read is a direct read or a read from a certain offset.
 */

static ESIF_INLINE eEsifError GetUIntFromActionContext(const size_t context, const EsifString parm, const EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;
	Int64 sysval = 0;

	if (context > 0) { // valid file descriptor pointing to an open sysfs node or dev node
		if (parm != NULL) { // parm is the offset
			long msrAddr = strtol(parm, NULL, 0);
			if (pread(context, &sysval, sizeof(UInt64), msrAddr) != sizeof(UInt64)) {
				ESIF_TRACE_WARN("Failed to get action context, will attemp to read directly the device file\n");
				rc = ESIF_E_IO_ERROR;
				goto exit;
			}
		} else { // parm is not used, do a direct read
			if (SysfsGetInt64Direct((int) context, &sysval) <= 0) {
				ESIF_TRACE_WARN("Failed to get action context, will attemp to read directly from the sysfs\n");
				rc = ESIF_E_UNSPECIFIED;
				goto exit;
			}
		}
		if (responsePtr->buf_len < sizeof(u64)) {
			*(u32 *) responsePtr->buf_ptr = (u32) sysval;
		} else {
			*(u64 *) responsePtr->buf_ptr = sysval;
		}
	} else {
		rc = ESIF_E_INVALID_HANDLE;
	}

exit:
    return rc;
}


static ESIF_INLINE eEsifError SetUIntFromActionContext(const size_t context, Int32 val)
{
	eEsifError rc = ESIF_OK;

	if (context > 0) { // valid file descriptor pointing to an open sysfs node or dev node
		if (SysfsSetInt64Direct((int)context, val) < 0) {
			ESIF_TRACE_WARN("Failed to set action context, will attemp to read directly from the sysfs\n");
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
	} 
	else {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

exit:
    return rc;
}

// Checking the supported Capability 
static Bool IsCapabilitySupportedInDomain(const EsifUpDomainPtr domainPtr, enum esif_capability_type capabilityType)
{
	if (domainPtr->capability_for_domain.capability_flags & (1 << capabilityType)) {
		return ESIF_TRUE;
	}
	else {
		return ESIF_FALSE;
    }
}

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
	EsifString acpiDeviceName = NULL;
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
	UInt32 msrAddr = 0;
	Int64 sysval = 0;
	u64 tripval = 0;
	int node_idx = 0;
	int node2_idx = 0;
	int max_node_idx = MAX_NODE_IDX;
	enum esif_sysfs_command sysopt = 0;
	int cur_item_count = 0;
	UInt32 crystalClockFreq = 0;
	int target_item_count = 0;
	enum esif_sysfs_param calc_type = 0;
	Int64 pdl_val = 0;
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
	char table_str[BINARY_TABLE_SIZE];
	TableObject tableObject = {0};
	struct sysfsActionHashKey key = {0};
	size_t actionContext = 0;
	EsifUpDataPtr metaPtr = NULL;
	char batPwrSysfsPath[MAX_SYSFS_PATH] = { 0 };
	char participantUidSysfsPath[MAX_SYSFS_PATH] = { 0 };
	UInt32 sscPercentage = 0;
	UInt32 sscRegisterValue = 0;
	static UInt32 raplEnergyUnit = 0;
	UInt32 rawEnergyInUnits = 0;
	EsifData raplEunit = {ESIF_DATA_UINT32, &raplEnergyUnit, sizeof(raplEnergyUnit), sizeof(raplEnergyUnit) };
	Bool isWrite = ESIF_FALSE;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(requestPtr);

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
	acpiDeviceName = (EsifString)metaPtr->fAcpiDevice;

	if (NULL == devicePathPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("Failed to get device path ptr.\n");
		goto exit;
	}

	// Assemble hash table key to look for existing file pointer to sysfs node
	key.participantId = EsifUp_GetInstance(upPtr);
	key.primitiveTuple = primitivePtr->tuple;
	key.actionPriority = fpcActionPtr->priority;
	actionContext = (size_t) esif_ht_get_item(actionHashTablePtr, (u8 *)&key, sizeof(key));
	sysopt = *(enum esif_sysfs_command *) command;

	switch (sysopt) {
	case ESIF_SYSFS_DIRECT_PATH:
		if (ESIF_OK == GetUIntFromActionContext(actionContext, NULL, responsePtr))
			goto exit;

		if (0 == esif_ccb_strcmp(parm2, "alt") && deviceAltPathPtr != NULL) {
			deviceTargetPathPtr = deviceAltPathPtr;
		}
		else {
			deviceTargetPathPtr = devicePathPtr;
		}

		pathAccessReturn = SysfsGetInt64(devicePathPtr, parm1, &sysval);
		if (pathAccessReturn < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_WARN("Failed to get value from path: %s/%s . Error: %d \n",devicePathPtr,parm1,pathAccessReturn);
			goto exit;
		}
		if (SetActionContext(&key, deviceTargetPathPtr, parm1, isWrite)) {
			ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT ", primitive %d, domain %d, instance %d\n",
				esif_ccb_handle2llu(key.participantId), key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
		}
		tripval = sysval;
		*(u32 *) responsePtr->buf_ptr = (u32) tripval;
		break;
	case ESIF_SYSFS_DIRECT_ENUM:
		if (ESIF_OK == GetUIntFromActionContext(actionContext, NULL, responsePtr))
			goto exit;

		candidate_found = 0;
		for (node_idx = 0; node_idx < max_node_idx; node_idx++) {
			esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node_idx);
			if (replace_str(parm1, "%i%", idx_holder, cur_node_name, MAX_SYSFS_PATH) > 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
			if (SysfsGetInt64(devicePathPtr, cur_node_name, &sysval) > 0 && sysval > 0) {
				if (ESIF_OK != ValidateOutput(devicePathPtr, cur_node_name, sysval))
					continue;

				candidate_found = 1;
				if (SetActionContext(&key, devicePathPtr, cur_node_name, isWrite)) {
					ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT ", primitive %d, domain %d, instance %d\n",
						esif_ccb_handle2llu(key.participantId), key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
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
		if (ESIF_OK == GetUIntFromActionContext(actionContext, NULL, responsePtr))
			goto exit;

		if (SysfsGetInt64(parm1, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		if (SetActionContext(&key, parm1, parm2, isWrite)) {
			ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT " primitive %d, domain %d, instance %d\n",
				esif_ccb_handle2llu(key.participantId), key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
		}
		if (responsePtr->buf_len < sizeof(u64)) {
			*(u32 *) responsePtr->buf_ptr = (u32) sysval;
		} else {
			*(u64 *) responsePtr->buf_ptr = sysval;
		}
		break;
	case ESIF_SYSFS_DIRECT_QUERY:
		if (ESIF_OK == GetUIntFromActionContext(actionContext, NULL, responsePtr))
			goto exit;

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
			if (SysfsGetString(devicePathPtr, cur_node_name, sysvalstring, sizeof(sysvalstring)) > -1) {
				if (esif_ccb_stricmp(parm2, sysvalstring) == 0) {
					if (SysfsGetInt64(devicePathPtr, alt_node_name, &sysval) > 0 && sysval > 0 && node_idx >= min_idx) {
						if (SetActionContext(&key, devicePathPtr, alt_node_name, isWrite)) {
							ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT ", primitive %d, domain %d, instance %d\n",
								esif_ccb_handle2llu(key.participantId), key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
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
		if (ESIF_OK == GetUIntFromActionContext(actionContext, NULL, responsePtr))
			goto exit;
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

			if (SysfsGetString(cur_node_name, srchnm, sysvalstring, sizeof(sysvalstring)) > -1) {
				if (esif_ccb_stricmp(srchval, sysvalstring) == 0) {
					for (node2_idx = 0; node2_idx < max_node_idx; node2_idx++) {
						char *node_name_ptr = parm3;
						esif_ccb_sprintf(MAX_IDX_HOLDER, idx_holder, "%d", node2_idx);
						if (replace_str(parm3, "%i%", idx_holder, alt_node_name, MAX_SYSFS_PATH) == 0) {
							// replacement is successful, set node name pointer to alt
							node_name_ptr = alt_node_name;
						}
						if (SysfsGetInt64(cur_node_name, node_name_ptr, &sysval) > 0 && sysval > 0) {
							if (SetActionContext(&key, cur_node_name, node_name_ptr, isWrite)) {
								ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT ", primitive %d, domain %d, instance %d\n",
									esif_ccb_handle2llu(key.participantId), key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
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
		// Most CALC type reads happen very infrequently, also the read-back value is often massaged before they
		// are sent back to ESIF. Thereof we do not call GetUIntFromActionContext() blankly.
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

				if (SysfsGetInt64(parm1, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
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
			case ESIF_SYSFS_GET_PLATFORM_POWER_LIMIT_TIME_WINDOW:
			case ESIF_SYSFS_GET_RAPL_TIME_WINDOW: /* PL1 time window (Tau) */
				if (SysfsGetInt64(parm1, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}

				/* Convert microseconds to milliseconds after round off*/
				sysval = (sysval + 0.5) / 1000;
				*(u32 *) responsePtr->buf_ptr = (u32) sysval;
				break;
			case ESIF_SYSFS_GET_CPU_PDL: /* pdl */
				if (SysfsGetString(SYSFS_PSTATE_PATH, "num_pstates", sysvalstring, sizeof(sysvalstring)) > -1) {
					if ((SysfsGetInt64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < SYSFS_FILE_RETRIEVAL_SUCCESS) || (pdl_val > MAX_SYSFS_PSTATES)) {
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
			case ESIF_SYSFS_GET_GFX_PSTATE:
				rc = GetGfxPerfSupportStates(responsePtr);
				if (rc != ESIF_OK) {
					goto exit;
				}
				break;
			case ESIF_SYSFS_GET_SOC_PL1: /* power limit */
				if (SysfsGetInt64(parm1, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
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
				// Checking the TEMP_STATUS capability for Domain0
				if (IsCapabilitySupportedInDomain(Domain0,ESIF_CAPABILITY_TYPE_TEMP_STATUS)) {
					EsifPrimitiveTuple temp0Tuple = {GET_TEMPERATURE, Domain0->domain, 255};
					if (EsifUp_ExecutePrimitive(upPtr, &temp0Tuple, requestPtr, responsePtr)) {
						temp_val0 = ESIF_SDK_MIN_AUX_TRIP;
					} 
					else {
						temp_val0 = *((u32 *)responsePtr->buf_ptr);
					}
				}
				else {
					temp_val0=ESIF_SDK_MIN_AUX_TRIP;
				}
				// Checking the TEMP_STATUS capability for Domain1
				if (IsCapabilitySupportedInDomain(Domain1,ESIF_CAPABILITY_TYPE_TEMP_STATUS)) {
					EsifPrimitiveTuple temp1Tuple = {GET_TEMPERATURE, Domain1->domain, 255};
					if (EsifUp_ExecutePrimitive(upPtr, &temp1Tuple, requestPtr, responsePtr)) {
						temp_val1 = ESIF_SDK_MIN_AUX_TRIP;
					} 
					else {
						temp_val1 = *((u32 *) responsePtr->buf_ptr);
					}
				}
				else {
					temp_val1 = ESIF_SDK_MIN_AUX_TRIP;
				}
				// Even if there is only one sub-domain that has a valid temperature, we will take it
				if ((temp_val0 >= 0) || (temp_val1 >=0)) {
					temp_val0 = esif_ccb_max(temp_val0, temp_val1);
					esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_MILLIC, (esif_temp_t *)&temp_val0);
				} 
				else {
					// if both sub-domain has invalid temperature then we should disable the capability
					rc = ESIF_E_NOT_SUPPORTED;
				        goto exit;
				}
				*(u32 *)responsePtr->buf_ptr = temp_val0;
				break;
				//actions support for power Participant Data
			case ESIF_SYSFS_GET_PLATFORM_POWER_SOURCE:
			case ESIF_SYSFS_GET_PLATFORM_MAX_BATTERY_POWER:
			case ESIF_SYSFS_GET_ADAPTER_POWER_RATING:
			case ESIF_SYSFS_GET_PLATFORM_REST_OF_POWER:
			case ESIF_SYSFS_GET_CHARGER_TYPE:
			case ESIF_SYSFS_GET_BATTERY_HIGH_FREQUENCY_IMPEDANCE:
			case ESIF_SYSFS_GET_BATTERY_MAX_PEAK_CURRENT:
			case ESIF_SYSFS_GET_PLATFORM_BATTERY_STEADY_STATE:
			case ESIF_SYSFS_GET_BATTERY_CURRENT_DISCHARGE_CAPABILITY:
			case ESIF_SYSFS_GET_BATTERY_NO_LOAD_VOLTAGE:
				if (esif_ccb_strcmp("ipf_pwr",upPtr->fDspPtr->code_ptr) == 0) {
					esif_ccb_sprintf(sizeof(batPwrSysfsPath),batPwrSysfsPath, "%s/%s",deviceFullPathPtr,"dptf_power");
				}
				else if (esif_ccb_strcmp("ipf_bat",upPtr->fDspPtr->code_ptr) == 0) {
					esif_ccb_sprintf(sizeof(batPwrSysfsPath), batPwrSysfsPath, "%s/%s",deviceFullPathPtr,"dptf_battery");
				}

				if ( esif_ccb_strlen(batPwrSysfsPath, sizeof(batPwrSysfsPath)) == 0) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					ESIF_TRACE_WARN("batPwrSysfsPath Invalid\n");
					goto exit;
				}

				if (SysfsGetInt64(batPwrSysfsPath, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				*(u32 *) responsePtr->buf_ptr = (u32) sysval;
                                break;
			case ESIF_SYSFS_GET_PARTICIPANT_UNIQUE_ID:
				if(esif_ccb_strstr(deviceFullPathPtr, "cooling_device") || esif_ccb_strstr(deviceFullPathPtr, "thermal_zone"))
				{
					esif_ccb_sprintf(sizeof(participantUidSysfsPath),participantUidSysfsPath, "%s/device/%s", deviceFullPathPtr, "firmware_node");
				}
				else {
					esif_ccb_sprintf(sizeof(participantUidSysfsPath),participantUidSysfsPath, "%s/%s", deviceFullPathPtr, "firmware_node");
				}
				if (SysfsGetString(participantUidSysfsPath, parm2, sysvalstring, sizeof(sysvalstring)) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				responsePtr->data_len = esif_ccb_strlen(sysvalstring, MAX_SYSFS_PATH);
				if (responsePtr->buf_len < responsePtr->data_len) {
					rc = ESIF_E_NEED_LARGER_BUFFER;
					goto exit;
				}
				esif_ccb_sprintf(responsePtr->buf_len, (char *) responsePtr->buf_ptr, "%s", sysvalstring);
				break;
			case ESIF_SYSFS_GET_RFPROFILE_SSC:
				if (SysfsGetInt64(parm1, "spread_spectrum_clk_enable" , &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_INVALID_HANDLE;
					ESIF_TRACE_WARN("Invalid sysfs path\n");
					goto exit;
				}

				sscRegisterValue =  (UInt32)sysval;
				if(!(sscRegisterValue & ESIF_FIVR_SSC_CLK_MASK)) {
					rc = ESIF_E_DISABLED;
					ESIF_TRACE_WARN("Optional support disabled\n");
					goto exit;
				}

				if (SysfsGetInt64(parm1, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}

				sscRegisterValue = (UInt32)sysval & ESIF_FIVR_SSC_MASK;
				if(sscRegisterValue > ESIF_FIVR_SSC_0_2_RES_MAX_REG_VAL) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					ESIF_TRACE_ERROR("Error value %d is out of bounds\n", sscRegisterValue);
					goto exit;
				}

				if (sscRegisterValue <= ESIF_FIVR_SSC_0_1_RES_MAX_REG_VAL) {
					sscPercentage = (sscRegisterValue * ESIF_FIVR_SSC_0_1_RES) + ESIF_FIVR_SSC_0_1_RES_MIN_PER;
				}
				else {
					sscPercentage = ((sscRegisterValue - ESIF_FIVR_SSC_0_2_RES_MIN_REG_VAL) * ESIF_FIVR_SSC_0_2_RES) + ESIF_FIVR_SSC_0_2_RES_MIN_PER;
				}
				*(UInt32 *) responsePtr->buf_ptr = (UInt32) sscPercentage;
				break;
			case ESIF_SYSFS_GET_RFPROFILE_SSC_PCH:
			 	esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name,"%s/%s", devicePathPtr, SYSFS_FIVR_PCH_PATH);
				if (SysfsGetInt64(cur_node_name, SYSF_FIVR_PCH_SSC, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					ESIF_TRACE_WARN("Invalid Sysfs Path \n");
					goto exit;
				}

				sscRegisterValue = (UInt32)sysval;
				if (!(sscRegisterValue & ESIF_FIVR_SSC_ENABLE)) { 
					rc = ESIF_E_DISABLED; 
					goto exit; 
				}

				sscRegisterValue = (UInt32)sysval & ESIF_FIVR_SSC_MASK;
			        if(sscRegisterValue > ESIF_FIVR_SSC_0_2_RES_MAX_REG_VAL) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					ESIF_TRACE_ERROR("Error value %d is out of bounds\n", sscRegisterValue);
					goto exit;
				}

				if (sscRegisterValue <= ESIF_FIVR_SSC_0_1_RES_MAX_REG_VAL) {
					sscPercentage = (sscRegisterValue * ESIF_FIVR_SSC_0_1_RES) + ESIF_FIVR_SSC_0_1_RES_MIN_PER;
				}
				else {
					sscPercentage = ((sscRegisterValue - ESIF_FIVR_SSC_0_2_RES_MIN_REG_VAL) * ESIF_FIVR_SSC_0_2_RES) + ESIF_FIVR_SSC_0_2_RES_MIN_PER;
				}
				*(UInt32 *) responsePtr->buf_ptr = (UInt32) sscPercentage;
				break;
			case ESIF_SYSFS_GET_FIVR_VER_PCH:
				esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name,"%s/%s", devicePathPtr, SYSFS_FIVR_PCH_PATH);
				if (SysfsGetInt64(cur_node_name, SYSF_FIVR_PCH_VER, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					ESIF_TRACE_WARN("Invalid Sysfs Path \n");
					goto exit;
				}
				*(UInt32 *) responsePtr->buf_ptr = (UInt32) sysval;
				break;
			case ESIF_SYSFS_GET_RFPROFILE_MAX_FREQUENCY:
				rc = GetCrystalClockFrequency(&crystalClockFreq);
				if (rc != ESIF_OK) {
					goto exit;
				}
				
				//Get the Maximum frequency
				EsifPrimitiveTuple maxFreqTuple = {GET_RFPROFILE_MAX_FREQUENCY_XTAL_38_4, ESIF_PRIMITIVE_DOMAIN_D0, 255};
				if (!EsifUp_ExecutePrimitive(upPtr, &maxFreqTuple, NULL, responsePtr)) {
					ESIF_TRACE_DEBUG("Successfully get the Maximume frequency:\n");
				}
				break;
			case ESIF_SYSFS_GET_RFPROFILE_MIN_FREQUENCY:
				rc = GetCrystalClockFrequency(&crystalClockFreq);
				if (rc != ESIF_OK) {
					goto exit;
				}
				//Get the Maximum frequency
				EsifPrimitiveTuple minFreqTuple = {GET_RFPROFILE_MIN_FREQUENCY_XTAL_38_4, ESIF_PRIMITIVE_DOMAIN_D0, 255};
				if (!EsifUp_ExecutePrimitive(upPtr, &minFreqTuple, NULL, responsePtr)) {
					ESIF_TRACE_DEBUG("Successfully get the Minimume frequency:\n");
				}
				break;	
			case ESIF_SYSFS_GET_DDR_DVFS_DATA_RATE:
				rc = GetDdrDvfsDataRate(responsePtr, parm1);
				break;
			case ESIF_SYSFS_GET_RFPROFILE_CENTER_FREQUENCY_PCH:// Center frequency for PCH participant
				esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name,"%s/%s", devicePathPtr, SYSFS_FIVR_PCH_PATH);
				rc = GetRfprofileCenterFreq(responsePtr, cur_node_name, SYSFS_FIVR_PCH_NODE_GET);
				break;
			case ESIF_SYSFS_GET_RFPROFILE_CENTER_FREQUENCY:// Center frequency for TCPU participant
				rc = GetRfprofileCenterFreq(responsePtr, SYSFS_FIVR_PATH, SYSFS_FIVR_NODE);
				break;
			case ESIF_SYSFS_GET_RFPROFILE_FREQUENCY_ADJUST_RESOLUTION:
				rc = GetRfprofileFreqAdjuRes(responsePtr);
				break;
			case ESIF_SYSFS_GET_FAN_INFO:
                rc = GetFanInfo(responsePtr, devicePathPtr);
                break;
			case ESIF_SYSFS_GET_FAN_PERF_STATES:
				rc = GetFanPerfStates(acpiDeviceName, responsePtr, devicePathPtr);
				break;
			case ESIF_SYSFS_GET_FAN_STATUS:
				rc = GetFanStatus(responsePtr, devicePathPtr);
				break;
			case ESIF_SYSFS_GET_DISPLAY_BRIGHTNESS:
				rc = GetDisplayBrightness(parm1,responsePtr);
				break;
			case ESIF_SYSFS_GET_CSTATE_RESIDENCY:
				// Due to the frequent CSTATE_RESIDENCY queries, we want to try the hash table first
				// to look for a cached file descriptor to /dev/cpu/0/msr. We must also
				// re-evaluate the action context because all various instances of C-state residency
				// (PC2 - PC10) share the same file descriptor.
				key.primitiveTuple.instance = 255; // Do no care - all PCx reads share the same file path
				actionContext = (size_t) esif_ht_get_item(actionHashTablePtr, (u8 *)&key, sizeof(key));
				if (ESIF_OK == GetUIntFromActionContext(actionContext, parm4, responsePtr))
					goto exit;

				msrAddr = (UInt32) strtol(parm4, NULL, 0);
				if (msrAddr <= 0) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					break;
				}
				rc = GetCStateResidency(parm1, parm2, msrAddr, responsePtr);
				if (ESIF_OK == rc) {
					if (SetActionContext(&key, parm1, parm2, isWrite)) {
						ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT ", primitive %d, domain %d, instance %d\n",
							esif_ccb_handle2llu(key.participantId),
							key.primitiveTuple.id,
							key.primitiveTuple.domain,
							key.primitiveTuple.instance);
					}
				}
				break;
			case ESIF_SYSFS_GET_RAPL_ENERGY_UNIT:
				msrAddr = (UInt32) strtol(parm4, NULL, 0);
				if (msrAddr <= 0) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					break;
				}
				rc = GetRaplEnergyUnit(parm1, parm2, msrAddr, responsePtr);
				raplEnergyUnit = *(UInt32 *) responsePtr->buf_ptr;
				break;
			case ESIF_SYSFS_GET_RAPL_ENERGY:
				if (SysfsGetInt64(parm1, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				if (!raplEnergyUnit) {
					EsifPrimitiveTuple tuple0 = {GET_RAPL_ENERGY_UNIT, ESIF_PRIMITIVE_DOMAIN_D0, 255};
					rc = EsifUp_ExecutePrimitive(upPtr, &tuple0, requestPtr, &raplEunit);
					if (ESIF_OK != rc) {
						goto exit;
					}
				}
				rc = GetRaplRawEnergyInUnits(sysval, raplEnergyUnit, &rawEnergyInUnits);
				*(u32 *) responsePtr->buf_ptr = rawEnergyInUnits;
				break;
			case ESIF_SYSFS_GET_TCC_OFFSET: /* TCC Offset */
				if (SysfsGetInt64(parm1, parm2, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
					ESIF_TRACE_ERROR("Error retrieving TCC Offset value from sysfs.\n");
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				ESIF_TRACE_INFO("TCC Offset value from sysfs : %u\n", sysval);
				/* Convert Celsius to millicelsius */
				sysval *= MILLICELSIUS_PER_CELSIUS;
				*(u32 *) responsePtr->buf_ptr = (u32) sysval;
				break;
			default:
				break;
		}
		break;
	case ESIF_SYSFS_ATTRIBUTE:
		if (devicePathPtr != NULL) {
			esif_ccb_sprintf(MAX_SYSFS_PATH, (char *) responsePtr->buf_ptr, "%s", devicePathPtr);
		}
		break;
	case ESIF_SYSFS_DIRECT_QUERY_ENUM:
		if (ESIF_OK == GetUIntFromActionContext(actionContext, NULL, responsePtr))
			goto exit;

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
			if (SysfsGetString(devicePathPtr, cur_node_name, sysvalstring, sizeof(sysvalstring)) > -1) {
				if (esif_ccb_stricmp(srchnm, sysvalstring) == 0) {
					if (cur_item_count == target_item_count) {
						if (SysfsGetInt64(devicePathPtr, alt_node_name, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
							rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
							goto exit;
						}
						else {
							rc = ESIF_OK;
						}
						if (SetActionContext(&key, devicePathPtr, alt_node_name, isWrite)) {
							ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT ", primitive %d, domain %d, instance %d\n",
								esif_ccb_handle2llu(key.participantId), key.primitiveTuple.id, key.primitiveTuple.domain, key.primitiveTuple.instance);
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
		if (esif_ccb_stricmp("gddv", parm1) == 0) {
			rc = GetGddvData(responsePtr);
			if ( rc != ESIF_OK) {
				ESIF_TRACE_WARN("Error Retrieving GDDV Data");
			}
			break;
		}
		esif_ccb_memset(table_str, 0, BINARY_TABLE_SIZE);
		/*	domain str and participant id are not relevant in this
			case since we use device paths */
		TableObject_Construct(&tableObject, parm1, "D0", NULL, NULL, NULL, 0, 0, GET);
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
			if ( esif_ccb_strlen(g_ManagerSysfsPath, sizeof(g_ManagerSysfsPath)) == 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				ESIF_TRACE_ERROR("g_ManagerSysfsPath Invalid\n");
			}
			else {
				int lineNum = SysfsGetStringMultiline(g_ManagerSysfsPath, SYSFS_AVAILABLE_UUIDS, sys_long_string_val);
				FLAGS_CLEAR(tableObject.options, TABLEOPT_ALLOW_SELF_DEFINE);
				rc = get_supported_policies(table_str, lineNum, sys_long_string_val);
			}
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
		else if (esif_ccb_stricmp("odvp", parm1) == 0) {
			rc = GetOemVariables(table_str);
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
	char workloadType[MAX_SYSFS_PATH] = { 0 };
	Int32 cpuCore = 0;
	eEsifError rc = ESIF_OK;
	EsifData params[5] = {0};
	EsifString replacedStrs[5] = {0};
	EsifString replacedStr = NULL;
	UInt8 i = 0;
	enum esif_sysfs_command sysopt = 0;

	char srchnm[MAX_SEARCH_STRING] = { 0 };
	char srchval[MAX_SEARCH_STRING] = { 0 };
	enum esif_sysfs_param calc_type = 0;
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char idx_holder[MAX_IDX_HOLDER] = { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	struct sysfsActionHashKey key = {0};
	Bool isWrite = ESIF_TRUE;
	Bool isValidWorkloadType = ESIF_FALSE;
	size_t actionContext = 0;
	int node_idx = 0;
	int candidate_found = 0;
	int max_node_idx = MAX_NODE_IDX;
	Int64 sysval = 0;
	Int64 gtMinFreq = 0;
	Int32 eppValue = 0;
	Int64 pdl_val = 0;
	EsifUpDataPtr metaPtr = NULL;
	int rfkill_fd = 0;
	struct rfkill_event event = {0};
	int core = 0;
	UInt64 responseData = 0;
	EsifData response = { ESIF_DATA_UINT64, &responseData, sizeof(responseData), sizeof(responseData) };
	UInt64 maxFreq = 0;
	UInt64 minFreq = 0;

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

	if (NULL == devicePathPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("Failed to get device path ptr.\n");
		goto exit;
	}

	sysopt = *(enum esif_sysfs_command *) command;
	switch (sysopt) {
	case ESIF_SYSFS_DIRECT_PATH:
		if (0 == esif_ccb_strcmp(parm2, "alt") && deviceAltPathPtr != NULL) {
			deviceTargetPathPtr = deviceAltPathPtr;
		}
		else {
			deviceTargetPathPtr = devicePathPtr;
		}

		if (SysfsSetInt64(deviceTargetPathPtr, parm1, *(Int32 *) requestPtr->buf_ptr) < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		break;
	case ESIF_SYSFS_ALT_PATH:
		sysval = (u64)*(u32 *)requestPtr->buf_ptr;
		if (SysfsSetInt64(parm1, parm2, sysval) < 0) {
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
			if (SysfsGetString(cur_node_name, srchnm, sysvalstring, sizeof(sysvalstring)) > -1) {
				if (esif_ccb_stricmp(srchval, sysvalstring) == 0) {
					if (SysfsSetInt64(cur_node_name, parm3, *(u32 *) requestPtr->buf_ptr) == 0) {
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

				if (SysfsGetString(SYSFS_PSTATE_PATH, "num_pstates", sysvalstring, sizeof(sysvalstring)) > -1) {
					rc = SetIntelPState(sysval);
				}
				else {
					pdl_val = GetCpuFreqPdl();
					if ((sysval <= pdl_val) && (cpufreq != NULL)) {
						for(core = 0; core < (number_of_cores+1); core++) {
							char cpuString[MAX_SYSFS_PATH] = {0};
							esif_ccb_sprintf(MAX_SYSFS_PATH, cpuString,"/sys/devices/system/cpu/cpu%d/cpufreq", core);
							if (SysfsSetInt64(cpuString, "scaling_max_freq", cpufreq[sysval]) < 0) {
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
			case ESIF_SYSFS_SET_GFX_PSTATE:
				sysval = *(UInt32*) requestPtr->buf_ptr;
				if (sysval < 0 || sysval >= MAX_GFX_PSTATE) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					ESIF_TRACE_DEBUG(" Index value is out of bound: %d\n",sysval);
					goto exit;
				}

				if (g_gfxPstateFreqMapTable[sysval].freqValue <= 0) {
					  rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					  ESIF_TRACE_DEBUG("P state is not supported for Index %d: \n", sysval);
					  goto exit;
				}
				if (SysfsGetInt64(SYSFS_GFX_PATH, GT_MIN_FREQ_MHZ, &gtMinFreq) < 0) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				if (gtMinFreq <= g_gfxPstateFreqMapTable[sysval].freqValue) {
					if (SysfsSetInt64(SYSFS_GFX_PATH,GT_MAX_FREQ_MHZ, g_gfxPstateFreqMapTable[sysval].freqValue) < 0) {
						rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					}
					if (SysfsSetInt64(SYSFS_GFX_PATH,GT_MIN_FREQ_MHZ, g_gfxPstateFreqMapTable[sysval].freqValue) < 0) {
						rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					}
				}
				else {
					if (SysfsSetInt64(SYSFS_GFX_PATH,GT_MIN_FREQ_MHZ, g_gfxPstateFreqMapTable[sysval].freqValue) < 0) {
                                                rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
                                        }
                                        if (SysfsSetInt64(SYSFS_GFX_PATH,GT_MAX_FREQ_MHZ, g_gfxPstateFreqMapTable[sysval].freqValue) < 0) {
                                                rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
                                        }
				}
				if (rc != ESIF_OK) {
					goto exit;
				}
				ESIF_TRACE_DEBUG("Successfully set the GFX Pstate Value: %d\n", g_gfxPstateFreqMapTable[sysval].freqValue);
				break;
			case ESIF_SYSFS_SET_DDR_DVFS_RFI_RESTRICTION:
				sysval = *(UInt64 *) requestPtr->buf_ptr;
				rc = SetDdrRfiRestriction(sysval, parm1);
				break;
			case ESIF_SYSFS_SET_RFPROFILE_CENTER_FREQUENCY:
				sysval = *(UInt64 *) requestPtr->buf_ptr;
				//Get the Maximum frequency
				EsifPrimitiveTuple tuple0 = {GET_RFPROFILE_MAX_FREQUENCY, ESIF_PRIMITIVE_DOMAIN_D0, 255};
				if (!EsifUp_ExecutePrimitive(upPtr, &tuple0, NULL, &response)) {
					maxFreq = (UInt64)*(UInt32 *)response.buf_ptr;
				}

				//Get the min frequency
				EsifPrimitiveTuple tuple1 = {GET_RFPROFILE_MIN_FREQUENCY, ESIF_PRIMITIVE_DOMAIN_D0, 255};
				if (!EsifUp_ExecutePrimitive(upPtr, &tuple1, NULL, &response)) {
					minFreq = (UInt64)*(UInt32 *) response.buf_ptr;
				}
				//comparing the input value with max and min value
				if ((sysval > maxFreq) || (sysval < minFreq)) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					ESIF_TRACE_ERROR("sysvl value: %lld should be in the range of min value: %lld or max value: %lld \n",
						sysval,
						minFreq,
						maxFreq);
					goto exit;
				}
				rc = SetFivrCenterFreqCpu(upPtr, sysval);
				if ( rc != ESIF_OK) {
					ESIF_TRACE_ERROR("Failed to set the FIVR center freq\n");
					goto exit;
				}
				break;
			case ESIF_SYSFS_SET_RFPROFILE_CENTER_FREQUENCY_PCH:
				sysval = *(UInt64 *) requestPtr->buf_ptr;
				//Get the Maximum frequency
				EsifPrimitiveTuple maxFreqTuple = {GET_RFPROFILE_MAX_FREQUENCY, ESIF_PRIMITIVE_DOMAIN_D0, 255};
				if (!EsifUp_ExecutePrimitive(upPtr, &maxFreqTuple, NULL, &response)) {
					maxFreq = (UInt64)*(UInt32 *)response.buf_ptr;
				}

				//Get the min frequency
				EsifPrimitiveTuple minFreqTuple = {GET_RFPROFILE_MIN_FREQUENCY, ESIF_PRIMITIVE_DOMAIN_D0, 255};
				if (!EsifUp_ExecutePrimitive(upPtr, &minFreqTuple, NULL, &response)) {
					minFreq = (UInt64)*(UInt32 *) response.buf_ptr;
				}
				
				//comparing the input value with max and min value
				if ((sysval > maxFreq) || (sysval < minFreq)) {
					rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
					ESIF_TRACE_ERROR("sysvl value: %lld should be in the range of min value: %lld or max value: %lld \n",
						sysval,
						minFreq,
						maxFreq);
					goto exit;
				}

				esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name,"%s/%s", devicePathPtr, SYSFS_FIVR_PCH_PATH);
				rc = SetFivrCenterFreqPch(upPtr, sysval, cur_node_name);
				if ( rc != ESIF_OK) {
					ESIF_TRACE_ERROR("Failed to set the FIVR center freq\n");
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
			case ESIF_SYSFS_SET_PBOK:
				sysval = *(Int32 *) requestPtr->buf_ptr;
				esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name,"%s/%s", devicePathPtr, "dptf_power");
				if (SysfsSetInt64(cur_node_name, parm2, sysval) < 0) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				break;
			case ESIF_SYSFS_SET_OSC:  /* osc */
				rc = SetOsc(upPtr, requestPtr);
				break;
			case ESIF_SYSFS_SET_IMOK: /* imok */
				rc = SetImOk(upPtr, requestPtr);
				break;
			case ESIF_SYSFS_SET_FAN_LEVEL:
				rc = SetFanLevel(upPtr, requestPtr, devicePathPtr);
				break;
			case ESIF_SYSFS_SET_BRIGHTNESS_LEVEL:
				rc = SetBrightnessLevel(upPtr, requestPtr, parm1);
				break;
			case ESIF_SYSFS_SET_PLATFORM_POWER_LIMIT_TIME_WINDOW:
			case ESIF_SYSFS_SET_RAPL_TIME_WINDOW:
				sysval = (u64)*(u32 *)requestPtr->buf_ptr;
				/* Convert milliseconds to microseconds and round off*/
				sysval = (sysval + 0.5) * 1000;
				if (SysfsSetInt64(parm1, parm2, sysval) < 0) {
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				break;
			case ESIF_SYSFS_SET_TCC_OFFSET: /* TCC Offset */
				sysval = (u64)*(u32 *)requestPtr->buf_ptr;
				ESIF_TRACE_INFO("TCC Offset value to set in sysfs : %u\n", sysval/1000);
				/* Convert millicelsius to celsius */
				sysval /= MILLICELSIUS_PER_CELSIUS;
				if (SysfsSetInt64(parm1, parm2, sysval) < 0) {
					ESIF_TRACE_ERROR("Error setting TCC Offset value in sysfs.\n");
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				break;
			case ESIF_SYSFS_SET_EPP_WORKLOAD_TYPE: /* EPP Workload Type */
#ifdef ESIF_FEAT_EPP_MAILBOX	//For enabling ESIF_FEAT_EPP_MAILBOX enable it from Makefile 
				rc = SetEppWorkloadType(upPtr, requestPtr, parm1, parm2);
#else
				esif_ccb_strcpy(workloadType, (char *)requestPtr->buf_ptr,MAX_SYSFS_PATH);
				for (node_idx = 0;node_idx < sizeof(eppWorkloadMapTable)/sizeof(eppWorkloadMapTable[0]); node_idx++) {
					if (esif_ccb_strcmp(workloadType,eppWorkloadMapTable[node_idx].workloadType) == 0) {
						eppValue = eppWorkloadMapTable[node_idx].eppValue;
						isValidWorkloadType = ESIF_TRUE;
						break;
					}
				}
				// check for workload type is valid or not 
				if (isValidWorkloadType != ESIF_TRUE) {
					rc = ESIF_E_INVALID_REQUEST_TYPE;
					ESIF_TRACE_ERROR("Invalid Workload Type:\n");
					goto exit;
				}
				// Assemble hash table key to look for existing file pointer to sysfs node
				key.participantId = EsifUp_GetInstance(upPtr);
				key.primitiveTuple = primitivePtr->tuple;
				key.actionPriority = fpcActionPtr->priority;
				cpuCore = g_systemCpuIndexTable.numberOfCpus;
				for (node_idx = 0; node_idx < cpuCore; node_idx++) {
					key.primitiveTuple.instance = node_idx;
					actionContext = (size_t) esif_ht_get_item(actionHashTablePtr, (u8 *)&key, sizeof(key));
					if (ESIF_OK == SetUIntFromActionContext(actionContext, eppValue)) {
						ESIF_TRACE_INFO("Successfully set the value from the hash table.\n");
						continue;
					}
					esif_ccb_sprintf(MAX_SYSFS_PATH,cur_node_name,SYSFS_EPP_PATH,node_idx);
					if (SysfsSetInt64(cur_node_name, SYSFS_EPP_NODE, eppValue) < 0) {
						ESIF_TRACE_ERROR("Error while setting the Epp Value.\n");
						rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
						goto exit;
					}
					if (ESIF_OK == rc) {
						if (SetActionContext(&key, cur_node_name, SYSFS_EPP_NODE, isWrite)) {
							ESIF_TRACE_WARN("Fail to save context for participant " ESIF_HANDLE_FMT ", primitive %d, domain %d, instance %d\n",
							esif_ccb_handle2llu(key.participantId),
							key.primitiveTuple.id,
							key.primitiveTuple.domain,
							key.primitiveTuple.instance);
						}
					}
				}
#endif
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
	Int64 pdl_val = 0;
	double target_perc = 0.0;
	Int64 turbo_perc = 0;
	eEsifError rc = ESIF_OK;

	if ((SysfsGetInt64(SYSFS_PSTATE_PATH, "num_pstates", &pdl_val) < SYSFS_FILE_RETRIEVAL_SUCCESS) || (sysval >= pdl_val)) {
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
	if (SysfsSetInt64("/sys/devices/system/cpu/intel_pstate","max_perf_pct", (int) target_perc) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}
	// Logic to turn on or off turbo
	if (SysfsGetInt64(SYSFS_PSTATE_PATH, "turbo_pct", &turbo_perc) > 0) {
		// Sys FS node exposes turbo frequency range, we need to convert it to turbo target
		turbo_perc = 100 - turbo_perc;
		if (target_perc < turbo_perc) {
			if (SysfsSetInt64(SYSFS_PSTATE_PATH, "no_turbo",1) < 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
		}
		else {
			if (SysfsSetInt64(SYSFS_PSTATE_PATH, "no_turbo",0) < 0) {
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
	SysfsGetString(cpu_target_loc, cpu_target_node, cpu_path, sizeof(cpu_path));
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
	if (SysfsGetStringMultiline("/sys/devices/system/cpu/cpu0/cpufreq", "scaling_available_frequencies", sysvalstring) > 0) {
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
	if (SysfsGetStringMultiline("/sys/devices/system/cpu/cpu0/topology", "core_siblings_list", sysvalstring) > 0) {
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


static int SetActionContext(struct sysfsActionHashKey *keyPtr, EsifString devicePathName, EsifString deviceNodeName, Bool isWrite)
{
	int fd = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	int ret = 0;

	ESIF_TRACE_ENTRY();

	if (devicePathName == NULL || deviceNodeName == NULL) {
		return ret;
	}

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", devicePathName, deviceNodeName);
	if (isWrite == ESIF_TRUE) {
		fd = open(filepath, O_RDWR);
	}
	else {
		fd = open(filepath, O_RDONLY);
	}

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

static eEsifError SetFivrCenterFreqCpu(EsifUpPtr upPtr, UInt64 targetFreq)
{
	eEsifError rc = ESIF_OK;
	UInt32 vcoRefCode = 0;
	UInt32 vcoRefCodeHi = 0;
	UInt32 vcoRefCodeLo = 0;
	UInt32 crystalClockFreq = 0;

	ESIF_ASSERT( targetFreq != 0);

	rc = GetCrystalClockFrequency(&crystalClockFreq);
	if (rc != ESIF_OK) {
		goto exit;
	}
	vcoRefCode = (UInt32)(targetFreq * ESIF_FIVR_PF_MULT / crystalClockFreq);

	// Get the msb/lsb for the vco value 
	vcoRefCodeHi  = (vcoRefCode >> 3) & 0xFF;
	vcoRefCodeLo  = vcoRefCode & 0x7;

	ESIF_TRACE_DEBUG("vcoRefCodeLo: %d\t vcoRefCodeHi: %d\t crystal clock freq: %d\n",
		vcoRefCodeLo,
		vcoRefCodeHi,
		crystalClockFreq);
	//set hi value to the sysfs node
	if (SysfsSetInt64(SYSFS_FIVR_PATH,"vco_ref_code_hi", vcoRefCodeHi) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_ERROR("Set vco_ref_code_hi is failed\n");
		goto exit;
	}
	//set the lo value to the sysfs node
	if (SysfsSetInt64(SYSFS_FIVR_PATH,"vco_ref_code_lo", vcoRefCodeLo) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_ERROR("Set vco_ref_code_lo is failed\n");
		goto exit;
	}

	ESIF_TRACE_DEBUG("vco_ref_code_lo and vco_ref_code_hi is set sucessfully");

exit:
	return rc;
}

static eEsifError GetDdrDvfsDataRate(EsifDataPtr responsePtr, char *path)
{
	eEsifError rc = ESIF_OK;
	Int64 sysval = 0;
	Int64 temp = 0;
	UInt32 cpuSign = 0;

	ESIF_ASSERT(responsePtr != NULL);
	ESIF_ASSERT(path != NULL);
	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	responsePtr->data_len = sizeof(UInt64);
	if (responsePtr->buf_len < responsePtr->data_len) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	rc = GetProcessorSignature(&cpuSign);
	if (rc != ESIF_OK) {
		goto exit;
	}
	if (cpuSign == CPUID_FAMILY_MODEL_ADL_N ||
		cpuSign == CPUID_FAMILY_MODEL_ADL_P ||
		cpuSign == CPUID_FAMILY_MODEL_RPL_S ||
		cpuSign == CPUID_FAMILY_MODEL_RPL_P) {

		if (SysfsGetInt64(path, "ddr_data_rate", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Invalid Sysfs Path \n");
			goto exit;
		}
	}
	else {
		if (SysfsGetInt64(path, "ddr_data_rate_point_0", &temp) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Invalid Sysfs Path \n");
			goto exit;
		}
		sysval = sysval | temp;

		if (SysfsGetInt64(path, "ddr_data_rate_point_1", &temp) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Invalid Sysfs Path \n");
			goto exit;
		}
		sysval = sysval | (temp << DDR_DATA_RATE_P1_POS);

		if (SysfsGetInt64(path, "ddr_data_rate_point_2", &temp) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Invalid Sysfs Path \n");
			goto exit;
		}
		sysval = sysval | (temp << DDR_DATA_RATE_P2_POS);

		if (SysfsGetInt64(path, "ddr_data_rate_point_3", &temp) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Invalid Sysfs Path \n");
			goto exit;
		}
		sysval = sysval | (temp << DDR_DATA_RATE_P3_POS);
	}
	ESIF_TRACE_DEBUG("GET_DDR_DVFS_DATA_RATE sysval: %lld\n",sysval);
	*(UInt64 *) responsePtr->buf_ptr = (UInt64)sysval;
exit:
	return rc;
}

static eEsifError WaitForRfiRestrictionRunBusyIdle(char *path)
{
	eEsifError rc = ESIF_OK;
	Int64 sysVal = 0;
	UInt32 busyLoopCount = 0;
	do {
		if (SysfsGetInt64(path, "rfi_restriction_run_busy", &sysVal) < SYSFS_FILE_RETRIEVAL_SUCCESS)
		{
			rc = ESIF_E_IO_OPEN_FAILED;
			ESIF_TRACE_ERROR("Invalid Sysfs Path \n");
			goto exit;
		}
		busyLoopCount++;
		ESIF_TRACE_DEBUG("busyLoopCount: %u \n", busyLoopCount);
		esif_ccb_sleep_msec(1);
	} while (sysVal && busyLoopCount < MAX_RFI_BUSY_LOOP_COUNT);

	if (sysVal)
	{
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
	}
exit:
	return rc;
}

static eEsifError SetDdrRfiRestriction(UInt64 sysval, char *path)
{

	eEsifError rc = ESIF_OK;
	UInt32 cpuSign = 0;

	ESIF_ASSERT(path != NULL);

	rc = GetProcessorSignature(&cpuSign);
	if (rc != ESIF_OK) {
		goto exit;
	}

	if (cpuSign == CPUID_FAMILY_MODEL_ADL_N ||
		cpuSign == CPUID_FAMILY_MODEL_ADL_P ||
		cpuSign == CPUID_FAMILY_MODEL_RPL_S ||
		cpuSign == CPUID_FAMILY_MODEL_RPL_P) {

		if (SysfsSetInt64(path, "rfi_restriction", sysval) < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Set rfi_restriction is failed\n");
			goto exit;
		}
	}
	else {
		rc = WaitForRfiRestrictionRunBusyIdle(path);
		if(rc != ESIF_OK)
		{
			ESIF_TRACE_ERROR("rfi_restriction_run_busy check failed before set\n");
			goto exit;
		}

		if (SysfsSetInt64(path, "rfi_restriction_data_rate_base", sysval) < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Set rfi_restriction_data_rate_base is failed\n");
			goto exit;
		}

		if (SysfsSetInt64(path, "rfi_restriction_run_busy", 1) < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_ERROR("Set rfi_restriction_run_busy set failed\n");
			goto exit;
		}

		rc = WaitForRfiRestrictionRunBusyIdle(path);
		if(rc != ESIF_OK)
		{
			ESIF_TRACE_ERROR("rfi_restriction_run_busy check failed after set\n");
			goto exit;
		}
	}
exit:
		return rc;

}

static eEsifError GetRfprofileCenterFreq(EsifDataPtr responsePtr, char *path, char *node)
{
	eEsifError rc = ESIF_OK;
	Int64 sysval = 0;
	UInt32 crystalClockFreq = 0;

	ESIF_ASSERT(responsePtr != NULL);
	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	responsePtr->data_len = sizeof(UInt64);
	if (responsePtr->buf_len < responsePtr->data_len) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	rc = GetCrystalClockFrequency(&crystalClockFreq);
	if (rc != ESIF_OK) {
		goto exit;
	}
	if (SysfsGetInt64(path, node, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_ERROR("Invalid Sysfs Path \n");
		goto exit;
	}

	//return Fivr center freq
	ESIF_TRACE_DEBUG("Successfully Executed GET_RFPROFILE_CENTER_FREQUENCY_FIVR sysval: %lld\n",sysval);
	*(UInt64 *) responsePtr->buf_ptr = (UInt64)sysval * crystalClockFreq / ESIF_FIVR_PF_MULT;
exit:
	return rc;
}

static eEsifError SetFivrCenterFreqPch(EsifUpPtr upPtr, UInt64 targetFreq, char *path)
{
	eEsifError rc = ESIF_OK;
	UInt32 vcoRefCode = 0;
	UInt32 vcoRefCodeHi = 0;
	UInt32 vcoRefCodeLo = 0;
	Int64 sysval = 0;
	UInt32 fivrRegVal = 0;
	UInt64 responseData = 0;
	UInt32 crystalClockFreq = 0;
	EsifData response = { ESIF_DATA_UINT64, &responseData, sizeof(responseData), sizeof(responseData) };

	ESIF_ASSERT( targetFreq != 0);

	rc = GetCrystalClockFrequency(&crystalClockFreq);
	if (rc != ESIF_OK) {
		goto exit;
	}
	vcoRefCode = (UInt32)(targetFreq * ESIF_FIVR_PF_MULT / crystalClockFreq);

	// Get the msb/lsb for the Vco value 
	vcoRefCodeHi  = (vcoRefCode >> 3) & FIVR_MSB_MASK;
	vcoRefCodeLo  = vcoRefCode & FIVR_LSB_MASK;

	ESIF_TRACE_DEBUG("vcoRefCodeLo: %d\t vcoRefCodeHi: %d\t crystal clock freq: %d\n",
			vcoRefCodeLo,
			vcoRefCodeHi,
			crystalClockFreq);

	//Get the center Freq
	if (SysfsGetInt64(path, SYSFS_FIVR_PCH_NODE_SET, &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}
	fivrRegVal = (UInt32)sysval;

	// vcoRefCodehi is 8 bit and vcorefCodeLo is 3 bit
	fivrRegVal = (fivrRegVal & FIVR_CENTER_FREQ_MASK);
	fivrRegVal |= ((vcoRefCodeHi << FIVR_MSB_SHIFT) | (vcoRefCodeLo << FIVR_LSB_SHIFT));
	//set hi value to the sysfs node
	if (SysfsSetInt64(path,SYSFS_FIVR_PCH_NODE_SET, fivrRegVal) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_ERROR("Set fivrRegVal is failed\n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("Set Center freq for PCH participant sucessfully:%d\n",fivrRegVal);

exit:
	return rc;
}

static eEsifError GetRfprofileFreqAdjuRes(EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;
	UInt64 freqStep = 0;
	UInt32 crystalClockFreq = 0;

	ESIF_ASSERT(responsePtr != NULL);
	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	responsePtr->data_len = sizeof(freqStep);
	if (responsePtr->buf_len < responsePtr->data_len) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	rc = GetCrystalClockFrequency(&crystalClockFreq);
	if (rc != ESIF_OK) {
		goto exit;
	}
	freqStep = ESIF_FREQ_ADJ_STEP_15;
	ESIF_TRACE_DEBUG("GET_RFPROFILE_FREQUENCY_ADJUST_RESOLUTION executed successfully, freqStep: %lld",(UInt32)freqStep);

	*(UInt64 *)responsePtr->buf_ptr = freqStep;
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
		esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%s!", guidStr);
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

static enum esif_rc GetGfxPerfSupportStates(EsifDataPtr responsePtr)
{
	UInt32 pdlValue = 0;
	Int64 gtMinFreq = 0;
	Int64 gtMaxFreq = 0;
	UInt32 pstateIndex = 0;
	UInt32 pstateCounter = 0;
	UInt32 gpuFrequency = 0;
	UInt32 dataSize = 0;
	eEsifError rc = ESIF_OK;
	union esif_data_variant *curRespPtr = NULL;

	ESIF_ASSERT(responsePtr != NULL);
	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if ((SysfsGetInt64(SYSFS_GFX_PATH, GT_RP0_FREQ_MHZ, &gtMaxFreq) < SYSFS_FILE_RETRIEVAL_SUCCESS) || (gtMaxFreq > MAX_SYSFS_PERF_STATES)) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}
	if ((SysfsGetInt64(SYSFS_GFX_PATH, GT_RPN_FREQ_MHZ, &gtMinFreq) < SYSFS_FILE_RETRIEVAL_SUCCESS) || (gtMinFreq > MAX_SYSFS_PERF_STATES)) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	pdlValue = (UInt32)((gtMaxFreq - gtMinFreq) / GT_FREQ_STEP) + 1;
	dataSize = pdlValue * sizeof(union esif_data_variant);
	responsePtr->data_len = dataSize;
	if (responsePtr->buf_len < responsePtr->data_len) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}
	curRespPtr = (union esif_data_variant *)responsePtr->buf_ptr;

	for (pstateCounter = pdlValue; pstateCounter > 0; pstateCounter--) {
		gpuFrequency = (pstateCounter * GT_FREQ_STEP);
		g_gfxPstateFreqMapTable[pstateIndex].pstate = pstateIndex;
		g_gfxPstateFreqMapTable[pstateIndex].freqValue = gpuFrequency;
		curRespPtr->type = ESIF_DATA_UINT64;
		curRespPtr->integer.value = (UInt64)gpuFrequency;
		curRespPtr++;
		pstateIndex++;
	}

exit:
	return rc;
}

static enum esif_rc get_perf_support_states(char *table_str, char *participant_path)
{
	Int64 pdl_val = 0;
	u64 placeholder_val =0;
	int pcounter = 0;
	eEsifError rc = ESIF_OK;

	if ((SysfsGetInt64(participant_path, "max_state", &pdl_val) < SYSFS_FILE_RETRIEVAL_SUCCESS) || (pdl_val > MAX_SYSFS_PERF_STATES)){
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
	Int64 pdl_val = 0;
	u64 placeholder_val =0;
	int pcounter = 0;
	eEsifError rc = ESIF_OK;
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	char *token;
	char *next_token;
	const char s[2] = " ";
	int counter = 0;

	if (SysfsGetString(SYSFS_PSTATE_PATH, "num_pstates", sysvalstring, sizeof(sysvalstring)) > -1) { // Intel P State driver is loaded
		if ((SysfsGetInt64("/sys/devices/system/cpu/intel_pstate/", "num_pstates", &pdl_val) < SYSFS_FILE_RETRIEVAL_SUCCESS) || (pdl_val > MAX_SYSFS_PSTATES)) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
		/* Sysfs returns total states - we want the max state, so subtract one */
		if (pdl_val != 0) {
			pdl_val -= 1;
		}
		for (pcounter = 0;pcounter <= pdl_val;pcounter++) {
			esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu!",pcounter,placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
		}
	}
	else { // CPU Frequency Governor is loaded
		pdl_val = GetCpuFreqPdl();
		cpufreq = (int*)esif_ccb_malloc(sizeof(int) * (pdl_val + 1));
		if (cpufreq == NULL) {
			ESIF_TRACE_ERROR("Unable to allocate cpufreq\n");
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		if (SysfsGetStringMultiline("/sys/devices/system/cpu/cpu0/cpufreq", "scaling_available_frequencies", sysvalstring) > 0) {
			token = esif_ccb_strtok(sysvalstring,s,&next_token);
			while ((token != NULL) && (isdigit(*token))) {
				cpufreq[counter] = esif_atoi(token);
				esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d,%llu,%llu,%llu,%llu,%llu!",(cpufreq[counter]/1000),placeholder_val,placeholder_val,placeholder_val,placeholder_val,placeholder_val);
				token = esif_ccb_strtok(NULL,s,&next_token);
				counter++;
			}
		} else {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		}
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
	Int64 sysval = 0;
	char cur_path[MAX_SYSFS_PATH] = { 0 };
	char revision[] = "02";

	UNREFERENCED_PARAMETER(target_guid);

	if ( esif_ccb_strlen(g_TCPUSysfsPath, sizeof(g_TCPUSysfsPath)) == 0) {
		rc = ESIF_E_INVALID_HANDLE;
		ESIF_TRACE_ERROR("g_TCPUSysfsPath Invalid\n");
		goto exit;
	}
	
	esif_ccb_sprintf(MAX_SYSFS_PATH, cur_path, "%s/power_limits", g_TCPUSysfsPath);
	ESIF_TRACE_DEBUG("Current path: %s", cur_path);
	if (SysfsGetInt64(cur_path, "power_limit_0_min_uw", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		goto exit;
	}
	pl1Min = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_0_max_uw",  &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		goto exit;
	}
	pl1Max = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_0_step_uw", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		goto exit;
	}
	step1 = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_0_tmin_us", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		goto exit;
	}
	time1Min = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_0_tmax_us", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		goto exit;
	}
	time1Max = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_1_min_uw",  &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Min = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_1_max_uw",  &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	pl2Max = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_1_step_uw", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	step2 = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_1_tmin_us", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	time2Min = sysval;
	if (SysfsGetInt64(cur_path, "power_limit_1_tmax_us", &sysval) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		sysval = 0;  /* we don't care - dptf doesn't use this value anyways */
	}
	time2Max = sysval;
	pl1Min = (pl1Min > 0) ? pl1Min / 1000 : 0;
	pl1Max = (pl1Max > 0) ? pl1Max / 1000 : 0;
	time1Min = (time1Min > 0) ? time1Min / 1000 : 0;
	time1Max = (time1Max > 0) ? time1Max / 1000 : 0;
	step1 = (step1 > 0) ? step1 / 1000 : 0;
	pl2Min = (pl2Min > 0) ? pl2Min / 1000 : 0;
	pl2Max = (pl2Max > 0) ? pl2Max / 1000 : 0;
	time2Min = (time2Min > 0) ? time2Min / 1000 : 0;
	time2Max = (time2Max > 0) ? time2Max / 1000 : 0;
	step2 = (step2 > 0) ? step2 / 1000 : 0;
	esif_ccb_sprintf(BINARY_TABLE_SIZE, table_str, "%s:0,%llu,%llu,%llu,%llu,%llu!1,%llu,%llu,%llu,%llu,%llu",revision,pl1Min,pl1Max,time1Min,time1Max,step1,pl2Min,pl2Max,time2Min,time2Max,step2);
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
		len += esif_ccb_snprintf(table_str + len, 4, "00:");
	}

	while (count) {
		char full_acpi_scope[MAX_ACPI_SCOPE_LEN] = "";

		if (ART == type) {
			// If target device is CPU, make sure that it is named TCPU (TCPU cannot be source in ART)
			//replace_cpu_id(art_entry->target_device);

			// Format ART strings
			get_participant_scope(art_entry->art_source_device, full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, 2, ",");

			get_participant_scope(art_entry->art_target_device, full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, 2, ",");

			len += esif_ccb_snprintf(table_str + len, 8, "%llu,", art_entry->art_weight);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac0_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac1_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac2_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac3_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac4_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac5_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac6_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac7_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld,", (long long) art_entry->art_ac8_max_level);
			len += esif_ccb_snprintf(table_str + len, 8, "%lld", (long long) art_entry->art_ac9_max_level);
		} else {
			// If device is CPU, make sure that it is named TCPU
			//replace_cpu_id(trt_entry->source_device);
			//replace_cpu_id(trt_entry->target_device);

			// Format TRT strings
			get_participant_scope(trt_entry->trt_source_device, full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, 2, ",");

			get_participant_scope(trt_entry->trt_target_device, full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, MAX_ACPI_SCOPE_LEN, "%s", full_acpi_scope);
			len += esif_ccb_snprintf(table_str + len, 2, ",");

			len += esif_ccb_snprintf(table_str + len, 8, "%llu,", trt_entry->trt_influence);
			len += esif_ccb_snprintf(table_str + len, 8, "%llu,", trt_entry->trt_sample_period);
			len += esif_ccb_snprintf(table_str + len, 8, "%llu,", trt_entry->trt_reserved[0]);
			len += esif_ccb_snprintf(table_str + len, 8, "%llu,", trt_entry->trt_reserved[1]);
			len += esif_ccb_snprintf(table_str + len, 8, "%llu,", trt_entry->trt_reserved[2]);
			len += esif_ccb_snprintf(table_str + len, 8, "%llu", trt_entry->trt_reserved[3]);
		}

		count--;
		if (count) {
			len += esif_ccb_snprintf(table_str + len, 2, "!");
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

static eEsifError GetGddvData(const EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;
	union esif_data_variant dataHeader = {0};
	UInt8 *currentPtr = responsePtr->buf_ptr;

	if ( esif_ccb_strlen(g_ManagerSysfsPath, sizeof(g_ManagerSysfsPath)) == 0) {
		rc = ESIF_E_INVALID_HANDLE;
		ESIF_TRACE_ERROR("g_ManagerSysfsPath Invalid\n");
		goto exit;
	}
	size_t fileSize = 0;
	rc = SysfsGetFileSize(g_ManagerSysfsPath, SYSFS_DATA_VAULT, &fileSize);
	if (rc != ESIF_OK) {
		// ACPI data_vault path is not found.
		rc = ESIF_E_ACPI_OBJECT_NOT_FOUND;	
		goto exit;
	}

	responsePtr->data_len = sizeof(dataHeader) + fileSize;
	if (responsePtr->buf_len < sizeof(dataHeader) + fileSize) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		ESIF_TRACE_WARN("Response buffer too small. \n");
		goto exit;
	}

	// return esif_data_variant with binary string
	dataHeader.string.type = ESIF_DATA_BINARY;
	dataHeader.string.length = fileSize;
	esif_ccb_memcpy(currentPtr, &dataHeader ,sizeof(dataHeader));
	currentPtr += sizeof(dataHeader);

	rc = SysfsGetBinaryData(g_ManagerSysfsPath, SYSFS_DATA_VAULT, currentPtr, fileSize);
	if (rc != ESIF_OK) {
		goto exit;
	}

	responsePtr->type = ESIF_DATA_BINARY;
	ESIF_TRACE_INFO("Completed reading GDDV Data of length : %d bytes", responsePtr->data_len );

exit:
	return rc;
}

static eEsifError GetOemVariables(char *table_str)
{
	eEsifError rc = ESIF_OK;
	DIR *dir = NULL;
	struct dirent **namelist;
	UInt32 variableIndex = 0;
	Int32 numberOfFiles = 0;
	UInt32 fileIndex = 0;
	char cur_oem_file[MAX_SYSFS_PATH] = { 0 };

	dir = opendir(g_ManagerSysfsPath);
	if (dir == NULL) {
		ESIF_TRACE_DEBUG("No Manager sysfs path : %s\n",g_ManagerSysfsPath );
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	numberOfFiles = scandir(g_ManagerSysfsPath, &namelist, 0, alphasort);
	if (numberOfFiles < 0) {
		ESIF_TRACE_DEBUG("No files to scan\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	while (fileIndex < numberOfFiles) {
		Int32 oemVariable = 0;
		esif_ccb_sprintf(sizeof(cur_oem_file), cur_oem_file, "%s%d", SYSFS_OEM_VARIABLE, variableIndex);
		if (esif_ccb_strstr(namelist[fileIndex]->d_name, cur_oem_file) != NULL) {
			SysfsGetInt(g_ManagerSysfsPath, cur_oem_file, &oemVariable);
			esif_ccb_sprintf_concat(BINARY_TABLE_SIZE, table_str, "%d!",oemVariable);
			variableIndex++;
			ESIF_TRACE_INFO("OEM Variable %d : %d ", variableIndex , oemVariable);
		}
		esif_ccb_free(namelist[fileIndex]);
		fileIndex++;
	}
	esif_ccb_free(namelist);

	ESIF_TRACE_INFO("Completed reading All the OEM Variables. OEM Count : %d bytes", variableIndex );

exit:
	if (dir != NULL) {
		closedir(dir);
	}
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
	Int64 pdlVal = 0;
	Int64 curVal = 0;
	double target_perc = 0;

	if (devicePathPtr == NULL) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("devicePathPtr is NULL\n");
		goto exit;
	}

	UpdateFineGrainSupportedStatus(devicePathPtr);
	if ( g_isFanFineGrainSupported == ESIF_TRUE ) {
		// Fine Grain control is supported. The value received from policy will be in 0 - 100%
		// Need to be divided by step size and set that value to cur_state
		// e.g if we receive value as 90 and step size is 2 , 90/2 = 45 needs to be set in cur_state
		UpdateStepSize(devicePathPtr);
		curVal = *(UInt32 *) requestPtr->buf_ptr / g_fanStepSize;
	}
	else {
		// Legacy Flow
		if (SysfsGetInt64(devicePathPtr, "max_state", &pdlVal) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_WARN("Fail get participant %d's fan max_state\n", upPtr->fInstance);
			goto exit;
		}

		target_perc = ((double) *(u32 *) requestPtr->buf_ptr) / 100.0;
		curVal = round((target_perc + EPSILON_CONVERT_PERC) * pdlVal); // Avoid rounding errors
	}

	if (SysfsSetInt64(devicePathPtr, "cur_state", curVal) < 0) {
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
	Int64 bdlVal = 0;
	Int64 curVal = 0;
	double target_perc = 0;

	if (SysfsGetInt64(devicePathPtr, "max_brightness", &bdlVal) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail get participant %d's max_brightness\n", upPtr->fInstance);
		goto exit;
	}

	target_perc = ((double) *(u32 *) requestPtr->buf_ptr) / 100.0;
	curVal = (u64)((target_perc * bdlVal) + EPSILON_CONVERT_PERC); // Avoid rounding errors
	if (SysfsSetInt64(devicePathPtr, "brightness", curVal) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail set participant %d's brightness\n", upPtr->fInstance);
		goto exit;
	}

	ESIF_TRACE_DEBUG("Set upper participant %d's display brightness to %llu\n", upPtr->fInstance, curVal);

exit:
	return rc;
}

static eEsifError SetEppWorkloadType(const EsifUpPtr upPtr, const EsifDataPtr requestPtr, const EsifString devicePathPtr, const EsifString fileNamePtr)
{
	eEsifError rc = ESIF_OK;

	if (devicePathPtr == NULL || fileNamePtr == NULL) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("devicePathPtr/fileNamePtr is NULL\n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("Trying to set Workload type %s\n", (char *)requestPtr->buf_ptr);
	if (SysfsSetString(devicePathPtr, fileNamePtr, (char *)requestPtr->buf_ptr) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Set EPP Workload type %s is successful\n", (char *)requestPtr->buf_ptr);

exit:
	return rc;
}

static void UpdateFineGrainSupportedStatus(const EsifString devicePathPtr)
{
	Int64 fineGrainControl = 0;
	char fanACPIPath[MAX_SYSFS_PATH] = { 0 };

	ESIF_ASSERT(devicePathPtr != NULL);

	//Update the FineGrainSupported Status only once
	if (g_isFanFineGrainSupported != 0xFFFFFFFF ) {
		// This status is already updated return.
		goto exit;
	}

	esif_ccb_strcpy(fanACPIPath, devicePathPtr, sizeof(fanACPIPath));
	esif_ccb_strcat(fanACPIPath, "/device/firmware_node",sizeof(fanACPIPath));
	if (SysfsGetInt64(fanACPIPath, "fine_grain_control", &fineGrainControl) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		// Ignore if any failure
		ESIF_TRACE_WARN("Fail to get participant fine_grain_control\n");
		goto exit;
	}

	ESIF_TRACE_INFO("Fine Grain Control : %d", fineGrainControl);
	g_isFanFineGrainSupported = fineGrainControl;

exit:
	return;
}

static void UpdateStepSize(const EsifString devicePathPtr)
{
	Int64 maxPState = 0;

	ESIF_ASSERT(devicePathPtr != NULL);

	//Update the fanstep size only once
	if (g_fanStepSize != 0xFFFFFFFF ) {
		// This status is already updated return.
		goto exit;
	}

	if (SysfsGetInt64(devicePathPtr, "max_state", &maxPState) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		// Ignore if any failure
		ESIF_TRACE_ERROR("Fail to get participant fan max_state\n");
		g_fanStepSize = 1; // set default to 1 on failure and exit
		goto exit;
	}
	ESIF_TRACE_DEBUG("max P State : %d", maxPState);
	g_fanStepSize = 100 / maxPState;
	ESIF_TRACE_INFO("Fan Step Size : %d", g_fanStepSize);

exit:
	return;
}

static void UpdateFpsSupportedStatus(const EsifString devicePathPtr)
{
	char sysValString[MAX_SYSFS_PATH] = { 0 };
	char fanFPSPath[MAX_SYSFS_PATH] = { 0 };

	ESIF_ASSERT(devicePathPtr != NULL);

	//Update the FAN FPS Status only once
	if (g_isFanFpsSupported != 0xFFFFFFFF ) {
		// This status is already updated return.
		goto exit;
	}

	esif_ccb_strcpy(fanFPSPath, devicePathPtr, sizeof(fanFPSPath));
	esif_ccb_strcat(fanFPSPath, "/device/firmware_node",sizeof(fanFPSPath));
	//Check if FAN FPS Support is available
	if (SysfsGetString(fanFPSPath, "state0", sysValString, sizeof(sysValString)) > -1) {
		g_isFanFpsSupported = ESIF_TRUE;
	}
	else {
		g_isFanFpsSupported = ESIF_FALSE;
	}
	ESIF_TRACE_INFO("Fan FPS Supported : %s", (g_isFanFpsSupported == ESIF_TRUE) ? "TRUE" : "FALSE");

exit:
	return;
}

static eEsifError GetFanInfo(EsifDataPtr responsePtr, const EsifString devicePathPtr)
{
	// Since the ACPI object _FIF is not exposed to user space, just fake
	// the data. It's mostly a filler since DPTF only cares one field (see below)
	eEsifError rc = ESIF_OK;
	struct EsifDataBinaryFifPackage fif = {0};
	UInt32 maxPState = 0;

	ESIF_ASSERT(responsePtr != NULL);
	ESIF_ASSERT(devicePathPtr != NULL);

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

	UpdateFineGrainSupportedStatus(devicePathPtr);
	if ( g_isFanFineGrainSupported == ESIF_TRUE ) {
		// Latest kernel and FineGrain supported
		//Update the step size only if fine grain is supported
		UpdateStepSize(devicePathPtr);
		fif.hasFineGrainControl.integer.value = g_isFanFineGrainSupported;
	}
	else if ( g_isFanFineGrainSupported == ESIF_FALSE ) {
		// Latest kernel and FineGrain is NOT supported
		fif.hasFineGrainControl.integer.value = g_isFanFineGrainSupported;
		g_fanStepSize = 0;
	}
	else {
		// Legacy code
		fif.hasFineGrainControl.integer.value = ESIF_TRUE;
		g_fanStepSize = 0;
	}
	fif.stepSize.integer.value = g_fanStepSize;
	ESIF_TRACE_INFO("\n FanFineGrainSupported : %d fanStepSize : %d",g_isFanFineGrainSupported, g_fanStepSize);
	esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, &fif, sizeof(fif));

exit:
	return rc;
}

static int FpsEntryFilter(const struct dirent *entry)
{
	if (esif_ccb_strstr(entry->d_name, "state")) return 1;
	else return 0;
};

static eEsifError EnumerateFpsEntries(EsifDataPtr responsePtr, const EsifString devicePathPtr)
{
	DIR *dir = NULL;
	struct dirent **namelist;
	Int32 n = 0;
	char sysValString[MAX_SYSFS_PATH] = { 0 };
	char fanFPSPath[MAX_SYSFS_PATH] = { 0 };
	char fpsEntryName[MAX_SYSFS_PATH] = { 0 };
	eEsifError rc = ESIF_OK;
	union esif_data_variant revision = {0};
	struct EsifDataBinaryFpsPackage fps = {0};
	UInt32 size = 0;
	UInt32 totalSize = 0;
	char *token = NULL;
	char *next_token = NULL;

	ESIF_ASSERT(responsePtr != NULL);
	ESIF_ASSERT(devicePathPtr != NULL);

	esif_ccb_strcpy(fanFPSPath, devicePathPtr, sizeof(fanFPSPath));
	esif_ccb_strcat(fanFPSPath, "/device/firmware_node",sizeof(fanFPSPath));
	dir = opendir(fanFPSPath);
	if (!dir) {
		ESIF_TRACE_DEBUG("No FPS directory\n");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	n = scandir(fanFPSPath, &namelist, FpsEntryFilter, alphasort);
	if (n < 0) {
		//no scan
		rc = ESIF_E_NOT_SUPPORTED;
		ESIF_TRACE_WARN("FPS entries not supported\n");
		goto exit;
	}

	totalSize = sizeof(revision);
	totalSize += n  * sizeof(fps);
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
	for ( UInt32 i = 0 ; i < n ; i++ ) {
		esif_ccb_sprintf(MAX_SYSFS_PATH, fpsEntryName, "state%d", i);
		if (SysfsGetString(fanFPSPath, fpsEntryName , sysValString, sizeof(sysValString)) > -1) {
			ESIF_TRACE_DEBUG("SysValString=%s \n", sysValString);
			next_token = sysValString;

			token = esif_ccb_strtok(next_token, ":", &next_token);
			if (token != NULL) {
				fps.control.integer.value = (UInt64) strtol(token, NULL, 0);
				g_fanProperties[i].control = fps.control.integer.value;
			}

			token = esif_ccb_strtok(next_token, ":", &next_token);
			if (token != NULL) {
				fps.tripPoint.integer.value = (UInt64) strtol(token, NULL, 0);
				g_fanProperties[i].tripPoint = fps.tripPoint.integer.value;
			}

			token = esif_ccb_strtok(next_token, ":", &next_token);
			if (token != NULL) {
				fps.speed.integer.value	= (UInt64) strtol(token, NULL, 0);
				g_fanProperties[i].speed =  fps.speed.integer.value;
			}

			token = esif_ccb_strtok(next_token, ":", &next_token);
			if (token != NULL) {
				fps.noiseLevel.integer.value = (UInt64) strtol(token, NULL, 0);
				g_fanProperties[i].noiseLevel =  fps.noiseLevel.integer.value;
			}

			token = esif_ccb_strtok(next_token, ":", &next_token);
			if (token != NULL) {
				fps.power.integer.value = (UInt64) strtol(token, NULL, 0);
				g_fanProperties[i].power =  fps.power.integer.value;
			}
		}
		//cleanup each namelist entry
		esif_ccb_free(namelist[i]);
		esif_ccb_memcpy((u8 *) responsePtr->buf_ptr + size, &fps, sizeof(fps));
		size += sizeof(fps);
	}
	esif_ccb_free(namelist);
exit:
	if (dir) {
		closedir(dir);
	}
	return rc;
}

static eEsifError GetFanPerfStates(const EsifString acpiDevName, EsifDataPtr responsePtr, const EsifString devicePathPtr)
{
	eEsifError rc = ESIF_OK;
	union esif_data_variant revision = {0};
	struct EsifDataBinaryFpsPackage fps = {0};
	Int64 pdlVal = 0;
	int size = 0;
	int totalSize = 0;
	int i = 0;
	char sysValString[MAX_SYSFS_PATH] = { 0 };
	char fanFPSPath[MAX_SYSFS_PATH] = { 0 };

	UpdateFpsSupportedStatus(devicePathPtr);
	if ( g_isFanFpsSupported == ESIF_TRUE) {
		// Latest kernel and FPS entries are supported
		rc = EnumerateFpsEntries(responsePtr, devicePathPtr);
	}
	else {
		// Legacy flow
		// Since the ACPI object _FPS is not exposed to user space, just fake
		// the data. It's just a filler and all fields are ignored by DPTF.
		// However, its presence is required otherwise DPTF will abort fan control
		if (SysfsGetInt64(devicePathPtr, "max_state", &pdlVal) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			ESIF_TRACE_WARN("Fail to get participant fan max_state\n");
			goto exit;
		}

		totalSize = sizeof(revision);
		totalSize += (pdlVal) * sizeof(fps);
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
		for (i = 0; i <= pdlVal; i++) {
			fps.control.integer.value = round(i * 100 / pdlVal);
			esif_ccb_memcpy((u8 *) responsePtr->buf_ptr + size, &fps, sizeof(fps));
			size += sizeof(fps);
		}
	}

exit:
	return rc;
}

static eEsifError GetDisplayBrightness(char *path, EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;
	Int64 bdlVal = 0;
	Int64 curVal = 0;
	double target_perc = 0;

	if (SysfsGetInt64(path, "max_brightness", &bdlVal) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Failed get participant's max display brightness\n");
		goto exit;
	}

	if (SysfsGetInt64(path, "brightness", &curVal) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Failed get participant's current brightness\n");
		goto exit;
	}

	target_perc = ((double)curVal / (double)bdlVal) * 100;
	*(u32 *) responsePtr->buf_ptr = (u32) target_perc;

exit:
	return rc;
}

static eEsifError GetCStateResidency(char *path, char *node, UInt32 msrAddr, EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;
	rc = GetMsrValue(path, node, msrAddr, responsePtr);
	return rc;
}

static eEsifError GetRaplRawEnergyInUnits(UInt64 energyUjs, UInt32 energyUnit, UInt32 *rawEnergyInUnits)
{

	eEsifError rc = ESIF_OK;
	UInt32 uJPerEnergyUnit = 0;

	ESIF_ASSERT(NULL != rawEnergyInUnits);
	uJPerEnergyUnit = ENERGY_UNIT_CONVERSION_FACTOR / (1 << energyUnit);
	*rawEnergyInUnits =  (u32) energyUjs / uJPerEnergyUnit;
	ESIF_TRACE_DEBUG("energyUjs: %u, uJPerEnergyUnit: %u, rawEnergyInUnits: %u \n",
			energyUjs, uJPerEnergyUnit, *rawEnergyInUnits);
	return rc;
}

static eEsifError GetRaplEnergyUnit(char *path, char *node, UInt32 msrAddr, EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;

	rc = GetMsrValue(path, node, msrAddr, responsePtr);
	if (ESIF_OK == rc) {
		*(UInt64 *) responsePtr->buf_ptr = ExtractBits((UInt64*) responsePtr->buf_ptr,
							RAPL_ENERGY_UNIT_BIT_POS, RAPL_ENERGY_UNIT_NBITS);
		ESIF_TRACE_DEBUG("RAPL_ENERGY_UNIT Value= %u\n", *(UInt64 *) responsePtr->buf_ptr);
	}
	else {
		ESIF_TRACE_ERROR("Error retrieving MSR value for RAPL_ENERGY_UNIT\n");
	}

	return rc;
}
static UInt64 ExtractBits(UInt64 *number, UInt32 bitPos, UInt32 nBits)
{
	return ((*number >> bitPos) & ((1 << nBits) - 1));
}

static eEsifError GetMsrValue(char *path, char *node, UInt32 msrAddr, EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;
	char fileFullPath[MAX_SYSFS_PATH] = { 0 };

	if (path == NULL || node == NULL) {
		rc = ESIF_E_IO_INVALID_NAME;
		goto exit;
	}

	esif_ccb_sprintf(MAX_SYSFS_PATH, fileFullPath, "%s/%s", path, node);

	// First if check /dev/cpu/0/msr exists
	if (access(fileFullPath, F_OK) == -1) {
		rc = ESIF_E_IO_INVALID_NAME;
		goto exit;
	}

	// Open the device for read
	int fd = open(fileFullPath, O_RDONLY);
	if (fd < 0) {
		rc = ESIF_E_IO_OPEN_FAILED;
		goto exit;
	}

	UInt64 data;
	if (pread(fd, &data, sizeof(UInt64), msrAddr) != sizeof(UInt64)) {
		close(fd);
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
	close(fd);
	*(UInt64 *) responsePtr->buf_ptr = data;

exit:
	return rc;
}

static eEsifError GetFanStatus(EsifDataPtr responsePtr, const EsifString devicePathPtr)
{
	// Since the ACPI object _FST is not exposed to user space, use the value
	// set by SetFanLevel.
	eEsifError rc = ESIF_OK;
	Int64 curVal = 0;
	Int64 pdlVal = 0;
	struct EsifDataBinaryFstPackage fst = {0};
	char fanFPSPath[MAX_SYSFS_PATH] = { 0 };
	Int64 fanSpeed = 0;

	ESIF_ASSERT(responsePtr != NULL);
	ESIF_ASSERT(devicePathPtr != NULL);

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

	if (SysfsGetInt64(devicePathPtr, "max_state", &pdlVal) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_WARN("Fail to get participant fan max_state\n");
		goto exit;
	}

	if (SysfsGetInt64(devicePathPtr, "cur_state", &curVal) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
		// Do nothing, best effort.
		ESIF_TRACE_WARN("Fail to get participant cur_state\n");
		goto exit;
	}

	UpdateFineGrainSupportedStatus(devicePathPtr);
	UpdateFpsSupportedStatus(devicePathPtr);
	if ( (g_isFanFineGrainSupported == ESIF_TRUE) && (g_isFanFpsSupported == ESIF_TRUE) ) {
		// Latest Kernel with Finegrain and FPS support
		UpdateStepSize(devicePathPtr);
		fst.control.integer.value = curVal * g_fanStepSize;
		esif_ccb_strcpy(fanFPSPath, devicePathPtr, sizeof(fanFPSPath));
		esif_ccb_strcat(fanFPSPath, "/device/firmware_node",sizeof(fanFPSPath));
		if (SysfsGetInt64(fanFPSPath, "fan_speed_rpm", &fanSpeed) < SYSFS_FILE_RETRIEVAL_SUCCESS) {
			// Do nothing, best effort.
			ESIF_TRACE_WARN("Fail to get participant fan_speed_rpm\n");
			goto exit;
		}
		fst.speed.integer.value = fanSpeed;
	}
	else if (g_isFanFpsSupported == ESIF_TRUE) {
		// Kernel with only FPS support
		fst.control.integer.value = g_fanProperties[curVal].control;
		fst.speed.integer.value = g_fanProperties[curVal].speed;
	}
	else {
		// Legacy flow
		fst.control.integer.value = round(curVal * 100 / pdlVal);
	}
	ESIF_TRACE_INFO("fst.control=%d: fst.speed=%d\n", fst.control.integer.value, fst.speed.integer.value);

exit:
	esif_ccb_memcpy((u8 *) responsePtr->buf_ptr, &fst, sizeof(fst));
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

static eEsifError HandleOscMode(UInt32 capabilities, const char *cur_node_name)
{
	eEsifError rc = ESIF_OK;

	if (capabilities) {	// DPTF to take over thermal control
		if (SysfsSetString(cur_node_name, "mode", "enabled") < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		}
	} else {
		if (SysfsSetString(cur_node_name, "mode", "disabled") < 0) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		}
	}

	return rc;
}

static eEsifError HandleOscRequest(const struct esif_data_complex_osc *oscPtr, const char *cur_node_name)
{
	eEsifError rc = ESIF_OK;
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	char guidStr[ESIF_GUID_PRINT_SIZE] = { 0 };
	esif_guid_t *guidPtr = (esif_guid_t *) oscPtr->guid;
	char *dptfGuid = "B23BA85D-C8B7-3542-88DE-8DE2FFCFD698";
	static UInt32 newOsc = ESIF_INVALID_ENUM_VALUE;
	Bool isDptfGuid = ESIF_TRUE;
	static UInt32 prevOscCap = 0;
	char *policies_guid[3] = {
		"3A95C389-E4B8-4629-A526-C52C88626BAE", /* ACTIVE   */
		"42A441D6-AE6A-462B-A84B-4A8CE79027D3", /* PASSIVE  */
		"97C68AE7-15FA-499C-B8C9-5DA81D606E0A"  /* CRITICAL */
	};

	// Convert IDSP GUID from binary format to text format because
	// INT3400/INTC1040 driver only takes text format as input
	esif_guid_mangle(guidPtr);
	esif_guid_print(guidPtr, guidStr);

	if ( esif_ccb_strlen(g_ManagerSysfsPath, sizeof(g_ManagerSysfsPath)) == 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_ERROR("g_ManagerSysfsPath Invalid\n");
		goto exit;
	}

	ESIF_TRACE_DEBUG("GUID received: %s and oscCap: 0x%x\n", guidStr, oscPtr->capabilities);
	if (newOsc == ESIF_INVALID_ENUM_VALUE) {
		if (SysfsSetStringWithError(g_ManagerSysfsPath, SYSFS_CURRENT_UUID, dptfGuid, ESIF_GUID_PRINT_SIZE) < 0) {
			ESIF_TRACE_DEBUG("Legacy OSC interface\n");
			newOsc = ESIF_FALSE;
		}
		else {
			ESIF_TRACE_DEBUG("New OSC interface\n");
			newOsc = ESIF_TRUE;
		}
	}

	isDptfGuid = esif_ccb_strncmp(guidStr, dptfGuid, ESIF_GUID_PRINT_SIZE) ? ESIF_FALSE : ESIF_TRUE;
	if (newOsc) {
		/* New OSC interface. Ignore Policy GUID and handle DPTF GUID */
		if (!isDptfGuid) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}

		if (oscPtr->capabilities != prevOscCap) {
			/* Reset current_uuid by disabling the mode */
			if (SysfsSetStringWithError(cur_node_name, "mode", "disabled", sizeof("disabled")) < 0) {
				rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
				goto exit;
			}
		}

		if (!(oscPtr->capabilities & 0x1)) {
			ESIF_TRACE_INFO("Dynamic Tuning is disabled\n");
			prevOscCap = oscPtr->capabilities;
			goto exit;
		}

		prevOscCap = 0x1;
		for (int i = 1; i <= sizeof(policies_guid)/sizeof(policies_guid[0]); i++) {
			if (oscPtr->capabilities & (0x1 << i)) {
				esif_ccb_strncpy(guidStr, policies_guid[i - 1], ESIF_GUID_PRINT_SIZE);
				if (SysfsSetStringWithError(g_ManagerSysfsPath, SYSFS_CURRENT_UUID, guidStr, ESIF_GUID_PRINT_SIZE) < 0) {
					ESIF_TRACE_WARN("Failed to set _OSC for DPTF GUID: %s\n", guidStr);
					rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
					goto exit;
				}
				prevOscCap |= 0x1 << i;
			}
		}

		rc = HandleOscMode(oscPtr->capabilities, cur_node_name);
		if (rc != ESIF_OK) {
			goto exit;
		}
	} else {
		/* Legacy OSC interface. Ignore DPTF GUID and handle Policy GUID */
		rc = HandleOscMode(oscPtr->capabilities, cur_node_name);
		if (rc != ESIF_OK) {
			goto exit;
		}

		if (isDptfGuid) {
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}

		if (SysfsSetStringWithError(g_ManagerSysfsPath, SYSFS_CURRENT_UUID, guidStr, ESIF_GUID_PRINT_SIZE) < 0) {
			ESIF_TRACE_WARN("Failed to set _OSC for GUID: %s\n", guidStr);
			rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
			goto exit;
		}
	}

	if (SysfsGetString((char *)cur_node_name, "policy", sysvalstring, sizeof(sysvalstring)) > -1) {
		if (SysfsSetString(cur_node_name, "policy", "user_space") < 0) {
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
		if (SysfsGetString(cur_node_name, "type", sysvalstring, sizeof(sysvalstring)) > -1) {
			if (esif_ccb_strlen(sysvalstring, MAX_SYSFS_PATH) == 0 || esif_ccb_strlen(sysvalstring, MAX_SYSFS_PATH) == MAX_SYSFS_PATH) {
					ESIF_TRACE_ERROR("Unable to get policy data from sysfs. \n");
					rc = ESIF_E_UNSPECIFIED;
					goto exit;
			}
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

static eEsifError SetImOk(EsifUpPtr upPtr, const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	UInt32 sysval = 0;

	if (ESIF_DATA_UINT32 != requestPtr->type ||
		sizeof(UInt32) != requestPtr->buf_len) {
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	sysval = (UInt32)*(UInt32 *)requestPtr->buf_ptr;
	if (SysfsSetInt64(g_ManagerSysfsPath, SYSFS_IMOK, sysval) < 0) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		ESIF_TRACE_ERROR("Error setting IMOK file with %d", sysval);
		goto exit;
	}
	ESIF_TRACE_INFO("IMOK set succesfully with value : %d", sysval);

exit:
	return rc;
}

/*
 *******************************************************************************
 * Register ACTION with ESIF
 *******************************************************************************
 */

static eEsifError SetThermalZonePolicy()
{
	DIR *dir = NULL;
	struct dirent **namelist;
	int n = 0;
	char cur_node_name[MAX_SYSFS_PATH] = { 0 };
	char sysvalstring[MAX_SYSFS_PATH] = { 0 };
	eEsifError rc = ESIF_E_UNSPECIFIED;

	dir = opendir(SYSFS_THERMAL);
	if (!dir) {
		ESIF_TRACE_DEBUG("No thermal sysfs\n");
		goto exit;
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
			if (SysfsGetString(cur_node_name, "policy", sysvalstring, sizeof(sysvalstring)) > -1) {
				if (esif_ccb_strlen(sysvalstring, MAX_SYSFS_PATH) == 0 || esif_ccb_strlen(sysvalstring, MAX_SYSFS_PATH) == MAX_SYSFS_PATH) {
					ESIF_TRACE_ERROR("Unable to get policy data from sysfs. \n");
					rc = ESIF_E_UNSPECIFIED;
					goto exit;
				}
				esif_ccb_sprintf(MAX_SYSFS_PATH,tzPolicies[n].policy,"%s",sysvalstring);
				if (0 != esif_ccb_strncmp(sysvalstring, "user_space", MAX_SYSFS_PATH)) {
					if (SysfsSetString(cur_node_name, "policy", "user_space") < 0) {
						ESIF_TRACE_WARN("Failed to change thermal zone policy type to user_space mode\n");
					}
				}
			}
			esif_ccb_free(namelist[n]);
		}
		esif_ccb_free(namelist);
	}
exit:
	if (dir) {
		closedir(dir);
	}
	return rc;
}

static eEsifError ResetThermalZonePolicy()
{
	DIR *dir = NULL;
	struct dirent **namelist;
	int n = 0;
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
		goto exit;
	}
	n = scandir(SYSFS_THERMAL, &namelist, 0, alphasort);
	if (n < 0) {
		//no scan
	}
	else {
		while (n--) {
			esif_ccb_sprintf(MAX_SYSFS_PATH, cur_node_name, "%s/%s", SYSFS_THERMAL,namelist[n]->d_name);
			if (SysfsGetString(cur_node_name, "policy", sysvalstring, sizeof(sysvalstring)) > -1) {
				if (SysfsSetString(cur_node_name, "policy", tzPolicies[n].policy) < 0) {
					ESIF_TRACE_WARN("Failed to change thermal zone policy type to default\n");
				}
			}
			esif_ccb_free(namelist[n]);
		}
		esif_ccb_free(namelist);
	}

	esif_ccb_free(tzPolicies);
exit:
	if (dir) {
		closedir(dir);
	}
	return rc;
}

static int AllocateThermalZones(void)
{
	DIR *dir = NULL;
	struct dirent **namelist = NULL;
	int n = 0;
	UInt32 numEntries = 0; 

	dir = opendir(SYSFS_THERMAL);
	if (!dir) {
		ESIF_TRACE_DEBUG("No thermal sysfs\n");
		return -1;
	}
	n = scandir(SYSFS_THERMAL, &namelist, 0, alphasort);
	if (n < 0) {
		//no scan
	}
	else {
		numEntries = n;
		while (n--) {
			esif_ccb_free(namelist[n]);
		}
		esif_ccb_free(namelist);
	}
	closedir(dir);
	g_thermalZonePtr = (thermalZonePtr)esif_ccb_malloc(numEntries * sizeof(thermalZone));
	if ( g_thermalZonePtr == NULL ) {
		ESIF_TRACE_ERROR("\n Insufficient Memory");
		return -1;
	}
	return 0;
}

static int DeAllocateThermalZones(void)
{
	if ( g_thermalZonePtr == NULL ) {
		ESIF_TRACE_ERROR("\n Thermal Zone is NULL ");
		return -1;
	}
	esif_ccb_free(g_thermalZonePtr);
	g_thermalZonePtr = NULL;
	return 0;
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
	AllocateThermalZones();
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
	DeAllocateThermalZones();
	ESIF_TRACE_EXIT_INFO();
}

#endif
/* end compile out entire page without ESIF_FEAT_OPT_SYSFS */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
