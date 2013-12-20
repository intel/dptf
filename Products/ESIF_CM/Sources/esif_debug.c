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

#include "esif.h"
#include "esif_debug.h"


#ifdef ESIF_ATTR_OS_WINDOWS
/*
 *
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified
 * against Windows SDK/DDK included headers which we have no control over.
 *
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#ifdef ESIF_ATTR_KERNEL

/* Debug Data */
u32 g_esif_module_mask = 0xFFFFFFFF;
u32 g_esif_module_category_mask[ESIF_DEBUG_MOD_MAX];

/* Init Debug Modules */
void esif_debug_init_module_categories ()
{
	int i;

	for (i = 0; i < ESIF_DEBUG_MOD_MAX; i++)
		g_esif_module_category_mask[i] = ESIF_TRACE_CATEGORY_DEFAULT;
}


/* Set Debug Modules */
void esif_debug_set_modules (u32 module_mask)
{
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF,
		       ESIF_TRACE_CATEGORY_DEBUG,
		       "%s: setting debug modules: 0x%08X\n",
		       ESIF_FUNC,
		       module_mask);

	g_esif_module_mask = module_mask;
}


/* Get Debug Modules */
void esif_debug_get_modules (u32 *module_mask_ptr)
{
	*module_mask_ptr = g_esif_module_mask;
}


/* Set Module Level */
void esif_debug_set_module_category (
	u32 module,
	u32 module_level_mask
	)
{
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF,
		       ESIF_TRACE_CATEGORY_DEBUG,
		       "%s: %s(%d) debug level: 0x%08X\n",
		       ESIF_FUNC,
		       esif_debug_mod_str((enum esif_debug_mod)module),
		       module,
		       module_level_mask);

	g_esif_module_category_mask[module] = module_level_mask;
}


/* Get Module Level */
void esif_debug_get_module_category (
	u32 module,
	u32 *module_level_mask_ptr
	)
{
	*module_level_mask_ptr = g_esif_module_category_mask[module];
}


#else /* USER */

int g_traceLevel = ESIF_TRACELEVEL_DEFAULT;

#endif /* NOT ESIF_ATTR_KERNEL */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

