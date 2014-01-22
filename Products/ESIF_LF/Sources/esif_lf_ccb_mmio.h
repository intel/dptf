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

#ifndef _ESIF_CCB_MMIO_H_
#define _ESIF_CCB_MMIO_H_

#ifdef ESIF_ATTR_KERNEL

/* MMIO Read 32 */
static ESIF_INLINE void esif_ccb_mmio_read(
	const void __iomem *base_addr,
	u32 reg,
	u32 *val_ptr
	)
{
	u8 *p = (u8 *)val_ptr;
	int i = 0;

	/*
	 * Don't just readl, it will fail to get 4-byte of TSTR0 where
	 * TSTR0 (offset 3, len 1) and TSTTP0 (offset 4, len 4)
	 */
	for (i = 0; i < 4; i++, p++) {
#ifdef ESIF_ATTR_OS_LINUX
		*(u8 *)p = (readb(base_addr + reg + i));
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
		*(u8 *)p = (READ_REGISTER_UCHAR((PUCHAR)base_addr + reg + i));
#endif
	}
}


/* MMIO Write 32 */
static ESIF_INLINE void esif_ccb_mmio_write(
	const void __iomem *base_addr,
	u32 reg,
	u32 val
	)
{
	u8 *p = (u8 *)&val;
	int i = 0;

	for (i = 0; i < 4; i++, p++) {
#ifdef ESIF_ATTR_OS_LINUX
		writeb(*p, (void __iomem *)base_addr + reg + i);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
		WRITE_REGISTER_UCHAR((PUCHAR)base_addr + reg + i, *p);
#endif
	}
}


#endif /* ESIF_ATTR_KERNEL */
#endif /* _ESIF_CCB_MMIO_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
