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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_LINUX
#include "esif_ccb.h"
#include "esif_uf.h"
#include "esif_uf_appmgr.h"
#include "esif_pm.h"
#include "esif_version.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_cfgmgr.h"
#include "esif_lib_databank.h"
#include "esif_uf_sensors.h"
#include "esif_uf_sysfs_os_lin.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <math.h>
#include <dirent.h>

#define ESIF_IIO_SAMPLE_PERIOD 5 // In seconds
#define MAX_GFORCE (9.8 * 2) // All Chromebooks accel have default -2G to 2G range
#define MOTION_CHANGE_THRESHOLD 0.007 // Normalize threshold to declare motion state change
#define PLAT_TYPE_CLAMSHELL_ANGLE_MIN 5
#define PLAT_TYPE_CLAMSHELL_ANGLE_MAX 180
#define PLAT_TYPE_TABLET_ANGLE_MIN 355
#define MAX_WL_HINT_ROWS 140 // as per the test Doc
#define SCANF_STR_LEN 255
#define TOTAL_PIDS_READ 8
#define EPP_MODE_MIDPOINT	((EPP_BALANCE_POWER - EPP_BALANCE_PERFORMANCE) / 2)

static Bool IsWorkloadFgProcessOrDaemon(const char *workloadName);

typedef enum EppMode_e {
	EPP_PERFORMANCE = 0,
	EPP_BALANCE_PERFORMANCE = 128,
	EPP_BALANCE_POWER = 192,
	EPP_POWER = 255
} EppMode;

// DTT Power Slider Values for Esif_event
typedef enum PowerSliderMode_e {
	POWER_SLIDER_STATE_INVALID = 0,
	POWER_SLIDER_STATE_BATTERY_SAVER = 25,
	POWER_SLIDER_STATE_BETTER_BATTERY = 50,
	POWER_SLIDER_STATE_BETTER_PERF = 75,
	POWER_SLIDER_STATE_MAX_PERF = 100
} PowerSliderMode;

typedef enum SensorType_e {
	SENSOR_TYPE_ACCEL = 0,
	SENSOR_TYPE_LID_ANGLE,
	SENSOR_TYPE_NA
} SensorType;

typedef enum SensorLocation_e {
	SENSOR_LOC_BASE = 0,
	SENSOR_LOC_LID,
	SENSOR_LOC_NA
} SensorLoc;

typedef struct SensorBase_s {
	SensorType type;
	SensorLoc loc;
} SensorBase, *SensorBasePtr;

typedef struct Accelerometer_s {
	int fdX;	// File descriptors for each open sysfs node
	int fdY;
	int fdZ;
	int xRaw;
	int yRaw;
	int zRaw;
	float scale;
} Accelerometer, *AccelerometerPtr;

typedef struct LidAngle_s {
	int fdAngle;	// File descriptor for lid angle sysfs node
	int angleRaw;
} LidAngle, *LidAnglePtr;

/**
 * ToDo: Add definitions of gyro, pressure sensor, etc.
 */

typedef struct Sensor_s {
	SensorBase base;
	union {
		Accelerometer accel;
		LidAngle lidAngle;
	} data;
} Sensor, *SensorPtr;

static const char * const gPowerSliderModeNames [] = {
	[POWER_SLIDER_STATE_INVALID] = "Invalid",
	[POWER_SLIDER_STATE_BATTERY_SAVER] = "power",
	[POWER_SLIDER_STATE_BETTER_BATTERY] = "balance_power",
	[POWER_SLIDER_STATE_BETTER_PERF] = "balance_performance",
	[POWER_SLIDER_STATE_MAX_PERF] = "performance"
};


static EsifString gPrevWorkload = NULL;
static esif_thread_t gEsifSensorMgrThread;
static Bool gEsifSensorMgrStarted = ESIF_FALSE;
static const char gXRawNodeName[] = "in_accel_x_raw";
static const char gYRawNodeName[] = "in_accel_y_raw";
static const char gZRawNodeName[] = "in_accel_z_raw";
static const char gAngleRawNodeName[] = "in_angl_raw";
static const char gSensorBasePath[] =  "/sys/bus/iio/devices";
static char gLidStateBasePath[] = "/proc/acpi/button/lid/LID0";
static char gPowerSupplyPath[] = "/sys/class/power_supply";
static char gDockingBasePath[] = "/sys/bus/acpi/devices/GOOG6666:00";
static char gPowerSliderBasePath[] = "/sys/devices/system/cpu/cpu0/cpufreq";

static const InclinometerMinMaxConfig gInclinMinMaxConfig= {
	INCLIN_X_ORIENT_FLAT_UP_MIN,
	INCLIN_X_ORIENT_FLAT_UP_MAX,
	INCLIN_X_ORIENT_FLAT_DOWN_MIN
	INCLIN_X_ORIENT_FLAT_DOWN_MAX,
	INCLIN_X_ORIENT_UPRIGHT_MIN,
	INCLIN_X_ORIENT_UPRIGHT_MAX,
	INCLIN_X_ORIENT_UPRIGHT_INVERTED_MIN,
	INCLIN_X_ORIENT_UPRIGHT_INVERTED_MAX,
	INCLIN_Y_ORIENT_FLAT_MAX,
};

// Global sensors array
static Sensor *gSensors;
static int gSensorsNum;

// Global Pointers to common sensors for fast processing
static Sensor *gAccelBase;
static Sensor *gAccelLid;
static Sensor *gLidAngle;

// Global file descriptors for misc sensors not found in IIO bus, such as lid state, battery state, etc.
static int gFdLidState;
static int gFdDocking;

// Global variables keeping track of current x/y/z vectors and platform/display orientation/platform type
static AccelerometerData gCurAccelData;
static PlatformOrientation gPlatOrientation = ORIENTATION_PLAT_MAX;
static DisplayOrientation gDispOrientation = ORIENTATION_DISP_MAX;
static Motion gInMotion = MOTION_MAX;
static DockMode gDockMode = DOCK_MODE_INVALID;
static LidState gLidState = LID_STATE_CLOSED;
static PlatformType gPlatType = PLATFORM_TYPE_INVALID;
static PowerSliderMode gPowerSliderValue = POWER_SLIDER_STATE_INVALID;
static Bool gIsWLHintRegistered = ESIF_FALSE;

PowerSrc g_PowerSrc = POWER_SRC_AC;
UInt32 g_BatteryPercentage = 0;

