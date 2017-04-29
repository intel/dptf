/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#pragma once

/*
 * Class GUIDs
 */

#define ESIF_PARTICIPANT_DPTF_NAME "DPTFZ"
#define ESIF_PARTICIPANT_DPTF_DESC "DPTF Zone"
#define ESIF_PARTICIPANT_DPTF_CLASS_GUID { \
	0x23, 0xA9, 0xE1, 0x5F, 0x0B, 0xA4, 0x46, 0x9C, \
	0xB1, 0x6F, 0x1C, 0x46, 0x79, 0x75, 0x4F, 0x80 }

#define ESIF_PARTICIPANT_ACPI_NAME "ACPI"
#define ESIF_PARTICIPANT_ACPI_DESC "DPTF ACPI Device"
#define ESIF_PARTICIPANT_ACPI_CLASS_GUID { \
	0xD9, 0xD5, 0xBE, 0x64, 0x3C, 0xE2, 0x49, 0x27, \
	0x95, 0xA3, 0xEE, 0xE4, 0x0C, 0x9A, 0x58, 0x3B }

#define ESIF_PARTICIPANT_CPU_NAME "TCPU"
#define ESIF_PARTICIPANT_CPU_DESC "DPTF CPU Device"
#define ESIF_PARTICIPANT_CPU_CLASS_GUID { \
	0x53, 0x4A, 0x09, 0x8F, 0x5E, 0x42, 0x4C, 0x64, \
	0xBE, 0xB3, 0x91, 0x7A, 0xB3, 0x7C, 0x5D, 0xA5 }

#define ESIF_PARTICIPANT_PCH_NAME "TPCH"
#define ESIF_PARTICIPANT_PCH_DESC "DPTF PCH Device"
#define ESIF_PARTICIPANT_PCH_CLASS_GUID { \
	0x33, 0xAB, 0xB9, 0xB2, 0xE6, 0x86, 0x43, 0xB8, \
	0xB0, 0xDF, 0x3E, 0x91, 0x53, 0x90, 0xCB, 0xE5 }

#define ESIF_PARTICIPANT_PLAT_NAME "PLAT"
#define ESIF_PARTICIPANT_PLAT_DESC "DPTF Platform Device"
#define ESIF_PARTICIPANT_PLAT_CLASS_GUID { \
	0xD4, 0x08, 0x04, 0xF4, 0xDC, 0x73, 0x4B, 0x33, \
	0xB2, 0x8C, 0x19, 0x5A, 0xE9, 0x8F, 0x29, 0x0E }

#define ESIF_LF_PE_NAME "LFPE"
#define ESIF_LF_PE_DESC "LF Participant Extension"
#define ESIF_LF_PE_CLASS_GUID { \
	0x5E, 0xE4, 0x34, 0xCD, 0xA5, 0x12, 0x6B, 0x48, \
	0xBE, 0xD3, 0xF0, 0x7A, 0x6C, 0xFB, 0x48, 0xB2 }

