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

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

/*
 * Linux User: Use native x86/x64 type and __sync builtin functions
 * Requires gcc 4.4 or higher
 */
typedef long atomic_basetype;
#define  ATOMIC_FMT "%ld"

typedef volatile atomic_basetype atomic_t;

#if !defined(ATOMIC_LIB_DISABLE)
#define ATOMIC_INIT(i)		(i)
#define atomic_read(v)		__sync_add_and_fetch(v, 0)
#define atomic_set(v, i)	__sync_lock_test_and_set(v, i)
#define atomic_inc(v)		__sync_add_and_fetch(v, 1)
#define atomic_dec(v)		__sync_sub_and_fetch(v, 1)
#define atomic_add(i, v)	__sync_add_and_fetch(v, i)
#define atomic_sub(i, v)	__sync_sub_and_fetch(v, i)
#endif /* !DISABLE */

#endif /* LINUX USER */
