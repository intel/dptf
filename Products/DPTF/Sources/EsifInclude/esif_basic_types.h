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

#ifndef _ESIF_BASIC_TYPES_H_
#define _ESIF_BASIC_TYPES_H_

/*******************************************************************************
** OS Agnostic, Kernel/User Agnostic
*******************************************************************************/

typedef char* esif_string;
typedef esif_string EsifString;

typedef char                Int8;
typedef unsigned char       u8;
typedef unsigned char       UInt8;

typedef short               Int16;
typedef unsigned short      u16;
typedef unsigned short      UInt16;

typedef int                 Int32;
typedef unsigned int        u32;
typedef unsigned int        UInt32;

typedef long long           Int64;
typedef unsigned long long  u64;
typedef unsigned long long  UInt64;

typedef int                 IntN;
typedef unsigned int        UIntN;
typedef bool                Bool;

#endif /* _ESIF_BASIC_TYPES_H_ */