// The SIGUSR1 handler below is the alternative
// solution for Android where the pthread_cancel()
// is not implemented in Android NDK

/* SIGUSR1 Signal Handler */
static void sigusr1_handler(int signum)
{
	pthread_exit(0);
}

/* Enable or Disable SIGTERM Handler */
static void sigusr1_enable()
{
	struct sigaction action={0};
	action.sa_handler = sigusr1_handler;
	sigaction(SIGUSR1, &action, NULL);
}

static int IioDeviceFilter(const struct dirent *entry)
{
	if (esif_ccb_strstr(entry->d_name, "iio:device")) return 1;
	else return 0;
};

static eEsifError AccelGetLoc(SensorPtr sensorPtr, char *fullPath)
{
	char iioSysfsNode[IIO_STR_LEN] = { 0 };
	eEsifError rc = ESIF_OK;

	if (!sensorPtr)
		return ESIF_E_PARAMETER_IS_NULL;

	if (SysfsGetString(fullPath, "location", iioSysfsNode, sizeof(iioSysfsNode)) > 0) {
		if (esif_ccb_strstr(iioSysfsNode, "base")) {
			sensorPtr->base.loc = SENSOR_LOC_BASE;
			if (NULL == gAccelBase) gAccelBase = sensorPtr;
		} else {
			sensorPtr->base.loc = SENSOR_LOC_LID;
			if (NULL == gAccelLid) gAccelLid = sensorPtr;
		}
	}
	else {
		ESIF_TRACE_WARN("Accelerometer location is not found \n");
		rc = ESIF_E_NOT_SUPPORTED;
	}
	return rc;
}

static void AccelGetScale(SensorPtr sensorPtr, char *fullPath)
{
	float scale = 0;

	if (!sensorPtr)
		return;

	if (ESIF_OK == SysfsGetFloat(fullPath, "scale", &scale)) {
		sensorPtr->data.accel.scale = scale;
	} else if (ESIF_OK == SysfsGetFloat(fullPath, "in_accel_scale", &scale)) {
		sensorPtr->data.accel.scale = scale;
	}
}

static void AccelOpenFileDescriptors(SensorPtr sensorPtr, char *fullPath)
{
	char axixNodePath[MAX_PATH] = { 0 };

	if (!sensorPtr)
		return;

	esif_ccb_sprintf(MAX_PATH, axixNodePath, "%s/%s", fullPath, gXRawNodeName);
	sensorPtr->data.accel.fdX = open(axixNodePath, O_RDONLY);
	esif_ccb_sprintf(MAX_PATH, axixNodePath, "%s/%s", fullPath, gYRawNodeName);
	sensorPtr->data.accel.fdY = open(axixNodePath, O_RDONLY);
	esif_ccb_sprintf(MAX_PATH, axixNodePath, "%s/%s", fullPath, gZRawNodeName);
	sensorPtr->data.accel.fdZ = open(axixNodePath, O_RDONLY);
}

static void LidAngleOpenFileDescriptors(SensorPtr sensorPtr, char *fullPath)
{
	char nodePath[MAX_PATH] = { 0 };

	if (!sensorPtr)
		return;

	esif_ccb_sprintf(MAX_PATH, nodePath, "%s/%s", fullPath, gAngleRawNodeName);
	sensorPtr->data.lidAngle.fdAngle = open(nodePath, O_RDONLY);
	if (NULL == gLidAngle) gLidAngle = sensorPtr;
}

static void InitSensor(int index, char *devName)
{
	SensorPtr sensorPtr = &gSensors[index];
	eEsifError rc = ESIF_OK;
	char iioSysfsNode[IIO_STR_LEN] = { 0 };
	char fullPath[MAX_PATH + IIO_STR_LEN] = { 0 };

	sensorPtr->base.type = SENSOR_TYPE_NA;

	esif_ccb_sprintf(sizeof(fullPath), fullPath, "%s/%s", gSensorBasePath, devName);
	if (SysfsGetString(fullPath, "name", iioSysfsNode, sizeof(iioSysfsNode)) > 0) {
		// Init code for accelerometers
		if (esif_ccb_strstr(iioSysfsNode, "accel")) {
			sensorPtr->base.type = SENSOR_TYPE_ACCEL;
			rc = AccelGetLoc(sensorPtr, fullPath);
			if(ESIF_E_NOT_SUPPORTED == rc) {
				if (NULL == gAccelBase) {
					gAccelBase = sensorPtr;
				}
			}
			AccelGetScale(sensorPtr, fullPath);
			AccelOpenFileDescriptors(sensorPtr, fullPath);
		} else if (esif_ccb_strstr(iioSysfsNode, "lid-angle")) {
			sensorPtr->base.type = SENSOR_TYPE_LID_ANGLE;
			LidAngleOpenFileDescriptors(sensorPtr, fullPath);
		}
	}
}

static void DeinitSensor(int index)
{
	if (SENSOR_TYPE_ACCEL == gSensors[index].base.type) {
		int fd = gSensors[index].data.accel.fdX;
		if (fd > 0) close(fd);
		fd = gSensors[index].data.accel.fdY;
		if (fd > 0) close(fd);
		fd = gSensors[index].data.accel.fdZ;
		if (fd > 0) close(fd);
	} else if (SENSOR_TYPE_LID_ANGLE == gSensors[index].base.type) {
		int fd = gSensors[index].data.lidAngle.fdAngle;
		if (fd > 0) close(fd);
	}
}

static eEsifError EsifSensorMgr_RegisterSensors()
{
	eEsifError rc = ESIF_OK;
	int i = 0;
	struct dirent **namelist;

	gSensorsNum = scandir(gSensorBasePath, &namelist, IioDeviceFilter, alphasort);
	if (gSensorsNum < 1) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	gSensors = (SensorPtr) esif_ccb_malloc(gSensorsNum * sizeof(Sensor));
	if (NULL == gSensors) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	// Populate gSensors array and set up the pointers to base/lid accels
	for (i = 0; i < gSensorsNum; ++i) {
		InitSensor(i, namelist[i]->d_name);
		esif_ccb_free(namelist[i]);
	};
	esif_ccb_free(namelist);

exit:
	return rc;
}

