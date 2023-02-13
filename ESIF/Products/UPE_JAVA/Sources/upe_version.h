/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#pragma once

#include "Ver.h"

#define STR(arg) #arg
#define STR2(arg) STR(##arg)
#define EXPAND_TOSTR(arg) STR(arg)

/* TODO: Update/replace file as needeed */

#define UPE_VERSION_MAJOR	VER_MAJOR
#define UPE_VERSION_MINOR	VER_MINOR
#define UPE_VERSION_HOTFIX	VER_HOTFIX
#define UPE_VERSION_BUILD	VER_BUILD

#define UPE_VERSION EXPAND_TOSTR(UPE_VERSION_MAJOR.UPE_VERSION_MINOR.UPE_VERSION_HOTFIX.UPE_VERSION_BUILD)

/* Product and Copyright Definitions */
#define UPE_PRODUCT   "Intel(R) Innovation Platform Framework"
#define UPE_COMPONENT "Intel(R) Innovation Platform Framework JAVA Action"
#define UPE_COPYRIGHT "Copyright (c) 2013-2023 Intel Corporation All Rights Reserved"
#define UPE_COMPANY   "Intel Corporation"
#define UPE_FILENAME  "upe_java.so"

