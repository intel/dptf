/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "esif_uf_sensors.h"
#include "esif.h"
#include "esif_uf_trace.h"
#include "esif_uf_cfgmgr.h"
#include "math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define EPSILON 0.000001

PlatformOrientation ESIF_CALLCONV EsifAccelerometer_GetPlatOrientationFromPitch(
	float pitchDegrees,
	InclinometerMinMaxConfig inclinMinMaxConfig,
	PlatformOrientation curPlatOrientation)
{
	PlatformOrientation platOrientation = curPlatOrientation;

	if (pitchDegrees >= MIN_PITCH_DEGREES && pitchDegrees <= MAX_PITCH_DEGREES)
	{
		if ((pitchDegrees <= inclinMinMaxConfig.xFlatUpMax) && (pitchDegrees >= inclinMinMaxConfig.xFlatUpMin)) {
			platOrientation = ORIENTATION_PLAT_FLAT_UP;
		}
		else if ((pitchDegrees <= inclinMinMaxConfig.xFlatDownMax) || (pitchDegrees >= inclinMinMaxConfig.xFlatDownMin)) {
			platOrientation = ORIENTATION_PLAT_FLAT_DOWN;
		}
		else if ((pitchDegrees < inclinMinMaxConfig.xUprightMax) && (pitchDegrees > inclinMinMaxConfig.xUprightMin)) {
			platOrientation = ORIENTATION_PLAT_UPRIGHT;
		}
		else if ((pitchDegrees < inclinMinMaxConfig.xUprightInvMax) && (pitchDegrees > inclinMinMaxConfig.xUprightInvMin)) {
			platOrientation = ORIENTATION_PLAT_UPRIGHT_INVERTED;
		}
	}

	return platOrientation;
}

PlatformOrientation ESIF_CALLCONV EsifAccelerometer_GetPlatOrientationFromRoll(
	float rollDegrees,
	InclinometerMinMaxConfig inclinMinMaxConfig,
	PlatformOrientation xPlatOrientation,
	PlatformOrientation curPlatOrientation)
{
	PlatformOrientation platOrientation = curPlatOrientation;

	if (rollDegrees >= MIN_ROLL_DEGREES && rollDegrees <= MAX_ROLL_DEGREES)
	{
		float absRollVal = rollDegrees >= 0 ? rollDegrees : -(rollDegrees);

		if (absRollVal <= inclinMinMaxConfig.yFlatMax) {
			//
			// For Y-axis the angles range from 0 to +-90. So we cannot
			// distinguish between flat up/down; this is just an
			// optimization.
			//
			if (EsifAccelerometer_IsFlat(xPlatOrientation)) {
				platOrientation = xPlatOrientation;
			}
			else {
				platOrientation = ORIENTATION_PLAT_FLAT_UP;
			}
		}
		else if (rollDegrees < 0) {
			platOrientation = ORIENTATION_PLAT_UPRIGHT;
		}
		else {
			platOrientation = ORIENTATION_PLAT_UPRIGHT_INVERTED;
		}
	}

	return platOrientation;
}

DisplayOrientation ESIF_CALLCONV EsifAccelerometer_GetDisplayOrientation(
	float pitchDegrees,
	float rollDegrees,
	InclinometerMinMaxConfig inclinMinMaxConfig,
	DisplayOrientation curDispOrientation)
{
	DisplayOrientation dispOrientation = curDispOrientation;

	if (pitchDegrees >= MIN_PITCH_DEGREES && pitchDegrees <= MAX_PITCH_DEGREES)
	{
		if ((pitchDegrees < inclinMinMaxConfig.xUprightMax) && (pitchDegrees > inclinMinMaxConfig.xUprightMin)) {
			dispOrientation = ORIENTATION_DISP_LANDSCAPE;
		}
		else if ((pitchDegrees < inclinMinMaxConfig.xUprightInvMax) && (pitchDegrees > inclinMinMaxConfig.xUprightInvMin)) {
			dispOrientation = ORIENTATION_DISP_LANDSCAPE_INVERTED;
		}
		else if (rollDegrees >= MIN_ROLL_DEGREES && rollDegrees <= MAX_ROLL_DEGREES)
		{
			float absRollVal = rollDegrees >= 0 ? rollDegrees : -(rollDegrees);

			if (absRollVal >= inclinMinMaxConfig.yFlatMax) {
				if (rollDegrees < 0) {
					dispOrientation = ORIENTATION_DISP_PORTRAIT_INVERTED;
				}
				else {
					dispOrientation = ORIENTATION_DISP_PORTRAIT;
				}
			}
		}
	}

	return dispOrientation;
}