static eEsifError EsifSensorMgr_InitializeNonIioBusSensors()
{
	char filepath[MAX_PATH];
	eEsifError rc = ESIF_OK;

	// Initialize non-IIO bus sensors such as lid state and battery state
	// All we need is to obtain a file descriptor to each corresponding sysfs node
	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", gLidStateBasePath, "state");
	gFdLidState = open(filepath, O_RDONLY);
	
	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", gDockingBasePath, "docked");
	gFdDocking = open(filepath, O_RDONLY);

	if (gFdLidState <= 0 && gFdDocking <= 0) {
		rc = ESIF_E_NOT_SUPPORTED;
	}

	return rc;
}

static void EsifSensorMgr_DeregisterSensors()
{
	int i = 0;
	for (i = 0; i < gSensorsNum; ++i) {
		DeinitSensor(i);
	}
	esif_ccb_free(gSensors);
	gAccelBase = NULL;
	gAccelLid = NULL;
	gLidAngle = NULL;

	// Also close file descriptors for misc. sensors
	if (gFdLidState > 0) close(gFdLidState);
	if (gFdDocking > 0) close(gFdDocking);
}

static void AccelRawUpdate(SensorPtr sensorPtr)
{
	int fd = 0;

	if (!sensorPtr)
		return;

	fd = sensorPtr->data.accel.fdX;
	if (fd > 0) {
		SysfsGetIntDirect(fd, &sensorPtr->data.accel.xRaw);
	}

	fd = sensorPtr->data.accel.fdY;
	if (fd > 0) {
		SysfsGetIntDirect(fd, &sensorPtr->data.accel.yRaw);
	}

	fd = sensorPtr->data.accel.fdZ;
	if (fd > 0) {
		SysfsGetIntDirect(fd, &sensorPtr->data.accel.zRaw);
	}
}

static AccelerometerData NormalizeAccelRawData(SensorPtr sensorPtr)
{
	AccelerometerData data = { 0 };

	if (!sensorPtr)
		return data;

	data.xVal = sensorPtr->data.accel.xRaw * sensorPtr->data.accel.scale / MAX_GFORCE;
	data.yVal = sensorPtr->data.accel.yRaw * sensorPtr->data.accel.scale / MAX_GFORCE;
	data.zVal = sensorPtr->data.accel.zRaw * sensorPtr->data.accel.scale / MAX_GFORCE;

	return data;
};

static void CheckDispPlatOrientation(SensorPtr sensorPtr)
{
	PlatformOrientation newPlatOrientation = ORIENTATION_PLAT_MAX;
	DisplayOrientation newDispOrientation = ORIENTATION_DISP_MAX;
	EsifData evtData = { 0 };

	if (!sensorPtr)
		return;

	AccelerometerData data = NormalizeAccelRawData(sensorPtr);
	EsifAccelerometer_GetOrientations(
			gInclinMinMaxConfig,
			&data,
			gDispOrientation,
			gPlatOrientation,
			&newPlatOrientation,
			&newDispOrientation);

	if (newDispOrientation != gDispOrientation) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &newDispOrientation, sizeof(DisplayOrientation));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED, &evtData);
		gDispOrientation = newDispOrientation;
	}

	if (newPlatOrientation != gPlatOrientation) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &newPlatOrientation, sizeof(PlatformOrientation));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_DEVICE_ORIENTATION_CHANGED, &evtData);
		gPlatOrientation = newPlatOrientation;
	}
}

static UInt32 GetPowerSliderModeFromRawVal(UInt32 eppRawVal)
{
	UInt32 value = 0;

	switch(eppRawVal) {
		// Range: 0 ... 64
		case EPP_PERFORMANCE ... (EPP_BALANCE_PERFORMANCE/2):
			value = POWER_SLIDER_STATE_MAX_PERF;
		break;
		// Range: 65 ... 160
		case (EPP_BALANCE_PERFORMANCE / 2 + 1) ... (EPP_BALANCE_PERFORMANCE + EPP_MODE_MIDPOINT):
			value = POWER_SLIDER_STATE_BETTER_PERF;
		break;
		// Range: 161 ... 224
		case (EPP_BALANCE_PERFORMANCE + EPP_MODE_MIDPOINT + 1) ... (EPP_BALANCE_POWER + EPP_MODE_MIDPOINT):
			value = POWER_SLIDER_STATE_BETTER_BATTERY;
		break;
		// Range: 225 ... 255
		case (EPP_BALANCE_POWER + EPP_MODE_MIDPOINT + 1) ... EPP_POWER:
			value = POWER_SLIDER_STATE_BATTERY_SAVER;
		break;
		default:
			value = POWER_SLIDER_STATE_INVALID;
		break;
	}
	return value;
}

static void GetPowerSliderMode()
{
	UInt32 newPowerSliderValue = 0;
	UInt32 eppRawVal = 0;
	char eppMode[MAX_PATH] = {'\0'};
	EsifData evtData = { 0 };

	if (SysfsGetString(gPowerSliderBasePath, "energy_performance_preference", eppMode, sizeof(eppMode)) <= 0) {
			ESIF_TRACE_ERROR("Sysfs Read failed at path: %s/type \n", gPowerSliderBasePath);
			return;
	}

	if (esif_ccb_strncmp(eppMode, gPowerSliderModeNames[POWER_SLIDER_STATE_BATTERY_SAVER],
		esif_ccb_strlen(gPowerSliderModeNames[POWER_SLIDER_STATE_BATTERY_SAVER], MAX_PATH)) == 0) {
			newPowerSliderValue = POWER_SLIDER_STATE_BATTERY_SAVER;
	}
	else if (esif_ccb_strncmp(eppMode, gPowerSliderModeNames[POWER_SLIDER_STATE_BETTER_BATTERY],
			esif_ccb_strlen(gPowerSliderModeNames[POWER_SLIDER_STATE_BETTER_BATTERY], MAX_PATH)) == 0) {
				newPowerSliderValue = POWER_SLIDER_STATE_BETTER_BATTERY;
	}
	else if (esif_ccb_strncmp(eppMode, gPowerSliderModeNames[POWER_SLIDER_STATE_BETTER_PERF],
			esif_ccb_strlen(gPowerSliderModeNames[POWER_SLIDER_STATE_BETTER_PERF], MAX_PATH)) == 0) {
				newPowerSliderValue = POWER_SLIDER_STATE_BETTER_PERF;
	}
	else if (esif_ccb_strncmp(eppMode, gPowerSliderModeNames[POWER_SLIDER_STATE_MAX_PERF],
			esif_ccb_strlen(gPowerSliderModeNames[POWER_SLIDER_STATE_MAX_PERF], MAX_PATH)) == 0) {
				newPowerSliderValue = POWER_SLIDER_STATE_MAX_PERF;
	}
	else {
		eppRawVal = esif_atoi(eppMode);
		newPowerSliderValue = GetPowerSliderModeFromRawVal(eppRawVal);
	}

	if (newPowerSliderValue != gPowerSliderValue) {
		ESIF_TRACE_INFO("lastPowerSliderState: %s, PowerSliderState: %s \n",
			gPowerSliderModeNames[gPowerSliderValue],  gPowerSliderModeNames[newPowerSliderValue]);
		ESIF_DATA_UINT32_ASSIGN(evtData, &newPowerSliderValue, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_POWER_SLIDER_VALUE_CHANGED, &evtData);
		gPowerSliderValue = newPowerSliderValue;
	}
}

