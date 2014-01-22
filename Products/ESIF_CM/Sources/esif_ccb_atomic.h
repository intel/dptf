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
** C/C++ OS Agnostic Implementation of atomic integer operations.
**
** Interface is based on native Linux atomic_t kernel type (atomic.h)
** Note that atomic_t size varies depending on mode, platform, and OS:
**
** Platform		Mode	size	Implementation
** ------------	-------	-------	----------------------------------------------
** Linux x86	Kernel	32-bit	atomic_t native type
** Linux x86	User	32-bit	__sync* builtins
** Linux x64	Kernel	64-bit	atomic_t native type
** Linux x64	User	64-bit	__sync* builtins
** Windows x86	Kernel	32-bit	_Interlocked* intrinsic functions
** Windows x86	User	32-bit	_Interlocked* intrinsic functions
** Windows x64	Kernel	64-bit	_Interlocked*64 intrinsic functions
** Windows x64	User	64-bit	_Interlocked*64 intrinsic functions
*/

#ifdef ESIF_ATTR_KERNEL

/* Kernel mode always uses native x86/x64 type */

#ifdef ESIF_ATTR_OS_WINDOWS
/* Windows */
# include <wdm.h>

# ifdef ESIF_ATTR_64BIT
   typedef LONGLONG atomic_t;
   typedef LONGLONG _atomic_int;
#  define  ATOMIC_INIT(i)  (i)
#  define  ATOMIC_FMT "%lld"
#  pragma intrinsic(_InterlockedExchange64)
#  pragma intrinsic(_InterlockedIncrement64)
#  pragma intrinsic(_InterlockedDecrement64)
#  pragma intrinsic(_InterlockedExchangeAdd64)
#  define atomic_read(v)	_InterlockedExchangeAdd64(v, 0)
#  define atomic_set(v, i)	_InterlockedExchange64(v, i)
#  define atomic_inc(v)		_InterlockedIncrement64(v)
#  define atomic_dec(v)		_InterlockedDecrement64(v)
#  define atomic_add(i, v)	_InterlockedExchangeAdd64(v, i)
#  define atomic_sub(i, v)	_InterlockedExchangeAdd64(v, -(i))
# else
   typedef LONG atomic_t;
   typedef LONG _atomic_int;
#  define  ATOMIC_INIT(i)  (i)
#  define  ATOMIC_FMT "%ld"
#  pragma intrinsic(_InterlockedExchange)
#  pragma intrinsic(_InterlockedIncrement)
#  pragma intrinsic(_InterlockedDecrement)
#  pragma intrinsic(_InterlockedExchangeAdd)
#  define atomic_read(v)	_InterlockedExchangeAdd(v, 0)
#  define atomic_set(v, i)	_InterlockedExchange(v, i)
#  define atomic_inc(v)		_InterlockedIncrement(v)
#  define atomic_dec(v)		_InterlockedDecrement(v)
#  define atomic_add(i, v)	_InterlockedExchangeAdd(v, i)
#  define atomic_sub(i, v)	_InterlockedExchangeAdd(v, -(i))
# endif

#else
/* Linux */
# include <asm/atomic.h> /* native atomic_t typedef here */
  typedef long _atomic_int;
# define  ATOMIC_FMT "%ld"
#endif

#endif

#ifdef ESIF_ATTR_USER

#ifdef ESIF_ATTR_OS_WINDOWS
# ifdef ESIF_ATTR_64BIT
  typedef LONGLONG _atomic_int;
# define  ATOMIC_FMT "%lld"
# else
  typedef LONG _atomic_int;
# define  ATOMIC_FMT "%ld"
# endif
#else
  typedef long _atomic_int;
# define  ATOMIC_FMT "%ld"
#endif

/* Disable User-Mode Atomic operations. Use at your own risk */
#if defined(ATOMIC_LIB_DISABLE)

typedef volatile _atomic_int atomic_t;
# define ATOMIC_INIT(i)		(i)
# define atomic_read(v)		(*(v))
# define atomic_set(v, i)	(*(v) = (i))
# define atomic_inc(v)		((*(v))++)
# define atomic_dec(v)		((*(v))--)
# define atomic_add(i, v)	(*(v) += (i))
# define atomic_sub(i, v)	(*(v) -= (i))

#elif defined(ESIF_ATTR_OS_WINDOWS)
/* Windows: Use Interlocked intrinsic functions */

typedef volatile _atomic_int atomic_t;
#define ATOMIC_INIT(i)  (i)

  #include <intrin.h>
  #ifdef ESIF_ATTR_64BIT
  #  pragma intrinsic(_InterlockedExchange64)
  #  pragma intrinsic(_InterlockedIncrement64)
  #  pragma intrinsic(_InterlockedDecrement64)
  #  pragma intrinsic(_InterlockedExchangeAdd64)
  # define atomic_read(v)	_InterlockedExchangeAdd64(v, 0)
  # define atomic_set(v, i)	_InterlockedExchange64(v, i)
  # define atomic_inc(v)	_InterlockedIncrement64(v)
  # define atomic_dec(v)	_InterlockedDecrement64(v)
  # define atomic_add(i, v)	_InterlockedExchangeAdd64(v, i)
  # define atomic_sub(i, v)	_InterlockedExchangeAdd64(v, -(i))
  #else
  # pragma intrinsic(_InterlockedExchange)
  # pragma intrinsic(_InterlockedIncrement)
  # pragma intrinsic(_InterlockedDecrement)
  # pragma intrinsic(_InterlockedExchangeAdd)
  # define atomic_read(v)	_InterlockedExchangeAdd(v, 0)
  # define atomic_set(v, i)	_InterlockedExchange(v, i)
  # define atomic_inc(v)	_InterlockedIncrement(v)
  # define atomic_dec(v)	_InterlockedDecrement(v)
  # define atomic_add(i, v)	_InterlockedExchangeAdd(v, i)
  # define atomic_sub(i, v)	_InterlockedExchangeAdd(v, -(i))
  #endif

#else
/* Linux: Use __sync builtin functions (Requires gcc 4.4 or higher) */

typedef volatile _atomic_int atomic_t;
#define ATOMIC_INIT(i)  (i)

#define atomic_read(v)		__sync_add_and_fetch(v, 0)
#define atomic_set(v, i)	__sync_lock_test_and_set(v, i)
#define atomic_inc(v)		__sync_add_and_fetch(v, 1)
#define atomic_dec(v)		__sync_sub_and_fetch(v, 1)
#define atomic_add(i, v)	__sync_add_and_fetch(v, i)
#define atomic_sub(i, v)	__sync_sub_and_fetch(v, i)

#endif	/* Linux */

#endif	/* ESIF_ATTR_USER */

#endif	/* _ATOMIC_LIB_H */
