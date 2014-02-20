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

#ifndef _ESIF_DATA_MISC_H_
#define _ESIF_DATA_MISC_H_

#include "esif.h"
#include "esif_data_type.h"


/* Have ESIF Allocate Buffer */
#define ESIF_DATA_ALLOCATE 0xFFFFFFFF

// Cooling Mode Policy
struct esif_data_complex_scp {
    u32 cooling_mode;
    u32 acoustic_limit;
    u32 power_limit;
};

// Thermal Shutdown Event
struct esif_data_complex_shutdown {
    u32 temperature;
    u32 tripPointTemperature;
};

// Operating System Capabilities
struct esif_data_complex_osc {
    esif_guid_t guid;
    u32         revision;
    u32         count;
    u32         status;                     /* Required */
    u32         capabilities;               /* Always Used By DPTF */
};

#define ESIF_TABLE_NO_REVISION  0xffff

struct esif_table_hdr {
    u8 revision;
    u16 rows;
    u16 cols;
};

/* Allocate */
struct esif_data *esif_data_alloc(enum esif_data_type type, u32 data_len);
void esif_data_free(struct esif_data *data_ptr);

/* Init */
enum esif_rc esif_data_init(void);
void esif_data_exit(void);

#ifdef ESIF_ATTR_USER

    /*
        ESIF Data is used to marshal most data between ESIF and the
        application it contains.  ESIF data uses the popular TLV
        format for describing the data.  It can be used to request
        specific data from an object or to identify the data type that
        is returned.  While a simple structure it is very helpful.  Here
        are a set of convenience initializers to make working with this
        structure easier.
    */
    #define ESIF_DATA(var, type, buf_ptr, buf_len) \
        esif::EsifData var = { \
        ESIF_ELEMENT(.type) type,\
        ESIF_ELEMENT(.buf_ptr) buf_ptr,\
        ESIF_ELEMENT(.buf_len) buf_len }

    #define ESIF_DATA_VOID(var) \
        esif::EsifData var = { \
        ESIF_ELEMENT(.type)    ESIF_DATA_VOID, \
        ESIF_ELEMENT(.buf_ptr) NULL, \
        ESIF_ELEMENT(.buf_len) 0 }

    #define ESIF_DATA_AUTO(var) \
        esif::EsifData var = { \
        ESIF_ELEMENT(.type)    ESIF_DATA_AUTO, \
        ESIF_ELEMENT(.buf_ptr) ESIF_DATA_ALLOCATE, \
        ESIF_ELEMENT(.buf_len) 0 }

#endif /* ESIF_ATTR_USER */

#endif /* _ESIF_DATA_MISC_H_ */