static void CheckMotionChange(SensorPtr sensorPtr)
{
	EsifData evtData = { 0 };
	float delta = 0;
	AccelerometerData data = { 0 };
	Motion newMotionState = MOTION_OFF;

	if (!sensorPtr)
		return;

	data = NormalizeAccelRawData(sensorPtr);
	delta = sqrt((data.xVal - gCurAccelData.xVal) * (data.xVal - gCurAccelData.xVal) +
			(data.yVal - gCurAccelData.yVal) * (data.yVal - gCurAccelData.yVal) +
			(data.zVal - gCurAccelData.zVal) * (data.zVal - gCurAccelData.zVal));
	if (delta > MOTION_CHANGE_THRESHOLD) {
		newMotionState = MOTION_ON;
	}
	ESIF_TRACE_DEBUG("delta=%f , MOTION_CHANGE_THRESHOLD %d\n", delta, MOTION_CHANGE_THRESHOLD);

	if (newMotionState != gInMotion) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &newMotionState, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_MOTION_CHANGED, &evtData);
		gInMotion = newMotionState;
		ESIF_TRACE_DEBUG("delta=%f \n", delta);
	}
	gCurAccelData = data;
}

static void CheckPlatTypeChange(SensorPtr sensorPtr)
{
	EsifData evtData = { 0 };
	PlatformType newPlatType = PLATFORM_TYPE_INVALID;
	int fd = 0;

	if (!sensorPtr)
		return;

	if (SENSOR_TYPE_LID_ANGLE != sensorPtr->base.type)
		return;

	fd = sensorPtr->data.lidAngle.fdAngle;
	if (fd > 0) {
		SysfsGetIntDirect(fd, &sensorPtr->data.lidAngle.angleRaw);

		// We don't support tent mode yet and it will be treated as invalid
		if (sensorPtr->data.lidAngle.angleRaw >= PLAT_TYPE_CLAMSHELL_ANGLE_MIN &&
				sensorPtr->data.lidAngle.angleRaw < PLAT_TYPE_CLAMSHELL_ANGLE_MAX) {
			newPlatType = PLATFORM_TYPE_CLAMSHELL;
		} else if (sensorPtr->data.lidAngle.angleRaw >= PLAT_TYPE_TABLET_ANGLE_MIN) {
			newPlatType = PLATFORM_TYPE_TABLET;
		}

		if (newPlatType != gPlatType) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &newPlatType, sizeof(UInt32));
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED, &evtData);
			gPlatType = newPlatType;
		}
	}
}

static void CheckDockModeChange(void)
{
	DockMode dockMode = DOCK_MODE_INVALID;
	char sysvalstring[MAX_SYSFS_STRING] = { 0 };
	EsifData evtData = { 0 };

	if (gFdDocking > 0) {
		lseek(gFdDocking, 0 , SEEK_SET);
		if (read(gFdDocking, sysvalstring, sizeof(sysvalstring)) > 0) {
			if (esif_ccb_strstr(sysvalstring, "1"))
				dockMode = DOCK_MODE_DOCKED;
			else
				dockMode = DOCK_MODE_UNDOCKED;
		}
	}

	if (dockMode != gDockMode) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &dockMode, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_DOCK_MODE_CHANGED, &evtData);
		gDockMode = dockMode;
	}
}

static void CheckLidStateChange(void)
{
	LidState lidState = LID_STATE_CLOSED;
	char sysvalstring[MAX_SYSFS_STRING] = { 0 };
	EsifData evtData = { 0 };

	if (gFdLidState > 0) {
		lseek(gFdLidState, 0 , SEEK_SET);
		if (read(gFdLidState, sysvalstring, sizeof(sysvalstring)) > 0) {
			if (esif_ccb_strstr(sysvalstring, "open"))
				lidState = LID_STATE_OPEN;
			else
				lidState = LID_STATE_CLOSED;
		}
	}

	if (lidState != gLidState) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &lidState, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_LID_STATE_CHANGED, &evtData);
		gLidState = lidState;
	}
}

static void SendEventForegroundAppChanged(char *workload)
{

	EsifData evtDataStr = { ESIF_DATA_STRING };

	ESIF_DATA_STRING_ASSIGN(evtDataStr, workload, (u32)esif_ccb_strlen(workload, MAX_PATH)+1);
	EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_FOREGROUND_APP_CHANGED, &evtDataStr);

	esif_ccb_strcpy(gPrevWorkload, workload, (esif_ccb_strlen(workload, MAX_PATH)+1));
}

