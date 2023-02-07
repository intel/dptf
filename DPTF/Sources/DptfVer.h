/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#define VER_MAJOR  9
#define VER_MINOR  0
#define VER_HOTFIX 11001
#define VER_BUILD  28546

#define ARG_TOSTR(arg) #arg
#define VER_TOSTR(arg) ARG_TOSTR(arg)

#define VER_MAJOR_STR VER_TOSTR(VER_MAJOR)
#define VER_MINOR_STR VER_TOSTR(VER_MINOR)
#define VER_HOTFIX_STR VER_TOSTR(VER_HOTFIX)
#define VER_BUILD_STR VER_TOSTR(VER_BUILD)

#define VERSION_STR VER_MAJOR_STR "." VER_MINOR_STR "." VER_HOTFIX_STR "." VER_BUILD_STR

#define COPYRIGHT_STR "Copyright (c) 2013-2022 Intel Corporation All Rights Reserved \0"
