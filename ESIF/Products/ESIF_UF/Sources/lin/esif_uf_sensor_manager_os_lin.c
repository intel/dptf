/******************************************************************************
** Copyright (c) 2013-2018 Intel Corporation All Rights Reserved
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
#include "esif_uf_accelerometer.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <math.h>
#include <dirent.h>

#define IIO_STR_LEN 24
#define ESIF_IIO_SAMPLE_PERIOD 5 // In seconds
#define MAX_GFORCE (9.8 * 2) // All Chromebooks accel have default -2G to 2G range
#define MOTION_CHANGE_THRESHOLD 0.007 // Normalize threshold to declare motion state change

typedef enum SensorType_e {
	SENSOR_TYPE_ACCEL,
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

/**
 * ToDo: Add definitions of gyro, pressure sensor, etc.
 */

typedef struct Sensor_s {
	SensorBase base;
	union {
		Accelerometer accel;
	} data;
} Sensor, *SensorPtr;

static esif_thread_t gEsifSensorMgrThread;
static Bool gEsifSensorMgrStarted = ESIF_FALSE;
static const char gXRawNodeName[] = "in_accel_x_raw";
static const char gYRawNodeName[] = "in_accel_y_raw";
static const char gZRawNodeName[] = "in_accel_z_raw";
static const char gSensorBasePath[] =  "/sys/bus/iio/devices";
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

// Global Pointers to base and lid accelerometers for fast processing
static Sensor *gAccelBase;
static Sensor *gAccelLid;

// Global variables keeping track of current x/y/z vectors and platform/display orientation/platform type
static AccelerometerData gCurAccelData;
static PlatformOrientation gPlatOrientation = ORIENTATION_PLAT_MAX;
static DisplayOrientation gDispOrientation = ORIENTATION_DISP_MAX;
static Motion gInMotion = MOTION_MAX;
static DockMode gDockMode = DOCK_MODE_INVALID;

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

static int SysfsGetString(char *path, char *filename, char *str, size_t buf_len)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_PATH] = { 0 };
	char scanf_fmt[IIO_STR_LEN] = { 0 };

	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		return ret;
	}

	// Use dynamic format width specifier to avoid scanf buffer overflow
	esif_ccb_sprintf(sizeof(scanf_fmt), scanf_fmt, "%%%ds", (int)buf_len - 1);
	ret = esif_ccb_fscanf(fd, scanf_fmt, str);
	esif_ccb_fclose(fd);

	return ret;
}

