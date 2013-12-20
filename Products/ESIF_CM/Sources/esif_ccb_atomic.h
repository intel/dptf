/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#ifndef _ATOMIC_LIB_H
#define _ATOMIC_LIB_H

/*
** C/C++ OS Agnostic Implementation of user space atomic integer operations for
**Linux & Windows.
** Implementation interface is similar to Linux atomic_t in kernel source
**(atomic.h)
** Build Options: (enable ATOMIC_LIB_64BIT to use 64-bit data types)
**
** Platform		Data Type	Implementation
** ------------	-----------	--------------------------------------------
** Linux x86	32-bit		__sync* builtins
** Linux x86	64-bit		__sync* builtins
** Linux x64	32-bit		__sync* builtins
** Linux x64	64-bit		__sync* builtins
** Windows x86	32-bit		_Interlocked* intrinsic functions
** Windows x86	64-bit		CRITICAL_SECTION [requires atomic_ctor() &
**atomic_dtor()]
** Windows x64	32-bit		_Interlocked* intrinsic functions
** Windows x64	64-bit		_Interlocked*64 intrinsic fuctions
**
** Note that if ATOMIC_LIB_64BIT is enabled, x86 Windows will use Critical
**Sections,
** so only use 32-bit data types if you want to guarantee intrinsic atomic
**operations
** and maximum performance.
*/

#ifdef ESIF_ATTR_KERNEL
/* Not Implemented */
#endif

#ifdef ESIF_ATTR_USER

/* Use 32 or 64 bit atomic ints? */
#ifdef ATOMIC_LIB_64BIT
 #if _WIN32
typedef LONGLONG _atomic_int;
 #else
typedef long long _atomic_int;
 #endif
#else
 #if _WIN32
typedef LONG _atomic_int;
 #else
typedef int _atomic_int;
 #endif
#endif

/* Disable Atomic operations and use native integer operations. Use at your own
 * risk. */
#if defined(ATOMIC_LIB_DISABLE)

typedef volatile _atomic_int atomic_t;
# define ATOMIC_INIT(i)                         (i)
# define atomic_ctor(v)				/* No-Op */
# define atomic_dtor(v)				/* No-Op */
# define atomic_read(v)                         (*(v))
# define atomic_set(v, i)                        (*(v) = (i))
# define atomic_inc(v)                          ((*(v))++)
# define atomic_dec(v)                          ((*(v))--)
# define atomic_add(i, v)                        (*(v) += (i))
# define atomic_sub(i, v)                        (*(v) -= (i))

#elif defined(_WIN32)
/* Windows: Use Interlocked intrinsic functions (Requires WinXP/2003 (x86) or
 * Vista/2003 (x64)), except 64-bit data on x86 Windows */
  #include <intrin.h>
  #pragma intrinsic(_InterlockedExchange)
  #pragma intrinsic(_InterlockedIncrement)
  #pragma intrinsic(_InterlockedDecrement)
  #pragma intrinsic(_InterlockedExchangeAdd)

#ifdef _WIN64
/* 32-bit or 64-bit data on x64 Windows */
typedef volatile _atomic_int atomic_t;
#define ATOMIC_INIT(i)  (i)

#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#define atomic_ctor(v)	/* No-Op for x64 when atomic type is 32-bit or
				* 64-bit */
#define atomic_dtor(v)	/* No-Op for x64 when atomic type is 32-bit or
				* 64-bit */

#else	/* WIN32 */
#ifdef ATOMIC_LIB_64BIT
#  include <windows.h>
/* 64-bit data on x86 Windows requires a Critical Section since there are no
 * intrinsic atomic operators */
typedef struct {
	volatile _atomic_int  counter;
	CRITICAL_SECTION      crit;
} atomic_t;
#define ATOMIC_INIT(i)  {(i)}

/* Required for 64-bit data on x86 Windows since 64-bit Interlocked Intrinsic
 * functions are not available */
static __forceinline void atomic_ctor (atomic_t *v)
{
	InitializeCriticalSectionAndSpinCount(&v->crit, 0x00000400);
}


static __forceinline void atomic_dtor (atomic_t *v)
{
	DeleteCriticalSection(&v->crit);
}


#else
/* 32-bit data on x86 or x64 Windows */
typedef volatile _atomic_int atomic_t;
#define ATOMIC_INIT(i)  (i)

#define atomic_ctor(v)	/* No-Op for x86 when atomic type is 32-bit */
#define atomic_dtor(v)	/* No-Op for x86 when atomic type is 32-bit */
#endif	/* ATOMIC_LIB_64BIT */
#endif	/* WIN32 */

