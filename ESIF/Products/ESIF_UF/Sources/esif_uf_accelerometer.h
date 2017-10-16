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

#include "esif.h"
#include "esif_uf.h"

#ifndef _ESIF_UF_ACCELEROMETER
#define _ESIF_UF_ACCELEROMETER

typedef struct AccelerometerData_s
{
	float xVal;
	float yVal;
	float zVal;
} AccelerometerData, *AccelerometerDataPtr;

typedef struct InclinometerMinMaxConfig_s
{
	float xFlatUpMin;
	float xFlatUpMax;
	float xFlatDownMin;
	float xFlatDownMax;
	float xUprightMin;
	float xUprightMax;
	float xUprightInvMin;
	float xUprightInvMax;
	float yFlatMax;
} InclinometerMinMaxConfig, *InclinometerMinMaxConfigPtr;

#define DV_NAMESPACE_SENSOR "dptf"

// Default Pitch and Roll
#define MAX_PITCH_DEGREES 180.0f
#define MIN_PITCH_DEGREES -180.0f
#define MAX_ROLL_DEGREES 90.0f
#define MIN_ROLL_DEGREES -90.0f
#define INVALID_DEGREES 255.0f

// Default X Orientation
#define INCLIN_X_ORIENT_FLAT_UP_MIN -45.0
#define INCLIN_X_ORIENT_FLAT_UP_MAX 45.0
#define INCLIN_X_ORIENT_FLAT_DOWN_MAX -135.0
#define INCLIN_X_ORIENT_FLAT_DOWN_MIN 135.0
#define INCLIN_X_ORIENT_UPRIGHT_MIN 45.0
#define INCLIN_X_ORIENT_UPRIGHT_MAX 135.0
#define INCLIN_X_ORIENT_UPRIGHT_INVERTED_MIN -135.0
#define INCLIN_X_ORIENT_UPRIGHT_INVERTED_MAX -45.0

// Default Y Orientation
#define INCLIN_Y_ORIENT_FLAT_MAX 45.0

#define PLATFORM_TYPE_TABLET_ANGLE -0.99
#define PLATFORM_TYPE_TENT_ANGLE -0.1

// Default timeout
#define INCLIN_ORIENTATION_CHANGE_DETECT_TIMEOUT 5000 // ms
#define ACCEL_USER_GRASP_TIMEOUT 2000 // ms

// DV keys for angles
#define DV_KEY_INCLIN_X_ORIENT_FLAT_UP_MIN "/cmp/cem/x_flat_up_min"
#define DV_KEY_INCLIN_X_ORIENT_FLAT_UP_MAX "/cmp/cem/x_flat_up_max"
#define DV_KEY_INCLIN_X_ORIENT_FLAT_DOWN_MAX "/cmp/cem/x_flat_down_max"
#define DV_KEY_INCLIN_X_ORIENT_FLAT_DOWN_MIN "/cmp/cem/x_flat_down_min"
#define DV_KEY_INCLIN_X_ORIENT_UPRIGHT_MIN "/cmp/cem/x_upright_min"
#define DV_KEY_INCLIN_X_ORIENT_UPRIGHT_MAX "/cmp/cem/x_upright_max"
#define DV_KEY_INCLIN_X_ORIENT_UPRIGHT_INVERTED_MIN "/cmp/cem/x_upright_inverted_min"
#define DV_KEY_INCLIN_X_ORIENT_UPRIGHT_INVERTED_MAX "/cmp/cem/x_upright_inverted_max"
#define DV_KEY_INCLIN_Y_ORIENT_FLAT_MAX "/cmp/cem/y_flat_max"

// DV keys for timeout
#define DV_KEY_PROXIMITY_TIMEOUT "/cmp/cem/proximity_timeout"
#define DV_KEY_DEVICE_ORIENTATION_TIMEOUT "/cmp/cem/device_orientation_timeout"
#define DV_KEY_DISPLAY_ORIENTATION_TIMEOUT "/cmp/cem/display_orientation_timeout"


// We set these enums in angles so that we can pass it
// to DPTF as is
typedef enum Motion_e {
	MOTION_OFF = 0,
	MOTION_ON = 1,

	MOTION_MAX
} Motion, *MotionPtr;

typedef enum PlatformOrientation_e {
	ORIENTATION_PLAT_FLAT_UP = 0,
	ORIENTATION_PLAT_FLAT_DOWN = 180,
	ORIENTATION_PLAT_UPRIGHT = 90,
	ORIENTATION_PLAT_UPRIGHT_INVERTED = 270,

	ORIENTATION_PLAT_MAX
} PlatformOrientation, *PlatformOrientationPtr;

typedef enum DisplayOrientation_e {
	ORIENTATION_DISP_LANDSCAPE,
	ORIENTATION_DISP_PORTRAIT,
	ORIENTATION_DISP_LANDSCAPE_INVERTED,
	ORIENTATION_DISP_PORTRAIT_INVERTED,
	ORIENTATION_DISP_INDETERMINATE,

	ORIENTATION_DISP_MAX
} DisplayOrientation, *DisplayOrientationPtr;

