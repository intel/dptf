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


/* Linux User: Use native x86/x64 type and __atomic builtin functions [gcc 4.7.0 or higher] */

// atomic32_t = 32-bit atomic integer usable on both x86 and x64 platforms
typedef int atomic32_basetype;
typedef volatile atomic32_basetype atomic32_t;
#define atomic32_read(v)	atomic_read(v)
#define atomic32_set(v, i)	atomic_set(v, i)
#define atomic32_inc(v)		atomic_inc(v)
#define atomic32_dec(v)		atomic_dec(v)
#define atomic32_add(i, v)	atomic_add(i, v)
#define atomic32_sub(i, v)	atomic_sub(i, v)
#define ATOMIC32_FMT		"%d"
#define ATOMIC32_FMTX		"0x%x"
#define ATOMIC32_FMT0X		"0x%08x"

// atomic64_t = 64-bit atomic integer usable on both x86 and x64 platforms
typedef long long atomic64_basetype;
typedef volatile atomic64_basetype atomic64_t;
#define atomic64_read(v)	atomic_read(v)
#define atomic64_set(v, i)	atomic_set(v, i)
#define atomic64_inc(v)		atomic_inc(v)
#define atomic64_dec(v)		atomic_dec(v)
#define atomic64_add(i, v)	atomic_add(i, v)
#define atomic64_sub(i, v)	atomic_sub(i, v)
#define ATOMIC64_FMT		"%lld"
#define ATOMIC64_FMTX		"0x%llx"
#define ATOMIC64_FMT0X		"0x%016llx"

// atomic_t = native atomic integer type (x86=32-bit x64=64-bit)
typedef long atomic_basetype;
typedef volatile atomic_basetype atomic_t;
#define ATOMIC_FMT			"%ld"
#define ATOMIC_FMTX			"0x%lx"
#ifdef __x86_x64__
#define ATOMIC_FMT0X		"0x%016lx"
#else
#define ATOMIC_FMT0X		"0x%08lx"
#endif

#if !defined(ATOMIC_LIB_DISABLE)
#define ATOMIC_INIT(i)		(i)
#define atomic_read(v)		__atomic_load_n(v, __ATOMIC_SEQ_CST)
#define atomic_set(v, i)	__atomic_exchange_n(v, i, __ATOMIC_SEQ_CST)
#define atomic_inc(v)		__atomic_add_fetch(v ,1, __ATOMIC_SEQ_CST)
#define atomic_dec(v)		__atomic_sub_fetch(v, 1, __ATOMIC_SEQ_CST)
#define atomic_add(i, v)	__atomic_fetch_add(v, i, __ATOMIC_SEQ_CST)
#define atomic_sub(i, v)	__atomic_fetch_sub(v, i, __ATOMIC_SEQ_CST)
#endif /* !DISABLE */

