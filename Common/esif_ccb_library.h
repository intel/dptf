/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#if defined(ESIF_ATTR_USER)

#include "esif_ccb.h"

#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_library_win_user.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_library_lin_user.h"
#endif

#endif /* USER */
