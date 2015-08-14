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

#include "esif_uf.h"
#include "esif_pm.h"
#include "esif_dsp.h"

#define DEFAULT_DRIVER_NAME	""
#define DEFAULT_DEVICE_NAME	""

// Sysfs Paths
#define SYSFS_THERMAL	"/sys/class/thermal"
#define SYSFS_PCI		"/sys/bus/pci/devices"
#define SYSFS_PLATFORM	"/sys/bus/platform/devices"

#define MAX_SYSFS_STRING 4 * 1024
#define MAX_SYSFS_PATH 256
#define ACPI_NAME_LEN 5
#define HID_LEN 8
#define CORE "0000:00:04.0"
#define ATOM "0000:00:0b.0"
#define DPTF_PARTICIPANT_PREFIX "INT340"
#define ACPI_DPTF "INT3400:00"
#define SYSFS_DEFAULT_HID "INT3403"
#define SYSFS_PROCESSOR_HID "INT3401"
#define SYSFS_DPTF_HID "INT3400"
#define PARTICIPANT_FIELD_LEN 64

static int g_zone_count = 0;

static int sysfsGetString(char *path, char *filename, char *str);
static int sysfsGetU64(char *path, char *filename, u64 *p_u64);
static int sysfsSetU64(char *path, char *filename, u64 val);
static int sysfsSetString(char *path, char *filename, char *val);
static int scanPCI(void);
static int scanPlat(void);
static int scanThermal(void);
static void newParticipantCreate (
	UInt8 version,
	esif_guid_t classGuid,
	enum esif_participant_enum enumerator,
	UInt32 flags,
	char name[ESIF_NAME_LEN],
	char desc[ESIF_DESC_LEN],
	char driverName[ESIF_NAME_LEN],
	char deviceName[ESIF_NAME_LEN],
	char devicePath[ESIF_PATH_LEN],
	char objectId[ESIF_SCOPE_LEN],
	char acpiScope[ESIF_SCOPE_LEN],
	char acpiUID[ESIF_ACPI_UID_LEN],
	UInt32 acpiType
	);
	
enum zoneType {
	THERM,
	CDEV,
};
	
struct thermalZone {
	int zoneId;
	enum zoneType zoneType;
	char acpiCode[5];
	char thermalPath[MAX_SYSFS_PATH];
};

struct thermalZone thermalZones[MAX_PARTICIPANT_ENTRY] = { 0 };

static int sysfsGetString(char *path, char *filename, char *str)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (esif_ccb_fopen(&fd, filepath, "r") != 0) {
		return ret;
	}
	
	ret = esif_ccb_fscanf(fd, "%s", str);
	esif_ccb_fclose(fd);

	return ret;
}



static int sysfsGetU64(char *path, char *filename, u64 *p_u64)
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

static int sysfsSetU64(char *path, char *filename, u64 val)
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

static int sysfsSetString(char *path, char *filename, char *val)
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
	int id=0;
	
	sysPart.version = ESIF_PARTICIPANT_VERSION;
	sysPart.enumerator = ESIF_PARTICIPANT_ENUM_SYSFS;
	sysPart.flags = 1;
	sysPart.send_event = NULL;
	sysPart.recv_event = NULL;
	esif_ccb_strncpy(sysPart.name,spType,ACPI_NAME_LEN);
	esif_ccb_strncpy(sysPart.desc,spDesc,PARTICIPANT_FIELD_LEN);
	esif_ccb_strncpy(sysPart.object_id,spObjId,PARTICIPANT_FIELD_LEN);
	esif_ccb_strncpy(sysPart.device_path,spDevicePath,PARTICIPANT_FIELD_LEN);
	esif_ccb_strncpy(sysPart.device_name,spDevice,PARTICIPANT_FIELD_LEN);
	esif_ccb_sprintf(ESIF_NAME_LEN, sysPart.driver_name, "sysfs%s", ESIF_LIB_EXT);
	
	EsifUpPm_RegisterParticipant(origin, &sysPart, &newInstance);
	
	scanThermal();
	scanPCI();
	scanPlat();
	ESIF_TRACE_EXIT_INFO();
}

static void newParticipantCreate (
	UInt8 version,
	esif_guid_t classGuid,
	enum esif_participant_enum enumerator,
	UInt32 flags,
	char name[ESIF_NAME_LEN],
	char desc[ESIF_DESC_LEN],
	char driverName[ESIF_NAME_LEN],
	char deviceName[ESIF_NAME_LEN],
	char devicePath[ESIF_PATH_LEN],
	char objectId[ESIF_SCOPE_LEN],
	char  acpiScope[ESIF_SCOPE_LEN],
	char  acpiUID[ESIF_ACPI_UID_LEN],
	UInt32 acpiType
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
	esif_ccb_strncpy(sysPart.name,name,ESIF_NAME_LEN);
	esif_ccb_strncpy(sysPart.desc,desc,ESIF_DESC_LEN);
	esif_ccb_strncpy(sysPart.object_id,objectId,ESIF_SCOPE_LEN);
	esif_ccb_strncpy(sysPart.device_path,devicePath,ESIF_PATH_LEN);
	esif_ccb_strncpy(sysPart.driver_name,driverName,ESIF_NAME_LEN);
	esif_ccb_strncpy(sysPart.device_name,deviceName,ESIF_NAME_LEN);

	EsifUpPm_RegisterParticipant(origin, &sysPart, &newInstance);
	
}
	


