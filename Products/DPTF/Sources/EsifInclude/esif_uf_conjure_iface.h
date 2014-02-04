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

#ifndef _ESIF_UF_CONJURE_IFACE
#define _ESIF_UF_CONJURE_IFACE

#include "esif.h"
#include "esif_rc.h"
#include "esif_data.h"
#include "esif_uf_iface.h"
#include "esif_participant_iface.h"

#define ESIF_PARTICIPANT_CONJURE_CLASS_GUID {0xe3,0x78,0x02,0xdf,0xdf,0x3d,0x46,0xa7,0xb9,0x9b,0x1f,0x1c,0x78,0x5f,0xd9,0x1b }

/*
    =============================================================================
    Conjure Service Interface (Conjure -> ESIF)
    =============================================================================
*/

/*
    Upper Framework Register Participant. Provides the same functionality as the LF
    version.  Maybe called many times by one Conjure Library.
*/
typedef eEsifError (*RegisterParticipantFunction)(
    const EsifParticipantIfacePtr pi
);

/*
    Upper Framework UnRegister Participant.  Provides the same functionality as the LF
    version.  Maybe called many times by one Conjure Library.
*/
typedef eEsifError (*UnRegisterParticipantFunction)(
    const EsifParticipantIfacePtr pi
);

/* ESIF Conjure Services Interface */
typedef struct _t_EsifConjureServiceInterface {

     /* Header */
     eIfaceType                     fIfaceType;
     UInt16                         fIfaceVersion;
     UInt16                         fIfaceSize;

     /* Function Pointers */
     RegisterParticipantFunction    fRegisterParticipantFuncPtr;
     UnRegisterParticipantFunction  fUnRegisterParticipantFuncPtr;

} EsifConjureServiceInterface, *EsifConjureServiceInterfacePtr, **EsifConjureServiceInteracePtrLocation;

/*
    =============================================================================
    Conjure Interface (ESIF -> Conjure)
    =============================================================================
*/

typedef eEsifError (*ConjureGetAboutFunction)(esif::EsifDataPtr conjureAbout);
typedef eEsifError (*ConjureGetDescriptionFunction)(esif::EsifDataPtr conjureDescription);
typedef eEsifError (*ConjureGetGuidFunction)(esif::EsifDataPtr conjureGuid);
typedef eEsifError (*ConjureGetNameFunction)(esif::EsifDataPtr conjureName);
typedef eEsifError (*ConjureGetVersionFunction)(esif::EsifDataPtr conjureVersion);

typedef eEsifError (*ConjureCreateFunction)(
    EsifConjureServiceInterfacePtr esifServiceInterface,
    const void*                    esifHandle,           /* ESIF will provide Conjure MUST save for use with callbacks */
    void**                         conjureHandleLocation /* The Conjure MUST provide esif will save for use with callbacks */
);

/*
    DEPENDENCY
    The remaining functions are dependent on conjure create.
*/
typedef eEsifError (*ConjureDestroyFunction)(void* conjureHandle);

/* ESIF Conjure Interface */
typedef struct _t_EsifConjureInterface {

     /* Header */
     eIfaceType                     fIfaceType;
     UInt16                         fIfaceVersion;
     UInt16                         fIfaceSize;

     /* Function Pointers */
     ConjureCreateFunction          fConjureCreateFuncPtr;
     ConjureDestroyFunction         fConjureDestroyFuncPtr;
     ConjureGetAboutFunction        fConjureGetAboutFuncPtr;
     ConjureGetDescriptionFunction  fConjureGetDescriptionFuncPtr;
     ConjureGetGuidFunction         fConjureGetGuidFuncPtr;
     ConjureGetNameFunction         fConjureGetNameFuncPtr;
     ConjureGetVersionFunction      fConjureGetVersionFuncPtr;

} EsifConjureInterface, *EsifConjureInterfacePtr, **EsifConjureInteracePtrLocation;


#endif // _ESIF_UF_CONJURE_IFACE