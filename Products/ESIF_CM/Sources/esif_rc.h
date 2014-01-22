/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#ifndef _ESIF_RC_H_
#define _ESIF_RC_H_

#ifdef ESIF_ATTR_USER
    #include "esif.h"
#endif
/* Return Code ... may there be many be verbose in errror reporting */
enum esif_rc {
	ESIF_OK = 0,				/* Success */

	/*
	 **********************************************************************
	 * INFORMATIONAL
	 **********************************************************************
	 */

	ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT, /* Information Requested Trip
					       Point Not Present */
	ESIF_I_ACPI_OBJECT_NOT_PRESENT, /* Optional Object Such As _STR */
	ESIF_I_AGAIN,			/* Come Back And Try Again Later */
	ESIF_I_MSR_AFFINITY, /* Not All MSR Affinities Operaional/Supported */

	/*
	 **********************************************************************
	 * ERROR
	 **********************************************************************
	 */

	ESIF_E_NOT_IMPLEMENTED = 1000,  /* TBD / Not Implemented Yet */
	ESIF_E_NO_LOWER_FRAMEWORK,      /* No Lower Framework / Kernel Module */
	ESIF_E_NOT_SUPPORTED,           /* Operation Not Supported By Object */
	ESIF_E_UNSPECIFIED,             /* Catchall Error Code*/
	ESIF_E_INVALID_HANDLE,          /* INVALID Handle Provided */

	/* ACPI */
	ESIF_E_NO_ACPI_SUPPORT,		   /* No ACPI Platform Support */
	ESIF_E_NO_ACPII_SUPPORT,	   /* No Intel ACPI Platform Support */
	ESIF_E_NO_ACPI5_SUPPORT,	   /* No ACPI5 Support */
	ESIF_E_ACPI_RESULT_TYPE,	   /* Error Reading ACPI Data */
	ESIF_E_ACPI_REQUEST_TYPE,	   /* Error Reading ACPI Data */
	ESIF_E_ACPI_EVAL_FAILURE,	   /* ACPI Evaluation Failure */
	ESIF_E_ACPI_OBJECT_NOT_FOUND,      /* Requested ACPI Object Not Found */

	/* Action */
	/* ESIF Data Buffer Overflow Would've Occured */
	ESIF_E_OVERFLOWED_RESULT_TYPE,
	ESIF_E_UNSUPPORTED_ACTION_TYPE,	   /* Undefined Primitive Action Type */
	ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE, /* Unsupported Req Data Type */
	ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE,  /* Unsupported Result Data Type */
	ESIF_E_INVALID_REQUEST_TYPE,          /* Request Type Invalid */

	/* Buffer */
	ESIF_E_NEED_LARGER_BUFFER, /* Response Data Size Will Contain Needed
				   Buf_Size */
	ESIF_E_NEED_BINARY_BUFFER, /* Action Response Data Must Be Binary */


	/* Callback */
	ESIF_E_CALLBACK_IS_NULL, /* Callback Function Pointer Is NULL */

	/* Command */
	ESIF_E_COMMAND_DATA_INVALID, /* Command Data Invalid/ Short */

	/* CPC */
	ESIF_E_CPC_SHORT,     /* CPC Data To Short */
	ESIF_E_CPC_SIGNATURE, /* CPC Signature Failure Must Be %CPC */

	/* IPC */
	ESIF_E_IPC_DATA_INVALID, /* IPC Data Invalid / Short For Type */

	/* MEMORY */
	ESIF_E_NO_MEMORY, /* Memory Allocation Error */
	ESIF_E_NO_CREATE, /* OBJECT Creation Failure */

	/* MMIO */
	ESIF_E_NO_MMIO_SUPPORT,	 /* No MMIO support in this device */

	/* MBI */
	ESIF_E_NO_MBI_SUPPORT,   /* No MBI support in this device */

	/* MSR */
	ESIF_E_MSR_IO_FAILURE,   /* MSR Read or Write Error*/
	ESIF_E_MSR_MULTI_VALUES, /* Values Are Different On Different CPUs Such
				  * That They Cannot Be Presented By UINT
				  */

	/* Participant */
	ESIF_E_PARTICIPANT_NOT_FOUND, /* Participant Not Found */

	/* Parameter */
	ESIF_E_PARAMETER_IS_NULL,          /* Parameter Is NULL */
	ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS, /* Parameter Is Out Of Bounds */

	/* PRIMITIVE */
	ESIF_E_NULL_PRIMITIVE, /* Null Primitive Received During  Loookup */
	ESIF_E_NEED_DSP,       /* DSP Not Loaded */
	ESIF_E_PRIMITIVE_ACTION_FAILURE, /* Primitive Execution Failure */
	ESIF_E_PRIMITIVE_DST_UNAVAIL, /* Primitive Destination Unavailable */
	ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP, /* Primitive Not In Current DSP */
	ESIF_E_OPCODE_NOT_IMPLEMENTED, /* Primitive Opcode Not Implemented */

	/* Temperatue Conversion */
	ESIF_E_UNSUPPORTED_REQUEST_TEMP_TYPE, /* Unsupported Req Temp Type */
	ESIF_E_UNSUPPORTED_RESULT_TEMP_TYPE, /* Unsupported Result/Resp
						Temp Type */

	/* TIMER */
	ESIF_E_TIMEOUT,	/* Timeout*/

	/* TABLE */
	ESIF_E_NOT_TABLE, /* TABLE Header type */

	ESIF_E_UNSUPPORTED_ACPI_NOTIFY_EVENT,	/* ACPI Notify Event is Not
						 Supported/Recognized    */

