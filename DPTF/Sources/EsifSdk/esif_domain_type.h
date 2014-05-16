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

#ifndef _ESIF_DOMAIN_TYPE_H_
#define _ESIF_DOMAIN_TYPE_H_

#include "esif.h"

// EsifDomainTypeToDptfDomainType() translates esif_domain_type to DomainType::Type.
// It's 1:1.  There are no changes except for using the DomainType namespace and removing
// the dependency on the esif header files.  Without this translation we would have to
// expose the esif headers in the policies and participants.

#define ENUM_ESIF_DOMAIN_TYPE(ENUM) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_PROCESSOR, 0) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_GRAPHICS, 1) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_MEMORY, 2) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_TEMPERATURE, 3) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_FAN, 4) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_CHIPSET, 5) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_ETHERNET, 6) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_WIRELESS, 7) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_STORAGE, 8) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_MULTIFUNCTION, 9) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_DISPLAY, 10) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_CHARGER, 11) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_BATTERY, 12) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_AUDIO, 13) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_OTHER, 14) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_WWAN, 15) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_WGIG, 16) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_POWER, 17) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_THERMISTOR, 18) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_INFRARED, 19) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_WIRELESSRFEM, 20) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_VIRTUAL, 21) \
    ENUM##_VAL(ESIF_DOMAIN_TYPE_AMBIENT, 22) \

enum esif_domain_type {
#ifdef ESIF_ATTR_KERNEL
    ESIF_DOMAIN_TYPE_PROCESSOR     = 0,
    ESIF_DOMAIN_TYPE_GRAPHICS      = 1,
    ESIF_DOMAIN_TYPE_MEMORY        = 2,
    ESIF_DOMAIN_TYPE_TEMPERATURE   = 3,
    ESIF_DOMAIN_TYPE_FAN           = 4,
    ESIF_DOMAIN_TYPE_CHIPSET       = 5,
    ESIF_DOMAIN_TYPE_ETHERNET      = 6,
    ESIF_DOMAIN_TYPE_WIRELESS      = 7,
    ESIF_DOMAIN_TYPE_STORAGE       = 8,
    ESIF_DOMAIN_TYPE_MULTIFUNCTION = 9,
    ESIF_DOMAIN_TYPE_DISPLAY       = 10,
    ESIF_DOMAIN_TYPE_CHARGER       = 11,
    ESIF_DOMAIN_TYPE_BATTERY       = 12,
    ESIF_DOMAIN_TYPE_AUDIO         = 13,
    ESIF_DOMAIN_TYPE_OTHER         = 14,
    ESIF_DOMAIN_TYPE_WWAN          = 15,
    ESIF_DOMAIN_TYPE_WGIG          = 16,
    ESIF_DOMAIN_TYPE_POWER         = 17,
    ESIF_DOMAIN_TYPE_THERMISTOR    = 18,
    ESIF_DOMAIN_TYPE_INFRARED      = 19,
    ESIF_DOMAIN_TYPE_WIRELESSRFEM  = 20,
    ESIF_DOMAIN_TYPE_VIRTUAL       = 21,
    ESIF_DOMAIN_TYPE_AMBIENT       = 22,
#else
    ENUM_ESIF_DOMAIN_TYPE(ENUMDECL)
#endif
};

/* Implement these if they are needed */
extern enum esif_domain_type esif_domain_type_string2enum (esif_string str);
extern esif_string esif_domain_type_enum2string (enum esif_domain_type type);

static ESIF_INLINE esif_string esif_domain_type_str (enum esif_domain_type type)
{

#define CREATE_DOMAIN_TYPE(dt) case dt: str = (esif_string) #dt; break;
#define CREATE_DOMAIN_TYPE_VAL(dt,VAL) case dt: str = (esif_string) #dt; break;

    esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
    switch(type) {
#ifdef ESIF_ATTR_KERNEL
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_PROCESSOR)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_GRAPHICS)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_MEMORY)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_TEMPERATURE)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_FAN)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_CHIPSET)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_ETHERNET)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_WIRELESS)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_STORAGE)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_MULTIFUNCTION)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_DISPLAY)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_CHARGER)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_BATTERY)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_AUDIO)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_OTHER)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_WWAN)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_WGIG)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_POWER)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_THERMISTOR)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_INFRARED)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_WIRELESSRFEM)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_VIRTUAL)
    CREATE_DOMAIN_TYPE(ESIF_DOMAIN_TYPE_AMBIENT)
#else
        ENUM_ESIF_DOMAIN_TYPE(CREATE_DOMAIN_TYPE)
#endif
    }
    return str;
}

#endif /* _ESIF_DOMAIN_TYPE_H_ */