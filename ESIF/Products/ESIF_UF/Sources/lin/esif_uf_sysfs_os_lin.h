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

#ifndef _ESIF_UF_SYFS_LIN_
#define _ESIF_UF_SYFS_LIN_

#define MAX_CORE_COUNT_SUPPORTED    (32)    // Supports max 32 cores per CPU Type
#define MAX_CPU_TYPES_SUPPORTED     (4)     // Supports only 4 different CPU types
#define MAX_ZONE_NAME_LEN           (56)
#define IIO_STR_LEN                 (24)
#define MAX_SYSFS_FILENAME          (256)
#define MAX_SYSFS_SUFFIX            (26)
#define MAX_SYSFS_PATH              (MAX_SYSFS_FILENAME * 2 + MAX_SYSFS_SUFFIX)
#define MAX_STR_LINE_LEN            (64)
#define MAX_SYSFS_STRING            (4 * 1024)
#define MIN_INT64	            ((Int64) 0x8000000000000000)
#define MAX_INT64	            ((Int64) 0x7FFFFFFFFFFFFFFF)
typedef struct SysfsReadEntry_t {
	char participantName[ESIF_NAME_LEN];
	char sysfsPath[ESIF_PATH_LEN];
	UInt32 eventType;
	Int32 fd;
	Int64 value;
} SysfsReadEntry , *sysfsReadEntryPtr;

typedef struct SysfsAttrToEventMap_s {
	char attributeName[MAX_SYSFS_FILENAME];
	UInt32 eventType;
} SysfsAttrToEventMap , *SysfsAttrToEventMapPtr;

#define MAX_SYSFS_READ_ENTRY_SIZE  (16)
extern SysfsReadEntry gSysfsReadTable[MAX_SYSFS_READ_ENTRY_SIZE];
extern UInt32 gNumberOfSysfsReadEntries;
#pragma pack(push, 1)

typedef struct _PerfCpuMapping {
        UInt32 highestPerf;
        UInt32 numberOfIndexes;
        UInt32 indexes[MAX_CORE_COUNT_SUPPORTED];
} PerfCpuMapping, *PerfCpuMappingPtr;

typedef struct _SystemCpuIndexTable {
        Bool isInitialized;
        UInt32 highestPerformanceCpuIndex;
        UInt32 lowestPerformanceCpuIndex;
        UInt32 numberOfCpuTypes;
        UInt32 numberOfCpus;
        PerfCpuMapping perfCpuMapping[MAX_CPU_TYPES_SUPPORTED];
} SystemCpuIndexTable, *SystemCpuIndexTablePtr;

enum zoneType {
	THERM,
	CDEV,
};

typedef struct thermalZone_t {
	Bool bound;
	enum zoneType zoneType;
	char acpiCode[MAX_ZONE_NAME_LEN];
	char thermalPath[MAX_SYSFS_PATH];
} thermalZone, *thermalZonePtr;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

int SysfsSetString(const char *path, const char *filename, char *val);
int SysfsGetInt64Direct(int fd, Int64 *p64);
int SysfsSetInt64Direct(int fd, Int32 val);
int SysfsGetString(char *path, char *filename, char *str, size_t buf_len);
int SysfsGetStringMultiline(const char *path, const char *filename, char *str);
int SysfsGetInt64(const char *path, const char *filename, Int64 *p64);
int SysfsSetInt64(char *path, char *filename, Int64 val);
eEsifError SysfsGetInt(const char *path, const char *filename, int *pInt);
eEsifError SysfsGetIntDirect(int fd, int *pInt);
eEsifError SysfsGetFloat(const char *path, const char *filename, float *pFloat);
eEsifError SysfsGetFileSize(const char *path, const char *fileName, size_t *fileSize);
eEsifError SysfsGetBinaryData(const char *path, const char *fileName, UInt8 *buffer, size_t bufferLength);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_SYSFS_LIN_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

