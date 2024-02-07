/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_LINUX

#include "esif_uf.h"
#include "esif_pm.h"
#include "esif_dsp.h"
#include "esif_uf_ccb_imp_spec.h"
#include "esif_uf_sysfs_os_lin.h"
#include "esif_enum_ietm_hid.h"
#include "esif_enum_acpi_device.h"
#include <dirent.h>

#define DEFAULT_DRIVER_NAME	""
#define DEFAULT_DEVICE_NAME	""

// Sysfs Paths
#define SYSFS_THERMAL	         "/sys/class/thermal"
#define SYSFS_PCI                "/sys/bus/pci/devices"
#define SYSFS_PLATFORM	         "/sys/bus/platform/devices"
#define SYSFS_DEVICES_PLATFORM   "/sys/devices/platform"
#define INSTANCE_ID_0            ":00"

#define MAX_FMT_STR_LEN 15          // "%<Int32>s"
#define HID_LEN   9                 // e.g INTC1045 ( 8 + 1 null character)
#define ACPI_DEVICE_NAME_LEN 4
#define DPTF_PARTICIPANT_PREFIX "INT"
#define ACPI_DPTF "DPTF"
#define SYSFS_DPTF_HID "INT3400"
#define SYSFS_PROCESSOR_HID "INT3401"
#define SYSFS_DEFAULT_HID "INT3403"
#define SYSFS_FAN_HID "INT3404"
#define SYSFS_DISP_HID "INT3406"
#define SYSFS_WWAN_HID "INT3408"
#define PARTICIPANT_FIELD_LEN 64
#define NUM_CPU_LOCATIONS 3
#define SIZE_OF_UINT64	(sizeof(UInt64))

#define INTEL_VENDOR_ID_STRING  "0x8086"
#define INTEL_VENDOR_ID         (0x8086)
#define SYSFS_VENDOR_ATTRIBUTE  "vendor"
#define SYSFS_DEVICE_ATTRIBUTE  "device"

static int g_zone_count = 0;
const char *CPU_location[NUM_CPU_LOCATIONS] = {"0000:00:04.0", "0000:00:0b.0", "0000:00:00.1"};
static Bool gSocParticipantFound = ESIF_FALSE;
SysfsReadEntry gSysfsReadTable[MAX_SYSFS_READ_ENTRY_SIZE] = { 0 }; 
UInt32 gNumberOfSysfsReadEntries = 0; 
// EVENT Notification for battery and power participant
SysfsAttrToEventMap gSysfsAttrToEventMapList[] = { 
{"platform_power_source", ESIF_EVENT_PLATFORM_POWER_SOURCE_CHANGED}, 
{"rest_of_platform_power_mw", ESIF_EVENT_PLATFORM_REST_OF_POWER_CHANGED}, 
{"max_platform_power_mw", ESIF_EVENT_MAX_BATTERY_POWER_CHANGED }, 
{"max_steady_state_power_mw",ESIF_EVENT_PLATFORM_BATTERY_STEADY_STATE_CHANGED},
{"no_load_voltage_mv",ESIF_EVENT_BATTERY_NO_LOAD_VOLTAGE_CHANGED}, 
{"battery_steady_power_mw", ESIF_EVENT_PLATFORM_BATTERY_STEADY_STATE_CHANGED}, 
{"high_freq_impedance_mohm", ESIF_EVENT_BATTERY_HIGH_FREQUENCY_IMPEDANCE_CHANGED} 
};

#define MAX_CORE_COUNT_SUPPORTED    (32)    // Supports max 32 cores per CPU Type
#define MAX_CPU_TYPES_SUPPORTED     (4)     // Supports only 4 different CPU types
#define CPU_SYSFS_PATH              "/sys/bus/cpu/devices"
#define ACPI_CPPC_SYSFS_REL_PATH    "/acpi_cppc/"
#define HIGH_PERF_SYSFS_NAME        "highest_perf"


typedef struct CapabilityData_t {
	UInt32  capabilityMask;
	UInt32  capabilityArray[MAX_MASK_SIZE];
} CapabilityData , *CapabilityDataPtr;

static Bool IsSupportedDttPlatformDevice(const char *hidDevice);
static void SysfsRegisterCustomParticipant(char *targetHID, char *targetACPIName, UInt32 pType);
static Int32 scanPCI(void);
static int scanPlat(void);
static int scanThermal(void);
static Bool match_thermal_zone(const char *matchToName, char *participant_path);
static enum esif_rc get_participant_name_alias(const char *ACPI_name, char *ACPI_alias);
static eEsifError newParticipantCreate(
	const UInt8 version,
	const esif_guid_t classGuid,
	const enum esif_participant_enum enumerator,
	const UInt32 flags,
	const char *name,
	const char *desc,
	const char *driverName,
	const char *deviceName,
	const char *devicePath,
	const char *objectId,
	const UInt32 acpiType,
	UInt16 vendorId,
	UInt16 deviceId
	);
static Bool IsValidDirectoryPath(const char* dirPath);
static Bool IsValidFilePath(char* filePath);
static eEsifError GetManagerSysfsPath();
static void EnumerateSysfsReadAttributes(char *ACPI_name,char *participant_path);
static void AddSysfsReadTableEntry(char *ACPI_name, char *sysfsAtrributePath, Int32 eventType);
static Bool IsIETMDevice(const char *hidDevice);
static Bool IsTCPUPlatformDevice(const char *hidDevice);
static eEsifError GetPCIDeviceId();

