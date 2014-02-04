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

#ifndef _ESIF_DATA_VARIANT_H_
#define _ESIF_DATA_VARIANT_H_

#include "esif.h"
#include "esif_data_type.h"

/* Variant */

/* Binary data shall be identical to what upper framework sees */
/* Pack so we end up with exactly 12 bytes always */

#pragma pack(push, 1)
union esif_data_variant {
    enum esif_data_type type;

    /* Integer */
    struct {
        enum esif_data_type type;   /* 4 Bytes For Most Compilers */
        u64 value;                  /* 8 Byte Integer */
    } integer;

    /* ASCII String NULL Included In Length */
    struct {
        enum esif_data_type type;   /* 4 Bytes For Most Compilers  */
        u32 length;                 /* 4 Bytes */
        u32 reserved;               /* Align To 12 Bytes */
    } string;
    /* String Data Here */
};
#pragma pack(pop)

#endif /* _ESIF_DATA_VARIANT_H_ */