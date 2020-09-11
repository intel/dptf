/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "esif_sdk.h"

/*
 * PCI Devices
 */

typedef enum esif_pci_device_id {
	ESIF_PCI_DEVICE_ID_SNB = 0x0103,
	ESIF_PCI_DEVICE_ID_IVB = 0x0153,
	ESIF_PCI_DEVICE_ID_HSW_ULT = 0x0a03,
	ESIF_PCI_DEVICE_ID_HSW = 0x0c03,
	ESIF_PCI_DEVICE_ID_HSW_H = 0x0d03,
	ESIF_PCI_DEVICE_ID_BDW = 0x1603,
	ESIF_PCI_DEVICE_ID_SKL = 0x1903,
	ESIF_PCI_DEVICE_ID_CPT = 0x1c24,
	ESIF_PCI_DEVICE_ID_PPT = 0x1e24,
	ESIF_PCI_DEVICE_ID_CHV = 0x22dc,
	ESIF_PCI_DEVICE_ID_ADL = 0x461D,
	ESIF_PCI_DEVICE_ID_RKL = 0x4C03,
	ESIF_PCI_DEVICE_ID_JSL = 0x4E03,
	ESIF_PCI_DEVICE_ID_CNL = 0x5a03,
	ESIF_PCI_DEVICE_ID_ICL = 0x8a03,
	ESIF_PCI_DEVICE_ID_LPT = 0x8c24,
	ESIF_PCI_DEVICE_ID_LKF = 0x9820,
	ESIF_PCI_DEVICE_ID_TGL = 0x9a03,
	ESIF_PCI_DEVICE_ID_LPT_LP = 0x9c24,
	ESIF_PCI_DEVICE_ID_WCP = 0x9ca4,
} esif_pci_device_id_t;

static ESIF_INLINE esif_string esif_device_str(esif_pci_device_id_t device_id)
{
	switch (device_id) {
	ESIF_CASE(ESIF_PCI_DEVICE_ID_SNB,
		"DPTF Participant for 2nd Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_IVB,
		"DPTF Participant for 3nd Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_HSW_ULT,
		"DPTF Participant for 4th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_HSW,
		"DPTF Participant for 4th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_HSW_H,
		"DPTF Participant for 4th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_BDW,
		"DPTF Participant for 5th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_SKL,
		"DPTF Participant for 6th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_CPT,
		"Cougar Point(DPTF PCH)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_PPT,
		"Panther Point(DPTF PCH)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_CHV,
		"Cherry View SOC(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_ADL,
		"DPTF Participant for 10th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_RKL,
		"DPTF Participant for 8th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_JSL,
		"DPTF Processor Participant for Jasper Lake SoC(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_CNL,
		"DPTF Participant for 7th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_ICL,
		"DPTF Participant for 8th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_LPT,
		"Lynx Point(DPTF PCH)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_LKF,
		"LakeField SOC(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_TGL,
		"DPTF Participant for 9th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_LPT_LP,
		"Lynx Point Low Power(DPTF PCH)");
	ESIF_CASE(ESIF_PCI_DEVICE_ID_WCP,
		"Wild Cat Point(DPTF PCH)");
	}
	return ESIF_NOT_AVAILABLE;
}

/*
 * PCI Class
 */

static ESIF_INLINE esif_string esif_pci_class_str(u8 class_id)
{
	switch (class_id) {
	ESIF_CASE(0x00, "NA");
	ESIF_CASE(0x01, "Mass Storage Controller");
	ESIF_CASE(0x02, "Network Controller");
	ESIF_CASE(0x03, "Display Controller");
	ESIF_CASE(0x04, "Multimedia Device");
	ESIF_CASE(0x05, "Memory Controller");
	ESIF_CASE(0x06, "Bridge Device");
	ESIF_CASE(0x07, "Simple Communications Controller");
	ESIF_CASE(0x08, "Base System Peripherals");
	ESIF_CASE(0x09, "Input Devices");
	ESIF_CASE(0x0a, "Docking Stations");
	ESIF_CASE(0x0b, "Processors");
	ESIF_CASE(0x0c, "Serial Bus Controllers");
	ESIF_CASE(0x0d, "Wireless Controllers");
	ESIF_CASE(0x0e, "Intelligent I/O Controllers");
	ESIF_CASE(0x0f, "Satelite Communication Controllers");
	ESIF_CASE(0x10, "Encryption/Decryption Controllers");
	ESIF_CASE(0x11, "Data Acquisition and Signal Processing Controllers");
	ESIF_CASE(0xff, "Misc");
	}
	return ESIF_NOT_AVAILABLE;
}

#if defined(ESIF_FEAT_OPT_USE_VIRT_DRVRS)
#if defined(ESIF_ATTR_OS_LINUX)

#pragma pack(push, 1)

const struct pci_device_id esif_pci_cpu_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_SNB) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_IVB) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_HSW) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_HSW_H) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_HSW_ULT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_BDW) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_SKL) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_CNL) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_CHV) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_LKF) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_ICL) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_TGL) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_ADL) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_RKL) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_JSL) },
	{ 0 }
};

const struct pci_device_id esif_pci_pch_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_CPT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_PPT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_LPT) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_LPT_LP) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_WCP) },
	{ 0 }
};

#pragma pack(pop)

#endif
#endif
