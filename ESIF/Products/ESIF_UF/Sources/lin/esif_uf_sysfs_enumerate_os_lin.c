/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#define DEFAULT_DRIVER_NAME	""
#define DEFAULT_DEVICE_NAME	""

// Sysfs Paths
#define SYSFS_THERMAL	"/sys/class/thermal"
#define SYSFS_PCI		"/sys/bus/pci/devices"
#define SYSFS_PLATFORM	"/sys/bus/platform/devices"

#define MAX_SYSFS_STRING 4 * 1024
#define MAX_SYSFS_PATH 256
#define MAX_FMT_STR_LEN 15 // "%<Int32>s"
#define MAX_ZONE_NAME_LEN 56
#define HID_LEN 8
#define ACPI_DEVICE_NAME_LEN 4
#define DPTF_PARTICIPANT_PREFIX "INT340"
#define ACPI_DPTF "INT3400:00"
#define SYSFS_DPTF_HID "INT3400"
#define SYSFS_PROCESSOR_HID "INT3401"
#define SYSFS_DEFAULT_HID "INT3403"
#define SYSFS_FAN_HID "INT3404"
#define SYSFS_DISP_HID "INT3406"
#define SYSFS_WWAN_HID "INT3408"
#define PARTICIPANT_FIELD_LEN 64
#define NUM_CPU_LOCATIONS 3

static int g_zone_count = 0;
const char *CPU_location[NUM_CPU_LOCATIONS] = {"0000:00:04.0", "0000:00:0b.0", "0000:00:00.1"};
static Bool gSocParticipantFound = ESIF_FALSE;

static int sysfsGetString(char *path, char *filename, char *str, size_t buf_len);
static int sysfsGetU64(char *path, char *filename, u64 *p_u64);
static int sysfsSetU64(char *path, char *filename, u64 val);
static int sysfsSetString(char *path, char *filename, char *val);
static void SysfsRegisterCustomParticipant(char *targetHID, char *targetACPIName, UInt32 pType);
static int scanPCI(void);
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
	const UInt32 acpiType
	);

enum zoneType {
	THERM,
	CDEV,
};

struct participantInfo {
	char sysfsType[MAX_ZONE_NAME_LEN];
	char name[ESIF_NAME_LEN];
	char desc[ESIF_DESC_LEN];
	char deviceName[ESIF_NAME_LEN];
	char acpiScope[ESIF_SCOPE_LEN];
};

struct thermalZone {
	Bool bound;
	enum zoneType zoneType;
	char acpiCode[MAX_ZONE_NAME_LEN];
	char thermalPath[MAX_SYSFS_PATH];
};

struct thermalZone thermalZones[MAX_PARTICIPANT_ENTRY] = { {0} };

static int sysfsGetString(char *path, char *filename, char *str, size_t buf_len)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	char scanf_fmt[MAX_FMT_STR_LEN] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		return ret;
	}

	// Use dynamic format width specifier to avoid scanf buffer overflow
	esif_ccb_sprintf(sizeof(scanf_fmt), scanf_fmt, "%%%ds", (int)buf_len - 1);
	ret = esif_ccb_fscanf(fd, scanf_fmt, str);
	esif_ccb_fclose(fd);

	return ret;
}

static int sysfsGetU64(char *path, char *filename, u64 *p_u64)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		return ret;
	}
	ret = esif_ccb_fscanf(fd, "%llu", (u64 *)p_u64);

	esif_ccb_fclose(fd);

	return 0;
}

static int sysfsSetU64(char *path, char *filename, u64 val)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "w", NULL)) == NULL) {
		return ret;
	}

	ret = esif_ccb_fprintf(fd, "%llu", (u64)val);
	esif_ccb_fclose(fd);

	return 0;
}