char g_ManagerSysfsPath[MAX_SYSFS_PATH] = { 0 };
char g_TCPUSysfsPath[MAX_SYSFS_PATH] = { 0 };
char g_PciDeviceIdStr[HID_LEN] = {0};
UInt32 g_PciDeviceId = 0;

struct participantInfo {
	char sysfsType[MAX_ZONE_NAME_LEN];
	char name[ESIF_NAME_LEN];
	char desc[ESIF_DESC_LEN];
	char deviceName[ESIF_NAME_LEN];
	char acpiScope[ESIF_SCOPE_LEN];
};

thermalZonePtr g_thermalZonePtr = NULL;

static void createParticipantsFromThermalSysfs(void)
{
	const struct participantInfo partInfo[] = {
		{"x86_pkg_temp", "TCPU", ESIF_PARTICIPANT_CPU_DESC, SYSFS_PROCESSOR_HID, "\\_SB_.TCPU"},
		{"Fan_ioc", "TFN1", ESIF_PARTICIPANT_PLAT_DESC, SYSFS_FAN_HID, "\\_SB_.TFN1"},
		{"board_temp", "TSKN", ESIF_PARTICIPANT_PLAT_DESC, SYSFS_DEFAULT_HID, "\\_SB_.TSKN"},
		{"ambient_temp", "TAMB", ESIF_PARTICIPANT_PLAT_DESC, SYSFS_DEFAULT_HID, "\\_SB_.TAMB"}
	};
	unsigned int i = 0;
	unsigned int j = 0;
	esif_guid_t classGuidPlat = ESIF_PARTICIPANT_PLAT_CLASS_GUID;
	esif_guid_t classGuidCpu = ESIF_PARTICIPANT_CPU_CLASS_GUID;
	esif_guid_t *classGuidPtr = &classGuidPlat;

	if ( g_thermalZonePtr == NULL ) {
		ESIF_TRACE_WARN("No ThermalZone detected");
		return;
	}

	for (i = 0 ; i < g_zone_count ; i++ ) {
		if (ESIF_TRUE == g_thermalZonePtr[i].bound) {
			continue;
		}
		for (j = 0; j < sizeof(partInfo) / sizeof(struct participantInfo); j++) {
			if (0 == esif_ccb_stricmp(g_thermalZonePtr[i].acpiCode, partInfo[j].sysfsType)) {
				// Found a matching sysfs thermal zone/cooling device
				if (0 == esif_ccb_stricmp(partInfo[j].deviceName, SYSFS_PROCESSOR_HID)) {
					if (gSocParticipantFound) continue; // No need to instantiate another SoC participant
					else {
						gSocParticipantFound = ESIF_TRUE;
						classGuidPtr = &classGuidCpu;
					}
				}

				newParticipantCreate(ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION,
					*classGuidPtr,
					ESIF_PARTICIPANT_ENUM_SYSFS,
					0x0, // flags
					partInfo[j].name,
					partInfo[j].desc,
					"N/A", // driver name - N/A for all Linux implementation
					partInfo[j].deviceName, // "INT340X"
					g_thermalZonePtr[i].thermalPath,
					partInfo[j].acpiScope,
					ESIF_PARTICIPANT_INVALID_TYPE, // Only WWAN/WIFI need to use different types
					0,
					0);

				break;
			}
		}
	}
}

static eEsifError ParseCapabilityStringConstant(char *capabilityStr,CapabilityDataPtr capabilityDataPtr )
{
	eEsifError rc = ESIF_OK;
	char *token = NULL;
	char *next_token = NULL;
	UInt8 capabilityArrayIndex = 0;

	//Extract the capability mask
	token = esif_ccb_strtok(capabilityStr,":",&next_token);
	if ( token == NULL ) {
		rc = ESIF_E_INVALID_CAPABILITY_MASK;
		goto exit;
	}

	esif_ccb_sscanf(token,"%x",&capabilityDataPtr->capabilityMask);
	ESIF_TRACE_DEBUG("\n Capability Mask : %x",capabilityDataPtr->capabilityMask );

	//Extract the capability array
	token = esif_ccb_strtok(NULL,":",&next_token);
	while (token != NULL) {
		esif_ccb_sscanf(token,"%x",&capabilityDataPtr->capabilityArray[capabilityArrayIndex]);
		ESIF_TRACE_DEBUG("\n Capability Array[%d] : %x",capabilityArrayIndex, capabilityDataPtr->capabilityArray[capabilityArrayIndex] );
		capabilityArrayIndex++;
		token = esif_ccb_strtok(NULL,":",&next_token);
	}

exit:
	return rc;
}

static void DumpCapabilityData(const EsifDomainCapabilityPtr capabilityDataPtr)
{
	ESIF_ASSERT(NULL != capabilityDataPtr);

	ESIF_TRACE_DEBUG("\n Capability Mask : %x", capabilityDataPtr->capability_flags);
	for ( int i = 0; i < MAX_MASK_SIZE ; i++) {
		ESIF_TRACE_DEBUG("\n Capability Array[%d] : %x",i, capabilityDataPtr->capability_mask[i]);
	}
}

