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

/* This file is intended to be included by every .c module in this project.
* Its purpose is to enforce banning of SDL-deprecated functions and supply
* OS-agnostic implementations of SDL-decprecated functions such as memcpy.
*
* THIS FILE *MUST* BE THE LAST HEADER INCLUDED IN EVERY SOURCE FILE
* OTHERWISE IT WILL FLAG FUNCTIONS DECLARED IN OS SYSTEM HEADERS.
*/

#ifndef __ESIF_SDL_H
#define __ESIF_SDL_H

#ifdef _WIN32
#include <windows.h>
#define _SDL_BANNED_RECOMMENDED
#include "win/banned.h" /* Flag SDL Banned Functions during Build */
#endif

/* OS Abstraction Layer and Replacements for SDL-deprecated functions */
#ifdef _WIN32
#define MyMemcpy(dst, src, len)	memcpy_s(dst, len, src, len)
#else /* Linux-based OS*/
#include <memory.h>
#define MyMemcpy(dst, src, len)	memcpy(dst, src, len)
#endif

/* Define this flag to build with optional SDL enhancements to original LZMA SDK code.
 * This flag is required to satisfy Intel SDL Requirements for passing Klocwork scans.
 */
#define ESIF_LZMA_SDL_ENHANCEMENTS

#endif
