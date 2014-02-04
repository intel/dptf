/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_RC_H_
#define _ESIF_RC_H_

#include "esif.h"

enum esif_rc {
    ESIF_OK = 0,                          /* Success */

    /*
    **********************************************************************
    * INFORMATIONAL 
    **********************************************************************
    */

    ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT,   /* Information Requested Trip Point Not Present     */
    ESIF_I_ACPI_OBJECT_NOT_PRESENT,       /* Optional Object Such As _STR                     */
    ESIF_I_AGAIN,                         /* Come Back And Try Again Later                    */
    ESIF_I_MSR_AFFINITY,                  /* Not All MSR Affinities Are Operational/Supported */

    /*
    **********************************************************************
    * ERROR
    **********************************************************************
    */

    ESIF_E_NOT_IMPLEMENTED = 1000,        /* TBD / Not Implemented Yet                        */
    ESIF_E_NO_LOWER_FRAMEWORK,            /* No Lower Framework / Kernel Module               */
    ESIF_E_NOT_SUPPORTED,                 /* Operation Not Supported By Object                */
    ESIF_E_UNSPECIFIED,                   /* Catchall Error Code                              */
    ESIF_E_INVALID_HANDLE,                /* INVALID Handle Provided                          */

    /* ACPI */
    ESIF_E_NO_ACPI_SUPPORT,               /* No ACPI Platform Support                         */
    ESIF_E_NO_ACPII_SUPPORT,              /* No Intel ACPI Platform Support                   */
    ESIF_E_NO_ACPI5_SUPPORT,              /* No ACPI5 Support                                 */
    ESIF_E_ACPI_RESULT_TYPE,              /* Error Reading ACPI Data                          */
    ESIF_E_ACPI_REQUEST_TYPE,             /* Error Reading ACPI Data                          */
    ESIF_E_ACPI_EVAL_FAILURE,             /* ACPI Evaluation Failure                          */
    ESIF_E_ACPI_OBJECT_NOT_FOUND,         /* Requested ACPI Object Not Found                  */

    /* Action */
    ESIF_E_OVERFLOWED_RESULT_TYPE,        /* ESIF Data Buffer Overflow Would Have Occurred    */
    ESIF_E_UNSUPPORTED_ACTION_TYPE,       /* Undefined ESIF Primitive Action Type             */
    ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE, /* Unsupported Request Data Type                    */
    ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE,  /* Unsupported Result/Response Data Type            */
    ESIF_E_INVALID_REQUEST_TYPE,          /* Request Type Invalid                             */

    /* Buffer */
    ESIF_E_NEED_LARGER_BUFFER,            /* Response Data Size Will Contain Needed Buf_Size  */
    ESIF_E_NEED_BINARY_BUFFER,            /* Action Response Data Must Be Binary              */
   

    /* Callback */
    ESIF_E_CALLBACK_IS_NULL,              /* Callback Function Pointer Is NULL                */

    /* Command */
    ESIF_E_COMMAND_DATA_INVALID,          /* Command Data Invalid/ Short                      */

    /* CPC */
    ESIF_E_CPC_SHORT,                     /* CPC Data To Short                                */
    ESIF_E_CPC_SIGNATURE,                 /* CPC Signature Failure Must Be %CPC               */

    /* IPC */
    ESIF_E_IPC_DATA_INVALID,              /* IPC Data Invalid / Short For Type                */

    /* MEMORY */
    ESIF_E_NO_MEMORY,                     /* Memory Allocation Error                          */
    ESIF_E_NO_CREATE,                     /* OBJECT Creation Failure                          */

    /* MMIO */
    ESIF_E_NO_MMIO_SUPPORT,               /* No MMIO support in this device                   */

    /* MBI */
    ESIF_E_NO_MBI_SUPPORT,                /* No MBI support in this device                    */

    /* MSR */
    ESIF_E_MSR_IO_FAILURE,                /* MSR Read or Write Error                          */

    /* Participant */
    ESIF_E_PARTICIPANT_NOT_FOUND,         /* Participant Not Found */

    /* Parameter */
    ESIF_E_PARAMETER_IS_NULL,             /* Parameter Is NULL                                */
    ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS,    /* Parameter Is Out Of Bounds                       */

    /* PRIMITIVE */
    ESIF_E_NULL_PRIMITIVE,                /* Null Primitive Received During Primitive Lookup  */
    ESIF_E_NEED_DSP,                      /* DSP Not Loaded                                   */
    ESIF_E_PRIMITIVE_ACTION_FAILURE,      /* Primitive Execution Failure                      */
    ESIF_E_PRIMITIVE_DST_UNAVAIL,         /* Primitive Destination Unavailable                */
    ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP,    /* Primitive Not Found In Current DSP               */
    ESIF_E_OPCODE_NOT_IMPLEMENTED,        /* Primitive Opcode Not Implemented                 */

    /* Temperature Conversion */
    ESIF_E_UNSUPPORTED_REQUEST_TEMP_TYPE, /* Unsupported Request Temp Type                    */
    ESIF_E_UNSUPPORTED_RESULT_TEMP_TYPE,  /* Unsupported Result/Response Temp Type            */

    /* TIMER */
    ESIF_E_TIMEOUT,                       /* Timeout                                          */

    /* TABLE */
    ESIF_E_NOT_TABLE,                     /* TABLE Header type                                */

    ESIF_E_UNSUPPORTED_ACPI_NOTIFY_EVENT, /* ACPI Notify Event is Not Supported/Recognized    */

    /* Power Conversion */
    ESIF_E_UNSUPPORTED_REQUEST_POWER_TYPE, /* Unsupported Request Power Type                  */
    ESIF_E_UNSUPPORTED_RESULT_POWER_TYPE,  /* Unsupported Result/Response Power Type          */

    /* Xform Algorithm */
    ESIF_E_NEED_ALGORITHM,                 /* Unspecified Xform Algorithm in DSP              */
    ESIF_E_UNSUPPORTED_ALGORITHM,          /* Algorithm Is Not Supported                      */

    /* Repository */
    ESIF_E_NOT_FOUND=2000,                  /* Item Not Found */
    ESIF_E_READONLY,                        /* Item is Read-Only */
    ESIF_E_WS_DISC,                         /* WS client disconnected */
};

/* Convert Return Code To A String */
static ESIF_INLINE char 
*esif_rc_str(enum esif_rc type)
{
    #define ESIF_CREATE_RC(rc) case rc: str = (esif_string) #rc; break;
    esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
    switch(type) {
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
    ESIF_CREATE_RC(ESIF_E_WS_DISC)
    }
    return str;
}

typedef enum esif_rc eEsifError;

#endif  /* _ESIF_RC_H_ */