static eEsifError UpdateCapabilityInFpc(EsifUpPtr upPtr,CapabilityDataPtr capabilityDataPtr)
{
	eEsifError rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	eEsifError iterRc = ESIF_OK;
	EsifFpcDomainIterator dspDomainiterator = { 0 };
	EsifFpcDomainPtr fpcDomainPtr = NULL;

	ESIF_ASSERT(NULL != upPtr);
	ESIF_ASSERT(NULL != capabilityDataPtr);

	// Update the capability in fpcptr first
	dspPtr = EsifUp_GetDsp(upPtr);
	if (NULL == dspPtr) {
		rc = ESIF_E_NOT_FOUND;
		ESIF_TRACE_DEBUG("\n Get DSP FAILED");
		goto exit;
	}

	if ((NULL == dspPtr->init_fpc_iterator) ||
		(NULL == dspPtr->get_next_fpc_domain)) {
		ESIF_TRACE_DEBUG("\n DSP is not initialized");
		rc = ESIF_E_NOT_INITIALIZED;
		goto exit;
	}

	iterRc = dspPtr->init_fpc_iterator(dspPtr, &dspDomainiterator);
	if (ESIF_OK != iterRc) {
		ESIF_TRACE_DEBUG("\n Init iterator Failed");
		goto exit;
	}

	iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
	while (ESIF_OK == iterRc) {
		if (NULL == fpcDomainPtr) {
			iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
			continue;
		}

		if ((UInt16) fpcDomainPtr->descriptor.domain == ESIF_PRIMITIVE_DOMAIN_D0) {
			ESIF_TRACE_DEBUG("\n Matching Domain ID found");
			ESIF_TRACE_DEBUG("\n Default Capabilities:");
			DumpCapabilityData(&fpcDomainPtr->capability_for_domain);

			/* Reset capabilities on the fpcPtr */
			fpcDomainPtr->capability_for_domain.capability_flags = capabilityDataPtr->capabilityMask;
			for ( int capabilityIndex = 0; capabilityIndex < MAX_MASK_SIZE ; capabilityIndex++) {
				fpcDomainPtr->capability_for_domain.capability_mask[capabilityIndex] = capabilityDataPtr->capabilityArray[capabilityIndex];
				if (fpcDomainPtr->capability_for_domain.capability_flags & (1 << capabilityIndex)) {
					fpcDomainPtr->capability_for_domain.number_of_capability_flags++;
				}
			}
			ESIF_TRACE_DEBUG("\n New Capabilities after Update:");
			DumpCapabilityData(&fpcDomainPtr->capability_for_domain);
			break;
		}

		iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
	}

	ESIF_TRACE_INFO("\n Capability updated in fpc succesfully!");

exit:
	return rc;
}

static eEsifError ReevaluateParticipantCapabilities(EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple tuple = {SET_PARTICIPANT_CAPABILITIES_EVAL, ESIF_PRIMITIVE_DOMAIN_D0, 255};
	UInt32 dummyData = 0;
	EsifData request = {ESIF_DATA_UINT32, &dummyData, sizeof(dummyData), sizeof(dummyData)};

	ESIF_ASSERT(NULL != upPtr);

	// Evaluate participant capability based on the new updated capabilities
	rc = EsifUp_ExecutePrimitive(upPtr, &tuple, &request, NULL);
	if (rc != ESIF_OK) {
		ESIF_TRACE_WARN("Reevaluation of participant capabilities failed");
		goto exit;
	}

	ESIF_TRACE_INFO("\n Capability updated succesfully!");

exit:
	return rc;
}