static int scanPCI(void)
{
	DIR *dir;
	struct dirent **namelist;
	int n;
	int thermal_counter = 0;
	char participant_path[MAX_SYSFS_PATH] = { 0 };
	char firmware_path[MAX_SYSFS_PATH] = { 0 };
	char participant_scope[ESIF_SCOPE_LEN] = { 0 };
	char  acpiUID[ESIF_ACPI_UID_LEN] = { 0 };	
	UInt32 ptype = 0xffffffff;
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
			if (esif_ccb_strstr(namelist[n]->d_name, CORE) != NULL) {
				esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s/%s", SYSFS_PCI,namelist[n]->d_name);
				esif_ccb_sprintf(MAX_SYSFS_PATH, firmware_path, "%s/firmware_node", participant_path);
				
				if (sysfsGetString(firmware_path,"path", participant_scope) > -1) {
					int scope_len = esif_ccb_strlen(participant_scope,ESIF_SCOPE_LEN);
					char *ACPI_name = participant_scope + (scope_len - 4);
					/* map to thermal zone */
					for (thermal_counter=0; thermal_counter <= g_zone_count; thermal_counter++) {
						struct thermalZone tz = (struct thermalZone)thermalZones[thermal_counter];
						if (esif_ccb_strcmp(tz.acpiCode, ACPI_name)==0) {
							esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s", tz.thermalPath);
						}
					}
					
					newParticipantCreate(ESIF_PARTICIPANT_VERSION,classGuid,ESIF_PARTICIPANT_ENUM_SYSFS,0x0,ACPI_name,ESIF_PARTICIPANT_CPU_DESC,"dptf_cpu.sys",SYSFS_PROCESSOR_HID,participant_path,participant_scope,participant_scope,acpiUID,ptype);
				}
			}
			if (esif_ccb_strstr(namelist[n]->d_name, ATOM) != NULL) {
				esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s/%s", SYSFS_PCI,namelist[n]->d_name);
				esif_ccb_sprintf(MAX_SYSFS_PATH, firmware_path, "%s/firmware_node", participant_path);
				if (sysfsGetString(firmware_path,"path", participant_scope) > -1) {
					int scope_len = esif_ccb_strlen(participant_scope,ESIF_SCOPE_LEN);
					char *ACPI_name = participant_scope + (scope_len - 4);
					/* map to thermal zone */
					for (thermal_counter=0; thermal_counter <= g_zone_count; thermal_counter++) {
						struct thermalZone tz = (struct thermalZone)thermalZones[thermal_counter];
						if (esif_ccb_strcmp(tz.acpiCode, ACPI_name)==0) {
							esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s", tz.thermalPath);
						}
					}
				
					newParticipantCreate(ESIF_PARTICIPANT_VERSION,classGuid,ESIF_PARTICIPANT_ENUM_SYSFS,0x0,ACPI_name,ESIF_PARTICIPANT_CPU_DESC,"dptf_cpu.sys",SYSFS_PROCESSOR_HID,participant_path,participant_scope,participant_scope,acpiUID,ptype);
				}
			}
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
	int n;
	int prefix_len = esif_ccb_strlen(DPTF_PARTICIPANT_PREFIX,ESIF_NAME_LEN);
	char *dptf_prefix = DPTF_PARTICIPANT_PREFIX;
	int i;
	char participant_path[MAX_SYSFS_PATH] = { 0 };
	char firmware_path[MAX_SYSFS_PATH] = { 0 };
	char participant_scope[ESIF_SCOPE_LEN] = { 0 };
	char hid[HID_LEN] = { 0 };
	char  acpiUID[ESIF_ACPI_UID_LEN] = { 0 };
	UInt32 ptype = 0xffffffff;
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
			
			if (sysfsGetString(firmware_path,"hid", hid) < 1) {
				esif_ccb_strncpy(hid,SYSFS_DEFAULT_HID,HID_LEN);
			}
			
			if (sysfsGetString(firmware_path,"path", participant_scope) > -1) {
				int scope_len = esif_ccb_strlen(participant_scope,ESIF_SCOPE_LEN);
				char *ACPI_name = participant_scope + (scope_len - 4);
				int thermal_counter = 0;
				
				/* map to thermal zone */
				for (thermal_counter=0; thermal_counter <= g_zone_count; thermal_counter++) {
					struct thermalZone tz = (struct thermalZone)thermalZones[thermal_counter];
					if (esif_ccb_strcmp(tz.acpiCode, ACPI_name)==0) {
						esif_ccb_sprintf(MAX_SYSFS_PATH, participant_path, "%s", tz.thermalPath);
						break;
					}
				}
				
				newParticipantCreate(ESIF_PARTICIPANT_VERSION,classGuid,ESIF_PARTICIPANT_ENUM_SYSFS,0x0,ACPI_name,"dptf participant","dptf_fgen.sys",hid,participant_path,participant_scope,participant_scope,acpiUID,ptype);
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
			char acpi_name[ACPI_NAME_LEN] = { 0 };
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
			
			if (sysfsGetString(target_path,"type", acpi_name) > -1) {
				struct thermalZone tz = { 0 };
				tz.zoneId = 0;
				tz.zoneType = zt;
				esif_ccb_sprintf(ACPI_NAME_LEN,tz.acpiCode,"%s",acpi_name);
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
