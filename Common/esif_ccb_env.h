/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

// Get Environment variable
static ESIF_INLINE char *esif_ccb_getenv(const char *name)
{
    char *path = NULL;
    size_t len = 0;

    if (_dupenv_s(&path, &len, name) != 0)
        path = NULL;
    return path;
}

static ESIF_INLINE void esif_ccb_envfree(char *name)
{
	if (name) {
		free(name);
	}
}


#elif defined(ESIF_ATTR_OS_LINUX)

#include  <stdlib.h>

// Get Environment variable
#define esif_ccb_getenv(name)   getenv(name)

#define esif_ccb_envfree(arg) ((void)(0))

#endif /* LINUX */

#endif /* USER */