static __forceinline _atomic_int atomic_read (atomic_t *v)
{
#ifdef ATOMIC_LIB_64BIT
# ifdef _WIN64
	return _InterlockedExchangeAdd64(v, 0);

# else
	_atomic_int result;
	EnterCriticalSection(&v->crit);
	result = v->counter;
	LeaveCriticalSection(&v->crit);
	return result;

# endif
#else
	return _InterlockedExchangeAdd(v, 0);

#endif
}


static __forceinline void atomic_set (
	atomic_t *v,
	_atomic_int i
	)
{
#ifdef ATOMIC_LIB_64BIT
# ifdef _WIN64
	(void)_InterlockedExchange64(v, i);
# else
	EnterCriticalSection(&v->crit);
	v->counter = i;
	LeaveCriticalSection(&v->crit);
# endif
#else
	(void)_InterlockedExchange(v, i);
#endif
}


static __forceinline void atomic_inc (atomic_t *v)
{
#ifdef ATOMIC_LIB_64BIT
# ifdef _WIN64
	(void)_InterlockedIncrement64(v);
# else
	EnterCriticalSection(&v->crit);
	v->counter++;
	LeaveCriticalSection(&v->crit);
# endif
#else
	(void)_InterlockedIncrement(v);
#endif
}


static __forceinline void atomic_dec (atomic_t *v)
{
#ifdef ATOMIC_LIB_64BIT
# ifdef _WIN64
	(void)_InterlockedDecrement64(v);
# else
	EnterCriticalSection(&v->crit);
	v->counter--;
	LeaveCriticalSection(&v->crit);
# endif
#else
	(void)_InterlockedDecrement(v);
#endif
}


static __forceinline void atomic_add (
	_atomic_int i,
	atomic_t *v
	)
{
#ifdef ATOMIC_LIB_64BIT
# ifdef _WIN64
	(void)_InterlockedExchangeAdd64(v, i);
# else
	EnterCriticalSection(&v->crit);
	v->counter += i;
	LeaveCriticalSection(&v->crit);
# endif
#else
	(void)_InterlockedExchangeAdd(v, i);
#endif
}


static __forceinline void atomic_sub (
	_atomic_int i,
	atomic_t *v
	)
{
#ifdef ATOMIC_LIB_64BIT
# ifdef _WIN64
	(void)_InterlockedExchangeAdd64(v, -i);
# else
	EnterCriticalSection(&v->crit);
	v->counter -= i;
	LeaveCriticalSection(&v->crit);
# endif
#else
	(void)_InterlockedExchangeAdd(v, -i);
#endif
}


#else
/* Linux: Use __sync functions for 32-bit or 64-bit data on x86 or x64 Linux
 * (Requires gcc 4.4 or higher) */

typedef struct {
	volatile _atomic_int  counter;
} atomic_t;
#define ATOMIC_INIT(i)  {(i)}

# define atomic_ctor(v)	/* No-Op for Linux */
# define atomic_dtor(v)	/* No-Op for Linux */

/* Use macros for native x86/x64 since get/set operations are already atomic
 * (__sync builtins originally designed by Intel) */
#if defined(__x86_64__) || (defined(__i386__) && !defined(ATOMIC_LIB_64BIT))
# define atomic_read(v)         ((v)->counter)
# define atomic_set(v, i)        (((v)->counter) = (i))
#else
/* Use inline functions for other architectures */
static inline _atomic_int atomic_read (atomic_t *v)
{
	return __sync_add_and_fetch(&v->counter, 0);
}


static inline void atomic_set (
	atomic_t *v,
	_atomic_int i
	)
{
	(void)__sync_lock_test_and_set(&v->counter, i);
}


#endif

/* Use Inline functions for remaining atomic operations */
static inline void atomic_inc (atomic_t *v)
{
	(void)__sync_add_and_fetch(&v->counter, 1);
}


static inline void atomic_dec (atomic_t *v)
{
	(void)__sync_sub_and_fetch(&v->counter, 1);
}


static inline void atomic_add (
	_atomic_int i,
	atomic_t *v
	)
{
	(void)__sync_add_and_fetch(&v->counter, i);
}


static inline void atomic_sub (
	_atomic_int i,
	atomic_t *v
	)
{
	(void)__sync_sub_and_fetch(&v->counter, i);
}


#endif	/* Linux */

#endif	/* ESIF_ATTR_USER */

#endif	/* _ATOMIC_LIB_H */
