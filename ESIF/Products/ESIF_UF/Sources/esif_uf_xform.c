/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif


/*
** ===========================================================================
** PUBLIC
** ===========================================================================
*/

/* Temperature Transform */
enum esif_rc EsifUfXformTemp(
	const enum esif_temperature_type type,
	esif_temp_t *tempPtr,
	const enum esif_action_type action,
	const EsifDspPtr dspPtr,
	const enum esif_primitive_opcode opcode
	)
{
	enum esif_rc rc = ESIF_OK;
	enum esif_temperature_type tempInType  = type;
	enum esif_temperature_type tempOutType = type;
	struct esif_fpc_algorithm *algoPtr = NULL;
	esif_temp_t tempIn;
	esif_temp_t tempOut;

	UNREFERENCED_PARAMETER(tempIn);

	if ((tempPtr == NULL) || (dspPtr == NULL)) {
		ESIF_TRACE_ERROR("The temperature or dsp pointer is NULL\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}
	tempIn  = *tempPtr;
	tempOut = *tempPtr;

	algoPtr = dspPtr->get_algorithm(dspPtr, action);
	if (algoPtr == NULL) {
		ESIF_TRACE_ERROR("The algorithm is not available for the action [type=%d] in dsp\n",(u32)action);
		return ESIF_E_NEED_ALGORITHM;
	}

	switch (algoPtr->temp_xform) {
	case ESIF_ALGORITHM_TYPE_TEMP_C:
		/*
		 * Convert temp before/after action
		 *   For Get: From reading returned in Celcius
		 *     To normalized temp
		 *   For Set: From user request buffer in normalized temp
		 *     To celcius temp
		 */
		ESIF_TRACE_DYN_TEMP("Using algorithm Celcius (%s), for temp\n",
			esif_algorithm_type_str(algo_ptr->temp_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType = ESIF_TEMP_C;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from celcius
		}
		else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType = type;
			tempOutType = ESIF_TEMP_C;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized to celcius
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
			esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType  = ESIF_TEMP_MILLIC;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from millic
		} else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType  = type;
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
			esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType  = ESIF_TEMP_DECIC;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from decic
		} else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType  = type;
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
			esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType  = ESIF_TEMP_DECIK;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized from Kelvin
		} else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType  = type;
			tempOutType = ESIF_TEMP_DECIK;
			esif_convert_temp(tempInType, tempOutType, &tempOut);	// Normalized to Kelvin
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_TJMAX_CORE:
	{
		u32 tjmax = dspPtr->get_temp_tc1(dspPtr, action);
		ESIF_TRACE_DYN_TEMP("Using algorithm Tjmax %d %s, for MSR temp\n",
			tjmax, esif_algorithm_type_str(algo_ptr->temp_xform));

		/* Tjmax must be provided by DSP */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempOut = (tempOut > tjmax) ? 0 : tjmax - tempOut;
			tempInType  = ESIF_TEMP_C;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
		} else {
			tempInType  = type;
			tempOutType = ESIF_TEMP_C;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
			tempOut = (tempOut > tjmax) ? 0 : tjmax - tempOut;
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_PCH_CORE:
	{
		ESIF_TRACE_DYN_TEMP("Using algorithm %s, for PCH MMIO temp\n",
			esif_algorithm_type_str(algo_ptr->temp_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempOut      = (tempOut / 2) - 50;
			tempInType  = ESIF_TEMP_C;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
		} else {
			tempInType  = type;
			tempOutType = ESIF_TEMP_C;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
			tempOut = (tempOut + 50) * 2;
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_NONE:
		ESIF_TRACE_DYN_TEMP("Using algorithm none (%s), for Code and Konst temp\n",
			esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempInType  = ESIF_TEMP_C;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
		} else {/* ESIF_PRIMITIVE_OP_SET */
			tempInType  = type;
			tempOutType = ESIF_TEMP_C;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_C_DIV2:
	{
		ESIF_TRACE_DYN_TEMP("Using algorithm %s for temp\n",
			esif_algorithm_type_str(algo_ptr->temp_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			tempOut      = tempOut / 2;
			tempInType  = ESIF_TEMP_C;
			tempOutType = type;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
		} else {
			tempInType  = type;
			tempOutType = ESIF_TEMP_C;
			esif_convert_temp(tempInType, tempOutType, &tempOut);
			tempOut = tempOut * 2;
		}
		break;
	}

	default:
		ESIF_TRACE_DYN_POWER("Unknown algorithm (%s) to xform temp\n",
			esif_algorithm_type_str(algo_ptr->temp_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_TEMP("IN  temp %u %s(%d)\n",
		temp_in,
		esif_temperature_type_desc(tempInType),
		tempInType);

	ESIF_TRACE_DYN_TEMP("OUT temp %u %s(%d)\n",
		temp_out,
		esif_temperature_type_desc(tempOutType),
		tempOutType);

	*tempPtr = tempOut;
	return rc;
}


/* Power Transform */
enum esif_rc EsifUfXformPower(
	const enum esif_power_unit_type type,
	esif_power_t *powerPtr,
	const enum esif_action_type action,
	const EsifDspPtr dspPtr,
	const enum esif_primitive_opcode opcode
	)
{
	enum esif_power_unit_type powerInType  = type;
	enum esif_power_unit_type powerOutType = type;
	enum esif_rc rc = ESIF_OK;
	struct esif_fpc_algorithm *algoPtr = NULL;
	esif_power_t powerIn;
	esif_power_t powerOut;

	if ((powerPtr == NULL) || (dspPtr == NULL)) {
		ESIF_TRACE_ERROR("The power or dsp pointer is NULL\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}
	powerIn  = *powerPtr;
	powerOut = *powerPtr;

	algoPtr  = dspPtr->get_algorithm(dspPtr, action);
	if (algoPtr == NULL) {
		ESIF_TRACE_ERROR("The algorithm is not available for the action [type=%d] in dsp\n", (u32) action);
		return ESIF_E_NEED_ALGORITHM;
	}

	switch (algoPtr->power_xform) {
	case ESIF_ALGORITHM_TYPE_POWER_DECIW:
		ESIF_TRACE_DYN_POWER("Using algorithm DeciW (%s), for ACPI power\n",
			esif_algorithm_type_str(algo_ptr->power_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			/* Tenths Of A Watt To Milli Watts */
			powerInType  = ESIF_POWER_DECIW;
			powerOutType = type;
			// Normalized from DeciW
			esif_convert_power(powerInType, powerOutType, &powerOut);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			powerInType  = type;
			powerOutType = ESIF_POWER_DECIW;
			// Normalized to DeciW
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;
	case ESIF_ALGORITHM_TYPE_POWER_MICROW:
		ESIF_TRACE_DYN_POWER("Using algorithm MiroW (%s), for Code and Konst power\n",
			esif_algorithm_type_str(algo_ptr->power_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			powerInType = ESIF_POWER_MICROW;
			powerOutType = type;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		else {
			/* Micro Watts To Tenths Of A Watt */
			powerInType = type;
			powerOutType = ESIF_POWER_MICROW;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_MILLIW:
		ESIF_TRACE_DYN_POWER("Using algorithm MillW (%s), for Code and Konst power\n",
			esif_algorithm_type_str(algo_ptr->power_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			powerInType  = ESIF_POWER_MILLIW;
			powerOutType = type;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			powerInType  = type;
			powerOutType = ESIF_POWER_MILLIW;
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_UNIT_ATOM:
		ESIF_TRACE_DYN_POWER("Using algorithm %s, for hardware power\n",
			esif_algorithm_type_str(algo_ptr->power_xform));

		/* Hardware */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			powerInType  = ESIF_POWER_UNIT_ATOM;
			powerOutType = type;
			// Normalized from hardware
			esif_convert_power(powerInType, powerOutType, &powerOut);
		} else {
			powerInType  = type;
			powerOutType = ESIF_POWER_UNIT_ATOM;
			// Normalized to hardware
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_UNIT_CORE:
		ESIF_TRACE_DYN_POWER("Using algorithm %s, for hardware power\n",
			esif_algorithm_type_str(algo_ptr->power_xform));

		/* Hardware */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			powerInType  = ESIF_POWER_UNIT_CORE;
			powerOutType = type;
			// Normalized from hardware
			esif_convert_power(powerInType, powerOutType, &powerOut);
		} else {
			powerInType  = type;
			powerOutType = ESIF_POWER_UNIT_CORE;
			// Normalized to hardware
			esif_convert_power(powerInType, powerOutType, &powerOut);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_NONE:
		/* No algorithm specified, do not perform any xform */
		ESIF_TRACE_DYN_POWER("Using algorithm NONE (%s), no xform performed\n",
			esif_algorithm_type_str(algo_ptr->power_xform));
		break;

	default:
		ESIF_TRACE_DYN_POWER("Unknown algorithm (%s) to xform power\n",
			esif_algorithm_type_str(algo_ptr->power_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_POWER("IN power %u %s(%d)\n",
		power_in,
		esif_power_unit_desc(powerInType),
		powerInType);

	ESIF_TRACE_DYN_POWER("OUT power %u %s(%d)\n",
		power_out,
		esif_power_unit_desc(powerOutType),
		powerOutType);

	*powerPtr = powerOut;
	return rc;
}


/* Time Transform */
enum esif_rc EsifUfXformTime(
	const enum esif_time_type type,
	esif_time_t *time_ptr,
	const enum esif_action_type action,
	const EsifDspPtr dspPtr,
	const enum esif_primitive_opcode opcode
	)
{
	enum esif_rc rc = ESIF_OK;
	enum esif_time_type timeInType = type;
	enum esif_time_type timeOutType = type;
	struct esif_fpc_algorithm *algoPtr = NULL;
	esif_time_t timeIn;
	esif_time_t timeOut;

	if ((time_ptr == NULL) || (dspPtr == NULL)) {
		ESIF_TRACE_ERROR("The time or DSP pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit; 
	}

	algoPtr = dspPtr->get_algorithm(dspPtr, action);
	if (algoPtr == NULL) {
		ESIF_TRACE_ERROR("The algorithm is not available for the action [type=%d] in dsp\n", (u32) action);
		rc = ESIF_E_NEED_ALGORITHM;
		goto exit;
	}

	timeIn = *time_ptr;
	timeOut = *time_ptr;

	switch (algoPtr->time_xform) {
	case ESIF_ALGORITHM_TYPE_TIME_DECIS:
		/*
		 * Convert time before/after action
		 *    For Get: From reading returned by deciseconds
		 *             To   normalized time (s) back to user response buffer
		 *    For Set: From user request buffer in seconds
		 *             To   Deciseconds passing to action
		 */
		ESIF_TRACE_DYN_TIME("Using algorithm deciseconds (%s) for time\n",
			esif_algorithm_type_str(algo_ptr->temp_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			timeInType = ESIF_TIME_DECIS;
			timeOutType = type;
			esif_convert_time(timeInType, timeOutType, &timeOut);	// Normalized from deciseconds
		}
		else {/* ESIF_PRIMITIVE_OP_SET */
			timeInType = type;
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
			esif_algorithm_type_str(algo_ptr->temp_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			timeInType = ESIF_TIME_MILLIS;
			timeOutType = type;
			esif_convert_time(timeInType, timeOutType, &timeOut);	// Normalized from ms
		}
		else {/* ESIF_PRIMITIVE_OP_SET */
			timeInType = type;
			timeOutType = ESIF_TIME_MILLIS;
			esif_convert_time(timeInType, timeOutType, &timeOut);	// Normalized to ms
		}
		break;

	case ESIF_ALGORITHM_TYPE_TIME_NONE:
		/* No algorithm specified, do not perform any xform */
		ESIF_TRACE_DYN_TIME("Using algorithm NONE (%s), no xform performed\n",
			esif_algorithm_type_str(algo_ptr->time_xform));
		break;

	default:
		ESIF_TRACE_DYN_TIME("Unknown algorithm (%s) to xform time\n",
			esif_algorithm_type_str(algo_ptr->time_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_TIME("IN  time %u %s(%d)\n",
		time_in,
		esif_time_type_desc(timeInType),
		timeInType);

	ESIF_TRACE_DYN_TIME("OUT time %u %s(%d)\n",
		time_out,
		esif_time_type_desc(timeOutType),
		timeOutType);

	*time_ptr = timeOut;
exit:
	return rc;
}


enum esif_rc EsifUfXformPercent(
	const enum esif_percent_type type,
	u32 *valuePtr,
	const enum esif_action_type action,
	const EsifDspPtr dspPtr,
	const enum esif_primitive_opcode opcode
	)
{
	enum esif_rc rc = ESIF_OK;
	u32 value = 0;
	enum esif_percent_type percentInType = type;
	enum esif_percent_type percentOutType = type;
	struct esif_fpc_algorithm *algo_ptr = NULL;

	if ((valuePtr == NULL) || (dspPtr == NULL)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	algo_ptr = dspPtr->get_algorithm(dspPtr, action);
	if (algo_ptr == NULL) {
		rc = ESIF_E_NEED_ALGORITHM;
		goto exit;
	}

	value = *valuePtr;

	switch (algo_ptr->percent_xform) {
	case ESIF_ALGORITHM_TYPE_PERCENT_WHOLE:
		ESIF_TRACE_DYN_PERCENT(
			"Using algorithm percent (%s) for percentage\n",
			esif_algorithm_type_str(algo_ptr->percent_xform));

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
			esif_algorithm_type_str(algo_ptr->percent_xform));

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
			esif_algorithm_type_str(algo_ptr->percent_xform));

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

	case ESIF_ALGORITHM_TYPE_PERCENT_BINARY:
		ESIF_TRACE_DYN_PERCENT(
			"Using algorithm BINARY (%s) for percentage\n",
			esif_algorithm_type_str(algo_ptr->percent_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {

			/* XFORM to milli-percent to increase accuracy */
			value = value * 100 * 1000 / ESIF_PERCENT_HDC_CONV_FACTOR;

			percentInType = ESIF_PERCENT_MILLI;
			percentOutType = NORMALIZE_PERCENT_TYPE;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);

		} else {
			percentInType = NORMALIZE_PERCENT_TYPE;
			percentOutType = ESIF_PERCENT_MILLI;
			esif_convert_percent(percentInType,
				percentOutType,
				&value);

			if (value > (100 * 1000)) {
				rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
				goto exit;
			}
			/* value = 127 * percentage */
			value = value * ESIF_PERCENT_HDC_CONV_FACTOR / 1000 / 100;
		}
		break;

	case ESIF_ALGORITHM_TYPE_PERCENT_NONE:
		ESIF_TRACE_DYN_PERCENT(
			"Using algorithm NONE (%s), no xform performed\n",
			esif_algorithm_type_str(algo_ptr->percent_xform));
		break;

	default:
		ESIF_TRACE_DYN_PERCENT(
			"Unknown algorithm (%s) to xform percentage\n",
			esif_algorithm_type_str(algo_ptr->time_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
		goto exit;
		break;
	}

	ESIF_TRACE_DYN_TEMP("IN percent %u %s(%d)\n",
		*value_ptr,
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

void EsifUfExecuteTransform(
	const EsifDataPtr transformDataPtr,
	const enum esif_action_type action,
	const EsifDspPtr dsp_ptr,
	const enum esif_primitive_opcode opcode)
{
	switch (transformDataPtr->type) {
	case ESIF_DATA_TEMPERATURE:
		ESIF_TRACE_DEBUG("esif xform temp\n");
		EsifUfXformTemp(NORMALIZE_TEMP_TYPE,
			(esif_temp_t *) transformDataPtr->buf_ptr,
			action,
			dsp_ptr,
			opcode);
		break;
	case ESIF_DATA_POWER:
		ESIF_TRACE_DEBUG("esif xform power\n");
		EsifUfXformPower(NORMALIZE_POWER_UNIT_TYPE,
			(esif_power_t *) transformDataPtr->buf_ptr,
			action,
			dsp_ptr,
			opcode);
		break;
	case ESIF_DATA_TIME:
		ESIF_TRACE_DEBUG("esif xform time\n");
		EsifUfXformTime(NORMALIZE_TIME_TYPE,
			(esif_time_t *) transformDataPtr->buf_ptr,
			action,
			dsp_ptr,
			opcode);
		break;
	case ESIF_DATA_PERCENT:
		ESIF_TRACE_DEBUG("esif xform percent\n");
		EsifUfXformPercent(NORMALIZE_PERCENT_TYPE,
			(u32 *) transformDataPtr->buf_ptr,
			action,
			dsp_ptr,
			opcode);
		break;
	default:
		ESIF_TRACE_DEBUG("No transformation. \n");
		break;
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