	/* Power Conversion */
	ESIF_E_UNSUPPORTED_REQUEST_POWER_TYPE,	/* Unsupported Request Power
						 Type */
	ESIF_E_UNSUPPORTED_RESULT_POWER_TYPE,	/* Unsupported Result/Response
						 Power Type */

	/* Xform Algorithm */
	ESIF_E_NEED_ALGORITHM, /* Unspecified Xform Algorithm in DSP */
	ESIF_E_UNSUPPORTED_ALGORITHM,  /* Algorithm Is Not Supported */

	/* Repository */
	ESIF_E_NOT_FOUND = 2000,		/* Item Not Found */
	ESIF_E_READONLY,			/* Item is Read-Only */
};

/* Convert Return Code To A String */
static ESIF_INLINE char
*esif_rc_str(enum esif_rc type)
{
	/* TODO:  Lifu to fix */
    #define ESIF_CREATE_RC(rc) case rc: \
	str = (esif_string) #rc; break;
	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;
	switch (type) {
		ESIF_CREATE_RC(ESIF_OK)
		ESIF_CREATE_RC(ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT)
		ESIF_CREATE_RC(ESIF_I_ACPI_OBJECT_NOT_PRESENT)
		ESIF_CREATE_RC(ESIF_I_AGAIN)
		ESIF_CREATE_RC(ESIF_I_MSR_AFFINITY)
		ESIF_CREATE_RC(ESIF_E_NO_LOWER_FRAMEWORK)
		ESIF_CREATE_RC(ESIF_E_NOT_SUPPORTED)
		ESIF_CREATE_RC(ESIF_E_NO_ACPI_SUPPORT)
		ESIF_CREATE_RC(ESIF_E_NO_ACPII_SUPPORT)
		ESIF_CREATE_RC(ESIF_E_NO_ACPI5_SUPPORT)
		ESIF_CREATE_RC(ESIF_E_NULL_PRIMITIVE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_ACTION_TYPE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE)
		ESIF_CREATE_RC(ESIF_E_OVERFLOWED_RESULT_TYPE)
		ESIF_CREATE_RC(ESIF_E_ACPI_RESULT_TYPE)
		ESIF_CREATE_RC(ESIF_E_ACPI_REQUEST_TYPE)
		ESIF_CREATE_RC(ESIF_E_ACPI_EVAL_FAILURE)
		ESIF_CREATE_RC(ESIF_E_ACPI_OBJECT_NOT_FOUND)
		ESIF_CREATE_RC(ESIF_E_NO_MBI_SUPPORT)
		ESIF_CREATE_RC(ESIF_E_NO_MMIO_SUPPORT)
		ESIF_CREATE_RC(ESIF_E_MSR_IO_FAILURE)
		ESIF_CREATE_RC(ESIF_E_MSR_MULTI_VALUES)
		ESIF_CREATE_RC(ESIF_E_NOT_IMPLEMENTED)
		ESIF_CREATE_RC(ESIF_E_NO_MEMORY)
		ESIF_CREATE_RC(ESIF_E_NO_CREATE)
		ESIF_CREATE_RC(ESIF_E_NEED_DSP)
		ESIF_CREATE_RC(ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP)
		ESIF_CREATE_RC(ESIF_E_PRIMITIVE_ACTION_FAILURE)
		ESIF_CREATE_RC(ESIF_E_OPCODE_NOT_IMPLEMENTED)
		ESIF_CREATE_RC(ESIF_E_PARAMETER_IS_NULL)
		ESIF_CREATE_RC(ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS)
		ESIF_CREATE_RC(ESIF_E_CALLBACK_IS_NULL)
		ESIF_CREATE_RC(ESIF_E_TIMEOUT)
		ESIF_CREATE_RC(ESIF_E_NOT_TABLE)
		ESIF_CREATE_RC(ESIF_E_PRIMITIVE_DST_UNAVAIL)
		ESIF_CREATE_RC(ESIF_E_IPC_DATA_INVALID)
		ESIF_CREATE_RC(ESIF_E_COMMAND_DATA_INVALID)
		ESIF_CREATE_RC(ESIF_E_NEED_LARGER_BUFFER)
		ESIF_CREATE_RC(ESIF_E_NEED_BINARY_BUFFER)
		ESIF_CREATE_RC(ESIF_E_UNSPECIFIED)
		ESIF_CREATE_RC(ESIF_E_INVALID_REQUEST_TYPE)
		ESIF_CREATE_RC(ESIF_E_CPC_SHORT)
		ESIF_CREATE_RC(ESIF_E_CPC_SIGNATURE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_REQUEST_TEMP_TYPE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_RESULT_TEMP_TYPE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_REQUEST_POWER_TYPE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_RESULT_POWER_TYPE)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_ACPI_NOTIFY_EVENT)
		ESIF_CREATE_RC(ESIF_E_PARTICIPANT_NOT_FOUND)
		ESIF_CREATE_RC(ESIF_E_NEED_ALGORITHM)
		ESIF_CREATE_RC(ESIF_E_UNSUPPORTED_ALGORITHM)
		ESIF_CREATE_RC(ESIF_E_INVALID_HANDLE)
		ESIF_CREATE_RC(ESIF_E_NOT_FOUND)
		ESIF_CREATE_RC(ESIF_E_READONLY)
	}
	return str;
}


#ifdef ESIF_ATTR_USER
typedef enum esif_rc eEsifError;
#endif

#endif /* _ESIF_RC_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