static eEsifError SysfsGetInt(const char *path, const char *filename, int *pInt)
{
	FILE *fp = NULL;
	eEsifError rc = ESIF_OK;
	char filepath[MAX_PATH] = { 0 };
	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", path, filename);

	if ((fp = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}
	if (esif_ccb_fscanf(fp, "%d", pInt) <= 0) {
		rc = ESIF_E_INVALID_HANDLE;
	}

	esif_ccb_fclose(fp);

exit:
	return rc;
}

static eEsifError SysfsGetIntDirect(int fd, int *pInt)
{
	eEsifError rc = ESIF_OK;
	char buf[IIO_STR_LEN] = {0};

	lseek(fd, 0, SEEK_SET);
	if (read(fd, buf, IIO_STR_LEN) > 0) {
		if (esif_ccb_sscanf(buf, "%d", pInt) <= 0) {
			rc = ESIF_E_INVALID_HANDLE;
			ESIF_TRACE_WARN("Failed to get file scan. Error code: %d .\n",rc);
		}
	}
	else {
		rc = ESIF_E_INVALID_HANDLE;
		ESIF_TRACE_WARN("Error on context file read: %s\n", strerror(errno));
	}

	return rc;
}

static eEsifError SysfsGetFloat(const char *path, const char *filename, float *pFloat)
{
	FILE *fp = NULL;
	eEsifError rc = ESIF_OK;
	char filepath[MAX_PATH] = { 0 };
	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", path, filename);

	if ((fp = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}
	if (esif_ccb_fscanf(fp, "%f", pFloat) <= 0) {
		rc = ESIF_E_INVALID_HANDLE;
	}

	esif_ccb_fclose(fp);

exit:
	return rc;
}

static int IioDeviceFilter(const struct dirent *entry)
{
	if (esif_ccb_strstr(entry->d_name, "iio:device")) return 1;
	else return 0;
};

static void AccelGetLoc(SensorPtr sensorPtr, char *fullPath)
{
	char iioSysfsNode[IIO_STR_LEN] = { 0 };
	ESIF_ASSERT(sensorPtr != NULL);

	if (SysfsGetString(fullPath, "location", iioSysfsNode, sizeof(iioSysfsNode)) > 0) {
		if (esif_ccb_strstr(iioSysfsNode, "base")) {
			sensorPtr->base.loc = SENSOR_LOC_BASE;
			if (NULL == gAccelBase) gAccelBase = sensorPtr;
		} else {
			sensorPtr->base.loc = SENSOR_LOC_LID;
			if (NULL == gAccelLid) gAccelLid = sensorPtr;
		}
	}
}

static void AccelGetScale(SensorPtr sensorPtr, char *fullPath)
{
	float scale = 0;
	ESIF_ASSERT(sensorPtr != NULL);

	if (ESIF_OK == SysfsGetFloat(fullPath, "scale", &scale)) {
		sensorPtr->data.accel.scale = scale;
	}
}

static void AccelOpenFileDescriptors(SensorPtr sensorPtr, char *fullPath)
{
	char axixNodePath[MAX_PATH] = { 0 };
	ESIF_ASSERT(sensorPtr != NULL);

	esif_ccb_sprintf(MAX_PATH, axixNodePath, "%s/%s", fullPath, gXRawNodeName);
	sensorPtr->data.accel.fdX = open(axixNodePath, O_RDONLY);
	esif_ccb_sprintf(MAX_PATH, axixNodePath, "%s/%s", fullPath, gYRawNodeName);
	sensorPtr->data.accel.fdY = open(axixNodePath, O_RDONLY);
	esif_ccb_sprintf(MAX_PATH, axixNodePath, "%s/%s", fullPath, gZRawNodeName);
	sensorPtr->data.accel.fdZ = open(axixNodePath, O_RDONLY);
}

static void InitSensor(int index, char *devName)
{
	SensorPtr sensorPtr = &gSensors[index];
	char iioSysfsNode[IIO_STR_LEN] = { 0 };
	char fullPath[MAX_PATH] = { 0 };

	sensorPtr->base.type = SENSOR_TYPE_NA;

	esif_ccb_sprintf(MAX_PATH, fullPath, "%s/%s", gSensorBasePath, devName);
	if (SysfsGetString(fullPath, "name", iioSysfsNode, sizeof(iioSysfsNode)) > 0) {
		// Init code for accelerometers
		if (esif_ccb_strstr(iioSysfsNode, "accel")) {
			sensorPtr->base.type = SENSOR_TYPE_ACCEL;
			AccelGetLoc(sensorPtr, fullPath);
			AccelGetScale(sensorPtr, fullPath);
			AccelOpenFileDescriptors(sensorPtr, fullPath);
		}

		// ToDo: add init code for other type of sensors in the future
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
		free(namelist[i]);
	};
	free(namelist);

exit:
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
}

static void AccelRawUpdate(SensorPtr sensorPtr)
{
	int fd = 0;
	ESIF_ASSERT(sensorPtr != NULL);

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
	ESIF_ASSERT(sensorPtr != NULL);

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

	ESIF_ASSERT(sensorPtr != NULL);

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
		EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED, &evtData);
		gDispOrientation = newDispOrientation;
	}

	if (newPlatOrientation != gPlatOrientation) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &newPlatOrientation, sizeof(PlatformOrientation));
		EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_DEVICE_ORIENTATION_CHANGED, &evtData);
		gPlatOrientation = newPlatOrientation;
	}
}


static void CheckMotionChange(SensorPtr sensorPtr)
{
	EsifData evtData = { 0 };
	float delta = 0;
	AccelerometerData data = { 0 };
	Motion newMotionState = MOTION_OFF;

	ESIF_ASSERT(sensorPtr != NULL);

	data = NormalizeAccelRawData(sensorPtr);
	delta = sqrt((data.xVal - gCurAccelData.xVal) * (data.xVal - gCurAccelData.xVal) +
			(data.yVal - gCurAccelData.yVal) * (data.yVal - gCurAccelData.yVal) +
			(data.zVal - gCurAccelData.zVal) * (data.zVal - gCurAccelData.zVal));
	if (delta > MOTION_CHANGE_THRESHOLD) {
		newMotionState = MOTION_ON;
	}

	if (newMotionState != gInMotion) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &newMotionState, sizeof(UInt32));
		EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_MOTION_CHANGED, &evtData);
		gInMotion = newMotionState;
	}
	gCurAccelData = data;
}