static void GetWorkloadHints()
{
	eEsifError rc = ESIF_OK;
	char fullKeyPath[MAX_PATH] = {'\0'};
	EsifString namesp = DataBank_GetDefault();
	EsifString keyspec = "/shared/export/workload_hints";
	EsifData data_nspace = { ESIF_DATA_STRING };
	EsifData data_key	= { ESIF_DATA_STRING };
	EsifData data_value = { ESIF_DATA_STRING };
	char *workloadName = NULL;
	char *ctx =  NULL;
	esif_flags_t flags = 0;

	ESIF_DATA_STRING_ASSIGN(data_nspace, namesp, esif_ccb_strlen(namesp, MAX_PATH)+1);

	for(UInt32 subKey = 1; subKey <= MAX_WL_HINT_ROWS; subKey++) {

		esif_ccb_sprintf(MAX_PATH,fullKeyPath, "%s/%u", keyspec, subKey);
		ESIF_DATA_STRING_ASSIGN(data_key, fullKeyPath, sizeof(fullKeyPath));
		ESIF_DATA_STRING_ASSIGN(data_value, NULL, ESIF_DATA_ALLOCATE);
		if ((rc = EsifConfigGetItem(&data_nspace, &data_key, &data_value, &flags)) == ESIF_OK) {
			workloadName = esif_ccb_strtok((char *) data_value.buf_ptr, "|", &ctx);
			while(workloadName != NULL) {
				if (IsWorkloadFgProcessOrDaemon(workloadName)) {
					ESIF_TRACE_INFO("Workload Hint: %s \n", workloadName);
					goto exit;
				}
				workloadName = esif_ccb_strtok(NULL, "|", &ctx);
			}
		}
		// free the data_value.buf_ptr if data_value is not NULL && data_value.buf_len >= 1
		EsifData_Set(&data_value, ESIF_DATA_STRING, NULL, ESIF_DATA_ALLOCATE, 0);
	}
exit:
	if(workloadName == NULL)
	{
		if(esif_ccb_strcmp(gPrevWorkload, "<none>"))
		{
			SendEventForegroundAppChanged("<none>");
		}
	}
	else if (esif_ccb_strcmp(workloadName, gPrevWorkload))
	{
		SendEventForegroundAppChanged(workloadName);
		ESIF_TRACE_INFO("ESIF_EVENT_FOREGROUND_APP_CHANGED: %s\n", workloadName);
	}

	// free the data_value.buf_ptr if data_value is not NULL && data_value.buf_len >= 1
	EsifData_Set(&data_value, ESIF_DATA_STRING, NULL, ESIF_DATA_ALLOCATE, 0);
	return;
}

static Bool IsWorkloadFgProcessOrDaemon(const char *workloadName)
{
	char *processName = NULL;
	struct dirent *dirs;
	char fullDirPath[MAX_PATH] = {'\0'};
	FILE *fp = NULL;
	Int32 pid = 0;
	char proc_name[MAX_PATH] = {'\0'};
	char state = {'\0'};
	char scanf_fmt[SCANF_STR_LEN] = { 0 };
	Int32 ppid = 0;
	Int32 pgid = 0;
	Int32 session = 0;
	Int32 tty_nr = 0;
	Int32 tpgid = 0;
	Int32 ret = 0;
	Bool workloadHint = ESIF_FALSE;

	DIR* psyDirs = opendir("/proc");
	if (psyDirs == NULL)
	{
		ESIF_TRACE_ERROR("opendir() failed at path: /proc \n");
		goto exit;
	}

	while((dirs = readdir(psyDirs)) != NULL)
	{
		struct stat st;
		if (!esif_ccb_strcmp(dirs->d_name, ".") || !esif_ccb_strcmp(dirs->d_name, ".."))
		{
			continue;
		}

		if (fstatat(dirfd(psyDirs), dirs->d_name, &st, 0) < 0)
		{
			ESIF_TRACE_ERROR("fstatat() failed at : %s\n", dirs->d_name);
			goto exit;
		}
		if (S_ISDIR(st.st_mode) && isdigit(*dirs->d_name))
		{
			esif_ccb_sprintf(MAX_PATH,fullDirPath, "/proc/%s/stat", dirs->d_name);

			if ((fp = esif_ccb_fopen(fullDirPath, "r", NULL)) == NULL)
			{
				ESIF_TRACE_ERROR("fopen failed at : %s\n", fullDirPath);
				continue;
			}

			esif_ccb_sprintf(sizeof(scanf_fmt), scanf_fmt, "%%d %%%ds %%c %%d %%d %%d %%d %%d ", MAX_PATH - 1);
			ret = esif_ccb_fscanf(fp, scanf_fmt, &pid, proc_name, &state, &ppid, &pgid, &session, &tty_nr, &tpgid);
			if (ret < TOTAL_PIDS_READ)	
			{	
				ESIF_TRACE_DEBUG("fscanf failed, total field read : %u\n", ret);
				if(fp != NULL)
				{
					esif_ccb_fclose(fp);
					fp = NULL;
				}
				continue;
			}
			// Check if the process is FG/daemon && process status is running  or interruptible sleep.
			if ((state == 'R'|| state =='S') && ((pgid == tpgid) || ((tpgid == -1) && (pgid != 0))))
			{
				// Removing the brackets '()' from (process name)
				processName = proc_name + 1;
				processName[esif_ccb_strlen(processName, MAX_PATH) - 1] = '\0';
				if(!esif_ccb_strcmp(processName, workloadName))
				{
					workloadHint =  ESIF_TRUE;
					goto exit;
				}
			}
			if(fp != NULL)
			{
				esif_ccb_fclose(fp);
				fp = NULL;
			}
		}
	}
exit:
	if (psyDirs != NULL)
	{
		closedir(psyDirs);
	}

	if(fp != NULL)
	{
		esif_ccb_fclose(fp);
		fp = NULL;
	}
	return workloadHint;
}
/* Checking if charger is connected. If  online is 1 for any of the charger sysfs path,
 * it will detect the charger is connected. Ommiting battery sysfs sysfs path
*/
static Bool IsChargerConnected(const char *psyPath)
{
	struct dirent *dirs;
	Int64 online = 0;
	char fullDirPath[MAX_PATH] = {'\0'};
	char psyType[MAX_PATH] = {'\0'};
	Bool connected = ESIF_FALSE;

	DIR* psyDirs = opendir(psyPath);
	if (psyDirs == NULL)
	{
		ESIF_TRACE_ERROR("opendir() failed at path: %s\n", psyPath);
		goto exit;
	}

	while((dirs = readdir(psyDirs)) != NULL)
	{
		struct stat st;
		if (!esif_ccb_strcmp(dirs->d_name, ".") || !esif_ccb_strcmp(dirs->d_name, ".."))
		{
			continue;
		}

		if (fstatat(dirfd(psyDirs), dirs->d_name, &st, 0) < 0)
		{
			ESIF_TRACE_ERROR("fstatat() failed at : %s\n", dirs->d_name);
			goto exit;
		}
		if (S_ISDIR(st.st_mode))
		{
			esif_ccb_sprintf(MAX_PATH, fullDirPath, "%s/%s", psyPath, dirs->d_name);

			if (SysfsGetString(fullDirPath, "type", psyType, sizeof(psyType)) <= 0)
			{
				ESIF_TRACE_ERROR("Sysfs Read failed at path: %s/type \n", fullDirPath);
				goto exit;
			}
			if (esif_ccb_strncmp(psyType, "Battery", esif_ccb_strlen("Battery", MAX_PATH)) != 0)
			{
				if(SysfsGetInt64(fullDirPath, "online", &online) <= 0)
				{
					ESIF_TRACE_ERROR("Sysfs Read failed at path: %s/online \n", fullDirPath);
					goto exit;
				}
				ESIF_TRACE_INFO("%s/online: %ld\n", fullDirPath, online);
				if (online)
				{
					connected = ESIF_TRUE;
					break;
				}
			}
		}
	}
exit:
	if (psyDirs != NULL)
	{
		closedir(psyDirs);
	}
	return connected;
}