static int sysfsSetString(char *path, char *filename, char *val)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "w", NULL)) == NULL) {
		return ret;
	}

	ret = esif_ccb_fprintf(fd, "%s", val);
	esif_ccb_fclose(fd);

	return 0;
}

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

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (ESIF_TRUE == thermalZones[i].bound) continue;
		for (j = 0; j < sizeof(partInfo) / sizeof(struct participantInfo); j++) {
			if (0 == esif_ccb_stricmp(thermalZones[i].acpiCode, partInfo[j].sysfsType)) {
				// Found a matching sysfs thermal zone/cooling device
				if (0 == esif_ccb_stricmp(partInfo[j].deviceName, SYSFS_PROCESSOR_HID)) {
					if (gSocParticipantFound) continue; // No need to instantiate another SoC participant
					else {
						gSocParticipantFound = ESIF_TRUE;
						classGuidPtr = &classGuidCpu;
					}
				}

				newParticipantCreate(ESIF_PARTICIPANT_VERSION,
					*classGuidPtr,
					ESIF_PARTICIPANT_ENUM_SYSFS,
					0x0, // flags
					partInfo[j].name,
					partInfo[j].desc,
					"N/A", // driver name - N/A for all Linux implementation
					partInfo[j].deviceName, // "INT340X"
					thermalZones[i].thermalPath,
					partInfo[j].acpiScope,
					ESIF_PARTICIPANT_INVALID_TYPE); // Only WWAN/WIFI need to use different types

				break;
			}
		}
	}
}

void SysfsRegisterParticipants ()
{
	EsifParticipantIface sysPart;
	const eEsifParticipantOrigin origin = eParticipantOriginUF;
	UInt8 newInstance = ESIF_INSTANCE_INVALID;
	char *spDesc = "DPTF Zone";
	char *spObjId = "\\_SB_.IETM";
	char *spType = "IETM";
	char *spDevicePath = "NA";
	char *spDevice = SYSFS_DPTF_HID;
	const esif_guid_t classGuid = ESIF_PARTICIPANT_PLAT_CLASS_GUID;

	sysPart.version = ESIF_PARTICIPANT_VERSION;
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

	EsifUpPm_RegisterParticipant(origin, &sysPart, &newInstance);

	scanThermal();
	scanPCI();
	scanPlat();

	// On platforms without DPTF BIOS support, the above scanPCI() and
	// scanPlat() will not result in the creation of the SoC participant and
	// possibly others (fan, board temperature sensor, etc.)
	// In this case, we will manually create DPTF participants based on some
	// commonly known types exposed in sysfs such as "x86_pkg_temp"
	createParticipantsFromThermalSysfs();

	// Special handling for Android OS that may have Java based participants
	// Possibly add two more Java participants - Display and WWAN - but only if they do not exist yet
#ifdef ESIF_ATTR_OS_ANDROID
	if (!EsifUpPm_DoesAvailableParticipantExistByHID(SYSFS_DISP_HID)) {
		newParticipantCreate(ESIF_PARTICIPANT_VERSION,
			classGuid,
			ESIF_PARTICIPANT_ENUM_SYSFS,
			0x0,
			"TDSP",
			ESIF_PARTICIPANT_PLAT_DESC,
			"N/A",
			SYSFS_DISP_HID,
			"N/A",
			"\\_SB_.TDSP",
			ESIF_PARTICIPANT_INVALID_TYPE);
	}
	if (!EsifUpPm_DoesAvailableParticipantExistByHID(SYSFS_WWAN_HID)) {
		newParticipantCreate(ESIF_PARTICIPANT_VERSION,
			classGuid,
			ESIF_PARTICIPANT_ENUM_SYSFS,
			0x0,
			"WWAN",
			ESIF_PARTICIPANT_PLAT_DESC,
			"N/A",
			SYSFS_WWAN_HID,
			"N/A",
			"\\_SB_.WWAN",
			ESIF_DOMAIN_TYPE_WWAN);
	}
#else
	UNREFERENCED_PARAMETER(classGuid);
#endif
}

static eEsifError newParticipantCreate (
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
	const UInt32 acpiType
	)
{
	EsifParticipantIface sysPart;
	const eEsifParticipantOrigin origin = eParticipantOriginUF;
	UInt8 newInstance = ESIF_INSTANCE_INVALID;
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
	esif_ccb_strncpy(sysPart.name,name,ESIF_NAME_LEN);
	esif_ccb_strncpy(sysPart.desc,desc,ESIF_DESC_LEN);
	esif_ccb_strncpy(sysPart.object_id,objectId,ESIF_SCOPE_LEN);
	esif_ccb_strncpy(sysPart.device_path,devicePath,ESIF_PATH_LEN);
	esif_ccb_strncpy(sysPart.driver_name,driverName,ESIF_NAME_LEN);
	esif_ccb_strncpy(sysPart.device_name,deviceName,ESIF_NAME_LEN);

	EsifUpPm_RegisterParticipant(origin, &sysPart, &newInstance);

	return ESIF_OK;
}