static void CheckPlatTypeChange(SensorPtr baseSensorPtr, SensorPtr lidSensorPtr)
{
	EsifData evtData = { 0 };
	PlatformType platType = PLATFORM_TYPE_INVALID;
	AccelerometerData baseData = { 0 };
	AccelerometerData lidData = { 0 };

	ESIF_ASSERT(baseSensorPtr != NULL);
	ESIF_ASSERT(lidSensorPtr != NULL);

	baseData = NormalizeAccelRawData(baseSensorPtr);
	lidData = NormalizeAccelRawData(lidSensorPtr);

	EsifAccelerometer_GetPlatformType(&baseData, &lidData, &platType);
	/* ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED is currently a Windows system 
	 * metrics event which is not handled by Linux, because of this 
	 * we cannot send a gratuitous event on sensor registration. As a workaround
	 * we always send this event to DPTF regardless if its value has changed, 
	 * and this will make the DPTF UI happy.
	 */
	ESIF_DATA_UINT32_ASSIGN(evtData, &platType, sizeof(UInt32));
	EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED, &evtData);
}

static void CheckDockModeChange(void)
{
	DockMode dockMode = DOCK_MODE_INVALID;
	DIR* dir = opendir("/dev/input/by-id");
	EsifData evtData = { 0 };

	/* TODO:
	 * On Chromebooks the above directory will be created when system is docked.
	 * Need to check if this works for other Linux derived systems.
	 * Currently only enable docking/undocking detection for Chrome OS.
	 */
	if (dir) {
		dockMode = DOCK_MODE_DOCKED;
		closedir(dir);
	} else {
		dockMode = DOCK_MODE_UNDOCKED;
	}

	if (dockMode != gDockMode) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &dockMode, sizeof(UInt32));
		EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_DOCK_MODE_CHANGED, &evtData);
		gDockMode = dockMode;
	}
}

static void *EsifIio_Poll(void *ptr)
{
	UNREFERENCED_PARAMETER(ptr);

#ifdef ESIF_ATTR_OS_ANDROID
	sigusr1_enable();
#endif

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

		// Platform type change detection
		if (gAccelBase && gAccelLid) {
			CheckPlatTypeChange(gAccelBase, gAccelLid);
		};

		// Dock mode change - only for Chrome OS
#ifdef ESIF_ATTR_OS_CHROME
		CheckDockModeChange();
#endif

		esif_ccb_sleep(ESIF_IIO_SAMPLE_PERIOD);
	}

	return NULL;
}

static void StartEsifSensorMgr()
{
	if (!gEsifSensorMgrStarted) {
		ESIF_TRACE_DEBUG("Starting ESIF Sensor Manager\n");
		if (ESIF_OK == EsifSensorMgr_RegisterSensors()) {
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
#ifdef ESIF_ATTR_OS_ANDROID
		// Android NDK does not support pthread_cancel()
		// Use pthread_kill() to emualte
		pthread_kill(gEsifSensorMgrThread, SIGUSR1);
#else
		pthread_cancel(gEsifSensorMgrThread);
#endif
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
			EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, eventType, &evtData);
		}
		break;
	case ESIF_EVENT_DEVICE_ORIENTATION_CHANGED:
		if (gAccelLid) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &gPlatOrientation, sizeof(UInt32));
			EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, eventType, &evtData);
		}
		break;
	case ESIF_EVENT_MOTION_CHANGED:
		if (gAccelLid || gAccelBase) {
			ESIF_DATA_UINT32_ASSIGN(evtData, &gInMotion, sizeof(UInt32));
			EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, eventType, &evtData);
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

// The only supported system metrics notification is docking/undocking for Chrome OS
eEsifError register_for_system_metric_notification_lin(esif_guid_t *guid)
{
	const esif_guid_t guidDockMode = { 0x30, 0x8d, 0x0c, 0xc9, 0xba, 0x5b, 0x40, 0x0a, 0x99, 0x0a, 0xed, 0x27, 0x29, 0x29, 0xb6, 0xb6};
	eEsifError rc = ESIF_OK;
	EsifData evtData = { 0 };

	if (NULL == guid) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

#ifdef ESIF_ATTR_OS_CHROME
	// Send gratuitous event for docking/undokcing registration
	if (0 == memcmp(guid, guidDockMode, ESIF_GUID_LEN)) {
		ESIF_DATA_UINT32_ASSIGN(evtData, &gDockMode, sizeof(UInt32));
		EsifEventMgr_SignalEvent(0, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_OS_DOCK_MODE_CHANGED, &evtData);
		ESIF_TRACE_INFO("RegisterForSensorEvent: ESIF_EVENT_OS_DOCK_MODE_CHANGED\n");
		StartEsifSensorMgr();
	}
#endif

exit:
	return rc;
}