static void CheckPowerSrcChange(void)
{
	PowerSrc powerSrc = POWER_SRC_AC;
	char sysvalstring[MAX_SYSFS_STRING] = { 0 };
	EsifData evtData = { 0 };

	if (IsChargerConnected(gPowerSupplyPath)) {
		powerSrc = POWER_SRC_AC;
	}
	else {
		powerSrc = POWER_SRC_DC;
	}

	if (powerSrc != g_PowerSrc) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &powerSrc, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_POWER_SOURCE_CHANGED, &evtData);
		atomic_set(&g_PowerSrc, powerSrc);
	}
}

static EsifLinkListPtr GetSubDirsList(const char *basePath)
{
	struct dirent *dirs;
	Int64 online = 0;
	char psyType[MAX_PATH] = {'\0'};
	Bool connected = ESIF_FALSE;
	EsifLinkListPtr subDirListPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	char *fullDirPath = NULL;
	DIR* baseDirs = NULL;

	subDirListPtr = esif_link_list_create();
	if (subDirListPtr == NULL)
	{
		ESIF_TRACE_WARN("Memory allocation failed for subDirList \n");
		goto exit;
	}

	baseDirs = opendir(basePath);
	if (baseDirs == NULL)
	{
		ESIF_TRACE_ERROR("opendir() failed at path: %s\n", basePath);
		goto exit;
	}

	while((dirs = readdir(baseDirs)) != NULL)
	{
		struct stat st;
		if (!esif_ccb_strcmp(dirs->d_name, ".") || !esif_ccb_strcmp(dirs->d_name, ".."))
		{
			continue;
		}

		if (fstatat(dirfd(baseDirs), dirs->d_name, &st, 0) < 0)
		{
			ESIF_TRACE_ERROR("fstatat() failed at : %s\n", dirs->d_name);
			goto exit;
		}
		if (S_ISDIR(st.st_mode))
		{
			fullDirPath = (char *)esif_ccb_malloc(MAX_PATH);
			esif_ccb_sprintf(MAX_PATH, fullDirPath, "%s/%s", basePath, dirs->d_name);
			nodePtr = esif_link_list_create_node(fullDirPath);
			if (NULL == nodePtr) {
				ESIF_TRACE_WARN("esif_link_list Memory allocation failed for nodePtr \n");
				goto exit;
			}
			esif_link_list_add_node_at_back(subDirListPtr, nodePtr);
		}
	}
exit:
	if (baseDirs != NULL)
	{
		closedir(baseDirs);
	}

	return subDirListPtr;
}


static UInt64 ReadBatterySysfsInteger(
		const char *batSysfsPath,
		const char *fileNameAh,
		const char *fileNameWh
		)
{
	Int64 sysVal = 0;
	UInt64 temp = 0ULL;
	int ret = 0;

	ret =  SysfsGetInt64(batSysfsPath, fileNameAh, &sysVal);
	if (ret <= 0)
	{
		ret =  SysfsGetInt64(batSysfsPath, fileNameWh, &sysVal);
		if (ret <= 0)
		{
			ESIF_TRACE_ERROR("Sysfs Read Failed! at path:%s, file:%s\n", batSysfsPath, fileNameWh);
		}
	}
exit:
	temp  = ((UInt64)sysVal);
	return temp;
}

static UInt32 AggregateBattPercentage(char *psyBasePath)
{

	EsifLinkListPtr dirListPtr = NULL;
	EsifLinkListNodePtr curPtr = NULL;
	Int64 batPresent = 0;
	UInt64 energyNow = 0;
	UInt64 energyFull = 0;
	UInt64 totalEnergyNow = 0;
	UInt64 totalEnergyFull = 0;
	UInt64 batPerc = 0;

	char psyType[MAX_PATH] = {'\0'};
	char serialNumber[MAX_PATH] = {'\0'};
	UInt64 aggregateBatPercentage = 0;

	dirListPtr = GetSubDirsList(psyBasePath);
	if (dirListPtr == NULL)
	{
		ESIF_TRACE_DEBUG("dirList is NULL \n");
		goto exit;
	}
	else {
		curPtr = dirListPtr->head_ptr;
	}

	while(curPtr != NULL)
	{
		if (SysfsGetString((char *)curPtr->data_ptr, "type", psyType, sizeof(psyType)) <= 0)
		{
			ESIF_TRACE_ERROR("Sysfs Read failed at path: %s/type \n", (char *)curPtr->data_ptr);
			curPtr = curPtr->next_ptr;
			continue;
		}
		if (esif_ccb_strncmp(psyType, "Battery", esif_ccb_strlen("Battery", MAX_PATH)) == 0)
		{
			if((SysfsGetInt64((char *)curPtr->data_ptr, "present", &batPresent) > 0) &&
			(SysfsGetString((char *)curPtr->data_ptr, "serial_number", serialNumber, sizeof(serialNumber)) > 0))
			{
				energyFull = ReadBatterySysfsInteger((char *)curPtr->data_ptr, "energy_full", "charge_full");
				totalEnergyFull = totalEnergyFull + energyFull;

				energyNow = ReadBatterySysfsInteger((char *)curPtr->data_ptr, "energy_now", "charge_now");
				totalEnergyNow = totalEnergyNow + energyNow;

				ESIF_TRACE_DEBUG("%s/present: %lld, serial_number: %s, EnergyNow: %llu, TotalEnergyNow: %llu, EnergyFull: %llu TotalEnergyFull: %llu\n",
						(char *)curPtr->data_ptr, batPresent, serialNumber, energyNow, totalEnergyNow, energyFull, totalEnergyFull);
			}
		}
		curPtr = curPtr->next_ptr;
	}
	if (totalEnergyFull && (totalEnergyNow <= totalEnergyFull))
	{
		aggregateBatPercentage = (totalEnergyNow * 100) / totalEnergyFull;
	}
	else {
		ESIF_TRACE_DEBUG("Invalid totalEnergyNow/totalEnergyFull\n");
	}
exit:
	esif_link_list_free_data_and_destroy(dirListPtr, NULL);
	return ((UInt32) aggregateBatPercentage);

}

