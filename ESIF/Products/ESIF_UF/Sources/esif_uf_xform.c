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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_PRIMITIVE

#include "esif_uf_xform.h"
#include "esif_participant.h"
#include "esif_uf_trace.h"


#define ESIF_TRACE_DYN_TEMP(format, ...) \
	ESIF_TRACE_DYN(ESIF_TRACEMODULE_DEFAULT, \
		ESIF_TRACELEVEL_DEBUG, \
		format, \
		##__VA_ARGS__ \
		)
#define ESIF_TRACE_DYN_POWER(format, ...) \
	ESIF_TRACE_DYN(ESIF_TRACEMODULE_DEFAULT, \
		ESIF_TRACELEVEL_DEBUG, \
		format, \
		##__VA_ARGS__ \
		)
#define ESIF_TRACE_DYN_TIME(format, ...) \
	ESIF_TRACE_DYN(ESIF_TRACEMODULE_DEFAULT, \
		ESIF_TRACELEVEL_DEBUG, \
		format, \
		##__VA_ARGS__ \
		)
#define ESIF_TRACE_DYN_PERCENT(format, ...) \
	ESIF_TRACE_DYN(ESIF_TRACEMODULE_DEFAULT, \
		ESIF_TRACELEVEL_DEBUG, \
		format, \
		##__VA_ARGS__ \
		)

/*
** ===========================================================================
** PUBLIC
** ===========================================================================
*/

/* Temperature Transform */
enum esif_rc EsifUfXformTemp(
	const enum esif_temperature_type tempType,
	u32 algoType,
	const enum esif_primitive_opcode opcode,
	const EsifUpPtr upPtr,
	esif_temp_t *tempPtr
	)
{
	enum esif_rc rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	enum esif_temperature_type tempInType  = tempType;
	enum esif_temperature_type tempOutType = tempType;
	esif_temp_t tempIn = {0};
	esif_temp_t tempOut;

	if ((tempPtr == NULL) || (upPtr == NULL)) {
		ESIF_TRACE_ERROR("The temperature or dsp pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	dspPtr = EsifUp_GetDsp(upPtr);
	if (NULL == dspPtr) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	tempIn  = *tempPtr;
	tempOut = *tempPtr;

	switch (algoType) {
	case ESIF_ALGORITHM_TYPE_TEMP_C:
		/*
		 * Convert temp before/after action
		 *   For Get: From reading returned in Celsius
		 *     To normalized temp
		 *   For Set: From user request buffer in normalized temp
		 *     To celsius temp
		 */
		ESIF_TRACE_DYN_TEMP("Using algorithm Celsius (%s), for temp\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType = ESIF_TEMP_C;
			tempOutType = tempType;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from celsius
		}
		else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType = tempType;
			tempOutType = ESIF_TEMP_C;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized to celsius
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_MILLIC:
		/* 
		 * Convert Temp before/after action
		 *    For Get: From reading returned in millic
		 *      To normalized temp back to user response buffer
		 *    For Set: From user request buffer in normalized temperature
		 *      To millic
		 */
		ESIF_TRACE_DYN_TEMP("Using algorithm MilliC (%s) for temp\n",
			esif_algorithm_type_str(algoType));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType  = ESIF_TEMP_MILLIC;
			tempOutType = tempType;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from millic
		} else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType  = tempType;
			tempOutType = ESIF_TEMP_MILLIC;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized to millic
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_DECIC:
		/*
		 * Convert Temp before/after action
		 *    For Get: From reading returned in decic
		 *             To normalized temp back to user response buffer
		 *    For Set: From user request buffer in normalized temp
		 *             To decic
		 */
		ESIF_TRACE_DYN_TEMP("Using algorithm DeciC (%s), for temp\n",
			esif_algorithm_type_str(algoType));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType  = ESIF_TEMP_DECIC;
			tempOutType = tempType;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from decic
		} else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType  = tempType;
			tempOutType = ESIF_TEMP_DECIC;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized to decic
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_DECIK:
		/*
		 * Convert Temp before/after ACPI action
		 *    For Get: From reading returned by APCI device driver in Kelvin
		 *             To   normalized temp (C) back to user response buffer
		 *    For Set: From user request buffer in Kelvin or Celsius
		 *             To   Kelvin passing to ACPI device driver to set
		 */
		ESIF_TRACE_DYN_TEMP("Using algorithm DeciK (%s), for ACPI temp\n",
			esif_algorithm_type_str(algoType));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType  = ESIF_TEMP_DECIK;
			tempOutType = tempType;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from Kelvin
		} else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType  = tempType;
			tempOutType = ESIF_TEMP_DECIK;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized to Kelvin
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_NONE:
		ESIF_TRACE_DYN_TEMP("Using algorithm none (%s)\n",
			esif_algorithm_type_str(algoType));
		break;

	default:
		ESIF_TRACE_DYN_POWER("Unknown algorithm (%s) to xform temp\n",
			esif_algorithm_type_str(algoType));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_TEMP("IN  temp %u %s(%d)\n",
		tempIn,
		esif_temperature_type_desc(tempInType),
		tempInType);

	ESIF_TRACE_DYN_TEMP("OUT temp %u %s(%d)\n",
		tempOut,
		esif_temperature_type_desc(tempOutType),
		tempOutType);

	*tempPtr = tempOut;
exit:
	return rc;
}


