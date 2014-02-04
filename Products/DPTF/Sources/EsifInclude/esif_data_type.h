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

#ifndef _ESIF_DATA_TYPE_H_
#define _ESIF_DATA_TYPE_H_

#include "esif.h"

#define ENUM_ESIF_DATA_TYPE(ENUM) \
    ENUM##_VAL(ESIF_DATA_AUTO,36) \
    ENUM##_VAL(ESIF_DATA_BINARY,7) \
    ENUM##_VAL(ESIF_DATA_BIT,27) \
    ENUM##_VAL(ESIF_DATA_BLOB,34) \
    ENUM##_VAL(ESIF_DATA_DSP,33) \
    ENUM##_VAL(ESIF_DATA_ENUM,19) \
    ENUM##_VAL(ESIF_DATA_GUID,5) \
    ENUM##_VAL(ESIF_DATA_HANDLE,20) \
    ENUM##_VAL(ESIF_DATA_INSTANCE,30) \
    ENUM##_VAL(ESIF_DATA_INT16,12) \
    ENUM##_VAL(ESIF_DATA_INT32,13) \
    ENUM##_VAL(ESIF_DATA_INT64,14) \
    ENUM##_VAL(ESIF_DATA_INT8,11) \
    ENUM##_VAL(ESIF_DATA_IPV4,16) \
    ENUM##_VAL(ESIF_DATA_IPV6,17) \
    ENUM##_VAL(ESIF_DATA_PERCENT,29) \
    ENUM##_VAL(ESIF_DATA_POINTER,18) \
    ENUM##_VAL(ESIF_DATA_POWER,26) \
    ENUM##_VAL(ESIF_DATA_QUALIFIER,28) \
    ENUM##_VAL(ESIF_DATA_REGISTER,15) \
    ENUM##_VAL(ESIF_DATA_STRING,8) \
    ENUM##_VAL(ESIF_DATA_STRUCTURE,32) \
    ENUM##_VAL(ESIF_DATA_TABLE,35) \
    ENUM##_VAL(ESIF_DATA_TEMPERATURE,6) \
    ENUM##_VAL(ESIF_DATA_TIME,31) \
    ENUM##_VAL(ESIF_DATA_UINT16,2) \
    ENUM##_VAL(ESIF_DATA_UINT32,3) \
    ENUM##_VAL(ESIF_DATA_UINT64,4) \
    ENUM##_VAL(ESIF_DATA_UINT8,1) \
    ENUM##_VAL(ESIF_DATA_UNICODE,9) \
    ENUM##_VAL(ESIF_DATA_VOID,24) \
    ENUM##_VAL(ESIF_DATA_XML,38) \
    ENUM##_VAL(ESIF_DATA_DECIBEL,39) \
    ENUM##_VAL(ESIF_DATA_FREQUENCY, 40) \

enum esif_data_type {
#ifdef ESIF_ATTR_KERNEL
    ESIF_DATA_AUTO=36,
    ESIF_DATA_BINARY=7,
    ESIF_DATA_BIT=27,
    ESIF_DATA_BLOB=34,
    ESIF_DATA_DSP=33,
    ESIF_DATA_ENUM=19,
    ESIF_DATA_GUID=5,
    ESIF_DATA_HANDLE=20,
    ESIF_DATA_INSTANCE=30,
    ESIF_DATA_INT16=12,
    ESIF_DATA_INT32=13,
    ESIF_DATA_INT64=14,
    ESIF_DATA_INT8=11,
    ESIF_DATA_IPV4=16,
    ESIF_DATA_IPV6=17,
    ESIF_DATA_PERCENT=29,
    ESIF_DATA_POINTER=18,
    ESIF_DATA_POWER=26,
    ESIF_DATA_QUALIFIER=28,
    ESIF_DATA_REGISTER=15,
    ESIF_DATA_STRING=8,
    ESIF_DATA_STRUCTURE=32,
    ESIF_DATA_TABLE=35,
    ESIF_DATA_TEMPERATURE=6,
    ESIF_DATA_TIME=31,
    ESIF_DATA_UINT16=2,
    ESIF_DATA_UINT32=3,
    ESIF_DATA_UINT64=4,
    ESIF_DATA_UINT8=1,
    ESIF_DATA_UNICODE=9,
    ESIF_DATA_VOID=24,
    ESIF_DATA_XML=38,
    ESIF_DATA_DECIBEL=39,
    ESIF_DATA_FREQUENCY=40,
#else
    ENUM_ESIF_DATA_TYPE(ENUMDECL)
#endif
};

