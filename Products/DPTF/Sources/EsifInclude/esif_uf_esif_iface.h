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

#ifndef _ESIF_UF_ESIF_IFACE_
#define _ESIF_UF_ESIF_IFACE_

#include "esif.h"
#include "esif_rc.h"
#include "esif_data.h"
#include "esif_primitive_type.h"
#include "esif_domain_type.h"
#include "esif_participant_enum.h"
#include "esif_uf_app_event_type.h"
#include "esif_uf_iface.h"
#include "esif_uf_app_iface.h"

#define ESIF_INTERFACE_VERSION 1

/*
    INTERFACE Flags
    These flags will be used by the ESIF Configuration Management Data Base CMDB
*/

#define ESIF_SERVICE_CONFIG_PERSIST     0x00000001 /* Persist Data To Disk */
#define ESIF_SERVICE_CONFIG_ENCRYPT     0x00000002 /* Encrypt Data For Storage */
#define ESIF_SERVICE_CONFIG_READONLY    0x00000004 /* Data is Read-Only */
#define ESIF_SERVICE_CONFIG_NOCACHE     0x00000008 /* Data is not cached in memory (except key) */
#define ESIF_SERVICE_CONFIG_FILELINK    0x00000010 /* Data is a link to an external file */
#define ESIF_SERVICE_CONFIG_REGLINK     0x00000020 /* Data is a link to a Registry value (Windows only) */
#define ESIF_SERVICE_CONFIG_DELETE      0x40000000 /* Delete from memory and disk */
#define ESIF_SERVICE_CONFIG_STATIC      0x80000000 /* Statically Linked Repository */

/*
    EsifServiceInterface Callback Functions
    These functions are only used by the INTERFACE as method to loosely
    couple ESIF with its hosted application.  There are used for the application
    to request information and services from the ESIF.
*/

/* Get Config */
typedef eEsifError (*AppGetConfigFunction)(
    const void*                 esifHandle,                         /* ESIF provided context handle */
    const void*                 appHandle,                          /* handle allocated by ESIF hosted application */
    const esif::EsifDataPtr     nameSpace,                          /* Name space to use e.g. ESIF, DPTF, ACT, Etc. */
    const esif::EsifDataPtr     elementPath,                        /* Element path e.g. /a/b/c must be unique within a name space */
    esif::EsifDataPtr           elementValue                        /* Any valid esif_data_type maybe retrieved including ESIF_DATA_AUTO */
);

/* Set Config */
typedef eEsifError (*AppSetConfigFunction)(
    const void*                 esifHandle,                         /* ESIF provided context handle */
    const void*                 appHandle,                          /* handled allocated by ESIF hosted application */
    const esif::EsifDataPtr     nameSpace,                          /* Name space to use e.g. ESIF, DPTF, ACT, Etc. */
    const esif::EsifDataPtr     elementPath,                        /* Element path e.g. /a/b/c must be unique within a name space */
    const esif::EsifDataPtr     elementValue,                       /* Any valid esif_data_type maybe set accept ESIF_DATA_AUTO */
    const EsifFlags             elementFlags                        /* Flags for handling data including persist, encrypt, etc. */
);

/*
    Primitive Execution note that you may execute against any participant or the IETM object if the optional
    participant handle is ESIF_NO_HANDLE So for example if you wanted to retrieve the ART you would mark
    the participant as ESIF_E_NO_HANDLE.  If you wanted to retrieve GET_TEMPERATURE you would have to provide
    a valid participant handle and domain handle to that ESIF knows "which" temperature to retrieve.
*/
typedef eEsifError (*AppPrimitiveFunction)(
    const void*                 esifHandle,                         /* ESIF provided context handle */
    const void*                 appHandle,                          /* handled allocated by ESIF hosted application */
    const void*                 participantHandle,                  /* Optional participant handle ESIF_NO_HANDLE */
    const void*                 domainHandle,                       /* Optional required if participant handle is provided */
    const esif::EsifDataPtr     request,                            /* Request data for SET_* based primitives */
    esif::EsifDataPtr           response,                           /* Response data for GET_* based primitives */
    ePrimitiveType              primitive,                          /* Primitive ID e.g. GET_TEMPERATURE */
    const UInt8                 instance                            /* Primitive instance may be 255 or ESIF_NO_INSTANCE */
);

/* Write Log */
typedef eEsifError (*AppWriteLogFunction)(
    const void*                 esifHandle,                         /* ESIF provided context handle */
    const void*                 appHandle,                          /* handle allocated by ESIF hosted application */
    const void*                 participantHandle,                  /* optional may be ESIF_NO_HANDLE used to augment log detail*/
    const void*                 domainHandle,                       /* optional may be ESIF_NO_HANDLE used to augment log detail*/
    const esif::EsifDataPtr     message,                            /* Message For Log */
    const eLogType              logType                             /* Log Type e.g. critical, debug, info, etc */
);

/* Event Register */
typedef eEsifError (*AppEventRegisterFunction)(
    const void*                 esifHandle,                         /* ESIF provided context handle */
    const void*                 appHandle,                          /* handle allocated by ESIF hosted application */
    const void*                 participantHandle,                  /* optional may be ESIF_NO_HANDLE indicates app event registration */
    const void*                 domainHandle,                       /* optional may be ESIF_NO_HANDLE indicates participant event registration */
    const esif::EsifDataPtr     eventGuid                           /* Event GUID to be registered */
);

/* Event Unregister */
typedef eEsifError (*AppEventUnregisterFunction)(
    const void*                 esifHandle,                         /* ESIF provided context handle */
    const void*                 appHandle,                          /* handle allocated by ESIF hosted application */
    const void*                 participantHandle,                  /* optional may be ESIF_NO_HANDLE indicates app event registration */
    const void*                 domainHandle,                       /* optional may be ESIF_NO_HANDLE indicates participant event registration */
    const esif::EsifDataPtr     eventGuid                           /* Event GUID to be unregistered */
);

/*
    ESIF Service Interface ESIF <-- APPLICATION
    Forward declared and typedef in esif_uf_iface.h
*/

struct _t_EsifInterface {

    /* Interface */
    eIfaceType                  fIfaceType;
    UInt16                      fIfaceVersion;
    UInt16                      fIfaceSize;

    /* Version 1 */
    /* Configuration Management */
    AppGetConfigFunction        fGetConfigFuncPtr;
    AppSetConfigFunction        fSetConfigFuncPtr;

    /* Execute */
    AppPrimitiveFunction        fPrimitiveFuncPtr;
    AppWriteLogFunction         fWriteLogFuncPtr;

    /* Event Registration */
    AppEventRegisterFunction    fRegisterEventFuncPtr;
    AppEventUnregisterFunction  fUnregisterEventFuncPtr;

    /* Version 2 */
};

#endif // _ESIF_UF_ESIF_IFACE_