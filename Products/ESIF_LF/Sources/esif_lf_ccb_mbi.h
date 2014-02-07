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

/*
 *
 * Access to the message bus space is through the SoC Transaction Router’s PCI
 * configuration registers.
 * This unit relies on three 32-Bit PCI configuration registers to generate
 * messages:
 *
 * Message Bus Control Register (MCR) - PCI[B:0,D:0,F:0] + D0h
 * Message Data Register (MDR) - PCI[B:0,D:0,F:0] + D4h
 * Message Control Register eXtension (MCRX) - PCI[B:0,D:0,F:0] + D8h
 *
 * This indirect access mode is similar to PCI CAM. Software uses the MCR/MCRX
 * as an index register,
 * indicating which message bus space register to access (MCRX only when
 * required), and MDR as the data register.
 * It writes to the MCR trigger message bus transactions.
 *
 * The format of MCR and MCRX are shown below. Most message bus registers are
 * located in the SoC Transaction Router. The default opcode messages for those
 * registers are as follows:
 *
 * MESSAGE OPCODES:
 * Message ‘Read Register’  opcode: 0x10
 * Message ‘Write Register’ opcode: 0x11
 *
 * Some ports may use other R/W opcodes 0x00/0x01 & 0x06/0x07.
 *
 * D0h MCR Message Control Register: A write to this register issues a message
 * on the North Complex internal message network
 * with the fields specified by that write data. All Byte enables must be
 * enabled when writing this register.
 *
 * Bit   Type Reset Description
 * ----- ---- ----- -----------------
 * 31:24 WO   00h   Message Opcode
 * 23:16 WO   00h   Message Port
 * 15:8  WO   00h   Message Target Register Address
 * 7:4   WO   0h    Message Write Byte Enables: Active high Byte enables which
 * enable each of the corresponding Bytes in the MDR when high.
 * 3:0   WO   0h    Reserved
 *
 * D4h MDR Message Data Register: Provides the means to specify data to be
 * written or retrieving data that was read due
 * to a message operation. For messages with a data payload, MDR must be written
 * with the data to be sent prior to a write to MCR.
 * For messages that return data, MDR contains the data read after the write to
 * MCR completes.
 *
 * Bit   Type Reset Description
 * ----- ---- ----- -----------------------
 * 31:0  RW   00h   Message Data
 *
 * !!!!!! NOT SUPPORTED BY MBI.SYS Windows Driver or ESIF currently.
 *
 * D8h MCRX Offset/Register Extension: This is used for messages sent to end
 * points that require more than 8 Bits for the offset/register.
 * These Bits are a direct extension of MCR[15:8].
 *
 * Bit   Type Reset Description
 * ----- ---- ----- ------------------------
 * 31:08 RW   00h   Message Control Register Extension
 * 7:0   RO   00h   Reserved
 *
 * This handled by our companion driver but shown here for clarification.
 *
 * For Write:
 * 1. Write Data to MDR
 * 2. Write Upper Addr Bits to MCRX (Only if required)
 * 3. Write Command to MCR = {Command (8Bits), PortId (8Bits), Offset (8Bits),
 * ByteEnables (8Bits)}
 *
 * For Read:
 * 1. Write Upper Addr Bits to MCRX (Only if required)
 * 2. Write Command to MCR = {Command (8Bits), PortId (8Bits), Offset (8Bits),
 * ByteEnables (8Bits)}
 * 3. Read from MDR
 *
 */

#ifndef _ESIF_LF_CCB_MBI_H_
#define _ESIF_LF_CCB_MBI_H_

#ifdef ESIF_ATTR_KERNEL

#ifdef ESIF_ATTR_IOSF
#include <asm/iosf_mbi.h>
#endif

/* MBI Read 32 */
enum esif_rc esif_lf_win_mbi_read(u32 MCR, u32 *val_ptr);

static ESIF_INLINE enum esif_rc esif_ccb_mbi_read(
	u8 port,
	u8 punit,
	u32 *val_ptr
	)
{
#ifdef ESIF_ATTR_OS_WINDOWS
	u32 MCR = 0x10000000 | (port << 16) | (punit << 8) | 0xF0;
	return esif_lf_win_mbi_read(MCR, val_ptr);

#else
#ifdef ESIF_ATTR_IOSF
	return iosf_mbi_read(port, 0x10, punit, val_ptr);
#else
	*val_ptr = 0;
	return ESIF_E_NOT_IMPLEMENTED;

#endif
#endif /* ESIF_ATTR_OS_WINDOWS */
}


/* MBI Write 32 */
enum esif_rc esif_lf_win_mbi_write(u32 MCR, u32 val);

static ESIF_INLINE enum esif_rc esif_ccb_mbi_write(
	u8 port,
	u32 punit,
	u32 val
	)
{
#ifdef ESIF_ATTR_OS_WINDOWS
	u32 MCR = 0x11000000 | (port << 16) | (punit << 8) | 0xF0;
	return esif_lf_win_mbi_write(MCR, val);
#else
#ifdef ESIF_ATTR_IOSF
	return iosf_mbi_write(port, 0x11, punit, val);
#else
	return ESIF_E_NOT_IMPLEMENTED;

#endif
#endif /* ESIF_ATTR_OS_WINDOWS */
}


#endif /* ESIF_ATTR_KERNEL */
#endif /* _ESIF_LF_CCB_MBI_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