static void CheckBatteryPercentChange(void)
{
	UInt32 batteryPercentage = 0;
	char sysvalstring[MAX_SYSFS_STRING] = { 0 };
	EsifData evtData = { 0 };

	batteryPercentage = AggregateBattPercentage(gPowerSupplyPath);

	if (batteryPercentage != g_BatteryPercentage) {
		ESIF_TRACE_DEBUG("Prev AggregateBatteryPercentage: %lu , AggregateBatteryPercentage: %lu\n",
				g_BatteryPercentage, batteryPercentage);
		ESIF_DATA_UINT32_ASSIGN(evtData, &batteryPercentage, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED, &evtData);
		atomic_set(&g_BatteryPercentage, batteryPercentage);
	}
}

static void *EsifIio_Poll(void *ptr)
{

	UNREFERENCED_PARAMETER(ptr);


	while(gEsifSensorMgrStarted) {
		// Update sensor values
		if (gAccelLid) {
			AccelRawUpdate(gAccelLid);
		}

		if (gAccelBase) {
			AccelRawUpdate(gAccelBase);
		}

		// Platform and display orientation are only valid if the lid accel is present
		if (gAccelLid) {
			CheckDispPlatOrientation(gAccelLid);
		}

		// Motion detection
		if (gAccelLid) {
			CheckMotionChange(gAccelLid);
		} else if (gAccelBase) {
			CheckMotionChange(gAccelBase);
		}
		// Dock mode change detection
		CheckDockModeChange();
		// Power source(AC/DC)change detection
		CheckPowerSrcChange();

		// Lid state change detection
		CheckLidStateChange();


		// Platform type change detection (clamshell, tablet, tent, etc.)
		CheckPlatTypeChange(gLidAngle);
		if (gIsWLHintRegistered == ESIF_TRUE)
		{
			// AP Workload Hint feature
			GetWorkloadHints();
		}
		//Power Slider Value
		GetPowerSliderMode();

		// Battery percent change detection
		CheckBatteryPercentChange();

		esif_ccb_sleep(ESIF_IIO_SAMPLE_PERIOD);
	}

	return NULL;
}

static void StartEsifSensorMgr()
{
	eEsifError rc1 = ESIF_OK;
	eEsifError rc2 = ESIF_OK;

	if (!gEsifSensorMgrStarted) {
		ESIF_TRACE_DEBUG("Starting ESIF Sensor Manager\n");
		rc1 = EsifSensorMgr_RegisterSensors();
		rc2 = EsifSensorMgr_InitializeNonIioBusSensors();

		if (ESIF_OK == rc1 || ESIF_OK == rc2) {
			gEsifSensorMgrStarted = ESIF_TRUE;
			esif_ccb_thread_create(&gEsifSensorMgrThread, EsifIio_Poll, NULL);
		} else {
			ESIF_TRACE_DEBUG("ESIF Sensor Manager: could not find any sensor, abort\n");
		}
	}
}

static void StopEsifSensorMgr()
{
	if (gEsifSensorMgrStarted) {
		ESIF_TRACE_DEBUG("Stopping ESIF Sensor Manager...\n");
		gEsifSensorMgrStarted = ESIF_FALSE;
		pthread_cancel(gEsifSensorMgrThread);
		esif_ccb_thread_join(&gEsifSensorMgrThread);
		EsifSensorMgr_DeregisterSensors();
	}
}

/**
 * Public Interface of ESIF Sensor Manager
 */

void EsifSensorMgr_Init()
{
	// Nothing to do
}

void EsifSensorMgr_Exit()
{
	StopEsifSensorMgr();
}

eEsifError DeregisterCodeEvent()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_INFO("DeregisterCodeEvent: ESIF_EVENT_FOREGROUND_APP_CHANGED\n");

	if (gPrevWorkload)
	{
		esif_ccb_free(gPrevWorkload);
	}
	gIsWLHintRegistered =  ESIF_FALSE;

	return rc;
}

