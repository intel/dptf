/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#pragma once

#include "esif_ccb.h"

/*
 * Error Return Codes
 */

typedef enum esif_rc {
	ESIF_OK = 0,				/* Success */

	/*
	 **********************************************************************
	 * INFORMATIONAL
	 **********************************************************************
	 */

	ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT = 100,
	ESIF_I_ACPI_OBJECT_NOT_PRESENT, /* Optional Object Such As _STR */
	ESIF_I_AGAIN,	     /* Come Back And Try Again Later */
	ESIF_I_MSR_AFFINITY, /* Not All MSR Affinities Operational/Supported */
	ESIF_I_INIT_PAUSED,				/* Init paused - for later completion */

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
	ESIF_E_ITERATION_DONE,          /* Indicates iteration is complete */
	ESIF_E_STOP_POLL,               /* Indicates to stop the polling */
	ESIF_E_ORDERED_INSERT,          /* Adding to an ordered collection failed */
	ESIF_E_API_ERROR,				/* OS-specific API error */
	ESIF_E_MAXIMUM_CAPACITY_REACHED,/* Unable to insert object due to lack of entries */
	ESIF_E_DISABLED,				/* Optional support disabled by configuration */
	ESIF_E_NOT_INITIALIZED,			/* Required support has not been initialized */


	/* ACPI */
	ESIF_E_NO_ACPI_SUPPORT = 1100, /* No ACPI Platform Support */
	ESIF_E_NO_ACPII_SUPPORT,	   /* No Intel ACPI Platform Support */
	ESIF_E_NO_ACPI5_SUPPORT,	   /* No ACPI5 Support */
	ESIF_E_ACPI_RESULT_TYPE,	   /* Error Reading ACPI Data */
	ESIF_E_ACPI_REQUEST_TYPE,	   /* Error Reading ACPI Data */
	ESIF_E_ACPI_EVAL_FAILURE,	   /* ACPI Evaluation Failure */
	ESIF_E_ACPI_OBJECT_NOT_FOUND,      /* Requested ACPI Object
						  Not Found */
	ESIF_E_UNSUPPORTED_ACPI_NOTIFY_EVENT, /* ACPI Notify Event is Not
						Supported/Recognized */

	/* Action */
	ESIF_E_ACTION_NOT_IMPLEMENTED = 1200, /* Unsupported Action*/
	ESIF_E_OVERFLOWED_RESULT_TYPE, /* Data Buffer Overflow Result Type */
	ESIF_E_UNSUPPORTED_ACTION_TYPE,    /* Undefined Primitive Action Type */
	ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE, /* Unsupported Req Data Type */
	ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE,  /* Unsupported Resp Data Type */
	ESIF_E_INVALID_REQUEST_TYPE,          /* Request Type Invalid */
	ESIF_E_XFORM_NOT_AVAILABLE,           /* Xform Is Unavailable */
	ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS,    /* Request data invalid */
	ESIF_E_ACTION_ALREADY_STARTED,	/* Action already available */
	ESIF_E_APP_ALREADY_STARTED,	/* App already available */

	/* Buffer */
	ESIF_E_NEED_LARGER_BUFFER = 1300, /* Response Data Size Will Contain Needed Buf_Size */
	ESIF_E_NEED_BINARY_BUFFER, /* Action Response Data Must Be Binary */
	ESIF_E_REQ_SIZE_TYPE_MISTMATCH, /* The req data buffer is too small for specified type */
	ESIF_E_COMPRESSION_ERROR, /* Error Compressing or Decompressing Data */

	/* Callback */
	ESIF_E_CALLBACK_IS_NULL = 1400, /* Callback Function Pointer Is NULL */

	/* Command */
	ESIF_E_COMMAND_DATA_INVALID = 1500, /* Command Data Invalid/ Short */
	ESIF_E_DSP_ALREADY_LOADED,	/* DSP already loaded on participant */

	/* CPC */
	ESIF_E_CPC_SHORT = 1600,/* CPC Data Too Short */
	ESIF_E_CPC_SIZE_INVALID, /* Size present in the CPC is incorrect */
	ESIF_E_CPC_SIGNATURE, /* CPC Signature Failure Must Be %CPC */
	ESIF_E_CPC_PRIMITIVE_SIZE_INVALID, /* Primitive size incorrect */

	/* IPC */
	ESIF_E_IPC_DATA_INVALID = 1700, /* IPC Data Invalid / Short For Type */

	/* MEMORY */
	ESIF_E_NO_MEMORY = 1800, /* Memory Allocation Error */
	ESIF_E_NO_CREATE, /* OBJECT Creation Failure */

	/* MMIO */
	ESIF_E_NO_MMIO_SUPPORT = 1900,	 /* No MMIO support in this device */

	/* MBI */
	ESIF_E_NO_MBI_SUPPORT = 2000,   /* No MBI support in this device */

	/* MSR */
	ESIF_E_MSR_IO_FAILURE = 2100,   /* MSR Read or Write Error*/
	ESIF_E_MSR_MULTI_VALUES,
	ESIF_E_MSR_AFFINITY,		/* MSR access failed on a thread */

	/* Participant */
	ESIF_E_PARTICIPANT_NOT_FOUND = 2200, /* Participant Not Found */

	/* Parameter */
	ESIF_E_PARAMETER_IS_NULL = 2300,   /* Parameter Is NULL */
	ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS, /* Parameter Is Out Of Bounds */

	/* PRIMITIVE */
	ESIF_E_NULL_PRIMITIVE = 2400,   /* Null Primitive Received in Lookup */
	ESIF_E_NEED_DSP,                /* DSP Not Loaded */
	ESIF_E_PRIMITIVE_ACTION_FAILURE,/* Primitive Execution Failure */
	ESIF_E_PRIMITIVE_DST_UNAVAIL,   /* Primitive Destination Unavailable */
	ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP,/* Primitive Not In Current DSP */
	ESIF_E_OPCODE_NOT_IMPLEMENTED,  /* Primitive Opcode Not Implemented */
	ESIF_E_PRIMITIVE_NO_ACTION_SUCCESSFUL,/* Primitive action index is invalid */
	ESIF_E_PRIMITIVE_NO_ACTION_AVAIL,/* No actions defined for the primitive */
	ESIF_E_PRIMITIVE_SUR_NOT_FOUND_IN_DSP,/* Primitive Surrogate Not In Current DSP */

	/* Temperature Conversion */
	ESIF_E_UNSUPPORTED_REQUEST_TEMP_TYPE = 2500,
	ESIF_E_UNSUPPORTED_RESULT_TEMP_TYPE,

	/* TIMER */
	ESIF_E_TIMEOUT = 2600,	/* Timeout*/

	/* TABLE */
	ESIF_E_NOT_TABLE = 2700,    /* TABLE Header type */

	/* Power Conversion */
	ESIF_E_UNSUPPORTED_REQUEST_POWER_TYPE = 2800,
	ESIF_E_UNSUPPORTED_RESULT_POWER_TYPE,

	/* Xform Algorithm */
	ESIF_E_NEED_ALGORITHM = 2900,
	ESIF_E_UNSUPPORTED_ALGORITHM,

	/* Repository */
	ESIF_E_NOT_FOUND = 3000,/* Item Not Found */
	ESIF_E_READONLY,	/* Item is Read-Only */
	ESIF_E_IO_ERROR,	/* File I/O Error */
	ESIF_E_IO_OPEN_FAILED,	/* File Open/Create Failed */
	ESIF_E_IO_DELETE_FAILED,/* File Delete Failed */
	ESIF_E_IO_HASH_FAILED, /* Payload SHA1 Hash Failed */
	ESIF_E_IO_INVALID_NAME, /* Invalid Object or Filename */
	ESIF_E_IO_ALREADY_EXISTS, /* Object Already Exists or Opened */

	/* Web Server */
	ESIF_E_WS_DISC = 3100, /* WS client disconnected */
	ESIF_E_WS_INIT_FAILED, /* WS initialization failed */
	ESIF_E_WS_SOCKET_ERROR, /* WS socket error */
	ESIF_E_WS_SERVER_ERROR, /* WS server error */
	ESIF_E_WS_INVALID_ADDR, /* WS invalid IP Address or Port */
	ESIF_E_WS_ALREADY_STARTED, /* WS already started */
	ESIF_E_WS_INCOMPLETE, /* WS incomplete request */
	ESIF_E_WS_INVALID_REQUEST, /* WS invalid request */
	ESIF_E_WS_UNAUTHORIZED, /* WS restricted resource, access denied */

	/* Time Conversion */
	ESIF_E_UNSUPPORTED_REQUEST_TIME_TYPE = 3200,
	ESIF_E_UNSUPPORTED_RESULT_TIME_TYPE,

	/* Percentage Conversion */
	ESIF_E_UNSUPPORTED_REQUEST_PERCENT_TYPE = 3300,
	ESIF_E_UNSUPPORTED_RESULT_PERCENT_TYPE,

	/* Participant extension */
	ESIF_E_IFACE_DISABLED = 3400,
	ESIF_E_IFACE_NOT_SUPPORTED,

	/*Events*/
	ESIF_E_EVENT_NOT_FOUND = 3500,
	ESIF_E_EVENT_FILTERED,

	/*Participant data logging*/
	ESIF_E_INVALID_ARGUMENT_COUNT = 3600,
	ESIF_E_INVALID_PARTICIPANT_ID,
	ESIF_E_INVALID_DOMAIN_ID,
	ESIF_E_INVALID_CAPABILITY_MASK,

	/* IPF RPC Session Errors */
	ESIF_E_SESSION_DISCONNECTED = 3700,
	ESIF_E_SESSION_REQUEST_FAILED,
	ESIF_E_SESSION_ALREADY_STARTED,
	ESIF_E_SESSION_PERMISSION_DENIED,

	/* The following block is reserved for ABAT-specific error codes */
	ESIF_E_ABAT_ERRORS_RSVD = 10000,

} esif_error_t;

