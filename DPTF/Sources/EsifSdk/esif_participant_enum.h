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

#ifndef _ESIF_PARTICIPANT_ENUM_H_
#define _ESIF_PARTICIPANT_ENUM_H_

#include "esif.h"

/* Enumeration */
#define ENUM_ESIF_PARTICIPANT_ENUM(ENUM) \
    ENUM(ESIF_PARTICIPANT_ENUM_ACPI) \
    ENUM(ESIF_PARTICIPANT_ENUM_PCI) \
    ENUM(ESIF_PARTICIPANT_ENUM_PLAT) \
    ENUM(ESIF_PARTICIPANT_ENUM_CONJURE) \

enum esif_participant_enum {
#ifdef ESIF_ATTR_KERNEL
    ESIF_PARTICIPANT_ENUM_ACPI,
    ESIF_PARTICIPANT_ENUM_PCI,
    ESIF_PARTICIPANT_ENUM_PLAT,
    ESIF_PARTICIPANT_ENUM_CONJURE,
#else
    ENUM_ESIF_PARTICIPANT_ENUM(ENUMDECL)
#endif
};

// Implement these if they are needed
extern enum esif_participant_enum esif_participant_enum_string2enum(esif_string str);
extern esif_string esif_participant_enum_enum2string(enum esif_participant_enum type);

/* Enumeration String */
static ESIF_INLINE esif_string
    esif_participant_enum_str(enum esif_participant_enum index)
{
#define CREATE_PARTICIPANT_ENUM(pe) case pe: str = #pe; break;
    esif_string str = ESIF_NOT_AVAILABLE;
    switch(index) {
        CREATE_PARTICIPANT_ENUM(ESIF_PARTICIPANT_ENUM_ACPI)
        CREATE_PARTICIPANT_ENUM(ESIF_PARTICIPANT_ENUM_PCI)
        CREATE_PARTICIPANT_ENUM(ESIF_PARTICIPANT_ENUM_PLAT)
        CREATE_PARTICIPANT_ENUM(ESIF_PARTICIPANT_ENUM_CONJURE)
    }
    return str;
}

#endif /* _ESIF_PARTICIPANT_ENUM_H_ */