Bool ESIF_CALLCONV EsifAccelerometer_IsUpright(PlatformOrientation platOrientation)
{
	return ((platOrientation == ORIENTATION_PLAT_UPRIGHT) ||
		(platOrientation == ORIENTATION_PLAT_UPRIGHT_INVERTED));
}

Bool ESIF_CALLCONV EsifAccelerometer_IsFlat(PlatformOrientation platOrientation)
{
	return ((platOrientation == ORIENTATION_PLAT_FLAT_UP) ||
		(platOrientation == ORIENTATION_PLAT_FLAT_DOWN));
}

void ESIF_CALLCONV EsifAccelerometer_CalculatePitchAndRoll(AccelerometerDataPtr accelerometerDataPtr, float *pitchDegrees, float *rollDegrees)
{
	if (NULL == accelerometerDataPtr) {
		ESIF_TRACE_ERROR("accelerometerDataPtr == NULL\n");
		return;
	}

	float magnitude = (float)sqrt(powf(accelerometerDataPtr->xVal, 2.0f) + powf(accelerometerDataPtr->yVal, 2.0f) + powf(accelerometerDataPtr->zVal, 2.0f));
	// Check rotation along the X-axis
	*pitchDegrees = -1.0f * asinf(accelerometerDataPtr->yVal / magnitude) * MAX_PITCH_DEGREES / M_PI;

	ESIF_TRACE_DEBUG("Pitch angle from accelerometer = %f\n", *pitchDegrees);

	// Check rotation along the Y-axis
	*rollDegrees = asinf(accelerometerDataPtr->xVal / magnitude) * MAX_PITCH_DEGREES / M_PI;
	if (accelerometerDataPtr->zVal > 0)
	{
		*rollDegrees = *rollDegrees * -1.0f;
	}
	ESIF_TRACE_DEBUG("Roll angle from accelerometer = %f\n", *rollDegrees);
}

void ESIF_CALLCONV EsifAccelerometer_GetOrientations(
	InclinometerMinMaxConfig inclinMinMaxConfig,
	AccelerometerDataPtr accelerometerDataPtr,
	DisplayOrientation curDispOrientation,
	PlatformOrientation curPlatOrientation,
	PlatformOrientationPtr platOrientationPtr,
	DisplayOrientationPtr dispOrientationPtr)
{
	if (NULL == accelerometerDataPtr) {
		ESIF_TRACE_ERROR("accelerometerDataPtr == NULL\n");
		return;
	}

	float pitchDegrees = INVALID_DEGREES;
	float rollDegrees = INVALID_DEGREES;
	EsifAccelerometer_CalculatePitchAndRoll(accelerometerDataPtr, &pitchDegrees, &rollDegrees);

	// Determine Platform Orientation
	PlatformOrientation xPlatOrientation = EsifAccelerometer_GetPlatOrientationFromPitch(pitchDegrees, inclinMinMaxConfig, curPlatOrientation);

	if (accelerometerDataPtr->zVal > 0 && xPlatOrientation == ORIENTATION_PLAT_FLAT_UP)
	{
		xPlatOrientation = ORIENTATION_PLAT_FLAT_DOWN;
	}

	PlatformOrientation yPlatOrientation = EsifAccelerometer_GetPlatOrientationFromRoll(rollDegrees, inclinMinMaxConfig, xPlatOrientation, curPlatOrientation);

	if (EsifAccelerometer_IsUpright(xPlatOrientation)) {
		*platOrientationPtr = xPlatOrientation;
	}
	else if (EsifAccelerometer_IsUpright(yPlatOrientation)) {
		*platOrientationPtr = yPlatOrientation;
	}
	else {
		*platOrientationPtr = xPlatOrientation; // default to X
	}

	// Determine Display Orientation
	if (EsifAccelerometer_IsUpright(*platOrientationPtr))
	{
		*dispOrientationPtr = EsifAccelerometer_GetDisplayOrientation(pitchDegrees, rollDegrees, inclinMinMaxConfig, curDispOrientation);
	}

	ESIF_TRACE_DEBUG("Platform Orientation = [%s]\n", GetPlatOrientationStr(*platOrientationPtr));
	ESIF_TRACE_DEBUG("Display Orientation = [%s]\n", GetDispOrientationStr(*dispOrientationPtr));
}

