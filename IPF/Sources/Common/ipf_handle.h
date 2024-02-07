/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "esif_ccb.h"
#include "esif_ccb_lock.h"
#include "esif_ccb_random.h"

/*
** Generate a Unique Handle within the range (handle_min+1) and (handle_max-1)
** Seed Initial Handle with a Random value between (handle_min+1) and (handle_min+handle_seed)
** Increment Additional Handles by a Random value between 1 and handle_inc until Rollover
**
** This effectively creates a Handle pool of size ((handle_max-handle_min-handle_seed)/handle_inc)
** or ~140 trillion handles or ~4000 years worth of handles (before Rollover) if generated every second
**
** Note that Generated Handles are unique for each function that calls Ipf_GenerateHandle (not globally)
*/
static ESIF_INLINE esif_handle_t Ipf_GenerateHandle(
	esif_handle_t handle_min,	// Min Handle before Random Seed
	esif_handle_t handle_max,	// Max Handle before Rollover
	esif_handle_t handle_seed,	// Handle Initial Seed Random Mask
	esif_handle_t handle_inc)	// Handle Increment Random Mask
{
	static esif_ccb_spinlock_t spinlock = ATOMIC_INIT(0);
	static esif_handle_t handleFactory = 0;

	ESIF_ASSERT(handle_min < handle_max && handle_seed < handle_min && handle_inc < handle_seed);

	// Generate Random Handle Seed/Increment values
	esif_handle_t seed = 1;
	esif_handle_t increment = 1;
	if (esif_ccb_random(&seed, sizeof(seed)) == ESIF_OK) {
		seed = (seed & handle_seed);
		increment = (seed & handle_inc) + !(seed & handle_inc);
	}

	// Generate First or Next Handle or Reset Initial Handle if Rollover
	esif_ccb_spinlock_lock(&spinlock);
	esif_handle_t handle = (handleFactory ? handleFactory + increment : handle_min + seed);
	if (handle >= handle_max) {
		handle = handle_min + handle_seed + increment;
	}
	handleFactory = handle;
	esif_ccb_spinlock_unlock(&spinlock);

	return handle;}
