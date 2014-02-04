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

#ifndef _ESIF_DATA_H_
#define _ESIF_DATA_H_

#include "esif.h"
#include "esif_data_type.h"

/* All ESIF Data Buffers Are Of This Form */

struct esif_data {
    enum esif_data_type type;                                       /* Buffer Type */
    void*               buf_ptr;                                    /* Buffer */
    u32                 buf_len;                                    /* Size of Allocated Buffer By Memory Allocator */
    u32                 data_len;                                   /* Size of Actual Data Contents */
};

#ifdef ESIF_ATTR_USER
namespace esif
{
    typedef struct esif_data EsifData, *EsifDataPtr, **EsifDataPtrLocation;
}
#endif

#endif /* _ESIF_DATA_H_ */