eEsifError ESIF_CALLCONV EsifAccelerometer_ReadSensorAngleValueFromDV(EsifDataPtr nameSpace, EsifDataPtr key, float *dvValue)
{
	Int32 intVal = 0;
	EsifData value = { ESIF_DATA_INT32, &intVal, sizeof(intVal), sizeof(intVal) };
	eEsifError rc = ESIF_OK;

	rc = EsifConfigGet(nameSpace, key, &value);
	if (rc == ESIF_OK) {
		if (intVal < MIN_PITCH_DEGREES || intVal > MAX_PITCH_DEGREES)
		{
			ESIF_TRACE_WARN("Sensor config found in DV but out of range: namespace= %s, key = %s, value = %d\n",
				nameSpace->buf_ptr, key->buf_ptr, intVal);
		}
		else
		{
			*dvValue = (float)intVal;
			ESIF_TRACE_DEBUG("Sensor config found in DV: namespace= %s, key = %s, value = %f\n",
				nameSpace->buf_ptr, key->buf_ptr, &dvValue);
		}
	}

	return rc;
}

void ESIF_CALLCONV EsifAccelerometer_validateAngleRanges(InclinometerMinMaxConfigPtr inclinMinMaxConfigPtr)
{
	if (inclinMinMaxConfigPtr == NULL)
	{
		goto exit;
	}

	if (inclinMinMaxConfigPtr->xFlatUpMin != inclinMinMaxConfigPtr->xUprightInvMax)
	{
		ESIF_TRACE_DEBUG("Platform Orientation flat up min angle is not aligned with upright inverted max angle\n");
		inclinMinMaxConfigPtr->xFlatUpMin = inclinMinMaxConfigPtr->xUprightInvMin;
	}

	if (inclinMinMaxConfigPtr->xFlatUpMax != inclinMinMaxConfigPtr->xUprightMin)
	{
		ESIF_TRACE_DEBUG("Platform Orientation flat up max angle is not aligned with upright min angle\n");
		inclinMinMaxConfigPtr->xFlatUpMax = inclinMinMaxConfigPtr->xUprightMin;
	}

	if (inclinMinMaxConfigPtr->xFlatDownMax != inclinMinMaxConfigPtr->xUprightInvMin)
	{
		ESIF_TRACE_DEBUG("Platform Orientation flat down max angle is not aligned with upright inverted min angle\n");
		inclinMinMaxConfigPtr->xFlatDownMax = inclinMinMaxConfigPtr->xUprightInvMin;
	}

	if (inclinMinMaxConfigPtr->xFlatDownMin != inclinMinMaxConfigPtr->xUprightMax)
	{
		ESIF_TRACE_DEBUG("Platform Orientation flat down min angle is not aligned with upright max angle\n");
		inclinMinMaxConfigPtr->xFlatDownMin = inclinMinMaxConfigPtr->xUprightMax;
	}

exit:
	return;
}