static int scanPCI(void)
{
	DIR *dir;
	struct dirent **namelist;
	int n = 0;
	int cpu_loc_counter = 0;
	int thermal_counter = 0;
	char participant_path[MAX_SYSFS_PATH] = { 0 };
	char firmware_path[MAX_SYSFS_PATH] = { 0 };
	char participant_scope[ESIF_SCOPE_LEN] = { 0 };
	UInt32 ptype = ESIF_PARTICIPANT_INVALID_TYPE;
	esif_guid_t classGuid = ESIF_PARTICIPANT_CPU_CLASS_GUID;

	dir = opendir(SYSFS_PCI);
	if (!dir) {
		ESIF_TRACE_DEBUG("No PCI sysfs\n");
		return -1;
	}
	n = scandir(SYSFS_PCI, &namelist, 0, alphasort);
	if (n < 0) {
		//no scan
	}
	else {
		while (n--) {
			if (gSocParticipantFound) goto exit_participant;

			for (cpu_loc_counter = 0;cpu_loc_counter < NUM_CPU_LOCATIONS;cpu_loc_counter++) {
				if (esif_ccb_strstr(namelist[n]->d_name, CPU_location[cpu_loc_counter]) != NULL) {
					esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s/%s", SYSFS_PCI,namelist[n]->d_name);
					esif_ccb_sprintf(MAX_SYSFS_PATH, firmware_path, "%s/firmware_node", participant_path);

					if (sysfsGetString(firmware_path,"path", participant_scope, sizeof(participant_scope)) > -1) {
						int scope_len = esif_ccb_strlen(participant_scope,ESIF_SCOPE_LEN);
						if (scope_len < ACPI_DEVICE_NAME_LEN) {
							continue;
						}
						char *ACPI_name = participant_scope + (scope_len - ACPI_DEVICE_NAME_LEN);
						/* map to thermal zone (try pkg thermal zone first)*/
						for (thermal_counter=0; thermal_counter <= g_zone_count; thermal_counter++) {
							struct thermalZone tz = (struct thermalZone)thermalZones[thermal_counter];
							if (esif_ccb_strcmp(tz.acpiCode, ACPI_name)==0) {
								// Re-initialize the device path to one of the thermal zones
								esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s", tz.thermalPath);
								break;
							}
						}

						/* map to thermal zone (try pkg thermal zone first)*/
						newParticipantCreate(ESIF_PARTICIPANT_VERSION,
							classGuid,
							ESIF_PARTICIPANT_ENUM_SYSFS,
							0x0,
							ACPI_name,
							ESIF_PARTICIPANT_CPU_DESC,
							"N/A",
							SYSFS_PROCESSOR_HID,
							participant_path,
							participant_scope,
							ptype);
						gSocParticipantFound = ESIF_TRUE;
						break;
					}
				}
			}

exit_participant:
			free(namelist[n]);
		}
		free(namelist);
	}
	closedir(dir);

	return 0;
}