typedef enum PlatformType_e {
	PLATFORM_TYPE_INVALID = 0,
	PLATFORM_TYPE_CLAMSHELL = 1,
	PLATFORM_TYPE_TABLET = 2,
	PLATFORM_TYPE_TENT = 3
} PlatformType, *PlatformTypePtr;

/* TODO:
 * The following dock mode definition is the only common DPPE event supported
 * by both Windows and Linux that is not related to accelerometers. Once we
 * have more such common non-accelerometer related DPPE defined, we shall
 * bundle and move these definitions to a dedicated common DPPE header file.
 */
typedef enum e_DockMode {
	DOCK_MODE_INVALID = 0,
	DOCK_MODE_UNDOCKED = 1,
	DOCK_MODE_DOCKED = 2,
} DockMode, *DockModePtr;

static ESIF_INLINE char* GetPlatOrientationStr(PlatformOrientation platOrientation)
{
	char* str = "Undefined orientation";

	switch (platOrientation) {
		ESIF_CASE_ENUM(ORIENTATION_PLAT_FLAT_UP);
		ESIF_CASE_ENUM(ORIENTATION_PLAT_FLAT_DOWN);
		ESIF_CASE_ENUM(ORIENTATION_PLAT_UPRIGHT);
		ESIF_CASE_ENUM(ORIENTATION_PLAT_UPRIGHT_INVERTED);
	default: break;
	}

	return str;
}

static ESIF_INLINE char* GetDispOrientationStr(DisplayOrientation dispOrientation)
{
	char* str = "Undefined orientation";

	switch (dispOrientation) {
		ESIF_CASE_ENUM(ORIENTATION_DISP_PORTRAIT);
		ESIF_CASE_ENUM(ORIENTATION_DISP_PORTRAIT_INVERTED);
		ESIF_CASE_ENUM(ORIENTATION_DISP_LANDSCAPE);
		ESIF_CASE_ENUM(ORIENTATION_DISP_LANDSCAPE_INVERTED);
		ESIF_CASE_ENUM(ORIENTATION_DISP_INDETERMINATE);
	default: break;
	}

	return str;
}

static ESIF_INLINE char* GetPlatTypeStr(PlatformType platType)
{
	char* str = "Undefined platform type";

	switch (platType) {
		ESIF_CASE_ENUM(PLATFORM_TYPE_CLAMSHELL);
		ESIF_CASE_ENUM(PLATFORM_TYPE_TABLET);
		ESIF_CASE_ENUM(PLATFORM_TYPE_TENT);
	default: break;
	}

	return str;
}


#ifdef __cplusplus
extern "C" {
#endif
	PlatformOrientation ESIF_CALLCONV EsifAccelerometer_GetPlatOrientationFromPitch(
		float pitchDegrees,
		InclinometerMinMaxConfig inclinMinMaxConfig,
		PlatformOrientation curPlatOrientation);

	PlatformOrientation ESIF_CALLCONV EsifAccelerometer_GetPlatOrientationFromRoll(
		float rollDegrees,
		InclinometerMinMaxConfig inclinMinMaxConfig,
		PlatformOrientation xPlatOrientation,
		PlatformOrientation curPlatOrientation);

	DisplayOrientation ESIF_CALLCONV EsifAccelerometer_GetDisplayOrientation(
		float pitchDegrees,
		float rollDegrees,
		InclinometerMinMaxConfig inclinMinMaxConfig,
		DisplayOrientation curDispOrientation);

	Bool ESIF_CALLCONV EsifAccelerometer_IsUpright(PlatformOrientation platOrientation);

	Bool ESIF_CALLCONV EsifAccelerometer_IsFlat(PlatformOrientation platOrientation);

	void ESIF_CALLCONV EsifAccelerometer_CalculatePitchAndRoll(
		AccelerometerDataPtr accelerometerDataPtr,
		float *pitchDegrees,
		float *rollDegrees);

	void ESIF_CALLCONV EsifAccelerometer_GetOrientations(
		InclinometerMinMaxConfig inclinMinMaxConfig,
		AccelerometerDataPtr accelerometerDataPtr,
		DisplayOrientation curDispOrientation,
		PlatformOrientation curPlatOrientation,
		PlatformOrientationPtr platOrientationPtr,
		DisplayOrientationPtr dispOrientationPtr);

	eEsifError ESIF_CALLCONV EsifAccelerometer_ReadSensorAngleValueFromDV(
		EsifDataPtr nameSpace,
		EsifDataPtr key,
		float *dvValue);

	void ESIF_CALLCONV EsifAccelerometer_validateAngleRanges(InclinometerMinMaxConfigPtr inclinMinMaxConfigPtr);

	void ESIF_CALLCONV EsifAccelerometer_GetAngleValues(InclinometerMinMaxConfigPtr inclinMinMaxConfigPtr);

	void ESIF_CALLCONV EsifAccelerometer_GetPlatformType(
		AccelerometerDataPtr accelBasePtr,
		AccelerometerDataPtr accelLidPtr,
		PlatformTypePtr platTypePtr);

#ifdef __cplusplus
}
#endif

#endif