void ESIF_CALLCONV EsifAccelerometer_GetAngleValues(InclinometerMinMaxConfigPtr inclinMinMaxConfigPtr)
{
	EsifData nameSpace = { ESIF_DATA_STRING };
	EsifData key = { ESIF_DATA_STRING };

	if (inclinMinMaxConfigPtr == NULL)
	{
		goto exit;
	}

	ESIF_DATA_STRING_ASSIGN(nameSpace, DV_NAMESPACE_SENSOR, sizeof(DV_NAMESPACE_SENSOR));

	// INCLIN_X_ORIENT_FLAT_UP_MIN
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_FLAT_UP_MIN, sizeof(DV_KEY_INCLIN_X_ORIENT_FLAT_UP_MIN));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xFlatUpMin));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_FLAT_UP_MIN = %f\n", inclinMinMaxConfigPtr->xFlatUpMin);

	// INCLIN_X_ORIENT_FLAT_UP_MAX
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_FLAT_UP_MAX, sizeof(DV_KEY_INCLIN_X_ORIENT_FLAT_UP_MAX));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xFlatUpMax));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_FLAT_UP_MAX = %f\n", inclinMinMaxConfigPtr->xFlatUpMax);

	// INCLIN_X_ORIENT_FLAT_DOWN_MAX
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_FLAT_DOWN_MAX, sizeof(DV_KEY_INCLIN_X_ORIENT_FLAT_DOWN_MAX));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xFlatDownMax));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_FLAT_DOWN_MAX = %f\n", inclinMinMaxConfigPtr->xFlatDownMax);

	// INCLIN_X_ORIENT_FLAT_DOWN_MIN
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_FLAT_DOWN_MIN, sizeof(DV_KEY_INCLIN_X_ORIENT_FLAT_DOWN_MIN));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xFlatDownMin));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_FLAT_DOWN_MIN = %f\n", inclinMinMaxConfigPtr->xFlatDownMin);

	// INCLIN_X_ORIENT_UPRIGHT_MIN
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_UPRIGHT_MIN, sizeof(DV_KEY_INCLIN_X_ORIENT_UPRIGHT_MIN));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xUprightMin));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_UPRIGHT_MIN = %f\n", inclinMinMaxConfigPtr->xUprightMin);

	// INCLIN_X_ORIENT_UPRIGHT_MAX
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_UPRIGHT_MAX, sizeof(DV_KEY_INCLIN_X_ORIENT_UPRIGHT_MAX));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xUprightMax));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_UPRIGHT_MAX = %f\n", inclinMinMaxConfigPtr->xUprightMax);

	// INCLIN_X_ORIENT_UPRIGHT_INVERTED_MIN
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_UPRIGHT_INVERTED_MIN, sizeof(DV_KEY_INCLIN_X_ORIENT_UPRIGHT_INVERTED_MIN));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xUprightInvMin));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_UPRIGHT_INVERTED_MIN = %f\n", inclinMinMaxConfigPtr->xUprightInvMin);

	// INCLIN_X_ORIENT_UPRIGHT_INVERTED_MAX
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_X_ORIENT_UPRIGHT_INVERTED_MAX, sizeof(DV_KEY_INCLIN_X_ORIENT_UPRIGHT_INVERTED_MAX));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->xUprightInvMax));
	ESIF_TRACE_DEBUG("INCLIN_X_ORIENT_UPRIGHT_INVERTED_MAX = %f\n", inclinMinMaxConfigPtr->xUprightInvMax);

	// INCLIN_Y_ORIENT_FLAT_MAX
	ESIF_DATA_STRING_ASSIGN(key, DV_KEY_INCLIN_Y_ORIENT_FLAT_MAX, sizeof(DV_KEY_INCLIN_Y_ORIENT_FLAT_MAX));
	EsifAccelerometer_ReadSensorAngleValueFromDV(&nameSpace, &key, &(inclinMinMaxConfigPtr->yFlatMax));
	ESIF_TRACE_DEBUG("INCLIN_Y_ORIENT_FLAT_MAX = %f\n", inclinMinMaxConfigPtr->yFlatMax);


	EsifAccelerometer_validateAngleRanges(inclinMinMaxConfigPtr);

exit:
	return;
}

void ESIF_CALLCONV EsifAccelerometer_GetPlatformType(AccelerometerDataPtr accelBasePtr,
		AccelerometerDataPtr accelLidPtr,
		PlatformTypePtr platTypePtr)
{
	float dotProduct = 0;
	float magnitudeBase = 0, magnitudeLid = 0, magnitudeProduct = 0;
	float cosineVal = 0;

	if ((accelBasePtr == NULL) || (accelLidPtr == NULL) || (platTypePtr == NULL)) goto exit;

	dotProduct = accelBasePtr->xVal * accelLidPtr->xVal + accelBasePtr->yVal * accelLidPtr->yVal + accelBasePtr->zVal * accelLidPtr->zVal;
	magnitudeBase = accelBasePtr->xVal * accelBasePtr->xVal + accelBasePtr->yVal * accelBasePtr->yVal + accelBasePtr->zVal * accelBasePtr->zVal;
	magnitudeBase = (float) sqrt(magnitudeBase);
	magnitudeLid = accelLidPtr->xVal * accelLidPtr->xVal + accelLidPtr->yVal * accelLidPtr->yVal + accelLidPtr->zVal * accelLidPtr->zVal;
	magnitudeLid = (float) sqrt(magnitudeLid);

	// Avoid possible divde by zero exception
	magnitudeProduct = (float) esif_ccb_max(magnitudeBase * magnitudeLid, EPSILON);
	cosineVal = dotProduct / magnitudeProduct;

	if (cosineVal < PLATFORM_TYPE_TABLET_ANGLE) {
		*platTypePtr = PLATFORM_TYPE_TABLET;
	} else if (cosineVal < PLATFORM_TYPE_TENT_ANGLE) {
		*platTypePtr = PLATFORM_TYPE_TENT;
	} else {
		*platTypePtr = PLATFORM_TYPE_CLAMSHELL;
	}

exit:
	return;
}
