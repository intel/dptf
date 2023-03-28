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

#include "esif_ccb_sem.h"

typedef esif_ccb_sem_t	signal_t;
#define signal_init(sigPtr)				esif_ccb_sem_init(sigPtr)
#define signal_uninit(sigPtr)			esif_ccb_sem_uninit(sigPtr)
#define signal_post(sigPtr)				esif_ccb_sem_up(sigPtr)
#define signal_wait(sigPtr)				esif_ccb_sem_down(sigPtr)
#define signal_wait_timeout(sigPtr, ms)	esif_ccb_sem_try_down(sigPtr, ms)