static eEsifError UpdateCapabilityVersionIfRequired()
{
	eEsifError rc = ESIF_OK;
	CapabilityData capability = { 0 };
	EsifFpcDomainPtr fpcDomainPtr = NULL;
	EsifPrimitiveTuple capabilityTuple = { GET_CAPABILITY_LIST, ESIF_PRIMITIVE_DOMAIN_D0 , 255 };
	char capabilityStr[128] = { 0 };
	EsifData capabilityData = { ESIF_DATA_STRING, &capabilityStr, sizeof(capabilityStr), sizeof(capabilityStr) };
	EsifUpPtr upPtr = NULL;

	// Retrieve the PCI Device ID
	rc = GetPCIDeviceId();
	if ( rc!= ESIF_OK) {
		goto exit;
	}

	// Retrieve the TCPU Participant handle
	upPtr = EsifUpPm_GetAvailableParticipantByName(ESIF_PARTICIPANT_CPU_NAME);
	if ( upPtr == NULL ) {
		ESIF_TRACE_WARN("TCPU Participant Not Found");
		rc = ESIF_E_NOT_FOUND;
		goto exit;		
	}

	// Execute the GET_CAPABILITY_LIST primitive
	// On legacy platforms this will fail and exit
	// For MTL and later, evaluate SOC Specific primtive to update the capabiilty version accordingly
	rc = EsifUp_ExecutePrimitive(upPtr, &capabilityTuple, NULL, &capabilityData);
	if (rc != ESIF_OK) {
		ESIF_TRACE_DEBUG("Error Executing Primitive %d . Return code: %s\n",
			capabilityTuple.id,
			esif_rc_str(rc));
		goto exit;
	}

	// Parse the capability string to structure
	ESIF_TRACE_INFO("\n Capability Data Retrieved : %s", capabilityStr);
	rc = ParseCapabilityStringConstant( capabilityStr, &capability);
	if (rc != ESIF_OK) {
		ESIF_TRACE_WARN("Capability string parse failed");
		goto exit;
	}

	// Update the capability in fpc first
	rc = UpdateCapabilityInFpc( upPtr, &capability);
	if (rc != ESIF_OK) {
		goto exit;
	}

	// Re-Evaluate participant capability based on the new updated capabilities
	rc = ReevaluateParticipantCapabilities( upPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	ESIF_TRACE_INFO("\n Capability updated succesfully!");

exit:
	EsifUp_PutRef(upPtr);
	return rc;
}

void SysfsRegisterParticipants ()
{
	eEsifError rc = ESIF_OK;
	EsifParticipantIface sysPart;
	const eEsifParticipantOrigin origin = eParticipantOriginUF;
	esif_handle_t newInstance = ESIF_INVALID_HANDLE;
	char *spDesc = "DPTF Zone";
	char *spObjId = "\\_SB_.IETM";
	char *spType = "IETM";
	char *spDevicePath = "NA";
	char *spDevice = SYSFS_DPTF_HID;
	const esif_guid_t classGuid = ESIF_PARTICIPANT_PLAT_CLASS_GUID;

	sysPart.version = ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION;
	sysPart.enumerator = ESIF_PARTICIPANT_ENUM_SYSFS;
	sysPart.flags = 1;
	sysPart.send_event = NULL;
	sysPart.recv_event = NULL;
	esif_ccb_strncpy(sysPart.name,spType,ESIF_ACPI_NAME_LEN);
	esif_ccb_strncpy(sysPart.desc,spDesc,PARTICIPANT_FIELD_LEN);
	esif_ccb_strncpy(sysPart.object_id,spObjId,PARTICIPANT_FIELD_LEN);
	esif_ccb_strncpy(sysPart.device_path,spDevicePath,PARTICIPANT_FIELD_LEN);
	esif_ccb_strncpy(sysPart.device_name,spDevice,PARTICIPANT_FIELD_LEN);
	esif_ccb_sprintf(ESIF_NAME_LEN, sysPart.driver_name, "sysfs%s", ESIF_LIB_EXT);

	/* we forced IETM to index 0 for now because ESIF expects that */
	EsifUpPm_RegisterParticipant(origin, &sysPart, &newInstance);
	GetManagerSysfsPath();

	// Initialize TCPU sysfs path to 0
	esif_ccb_memset(g_TCPUSysfsPath,0,sizeof(g_TCPUSysfsPath));

	scanThermal();
	scanPCI();
	scanPlat();

	// On platforms without DPTF BIOS support, the above scanPCI() and
	// scanPlat() will not result in the creation of the SoC participant and
	// possibly others (fan, board temperature sensor, etc.)
	// In this case, we will manually create DPTF participants based on some
	// commonly known types exposed in sysfs such as "x86_pkg_temp"
	createParticipantsFromThermalSysfs();

	// Check if capability version needs to be update for TCPU participant
	rc = UpdateCapabilityVersionIfRequired();
	if ( rc != ESIF_OK ) {
		ESIF_TRACE_WARN("Failed to update the capability version\n");
	}

	// Special handling for Android OS that may have Java based participants
	// Possibly add two more Java participants - Display and WWAN - but only if they do not exist yet
	UNREFERENCED_PARAMETER(classGuid);
exit:
	return;
}

static eEsifError newParticipantCreate(
	const UInt8 version,
	const esif_guid_t classGuid,
	const enum esif_participant_enum enumerator,
	const UInt32 flags,
	const char *name,
	const char *desc,
	const char *driverName,
	const char *deviceName,
	const char *devicePath,
	const char *objectId,
	const UInt32 acpiType,
	UInt16 vendorId,
	UInt16 deviceId
	)
{
	EsifParticipantIface sysPart;
	const eEsifParticipantOrigin origin = eParticipantOriginUF;
	esif_handle_t newInstance = ESIF_INVALID_HANDLE;
	int guid_element_counter = 0;

	ESIF_ASSERT(NULL != classGuid);

	for (guid_element_counter = 0; guid_element_counter < ESIF_GUID_LEN; guid_element_counter++) {
		sysPart.class_guid[guid_element_counter] = *(classGuid + guid_element_counter);
	}

	sysPart.version = version;
	sysPart.enumerator = enumerator;
	sysPart.flags = flags;
	sysPart.send_event = NULL;
	sysPart.recv_event = NULL;
	sysPart.acpi_type = acpiType;
	sysPart.pciVendorId = vendorId;
	sysPart.pciDeviceId = deviceId;

	esif_ccb_strncpy(sysPart.name,name,ESIF_NAME_LEN);
	esif_ccb_strncpy(sysPart.desc,desc,ESIF_DESC_LEN);
	esif_ccb_strncpy(sysPart.object_id,objectId,ESIF_SCOPE_LEN);
	esif_ccb_strncpy(sysPart.device_path,devicePath,ESIF_PATH_LEN);
	esif_ccb_strncpy(sysPart.driver_name,driverName,ESIF_NAME_LEN);
	esif_ccb_strncpy(sysPart.device_name,deviceName,ESIF_NAME_LEN);

	EsifUpPm_RegisterParticipant(origin, &sysPart, &newInstance);

	return ESIF_OK;
}

static Int32 PciDeviceFilter(const struct dirent *entry)
{
	//Filter only the available CPU Locations
	for (UInt32 cpu_loc_counter = 0;cpu_loc_counter < NUM_CPU_LOCATIONS; cpu_loc_counter++) {
		if (esif_ccb_strstr(entry->d_name, CPU_location[cpu_loc_counter]) != NULL ) {
			return 1;
		}
	}
	return 0;
};

static Bool IsIntelDevice(const char * pciDevicePath)
{
	Bool vendorIdFound = ESIF_FALSE;
	char vendorIdStr[ESIF_SCOPE_LEN] = { 0 };
	ESIF_ASSERT(NULL != pciDevicePath);

	if (SysfsGetString(pciDevicePath,SYSFS_VENDOR_ATTRIBUTE, vendorIdStr, sizeof(vendorIdStr)) < 0) {
		ESIF_TRACE_ERROR("No vendor info found for PCI Device\n");
		goto exit;
	}

	if (esif_ccb_strcmp(vendorIdStr, INTEL_VENDOR_ID_STRING) != 0) {
		// Not found, exiting
		goto exit;
	}

	// Vendor ID Found
	vendorIdFound = ESIF_TRUE;
exit:
	return vendorIdFound;
}

static eEsifError GetPCIDeviceId()
{
	eEsifError rc = ESIF_OK;

	if ( esif_ccb_strlen(g_TCPUSysfsPath, sizeof(g_TCPUSysfsPath)) == 0) {
		ESIF_TRACE_WARN("TCPU PCI path is not valid\n");
		rc = ESIF_E_NOT_SUPPORTED;
	}

	if (SysfsGetString(g_TCPUSysfsPath,SYSFS_DEVICE_ATTRIBUTE, g_PciDeviceIdStr, sizeof(g_PciDeviceIdStr)) < 0) {
		ESIF_TRACE_ERROR("No Device info found for PCI Device\n");
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	errno = 0;
	g_PciDeviceId= strtol(g_PciDeviceIdStr, NULL, 0);
	if (errno != 0) {
		ESIF_TRACE_WARN("\n strtol falied with error code : %d", errno);
		g_PciDeviceId = 0;
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	ESIF_TRACE_INFO("\n PCI Device ID : %s (%x)", g_PciDeviceIdStr, g_PciDeviceId);

exit:
	return rc;
}

static Int32 scanPCI(void)
{
	eEsifError rc = ESIF_OK;
	Int32 returnCode = 0;
	DIR *dir = NULL;
	struct dirent **namelist = NULL;
	int n = 0;
	char participant_path[MAX_SYSFS_PATH] = { 0 };
	char firmware_path[MAX_SYSFS_PATH] = { 0 };
	char participant_scope[ESIF_SCOPE_LEN] = { 0 };
	UInt32 ptype = ESIF_DOMAIN_TYPE_INVALID;
	esif_guid_t classGuid = ESIF_PARTICIPANT_CPU_CLASS_GUID;

	dir = opendir(SYSFS_PCI);
	if (dir == NULL) {
		ESIF_TRACE_DEBUG("No PCI sysfs\n");
		returnCode = -1;
		goto exit;
	}
	n = scandir(SYSFS_PCI, &namelist, PciDeviceFilter, alphasort);
	if (n < 0) {
		ESIF_TRACE_DEBUG("No Matching PCI path exists\n");
		returnCode = -1;
		goto exit;
	}
	// We need to iterate this loop in ascending order so that 0/0/4 will be enumerated 
	// before 0/0/b PCI Device
	for (UInt32 i = 0 ; i < n ; i++) {

		// We should not break out of this loop and it can result in memory leaks
		// we need to continue inside the loop to free each namelist entries
		if ( gSocParticipantFound == ESIF_TRUE ) {
			esif_ccb_free(namelist[i]);
			continue;
		}

		esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s/%s", SYSFS_PCI,namelist[i]->d_name);
		// Skip if it is not an Intel Device
		if (IsIntelDevice(participant_path) == ESIF_FALSE) {
			esif_ccb_free(namelist[i]);
			continue;
		}

		esif_ccb_sprintf(MAX_SYSFS_PATH, firmware_path, "%s/firmware_node", participant_path);

		if (SysfsGetString(firmware_path,"path", participant_scope, sizeof(participant_scope)) < 0) {
			esif_ccb_free(namelist[i]);
			continue;
		}

		int scope_len = esif_ccb_strlen(participant_scope,ESIF_SCOPE_LEN);
		if (scope_len < ACPI_DEVICE_NAME_LEN) {
			esif_ccb_free(namelist[i]);
			continue;
		}

		char *ACPI_name = participant_scope + (scope_len - ACPI_DEVICE_NAME_LEN);
		esif_ccb_sprintf(sizeof(g_TCPUSysfsPath), g_TCPUSysfsPath, "%s", participant_path);

		/* map to thermal zone (try pkg thermal zone first)*/
		for (UInt32 thermal_counter=0; thermal_counter < g_zone_count; thermal_counter++) {
			thermalZone tz = (thermalZone)g_thermalZonePtr[thermal_counter];
			if (esif_ccb_strcmp(tz.acpiCode, ACPI_name)==0) {
				// Re-initialize the device path to one of the thermal zones
				esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s", tz.thermalPath);
				break;
			}
		}

		rc = GetPCIDeviceId();
		if ( rc != 0 ) {
			continue;
		}

		if ( esif_ccb_strcmp(esif_device_str(g_PciDeviceId), ESIF_NOT_AVAILABLE) == 0) {
			ESIF_TRACE_WARN("Unsupported PCI Device ID detected. Enumerating as legacy tcpu participant device\n");
			newParticipantCreate(ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION,
				classGuid,
				ESIF_PARTICIPANT_ENUM_SYSFS,
				0x0,
				ESIF_PARTICIPANT_CPU_NAME,
				ESIF_PARTICIPANT_CPU_DESC,
				"N/A",
				SYSFS_PROCESSOR_HID,
				participant_path,
				participant_scope,
				ptype,
				0,
				0
				);
		}
		else {
			//Enumerating as PCI Participant
			newParticipantCreate(ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION,
				classGuid,
				ESIF_PARTICIPANT_ENUM_PCI,
				0x0,
				ESIF_PARTICIPANT_CPU_NAME,
				ESIF_PARTICIPANT_CPU_DESC,
				"N/A",
				"N/A",
				participant_path,
				participant_scope,
				ptype,
				INTEL_VENDOR_ID, // TBAKP
				g_PciDeviceId);
		}

		ESIF_TRACE_INFO("TCPU Participant Created\n");
		gSocParticipantFound = ESIF_TRUE;
		esif_ccb_free(namelist[i]);
	}

	esif_ccb_free(namelist);

exit:
	if ( dir != NULL) {
		closedir(dir);
	}

	return returnCode;
}

static Int32 PlatformDeviceFilter(const struct dirent *entry)
{
	Int32 returnValue = 0;
	char *inputStr = esif_ccb_strdup(entry->d_name);
	if ( inputStr == NULL ) {
		ESIF_TRACE_ERROR("\nNO MEMORY");
		goto exit;
	}
	// Returns first token 
	// e.g If string INT3401:00 we extract only INT3401
	char *hidStr = strtok(inputStr, ":");

	if (esif_ccb_strcmp(esif_acpi_device_str(hidStr), ESIF_NOT_AVAILABLE) != 0) {
		ESIF_TRACE_DEBUG("Platform Device Found: %s\n", inputStr);
		returnValue = 1;
	}
exit:
	esif_ccb_free(inputStr);
	return returnValue;
};

static int scanPlat(void)
{
	int returnCode = -1;
	DIR *dir;
	struct dirent **namelist;
	int n = 0;
	char participant_path[MAX_SYSFS_PATH] = { 0 };
	char firmware_path[MAX_SYSFS_PATH] = { 0 };
	char participant_scope[ESIF_SCOPE_LEN] = { 0 };
	char hid[HID_LEN] = { 0 };
	UInt32 ptype = ESIF_PARTICIPANT_INVALID_TYPE;
	esif_guid_t classGuid = ESIF_PARTICIPANT_PLAT_CLASS_GUID;

	dir = opendir(SYSFS_PLATFORM);
	if (!dir) {
		ESIF_TRACE_INFO("No platform sysfs\n");
		returnCode = -1;
		goto exit;
	}

	n = scandir(SYSFS_PLATFORM, &namelist, PlatformDeviceFilter, alphasort);
	if (n < 0) {
		ESIF_TRACE_WARN("No Matching Platform Device path exists\n");
		returnCode = -1;
		goto exit;
	}

	while (n--) {

		esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s/%s", SYSFS_PLATFORM,namelist[n]->d_name);
		esif_ccb_sprintf(MAX_SYSFS_PATH, firmware_path, "%s/firmware_node", participant_path);

		if (SysfsGetString(firmware_path,"hid", hid, sizeof(hid)) < 1) {
			esif_ccb_strncpy(hid,SYSFS_DEFAULT_HID,HID_LEN);
		}

		if (IsIETMDevice(hid)) {
			// DPTF (IETM) participant has already been created prior to the scanPlat() call
			ESIF_TRACE_INFO("\nIETM Device found. Skipping...");
			goto exit_participant;
		}

		if (SysfsGetString(firmware_path,"path", participant_scope, sizeof(participant_scope)) < 1) {
			ESIF_TRACE_ERROR("\n Error reading path for in paht : %s" , firmware_path);
			goto exit_participant;
		}

		int scope_len = esif_ccb_strlen(participant_scope,ESIF_SCOPE_LEN);
		if (scope_len < ACPI_DEVICE_NAME_LEN) {
			ESIF_TRACE_ERROR("\n Invalid ACPI Scope Length in participant Scope: %s" , participant_scope);
			goto exit_participant;;
		}

		char *ACPI_name = participant_scope + (scope_len - ACPI_DEVICE_NAME_LEN);
		char *ACPI_alias = NULL;
		ACPI_alias = esif_ccb_malloc(MAX_ZONE_NAME_LEN);
		if (ACPI_alias == NULL) {
			//no memory
			ESIF_TRACE_ERROR("\nNO MEMORY");
			goto exit_participant;
		}

		// Check if it is TCPU Participant device by comparing the HID to INT3401
		if ( gSocParticipantFound == ESIF_FALSE && IsTCPUPlatformDevice(hid) == ESIF_TRUE) {
			esif_ccb_sprintf(sizeof(g_TCPUSysfsPath), g_TCPUSysfsPath, "%s", participant_path);
			gSocParticipantFound = ESIF_TRUE;
		}

		/* map to thermal zone */
		get_participant_name_alias(ACPI_name, ACPI_alias);
		if (match_thermal_zone(ACPI_alias, participant_path) == ESIF_TRUE) {
			newParticipantCreate(ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION,
					classGuid,
					ESIF_PARTICIPANT_ENUM_SYSFS,
					0x0,
					ACPI_name,
					ACPI_name,
					"N/A",
					hid,
					participant_path,
					participant_scope,
					ptype,
					0,
					0);
		}
		// For devices with no mapping in thermal zone,
		else {
			newParticipantCreate(ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION,
					classGuid,
					ESIF_PARTICIPANT_ENUM_SYSFS,
					0x0,
					ACPI_name,
					ACPI_name,
					"N/A",
					hid,
					participant_path,
					participant_scope,
					ptype,
					0,
					0);
			ESIF_TRACE_INFO("Participant created with HID : %s", hid);
			EnumerateSysfsReadAttributes(ACPI_name,participant_path);
		}
		esif_ccb_free(ACPI_alias);
exit_participant:
		esif_ccb_free(namelist[n]);
	}
	esif_ccb_free(namelist);
	returnCode = 0;
exit:
	if (dir != NULL) {
		closedir(dir);
	}

	return returnCode;
}

static void AddSysfsReadTableEntry(char *ACPI_name, char *sysfsAtrributePath, Int32 eventType)
{
	ESIF_ASSERT(ACPI_name != NULL);
	ESIF_ASSERT(sysfsAtrributePath != NULL);

	esif_ccb_strcpy(gSysfsReadTable[gNumberOfSysfsReadEntries].participantName, 
		ACPI_name,
		sizeof(gSysfsReadTable[gNumberOfSysfsReadEntries].participantName));

	esif_ccb_strcpy(gSysfsReadTable[gNumberOfSysfsReadEntries].sysfsPath, 
		sysfsAtrributePath,
		sizeof(gSysfsReadTable[gNumberOfSysfsReadEntries].sysfsPath));

	gSysfsReadTable[gNumberOfSysfsReadEntries].eventType = eventType;
	gNumberOfSysfsReadEntries++;

	ESIF_TRACE_INFO("Valid Sysfs Poll Entry found : %s\n", sysfsAtrributePath);
}

static void EnumerateSysfsReadAttributes(char *ACPI_name,char *participant_path)
{
	ESIF_ASSERT(ACPI_name != NULL);
	ESIF_ASSERT(participant_path != NULL);

	for (UInt32 i = 0; i < sizeof(gSysfsAttrToEventMapList)/sizeof(gSysfsAttrToEventMapList[0]); i++) {
		char filepath[MAX_SYSFS_FILENAME] = { 0 };
		esif_ccb_strcpy(filepath,participant_path,MAX_SYSFS_FILENAME);
		esif_ccb_strcat(filepath,"/dptf_power/",MAX_SYSFS_FILENAME);
		esif_ccb_strcat(filepath,gSysfsAttrToEventMapList[i].attributeName,MAX_SYSFS_FILENAME);
		if(IsValidFilePath(filepath) && gNumberOfSysfsReadEntries < MAX_SYSFS_READ_ENTRY_SIZE) {
			AddSysfsReadTableEntry(ACPI_name, filepath ,gSysfsAttrToEventMapList[i].eventType);
		}

		esif_ccb_memset(filepath, 0 , sizeof(filepath));
		esif_ccb_strcpy(filepath,participant_path,MAX_SYSFS_FILENAME);
		esif_ccb_strcat(filepath,"/dptf_battery/",MAX_SYSFS_FILENAME);
		esif_ccb_strcat(filepath,gSysfsAttrToEventMapList[i].attributeName,MAX_SYSFS_FILENAME);
		if(IsValidFilePath(filepath) && gNumberOfSysfsReadEntries < MAX_SYSFS_READ_ENTRY_SIZE) {
			AddSysfsReadTableEntry(ACPI_name, filepath ,gSysfsAttrToEventMapList[i].eventType);
		}
	}
	ESIF_TRACE_DEBUG("Number of Entries : %d\n", gNumberOfSysfsReadEntries);
	return;
}

static int scanThermal(void)
{
	DIR *dir;
	struct dirent **namelist;
	int n;
	int thermal_counter = 0;

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
		while (n--) {
			enum zoneType zt = THERM;
			char target_path[MAX_SYSFS_PATH] = { 0 };
			char acpi_name[MAX_ZONE_NAME_LEN] = { 0 };
			char *zone_indicator = NULL;

			/* find out if cooling device or thermal zone (or invalid) */
			zone_indicator = esif_ccb_strstr(namelist[n]->d_name,"cooling_device");
			if (zone_indicator == NULL) {
				zone_indicator = esif_ccb_strstr(namelist[n]->d_name,"thermal_zone");
			}
			else {
				zt = CDEV;
			}
			/* invalid directory */
			if (zone_indicator == NULL) {
				goto exit_zone;
			}
			esif_ccb_sprintf(MAX_SYSFS_PATH, target_path, "%s/%s", SYSFS_THERMAL,namelist[n]->d_name);

			if (SysfsGetString(target_path,"type", acpi_name, sizeof(acpi_name)) > -1) {
				thermalZone tz = { 0 };
				tz.zoneType = zt;
				esif_ccb_sprintf(MAX_ZONE_NAME_LEN,tz.acpiCode,"%s",acpi_name);
				esif_ccb_sprintf(MAX_SYSFS_PATH,tz.thermalPath,"%s",target_path);
				g_thermalZonePtr[thermal_counter] = tz;
				thermal_counter++;
			}

exit_zone:
			esif_ccb_free(namelist[n]);
		}
		esif_ccb_free(namelist);
	}
	closedir(dir);

	g_zone_count = thermal_counter ? thermal_counter + 1 : 0 ;

	return 0;
}

static Bool match_thermal_zone(const char *matchToName, char *participant_path)
{
	int thermal_counter = 0;
	Bool thermalZoneFound = ESIF_FALSE;

	for (thermal_counter=0; thermal_counter < g_zone_count; thermal_counter++) {
		thermalZone tz = (thermalZone)g_thermalZonePtr[thermal_counter];
		if (esif_ccb_strcmp(tz.acpiCode, matchToName)==0) {
			thermalZoneFound = ESIF_TRUE;
			g_thermalZonePtr[thermal_counter].bound = ESIF_TRUE;
			esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s", tz.thermalPath);
			break;
		}
	}

	return thermalZoneFound;
}

static enum esif_rc get_participant_name_alias(const char *ACPI_name, char *ACPI_alias)
{
	enum esif_rc rc = ESIF_OK;
	struct { const char *name; const char *alias; } participant_aliases[] = {
		{ NULL, NULL }
	};
	int j = 0;

	if (ACPI_name == NULL || ACPI_alias == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Lookup Participant Name Alias in list and return it if found otherwise use name
	for (j = 0; participant_aliases[j].name != NULL && participant_aliases[j].alias != NULL; j++) {
		if (esif_ccb_strcmp(ACPI_name, participant_aliases[j].name) == 0) {
			esif_ccb_strncpy(ACPI_alias, participant_aliases[j].alias, MAX_ZONE_NAME_LEN);
			goto exit;
		}
	}
	esif_ccb_strncpy(ACPI_alias, ACPI_name, MAX_ZONE_NAME_LEN);

exit:
	return rc;
}

static Bool IsValidDirectoryPath(const char* dirPath)
{
	Bool isValid = ESIF_FALSE;
	DIR* dir = opendir(dirPath);
	if (dir != NULL) { // directory is valid
		isValid = ESIF_TRUE;
		closedir(dir);
	}

	ESIF_TRACE_DEBUG("Directory Path %s is %s" , dirPath , isValid ? "Valid" : "Not Valid");
	return isValid;
}

static Bool IsValidFilePath(char* filePath)
{
	ESIF_ASSERT(NULL != filePath);

	Bool isValid = ESIF_FALSE;
	char buffer[SIZE_OF_UINT64] = {'\0'};

	FILE *fp = esif_ccb_fopen(filePath, "r", NULL);
	if (fp != NULL) {
		if ((esif_ccb_fseek(fp, 0, SEEK_SET) == 0) && (esif_ccb_fread(buffer, 1, 1, 1, fp) > 0)) {
			isValid = ESIF_TRUE;
		}
		esif_ccb_fclose(fp);
	}
	ESIF_TRACE_DEBUG("File Path %s for Read is %s" , filePath , isValid ? "Valid" : "Not Valid");
	return isValid;
}

static eEsifError GetManagerSysfsPath()
{
	eEsifError rc = ESIF_OK;
	esif_ccb_memset( g_ManagerSysfsPath, 0, sizeof(g_ManagerSysfsPath));

	const char **hids = esif_enum_ietm_hid();
	for (UInt32 i = 0; hids != NULL && hids[i] != NULL; i++) {
		ESIF_TRACE_DEBUG("Device ID : %s\n", hids[i]);
		esif_ccb_sprintf(sizeof(g_ManagerSysfsPath), g_ManagerSysfsPath, "%s/%s%s", SYSFS_DEVICES_PLATFORM,hids[i],INSTANCE_ID_0);

		if (IsValidDirectoryPath(g_ManagerSysfsPath) == ESIF_TRUE) {
			//Found a valid manager sysfs path, return
			ESIF_TRACE_INFO("Manager Sysfs Path : %s\n", g_ManagerSysfsPath);
			break;
		}

		esif_ccb_memset( g_ManagerSysfsPath, 0, sizeof(g_ManagerSysfsPath));
	}

	if ( esif_ccb_strlen(g_ManagerSysfsPath, sizeof(g_ManagerSysfsPath)) == 0) {
		rc = ESIF_E_NOT_FOUND;
		ESIF_TRACE_ERROR("GetManagerSysfsPathFromAcpiId failed\n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("GetManagerSysfsPathFromAcpiId completed succesfully\n");

exit:
	return rc;
}

static Bool IsSupportedDttPlatformDevice(const char *hidDevice)
{
	Bool isSupported = ESIF_FALSE;
	ESIF_ASSERT(NULL != hidDevice);

	if (esif_ccb_strcmp(esif_acpi_device_str((esif_string)hidDevice), ESIF_NOT_AVAILABLE) != 0) {
		ESIF_TRACE_INFO("Platform Device Found: %s\n", hidDevice);
		isSupported = ESIF_TRUE;
	}

	return isSupported;
}

static Bool IsIETMDevice(const char *hidDevice)
{
	const char **hids = esif_enum_ietm_hid();
	if (hids == NULL) {
		return ESIF_FALSE;
	}

	for (size_t i = 0; hids != NULL && hids[i] != NULL; i++) {
		if (esif_ccb_strcmp(hidDevice, hids[i]) == 0) {
			// Found the manager participant return true
			ESIF_TRACE_INFO("\n Found Manager Device ");
			return ESIF_TRUE;
		}
	}
	return ESIF_FALSE;
}

static Bool IsTCPUPlatformDevice(const char *hidDevice)
{
	if (esif_ccb_strcmp(hidDevice, ESIF_ACPI_DEVICE_INT3401) == 0) {
		ESIF_TRACE_INFO("\n TCPU Platform Device Found ");
		return ESIF_TRUE;
	}
	return ESIF_FALSE;
}

