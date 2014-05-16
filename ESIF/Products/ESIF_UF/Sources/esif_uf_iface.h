/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_IFACE_
#define _ESIF_UF_IFACE_

#include "esif.h"

/* Log Type */
typedef enum _t_eLogType {
	eLogTypeFatal   = 0,
	eLogTypeError   = 1,
	eLogTypeWarning = 2,
	eLogTypeInfo    = 3,
	eLogTypeDebug   = 4
} eLogType;

typedef enum _t_eIfaceType {
	eIfaceTypeApplication = 0,
	eIfaceTypeEsifService,
	eIfaceTypeAction,
	eIfaceTypeParticipant,
	eIfaceTypeConjure,
	eIfaceTypeConjureService
} eIfaceType;

/* Application Interface */
struct _t_AppInterface;
typedef struct _t_AppInterface AppInterface, *AppInterfacePtr, **AppInterfacePtrLocation;

/* ESIF Interface */
struct _t_EsifInterface;
typedef struct _t_EsifInterface EsifInterface, *EsifInterfacePtr, **EsifInterfacePtrLocation;

typedef enum esif_primitive_type ePrimitiveType;
typedef enum esif_participant_enum eParticipantBus;
typedef enum esif_domain_type eDomainType;
typedef esif_flags_t EsifFlags;

#endif	// _ESIF_UF_IFACE_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/