/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include <Ntddscsi.h>
#include "esif_uf_ccb_ata.h"

#define STORAGE_IDENTIFY_OUTPUT_BUFFER_SIZE (4 * 1024)
#define STORAGE_DEVICE_DESCRIPTOR_OUTPUT_BUFFER_SIZE (4 * 1024)
#define LOG_PAGE_BUFFER_SIZE 512
#define LOG_PAGE_TEMPERATURE_OFFSET 1
#define IDENTIFY_PDL_OFFSET 263
#define POWER_STATES_OFFSET 2048
#define POWER_STATES_MULTIPLIER_OFFSET 3
#define POWER_STATES_SIZE 32

#define NVME_STORPORT_DRIVER 0xE000
#define NVME_PT_TIMEOUT 40
#define NVME_IOCTL_VENDOR_SPECIFIC_DW_SIZE 6
#define NVME_IOCTL_CMD_DW_SIZE 16
#define NVME_IOCTL_COMPLETE_DW_SIZE 4
#define NVME_FROM_DEV_TO_HOST 2
#define NVME_RETURN_TO_HOST 1
#define NVME_PASS_THROUGH_SRB_IO_CODE \
	CTL_CODE( NVME_STORPORT_DRIVER, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define NVME_SIG_STR "NvmeMini"
#define NVME_SIG_STR_LEN 8
#define NVME_SET_FEATURE_OPT 9
#define NVME_PSTATE_OPT 2
#define NVME_DATA_VALUE_LOG_PAGE 2
#define NVME_DATA_VALUE_NONE 0
#define NVME_GET_LOG_PAGE_OPT 2
#define NVME_GET_IDENTIFY_PAGE_OPT 6
#define NVME_NAMESPACE_NA 0xFFFFFFFF
#define NVME_LOG_PAGE_IDENTIFIER_SMART 0x00000002

#define DRIVE_HEAD_REGISTER 160
#define DFP_SEND_DRIVE_COMMAND 0x0007c084
#define DFP_RECEIVE_DRIVE_DATA 0x0007c088
#define ATA_NUM_POWER_STATES 254
#define ATA_MIN_POWER_STATE 1
#define MAX_ATA_SMART_ATTRIBUTES 30
#define SETFEATURES_EN_APM 0x05

#pragma pack(push, 1)

struct _pwr_flags {
	unsigned int multiplier : 1;
	unsigned int nops : 1;
};

struct _scsi_address {
	unsigned long len;
	unsigned char port_number;
	unsigned char path_id;
	unsigned char target_id;
	unsigned char lun;
};

typedef struct NVME_PASS_THROUGH_IOCTL {
	SRB_IO_CONTROL SrbIoCtrl;
	DWORD          VendorSpecific[NVME_IOCTL_VENDOR_SPECIFIC_DW_SIZE];
	DWORD          NVMeCmd[NVME_IOCTL_CMD_DW_SIZE];
	DWORD          CplEntry[NVME_IOCTL_COMPLETE_DW_SIZE];
	DWORD          Direction;
	DWORD          QueueId;
	DWORD          DataBufferLen;
	DWORD          MetaDataLen;
	DWORD          ReturnBufferLen;
	UCHAR          DataBuffer[STORAGE_IDENTIFY_OUTPUT_BUFFER_SIZE];
} NVMEPassThroughIOCTL, *NVMEPassThroughIOCTLPtr;

typedef	struct _SMART_READ_DATA_OUTDATA
{
	SENDCMDOUTPARAMS	SendCmdOutParam;
	BYTE				Data[READ_ATTRIBUTE_BUFFER_SIZE - 1];
} SMART_READ_DATA_OUTDATA, *PSMART_READ_DATA_OUTDATA;

typedef	struct _SMART_ATTRIBUTE
{
	BYTE	Id;
	WORD	StatusFlags;
	BYTE	CurrentValue;
	BYTE	WorstValue;
	BYTE	RawValue[6];
	BYTE	Reserved;
} SMARTAttribute;

typedef	struct _IDENTIFY_DEVICE_OUTDATA
{
	SENDCMDOUTPARAMS	SendCmdOutParam;
	BYTE				Data[IDENTIFY_BUFFER_SIZE - 1];
} IDENTIFY_DEVICE_OUTDATA, *PIDENTIFY_DEVICE_OUTDATA;


#pragma pack(pop)

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