eEsifError RegisterCodeEvent()
{
	eEsifError rc = ESIF_OK;

	gPrevWorkload = (EsifString) esif_ccb_malloc(MAX_PATH);
	if(!gPrevWorkload)
	{
		ESIF_TRACE_WARN(" Memory allocation failed for gPrevWorkload \n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	esif_ccb_memset(gPrevWorkload, 0, MAX_PATH);
	esif_ccb_strcpy(gPrevWorkload, "<none>", esif_ccb_strlen("<none>", MAX_PATH)+1);
	SendEventForegroundAppChanged(gPrevWorkload);
	ESIF_TRACE_INFO("RegisterForCodeEvent: ESIF_EVENT_FOREGROUND_APP_CHANGED: %s\n", gPrevWorkload);
	gIsWLHintRegistered =  ESIF_TRUE;
exit:
	return rc;
}

eEsifError disable_code_event_lin(
	eEsifEventType eventType
	)
{
	eEsifError rc = ESIF_OK;
	EsifData evtDataStr = { ESIF_DATA_STRING };

	ESIF_TRACE_INFO("Code Event: %s\n", esif_event_type_str(eventType));

	switch (eventType) {
	case ESIF_EVENT_FOREGROUND_APP_CHANGED:
		rc = DeregisterCodeEvent();
		break;
	case ESIF_EVENT_EXTERNAL_MONITOR_CONNECTION_STATE_CHANGED:
	case ESIF_EVENT_FOREGROUND_BACKGROUND_RATIO_CHANGED:
	case ESIF_EVENT_COLLABORATION_CHANGED:
	case ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED:
		break;
	case ESIF_EVENT_OS_USER_INTERACTION_CHANGED:
		break;
	default:
		break;
	}
	return rc;
}

eEsifError enable_code_event_lin(
	eEsifEventType eventType
	)
{
	eEsifError rc = ESIF_OK;
	EsifData evtDataStr = { ESIF_DATA_STRING };

	ESIF_TRACE_INFO("Code Event: %s\n", esif_event_type_str(eventType));

	switch (eventType) {
	case ESIF_EVENT_FOREGROUND_APP_CHANGED:
		rc = RegisterCodeEvent();
		break;
	case ESIF_EVENT_EXTERNAL_MONITOR_CONNECTION_STATE_CHANGED:
	case ESIF_EVENT_FOREGROUND_BACKGROUND_RATIO_CHANGED:
	case ESIF_EVENT_COLLABORATION_CHANGED:
	case ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED:
		break;
	case ESIF_EVENT_OS_USER_INTERACTION_CHANGED:
		break;
	default:
		break;
	}
	return rc;
}

/**
 * These functions are called indirectly by ESIF apps such as DPTF.
 * For the registration function, return success only if the physical
 * or logical sensor (and related events) are present in Linux.
 * There is not much else to do since Linux OS has already handled
 * sensor enumeration/registration by its IIO bus driver.
 */
eEsifError esif_register_sensor_lin(eEsifEventType eventType)
{
	eEsifError rc = ESIF_OK;
	EsifData evtData = { 0 };

	/* Send gratuitous events when DPTF registers for
	 * sensor events, if we do support such events.
	 * This is to make the DPTF UI happy, otherwise
	 * the UI will show that the corresponding event
	 * is not supported.
	 */
	switch (eventType) {
   	case ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED:
		if (gAccelLid) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &gDispOrientation, sizeof(UInt32));
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, eventType, &evtData);
		}
		break;
	case ESIF_EVENT_DEVICE_ORIENTATION_CHANGED:
		if (gAccelLid) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &gPlatOrientation, sizeof(UInt32));
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, eventType, &evtData);
		}
		break;
	case ESIF_EVENT_MOTION_CHANGED:
		if (gAccelLid || gAccelBase) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &gInMotion, sizeof(UInt32));
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, eventType, &evtData);
		}
		break;
	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}

	if (ESIF_OK == rc) { // Upon successful registration
		ESIF_TRACE_INFO("RegisterForSensorEvent: %s\n", esif_event_type_str(eventType));
		StartEsifSensorMgr();
	}

	return rc;
}

/**
 * Always return success even if the sensor/sensor events are
 * not available in Linux.
 */
eEsifError esif_unregister_sensor_lin(eEsifEventType eventType)
{
	return ESIF_OK;
}

eEsifError register_for_system_metric_notification_lin(esif_guid_t *guid)
{
	const esif_guid_t guidDockMode = {0x30, 0x8d, 0x0c, 0xc9, 0xba, 0x5b, 0x40, 0x0a, 0x99, 0x0a, 0xed, 0x27, 0x29, 0x29, 0xb6, 0xb6};
	const esif_guid_t guidPowerSrc = {0x5d, 0x3e, 0x9a, 0x59, 0xe9, 0xd5, 0x4b, 0x00, 0xa6, 0xbd, 0xff, 0x34, 0xff, 0x51, 0x65, 0x48};
	const esif_guid_t guidBattPercent = {0xa7, 0xad, 0x80, 0x41, 0xb4, 0x5a, 0x4c, 0xae, 0x87, 0xa3, 0xee, 0xcb, 0xb4, 0x68, 0xa9, 0xe1};
	const esif_guid_t guidLidState = {0xba, 0x3e, 0x0f, 0x4d, 0xb8, 0x17, 0x40, 0x94, 0xa2, 0xd1, 0xd5, 0x63, 0x79, 0xe6, 0xa0, 0xf3};
	const esif_guid_t guidPlatformType = {0x38, 0x92, 0xb5, 0x8c, 0xc8, 0x74, 0x45, 0xbe, 0xb2, 0x19, 0xab, 0x87, 0x49, 0x51, 0x9b, 0xfb};
	const esif_guid_t guidPowerSliderMode = {0xA4, 0xF0, 0x60, 0x79, 0xf3, 0xe9, 0x45, 0xe0, 0x85, 0x62, 0x8a, 0xa4, 0x5a, 0xe2, 0x21, 0xfa};
	eEsifError rc = ESIF_OK;
	EsifData evtData = { 0 };

	if (NULL == guid) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Send gratuitous event for OS metric registration
	if (0 == memcmp(guid, guidDockMode, ESIF_GUID_LEN)) {
		if (gDockMode != DOCK_MODE_INVALID) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &gDockMode, sizeof(UInt32));
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_DOCK_MODE_CHANGED, &evtData);
			ESIF_TRACE_INFO("RegisterForSensorEvent: ESIF_EVENT_OS_DOCK_MODE_CHANGED\n");
		}
		StartEsifSensorMgr();
	}

	if (0 == memcmp(guid, guidPowerSrc, ESIF_GUID_LEN)) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &g_PowerSrc, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_POWER_SOURCE_CHANGED, &evtData);
		ESIF_TRACE_INFO("RegisterForSensorEvent: ESIF_EVENT_OS_POWER_SOURCE_CHANGED\n");
		StartEsifSensorMgr();
	}

	if (0 == memcmp(guid, guidBattPercent, ESIF_GUID_LEN)) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &g_BatteryPercentage, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED, &evtData);
		ESIF_TRACE_INFO("RegisterForSensorEvent: ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED\n");
		StartEsifSensorMgr();
	}

	if (0 == memcmp(guid, guidLidState, ESIF_GUID_LEN)) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &gLidState, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_LID_STATE_CHANGED, &evtData);
		ESIF_TRACE_INFO("RegisterForSensorEvent: ESIF_EVENT_OS_LID_STATE_CHANGED\n");
		StartEsifSensorMgr();
	}

	if (0 == memcmp(guid, guidPlatformType, ESIF_GUID_LEN)) {
		if (gPlatType != PLATFORM_TYPE_INVALID) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &gPlatType, sizeof(UInt32));
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED, &evtData);
			ESIF_TRACE_INFO("RegisterForSensorEvent: ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED\n");
		}
		StartEsifSensorMgr();
	}

	if (0 == memcmp(guid, guidPowerSliderMode, ESIF_GUID_LEN)) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &gPowerSliderValue, sizeof(UInt32));
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_POWER_SLIDER_VALUE_CHANGED, &evtData);
		ESIF_TRACE_INFO("RegisterForSensorEvent: ESIF_EVENT_OS_POWER_SLIDER_VALUE_CHANGED\n");
		StartEsifSensorMgr();
	}

exit:
	return rc;
}
