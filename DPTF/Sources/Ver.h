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

#pragma once

#include "../../Common/Ver.h"

#if (DEV_BUILD)
#define VER_SPECIAL_BUILD "(Dev Build) "
#else
#define VER_SPECIAL_BUILD
#endif

#ifdef DBG
#define VER_DEBUG_TAG " (DBG)"
#else
#define VER_DEBUG_TAG
#endif

#define VER_SEPARATOR_STR "."
#define VERSION_STR VER_SPECIAL_BUILD VER_MAJOR_STR VER_SEPARATOR_STR VER_MINOR_STR VER_SEPARATOR_STR VER_HOTFIX_STR VER_SEPARATOR_STR VER_BUILD_STR VER_DEBUG_TAG

#define COPYRIGHT_STR "Copyright(C) 2003-2014 Intel Corporation \0"