// Implement these if they are needed
extern enum esif_data_type esif_data_type_string2enum(esif_string str);
extern esif_string esif_data_type_enum2string(enum esif_data_type type);

static ESIF_INLINE esif_string esif_data_type_str(enum esif_data_type type)
{
#define CREATE_DATA_TYPE(dt) case dt: str = (esif_string) #dt; break;
#define CREATE_DATA_TYPE_VAL(dt,VAL) case dt: str = (esif_string) #dt; break;
    esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
    switch(type) {
#ifdef ESIF_ATTR_KERNEL
    CREATE_DATA_TYPE (ESIF_DATA_AUTO)
    CREATE_DATA_TYPE (ESIF_DATA_BINARY)
    CREATE_DATA_TYPE (ESIF_DATA_BIT)
    CREATE_DATA_TYPE (ESIF_DATA_BLOB)
    CREATE_DATA_TYPE (ESIF_DATA_DSP)
    CREATE_DATA_TYPE (ESIF_DATA_ENUM)
    CREATE_DATA_TYPE (ESIF_DATA_GUID)
    CREATE_DATA_TYPE (ESIF_DATA_HANDLE)
    CREATE_DATA_TYPE (ESIF_DATA_INSTANCE)
    CREATE_DATA_TYPE (ESIF_DATA_INT16)
    CREATE_DATA_TYPE (ESIF_DATA_INT32)
    CREATE_DATA_TYPE (ESIF_DATA_INT64)
    CREATE_DATA_TYPE (ESIF_DATA_INT8)
    CREATE_DATA_TYPE (ESIF_DATA_IPV4)
    CREATE_DATA_TYPE (ESIF_DATA_IPV6)
    CREATE_DATA_TYPE (ESIF_DATA_PERCENT)
    CREATE_DATA_TYPE (ESIF_DATA_POINTER)
    CREATE_DATA_TYPE (ESIF_DATA_POWER)
    CREATE_DATA_TYPE (ESIF_DATA_QUALIFIER)
    CREATE_DATA_TYPE (ESIF_DATA_REGISTER)
    CREATE_DATA_TYPE (ESIF_DATA_STRING)
    CREATE_DATA_TYPE (ESIF_DATA_STRUCTURE)
    CREATE_DATA_TYPE (ESIF_DATA_TABLE)
    CREATE_DATA_TYPE (ESIF_DATA_TEMPERATURE)
    CREATE_DATA_TYPE (ESIF_DATA_TIME)
    CREATE_DATA_TYPE (ESIF_DATA_UINT16)
    CREATE_DATA_TYPE (ESIF_DATA_UINT32)
    CREATE_DATA_TYPE (ESIF_DATA_UINT64)
    CREATE_DATA_TYPE (ESIF_DATA_UINT8)
    CREATE_DATA_TYPE (ESIF_DATA_UNICODE)
    CREATE_DATA_TYPE (ESIF_DATA_VOID)
    CREATE_DATA_TYPE (ESIF_DATA_XML)
    CREATE_DATA_TYPE (ESIF_DATA_DECIBEL)
    CREATE_DATA_TYPE (ESIF_DATA_FREQUENCY)
#else
    ENUM_ESIF_DATA_TYPE(CREATE_DATA_TYPE)
#endif
    }
    return str;
}

#endif /* _ESIF_DATA_TYPE_H_ */