/* Convert Return Code To A String */
static ESIF_INLINE char *esif_error_str(esif_error_t type)
{
	switch (type) {
	ESIF_CASE_ENUM(ESIF_OK);

	ESIF_CASE_ENUM(ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT);
	ESIF_CASE_ENUM(ESIF_I_ACPI_OBJECT_NOT_PRESENT);
	ESIF_CASE_ENUM(ESIF_I_AGAIN);
	ESIF_CASE_ENUM(ESIF_I_MSR_AFFINITY);
	ESIF_CASE_ENUM(ESIF_I_INIT_PAUSED);

	ESIF_CASE_ENUM(ESIF_E_NOT_IMPLEMENTED);
	ESIF_CASE_ENUM(ESIF_E_NO_LOWER_FRAMEWORK);
	ESIF_CASE_ENUM(ESIF_E_NOT_SUPPORTED);
	ESIF_CASE_ENUM(ESIF_E_UNSPECIFIED);
	ESIF_CASE_ENUM(ESIF_E_INVALID_HANDLE);
	ESIF_CASE_ENUM(ESIF_E_ITERATION_DONE);
	ESIF_CASE_ENUM(ESIF_E_STOP_POLL);
	ESIF_CASE_ENUM(ESIF_E_ORDERED_INSERT);
	ESIF_CASE_ENUM(ESIF_E_API_ERROR);
	ESIF_CASE_ENUM(ESIF_E_MAXIMUM_CAPACITY_REACHED);
	ESIF_CASE_ENUM(ESIF_E_DISABLED);
	ESIF_CASE_ENUM(ESIF_E_NOT_INITIALIZED);

	ESIF_CASE_ENUM(ESIF_E_NO_ACPI_SUPPORT);
	ESIF_CASE_ENUM(ESIF_E_NO_ACPII_SUPPORT);
	ESIF_CASE_ENUM(ESIF_E_NO_ACPI5_SUPPORT);
	ESIF_CASE_ENUM(ESIF_E_ACPI_RESULT_TYPE);
	ESIF_CASE_ENUM(ESIF_E_ACPI_REQUEST_TYPE);
	ESIF_CASE_ENUM(ESIF_E_ACPI_EVAL_FAILURE);
	ESIF_CASE_ENUM(ESIF_E_ACPI_OBJECT_NOT_FOUND);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_ACPI_NOTIFY_EVENT);

	ESIF_CASE_ENUM(ESIF_E_ACTION_NOT_IMPLEMENTED);
	ESIF_CASE_ENUM(ESIF_E_OVERFLOWED_RESULT_TYPE);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_ACTION_TYPE);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE);
	ESIF_CASE_ENUM(ESIF_E_INVALID_REQUEST_TYPE);
	ESIF_CASE_ENUM(ESIF_E_XFORM_NOT_AVAILABLE);
	ESIF_CASE_ENUM(ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS);
	ESIF_CASE_ENUM(ESIF_E_ACTION_ALREADY_STARTED);
	ESIF_CASE_ENUM(ESIF_E_APP_ALREADY_STARTED);

	
	ESIF_CASE_ENUM(ESIF_E_NEED_LARGER_BUFFER);
	ESIF_CASE_ENUM(ESIF_E_NEED_BINARY_BUFFER);
	ESIF_CASE_ENUM(ESIF_E_REQ_SIZE_TYPE_MISTMATCH);
	ESIF_CASE_ENUM(ESIF_E_COMPRESSION_ERROR);

	ESIF_CASE_ENUM(ESIF_E_CALLBACK_IS_NULL);

	ESIF_CASE_ENUM(ESIF_E_COMMAND_DATA_INVALID);
	ESIF_CASE_ENUM(ESIF_E_DSP_ALREADY_LOADED);

	ESIF_CASE_ENUM(ESIF_E_CPC_SHORT);
	ESIF_CASE_ENUM(ESIF_E_CPC_SIZE_INVALID);
	ESIF_CASE_ENUM(ESIF_E_CPC_SIGNATURE);
	ESIF_CASE_ENUM(ESIF_E_CPC_PRIMITIVE_SIZE_INVALID);
	
	ESIF_CASE_ENUM(ESIF_E_IPC_DATA_INVALID);

	ESIF_CASE_ENUM(ESIF_E_NO_MEMORY);
	ESIF_CASE_ENUM(ESIF_E_NO_CREATE);

	ESIF_CASE_ENUM(ESIF_E_NO_MMIO_SUPPORT);

	ESIF_CASE_ENUM(ESIF_E_NO_MBI_SUPPORT);

	ESIF_CASE_ENUM(ESIF_E_MSR_IO_FAILURE);
	ESIF_CASE_ENUM(ESIF_E_MSR_MULTI_VALUES);
	ESIF_CASE_ENUM(ESIF_E_MSR_AFFINITY);
	
	ESIF_CASE_ENUM(ESIF_E_PARTICIPANT_NOT_FOUND);

	ESIF_CASE_ENUM(ESIF_E_PARAMETER_IS_NULL);
	ESIF_CASE_ENUM(ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS);

	ESIF_CASE_ENUM(ESIF_E_NULL_PRIMITIVE);
	ESIF_CASE_ENUM(ESIF_E_NEED_DSP);
	ESIF_CASE_ENUM(ESIF_E_PRIMITIVE_ACTION_FAILURE);
	ESIF_CASE_ENUM(ESIF_E_PRIMITIVE_DST_UNAVAIL);
	ESIF_CASE_ENUM(ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP);
	ESIF_CASE_ENUM(ESIF_E_OPCODE_NOT_IMPLEMENTED);
	ESIF_CASE_ENUM(ESIF_E_PRIMITIVE_NO_ACTION_SUCCESSFUL);
	ESIF_CASE_ENUM(ESIF_E_PRIMITIVE_NO_ACTION_AVAIL);
	ESIF_CASE_ENUM(ESIF_E_PRIMITIVE_SUR_NOT_FOUND_IN_DSP);
	
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_REQUEST_TEMP_TYPE);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_RESULT_TEMP_TYPE);

	ESIF_CASE_ENUM(ESIF_E_TIMEOUT);

	ESIF_CASE_ENUM(ESIF_E_NOT_TABLE);

	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_REQUEST_POWER_TYPE);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_RESULT_POWER_TYPE);

	ESIF_CASE_ENUM(ESIF_E_NEED_ALGORITHM);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_ALGORITHM);

	ESIF_CASE_ENUM(ESIF_E_NOT_FOUND);
	ESIF_CASE_ENUM(ESIF_E_READONLY);
	ESIF_CASE_ENUM(ESIF_E_IO_ERROR);
	ESIF_CASE_ENUM(ESIF_E_IO_OPEN_FAILED);
	ESIF_CASE_ENUM(ESIF_E_IO_DELETE_FAILED);
	ESIF_CASE_ENUM(ESIF_E_IO_HASH_FAILED);
	ESIF_CASE_ENUM(ESIF_E_IO_INVALID_NAME);
	ESIF_CASE_ENUM(ESIF_E_IO_ALREADY_EXISTS);

	ESIF_CASE_ENUM(ESIF_E_WS_DISC);
	ESIF_CASE_ENUM(ESIF_E_WS_INIT_FAILED);
	ESIF_CASE_ENUM(ESIF_E_WS_SOCKET_ERROR);
	ESIF_CASE_ENUM(ESIF_E_WS_SERVER_ERROR);
	ESIF_CASE_ENUM(ESIF_E_WS_INVALID_ADDR);
	ESIF_CASE_ENUM(ESIF_E_WS_ALREADY_STARTED);
	ESIF_CASE_ENUM(ESIF_E_WS_INCOMPLETE);
	ESIF_CASE_ENUM(ESIF_E_WS_INVALID_REQUEST);
	ESIF_CASE_ENUM(ESIF_E_WS_UNAUTHORIZED);

	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_REQUEST_TIME_TYPE);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_RESULT_TIME_TYPE);

	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_REQUEST_PERCENT_TYPE);
	ESIF_CASE_ENUM(ESIF_E_UNSUPPORTED_RESULT_PERCENT_TYPE);


	ESIF_CASE_ENUM(ESIF_E_IFACE_DISABLED);
	ESIF_CASE_ENUM(	ESIF_E_IFACE_NOT_SUPPORTED);

	ESIF_CASE_ENUM(ESIF_E_EVENT_NOT_FOUND);
	ESIF_CASE_ENUM(ESIF_E_EVENT_FILTERED);

	ESIF_CASE_ENUM(ESIF_E_INVALID_ARGUMENT_COUNT);
	ESIF_CASE_ENUM(ESIF_E_INVALID_PARTICIPANT_ID);
	ESIF_CASE_ENUM(ESIF_E_INVALID_DOMAIN_ID);
	ESIF_CASE_ENUM(ESIF_E_INVALID_CAPABILITY_MASK);

	ESIF_CASE_ENUM(ESIF_E_SESSION_DISCONNECTED);
	ESIF_CASE_ENUM(ESIF_E_SESSION_REQUEST_FAILED);
	ESIF_CASE_ENUM(ESIF_E_SESSION_ALREADY_STARTED);
	ESIF_CASE_ENUM(ESIF_E_SESSION_PERMISSION_DENIED);

	ESIF_CASE_ENUM(ESIF_E_ABAT_ERRORS_RSVD);

	}
	return ESIF_NOT_AVAILABLE;
}

#define esif_rc_str(type) esif_error_str(type)

typedef enum esif_rc eEsifError;