static int scanPlat(void)
{
	DIR *dir;
	struct dirent **namelist;
	struct dirent **participantNamelist;
	int n;
	int p;
	int prefix_len = esif_ccb_strlen(DPTF_PARTICIPANT_PREFIX,ESIF_NAME_LEN);
	char *dptf_prefix = DPTF_PARTICIPANT_PREFIX;
	int i;
	char participant_path[MAX_SYSFS_PATH] = { 0 };
	char firmware_path[MAX_SYSFS_PATH] = { 0 };
	char participant_scope[ESIF_SCOPE_LEN] = { 0 };
	char hid[HID_LEN] = { 0 };
	UInt32 ptype = ESIF_PARTICIPANT_INVALID_TYPE;
	esif_guid_t classGuid = ESIF_PARTICIPANT_PLAT_CLASS_GUID;

	dir = opendir(SYSFS_PLATFORM);
	if (!dir) {
		ESIF_TRACE_DEBUG("No platform sysfs\n");
		return -1;
	}
	n = scandir(SYSFS_PLATFORM, &namelist, 0, alphasort);
	if (n < 0) {
		//no scan
	}
	else {
		while (n--) {
			/* we forced IETM to index 0 for now because ESIF expects that */
			if (esif_ccb_strcmp(namelist[n]->d_name, ACPI_DPTF)==0) {
				goto exit_participant;
			}
			for (i=0;i < prefix_len;i++) {
				if (namelist[n]->d_name[i] != dptf_prefix[i]) {
					goto exit_participant;
				}
			}

			esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s/%s", SYSFS_PLATFORM,namelist[n]->d_name);
			esif_ccb_sprintf(MAX_SYSFS_PATH, firmware_path, "%s/firmware_node", participant_path);

			if (sysfsGetString(firmware_path,"hid", hid, sizeof(hid)) < 1) {
				esif_ccb_strncpy(hid,SYSFS_DEFAULT_HID,HID_LEN);
			}

			if (sysfsGetString(firmware_path,"path", participant_scope, sizeof(participant_scope)) > -1) {
				int scope_len = esif_ccb_strlen(participant_scope,ESIF_SCOPE_LEN);
				if (scope_len < ACPI_DEVICE_NAME_LEN) {
					continue;
				}
				char *ACPI_name = participant_scope + (scope_len - ACPI_DEVICE_NAME_LEN);
				char *ACPI_alias = NULL;

				ACPI_alias = esif_ccb_malloc(MAX_ZONE_NAME_LEN);
				if (ACPI_alias == NULL) {
					//no memory
					goto exit_participant;
				}

				/* map to thermal zone */
				get_participant_name_alias(ACPI_name, ACPI_alias);
				if (match_thermal_zone(ACPI_alias, participant_path) == ESIF_FALSE) {
					//retry without alias, best effort
					if (match_thermal_zone(ACPI_name, participant_path) == ESIF_FALSE) {
						// Obtain device path for participants that are not located in normal thermal zones
						p = scandir(participant_path, &participantNamelist, 0, alphasort);
						if (p < 0) {
						//no scan
						}
						else {
							while (p--) {
								if (esif_ccb_strcmp(participantNamelist[p]->d_name, "thermal_cooling")==0) {
									esif_ccb_sprintf(MAX_SYSFS_PATH,
											participant_path,
											"%s/%s/%s",
											SYSFS_PLATFORM,
											namelist[n]->d_name,
											participantNamelist[p]->d_name);
									break;
								}
							}
						}
					}
				}

				newParticipantCreate(ESIF_PARTICIPANT_VERSION,
						classGuid,
						ESIF_PARTICIPANT_ENUM_SYSFS,
						0x0,
						ACPI_name,
						ACPI_name,
						"N/A",
						hid,
						participant_path,
						participant_scope,
						ptype);
				esif_ccb_free(ACPI_alias);
			}

exit_participant:
			free(namelist[n]);
		}
		free(namelist);
	}
	closedir(dir);

	return 0;
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

			if (sysfsGetString(target_path,"type", acpi_name, sizeof(acpi_name)) > -1) {
				struct thermalZone tz = { 0 };
				tz.zoneType = zt;
				esif_ccb_sprintf(MAX_ZONE_NAME_LEN,tz.acpiCode,"%s",acpi_name);
				esif_ccb_sprintf(MAX_SYSFS_PATH,tz.thermalPath,"%s",target_path);
				thermalZones[thermal_counter] = tz;
				thermal_counter++;
			}

exit_zone:
			free(namelist[n]);
		}
		free(namelist);
	}
	closedir(dir);

	g_zone_count = thermal_counter;

	return 0;
}

static Bool match_thermal_zone(const char *matchToName, char *participant_path)
{
	int thermal_counter = 0;
	Bool thermalZoneFound = ESIF_FALSE;

	for (thermal_counter=0; thermal_counter <= g_zone_count; thermal_counter++) {
		struct thermalZone tz = (struct thermalZone)thermalZones[thermal_counter];
		if (esif_ccb_strcmp(tz.acpiCode, matchToName)==0) {
			thermalZoneFound = ESIF_TRUE;
			thermalZones[thermal_counter].bound = ESIF_TRUE;
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
#ifdef ESIF_ATTR_OS_ANDROID
		{ "TCHG", "bq25890-charger" },
#endif
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