/* Power Transform */
enum esif_rc EsifUfXformPower(
	const enum esif_power_unit_type powerType,
	u32 algoType,
	const enum esif_primitive_opcode opcode,
	const EsifUpPtr upPtr,
	esif_power_t *powerPtr
	)
{
	enum esif_power_unit_type powerInType  = powerType;
	enum esif_power_unit_type powerOutType = powerType;
	enum esif_rc rc = ESIF_OK;
	esif_power_t powerIn;
	esif_power_t powerOut;

	if ((NULL == powerPtr) || (NULL == upPtr)) {
		ESIF_TRACE_ERROR("The power or dsp pointer is NULL\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}
	powerIn  = *powerPtr;
	powerOut = *powerPtr;

	switch (algoType) {
	case ESIF_ALGORITHM_TYPE_POWER_DECIW:
		ESIF_TRACE_DYN_POWER("Using algorithm DeciW (%s), for ACPI power\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			/* Tenths Of A Watt To Milli Watts */
			powerInType  = ESIF_POWER_DECIW;
			powerOutType = powerType;
			// Normalized from DeciW
			esif_convert_power(powerInType, powerOutType, &powerOut);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			powerInType  = powerType;
			powerOutType = ESIF_POWER_DECIW;
			// Normalized to DeciW
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;
	case ESIF_ALGORITHM_TYPE_POWER_MICROW:
		ESIF_TRACE_DYN_POWER("Using algorithm MiroW (%s), for Code and Konst power\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			powerInType = ESIF_POWER_MICROW;
			powerOutType = powerType;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		else {
			/* Micro Watts To Tenths Of A Watt */
			powerInType = powerType;
			powerOutType = ESIF_POWER_MICROW;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_MILLIW:
		ESIF_TRACE_DYN_POWER("Using algorithm MillW (%s), for Code and Konst power\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			powerInType  = ESIF_POWER_MILLIW;
			powerOutType = powerType;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			powerInType  = powerType;
			powerOutType = ESIF_POWER_MILLIW;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_NONE:
		/* No algorithm specified, do not perform any xform */
		ESIF_TRACE_DYN_POWER("Using algorithm NONE (%s), no xform performed\n",
			esif_algorithm_type_str(algoType));
		break;

	default:
		ESIF_TRACE_DYN_POWER("Unknown algorithm (%s) to xform power\n",
			esif_algorithm_type_str(algoType));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_POWER("IN power %u %s(%d)\n",
		powerIn,
		esif_power_unit_desc(powerInType),
		powerInType);

	ESIF_TRACE_DYN_POWER("OUT power %u %s(%d)\n",
		powerOut,
		esif_power_unit_desc(powerOutType),
		powerOutType);

	*powerPtr = powerOut;
	return rc;
}


/* Time Transform */
enum esif_rc EsifUfXformTime(
	const enum esif_time_type timeType,
	u32 algoType,
	const enum esif_primitive_opcode opcode,
	const EsifUpPtr upPtr,
	esif_time_t *time_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	enum esif_time_type timeInType = timeType;
	enum esif_time_type timeOutType = timeType;
	esif_time_t timeIn;
	esif_time_t timeOut;

	UNREFERENCED_PARAMETER(upPtr);

	if (time_ptr == NULL) {
		ESIF_TRACE_ERROR("The time or DSP pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit; 
	}

	timeIn = *time_ptr;
	timeOut = *time_ptr;

	switch (algoType) {
	case ESIF_ALGORITHM_TYPE_TIME_DECIS:
		/*
		 * Convert time before/after action
		 *    For Get: From reading returned by deciseconds
		 *             To   normalized time (s) back to user response buffer
		 *    For Set: From user request buffer in seconds
		 *             To   Deciseconds passing to action
		 */
		ESIF_TRACE_DYN_TIME("Using algorithm deciseconds (%s) for time\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			timeInType = ESIF_TIME_DECIS;
			timeOutType = timeType;
			esif_convert_time(timeInType, timeOutType, &timeOut);	// Normalized from deciseconds
		}
		else {/* ESIF_PRIMITIVE_OP_SET */
			timeInType = timeType;
			timeOutType = ESIF_TIME_DECIS;
			esif_convert_time(timeInType, timeOutType, &timeOut);	// Normalized to deciseconds
		}
		break;

	case ESIF_ALGORITHM_TYPE_TIME_MILLIS:
		/*
		 * Convert time before/after action
		 *    For Get: From reading returned by milliseconds
		 *             To   normalized time (s) back to user response buffer
		 *    For Set: From user request buffer in seconds
		 *             To   milliseconds passing to action
		 */
		ESIF_TRACE_DYN_TEMP("Using algorithm milliseconds (%s) for time\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			timeInType = ESIF_TIME_MILLIS;
			timeOutType = timeType;
			esif_convert_time(timeInType, timeOutType, &timeOut);	// Normalized from ms
		}
		else {/* ESIF_PRIMITIVE_OP_SET */
			timeInType = timeType;
			timeOutType = ESIF_TIME_MILLIS;
			esif_convert_time(timeInType, timeOutType, &timeOut);	// Normalized to ms
		}
		break;

	case ESIF_ALGORITHM_TYPE_TIME_NONE:
		/* No algorithm specified, do not perform any xform */
		ESIF_TRACE_DYN_TIME("Using algorithm NONE (%s), no xform performed\n",
			esif_algorithm_type_str(algoType));
		break;

	default:
		ESIF_TRACE_DYN_TIME("Unknown algorithm (%s) to xform time\n",
			esif_algorithm_type_str(algoType));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_TIME("IN  time %u %s(%d)\n",
		timeIn,
		esif_time_type_desc(timeInType),
		timeInType);

	ESIF_TRACE_DYN_TIME("OUT time %u %s(%d)\n",
		timeOut,
		esif_time_type_desc(timeOutType),
		timeOutType);

	*time_ptr = timeOut;
exit:
	return rc;
}


enum esif_rc EsifUfXformPercent(
	const enum esif_percent_type pctType,
	u32 algoType,
	const enum esif_primitive_opcode opcode,
	const EsifUpPtr upPtr,
	u32 *valuePtr
	)
{
	enum esif_rc rc = ESIF_OK;
	u32 value = 0;
	enum esif_percent_type percentInType = pctType;
	enum esif_percent_type percentOutType = pctType;

	UNREFERENCED_PARAMETER(upPtr);

	if (valuePtr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	value = *valuePtr;

	switch (algoType) {
	case ESIF_ALGORITHM_TYPE_PERCENT_WHOLE:
		ESIF_TRACE_DYN_PERCENT(
			"Using algorithm percent (%s) for percentage\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			percentInType = ESIF_PERCENT;
			percentOutType = NORMALIZE_PERCENT_TYPE;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);
		} else {
			percentInType = NORMALIZE_PERCENT_TYPE;
			percentOutType = ESIF_PERCENT;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);
		}
		break;

	case ESIF_ALGORITHM_TYPE_PERCENT_DECI:
		ESIF_TRACE_DYN_PERCENT(
			"Using algorithm deci-percent (%s) for percentage\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			percentInType = ESIF_PERCENT_DECI;
			percentOutType = NORMALIZE_PERCENT_TYPE;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);
		} else {
			percentInType = NORMALIZE_PERCENT_TYPE;
			percentOutType = ESIF_PERCENT_DECI;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);
		}
		break;

	case ESIF_ALGORITHM_TYPE_PERCENT_CENTI:
		ESIF_TRACE_DYN_PERCENT(
			"Using algorithm centi-percent (%s) for percentage\n",
			esif_algorithm_type_str(algoType));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			percentInType = ESIF_PERCENT_CENTI;
			percentOutType = NORMALIZE_PERCENT_TYPE;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);
		} else {
			percentInType = NORMALIZE_PERCENT_TYPE;
			percentOutType = ESIF_PERCENT_CENTI;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);
		}
		break;

	case ESIF_ALGORITHM_TYPE_PERCENT_NONE:
		ESIF_TRACE_DYN_PERCENT(
			"Using algorithm NONE (%s), no xform performed\n",
			esif_algorithm_type_str(algoType));
		break;

	default:
		ESIF_TRACE_DYN_PERCENT(
			"Unknown algorithm (%s) to xform percentage\n",
			esif_algorithm_type_str(algoType));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
		goto exit;
		break;
	}

	ESIF_TRACE_DYN_TEMP("IN percent %u %s(%d)\n",
		*valuePtr,
		esif_percent_type_desc(percentInType),
		percentInType);

	ESIF_TRACE_DYN_TEMP("OUT percent %u %s(%d)\n",
		value,
		esif_percent_type_desc(percentOutType),
		percentOutType);

	*valuePtr = value;
exit:
	return rc;
}

eEsifError EsifUfExecuteTransform(
	const EsifDataPtr transformDataPtr,
	const EsifUpPtr upPtr,
	const enum esif_action_type actionType,
	const enum esif_primitive_opcode opcode
	)
{
	eEsifError rc = ESIF_OK;
	enum esif_data_type  dataType = 0;
	EsifDspPtr dspPtr = NULL;
	struct esif_fpc_algorithm *algoPtr = NULL;

	if ((NULL == transformDataPtr) || (NULL == upPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	dataType = transformDataPtr->type;

	/* Exit if not a type requiring transform */
	if ((dataType != ESIF_DATA_TEMPERATURE) &&
	    (dataType != ESIF_DATA_POWER) &&
	    (dataType != ESIF_DATA_TIME) &&
	    (dataType != ESIF_DATA_PERCENT)) {
			goto exit;
	}

	/* Get the algorithm pointer */
	dspPtr = EsifUp_GetDsp(upPtr);
	if (NULL == dspPtr) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	algoPtr = dspPtr->get_algorithm(dspPtr, actionType);
	if (algoPtr == NULL) {
		rc = ESIF_E_NEED_ALGORITHM;
		goto exit;
	}

	switch (dataType) {
	case ESIF_DATA_TEMPERATURE:
		ESIF_TRACE_DEBUG("esif xform temp\n");
		rc = EsifUfXformTemp(NORMALIZE_TEMP_TYPE,
			algoPtr->temp_xform,
			opcode,
			upPtr,
			(esif_temp_t *) transformDataPtr->buf_ptr);
		break;

	case ESIF_DATA_POWER:
		ESIF_TRACE_DEBUG("esif xform power\n");
		rc = EsifUfXformPower(NORMALIZE_POWER_UNIT_TYPE,
			algoPtr->power_xform,
			opcode,
			upPtr,
			(esif_power_t *) transformDataPtr->buf_ptr);
		break;

	case ESIF_DATA_TIME:
		ESIF_TRACE_DEBUG("esif xform time\n");
		rc = EsifUfXformTime(NORMALIZE_TIME_TYPE,
			algoPtr->time_xform,
			opcode,
			upPtr,
			(esif_time_t *) transformDataPtr->buf_ptr);
		break;

	case ESIF_DATA_PERCENT:
		ESIF_TRACE_DEBUG("esif xform percent\n");
		rc = EsifUfXformPercent(NORMALIZE_PERCENT_TYPE,
			algoPtr->percent_xform,
			opcode,
			upPtr,
			(u32 *) transformDataPtr->buf_ptr);
		break;

	default:
		rc = ESIF_E_UNSPECIFIED; /* Impossible to reach; but there for SCA */
		break;
	}
exit:
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

