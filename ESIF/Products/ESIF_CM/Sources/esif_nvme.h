/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#if defined(ESIF_ATTR_OS_WINDOWS)
#include <Ntddscsi.h>
#include <winioctl.h>

#define STORAGE_IDENTIFY_OUTPUT_BUFFER_SIZE (4 * 1024)
#define STORAGE_DEVICE_DESCRIPTOR_OUTPUT_BUFFER_SIZE (4 * 1024)
#define LOG_PAGE_BUFFER_SIZE 512
#define LOG_PAGE_TEMPERATURE_OFFSET 1
#define IDENTIFY_PDL_OFFSET 263
#define POWER_STATES_OFFSET 2048
#define POWER_STATES_MULTIPLIER_OFFSET 3
#define POWER_STATES_SIZE 32
#define SATA_NUM_EFFECTIVE_STATES 3
#define SATA_PERF_HIGH 236
#define SATA_PERF_MEDIUM 128
#define SATA_PERF_LOW 1

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
#define NVME_IDENTIFY_CNS_CONTROLLER 1
#define NVME_GET_LOG_PAGE_OPT 2
#define NVME_GET_IDENTIFY_PAGE_OPT 6
#define NVME_NAMESPACE_NA 0xFFFFFFFF
#define NVME_LOG_PAGE_IDENTIFIER_SMART 0x00000002
#define NVME_LOG_PAGE_IDENTIFIER_SMART_RST 0x7f0002
#define INTEL_NVME_PASSTHROUGH_VERSION 1
#define INTEL_NVME_PASSHTROUGH_HEADER_SIZE 28

#define DRIVE_HEAD_REGISTER 160
#define DFP_SEND_DRIVE_COMMAND 0x0007c084
#define DFP_RECEIVE_DRIVE_DATA 0x0007c088
#define ATA_NUM_POWER_STATES 254
#define ATA_MIN_POWER_STATE 1
#define MAX_ATA_SMART_ATTRIBUTES 30
#define SETFEATURES_EN_APM 0x05

#define NUMBER_PPSS_FIELDS 8
#define POWER_FIELD_INDEX 5
#define UNIT_FIELD_INDEX 7
#define INDEX_TO_COUNT_OFFSET 1

#define SMART_ATTRIBUTE_TEMPERATURE 0XBE
#define ATA_SETFEATURES 0xEF
#define ATA_SET_PERFORMANCE_LEVEL 0xCF
#define ATA_SETFEATURES_TIMEOUT 2
#define TASK_DEFINITION_INDEX 0
#define TASK_DEFINITION_VALUE_INDEX 1
#define TASK_DEFINITION_TARGET_INDEX 5
#define TASK_DEFINITION_TYPE_INDEX 6
#define INTEL_NVME_TIMEOUT 60
#define INTELRMP_SIGNATURE "IntelRmp"
#define INTELNVM_SIGNATURE "IntelNvm"
#define IOCTL_REMAPPORT_GET_PATHIDS_IMPLEMENTED 0x80000D02
#define IOCTL_REMAPPORT_GET_TARGETIDS_IMPLEMENTED   0x80000D03
#define IOCTL_INTEL_NVME_PASSTHROUGH 0xF0002808
#define TEMPERATURE_THRESHOLD 0x04
#define IOCTL_NVME_PASS_THROUGH \
    CTL_CODE(0xF000u, 0xA02, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define AEN_MAX_EVENT_NAME_LENGTH 32
#define NVME_GET_AER_DATA_MAX_COMPLETIONS 10
#define IOCTL_NVME_REGISTER_AER \
    CTL_CODE(0xF000u, 0xC00, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NVME_GET_AER_DATA \
    CTL_CODE(0xF000u, 0xE00, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define NVME_GLOBAL_NAMESPACE_ID (0xffffffff)
#define NVME_GLOBAL_NAMESPACE_ID_DEVICE_NAME (0x0)

// IOCTL Control Codes (IoctlHeader.ControlCode)
// Control Codes requiring CSMI_ALL_SIGNATURE
#define CC_CSMI_SAS_GET_DRIVER_INFO 1                   /**< RST SUPPORTED */
#define CC_CSMI_SAS_GET_RAID_INFO 10                    /**< RST SUPPORTED */
#define CC_CSMI_SAS_GET_RAID_CONFIG 11                  /**< RST SUPPORTED */

// Signature value
// (IoctlHeader.Signature)
#define CSMI_ALL_SIGNATURE "CSMIALL"                    /**< RST SUPPORTED - CC_CSMI_SAS_GET_DRIVER_INFO, CC_CSMI_SAS_GET_CNTLR_STATUS, CC_CSMI_SAS_GET_CNTLR_CONFIG */
#define CSMI_RAID_SIGNATURE "CSMIARY"                   /**< RST SUPPORTED - CC_CSMI_SAS_GET_RAID_INFO, CC_CSMI_SAS_GET_RAID_CONFIG */

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

typedef struct Opcode_s {
	union {
		struct {
			UInt32 DataTransfer : 2;
			UInt32 Function : 5;
			UInt32 GenericCommand : 1;
		}OpcodeStruct;

		UInt8 Raw;
	}OpcodeUnion;
}Opcode, *OpcodePtr;

typedef struct CommandDword0_s {
	Opcode OPC;
}CommandDword0, *CommandDword0Ptr;

struct GENERIC_COMMAND {
	UInt32				CDW0;
	UInt32              NSID;
	UInt32              CDW2;
	UInt32              CDW3;
	UInt32              CDW4;
	UInt32              CDW5;
	UInt64              PRP1;
	UInt64              PRP2;
	UInt32              CDW10;
	UInt32              CDW11;
	UInt32              CDW12;
	UInt32              CDW13;
	UInt32              CDW14;
	UInt32              CDW15;
};

struct ADMIN_ABORT {
	UInt16 SQID;
	UInt16 CID;
	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

struct ADMIN_ASYNCHRONOUS_EVENT_INFORMATION {
	UInt32 CDW10;
	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

struct ADMIN_CREATE_IO_COMPLETION_QUEUE {
	UInt16 QID;           // Queue Identifier
	UInt16 QSIZE;         // Queue Size

	UInt32 PC : 1;  // Physically Contiguous
	UInt32 IEN : 1;  // Interrupts Enabled
	UInt32 Reserved : 14;
	UInt16 IV;            // Interrupt Vector

	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

struct ADMIN_CREATE_IO_SUBMISSION_QUEUE {
	UInt16 QID;           // Queue Identifier
	UInt16 QSIZE;         // Queue Size

	UInt32 PC : 1;  // Physically Contiguous
	UInt32 QPRIO : 2;  // Queue Priority
	UInt32 Reserved : 13;
	UInt16 CQID;          // Completion Queue Identifier

	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

struct ADMIN_DELETE_IO_COMPLETION_QUEUE {
	UInt16 QID;
	UInt16 Reserved;

	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// ********** ADMIN COMMAND SET DEFINITION **************** //
#define ADMIN_COMMAND_SET_FEATURES                 0x09
#define ADMIN_COMMAND_GET_FEATURES                 0x0A
#define ADMIN_COMMAND_IDENTIFY                     0x06

// Section 5.6
struct ADMIN_DELETE_IO_SUBMISSION_QUEUE {
	UInt16 QID;
	UInt16 Reserved;

	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// Section 5.7
struct ADMIN_FIRMWARE_COMMIT {
	UInt32 FS : 3;         // Firmware Slot
	UInt32 CA : 3;         // Commit Action
	UInt32 Reserved : 26;

	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// Section 5.8
struct ADMIN_FIRMWARE_IMAGE_DOWNLOAD {
	UInt32 NUMD;
	UInt32 OFST;

	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// Section 5.10
struct ADMIN_GET_LOG_PAGE {
	UInt32 LID : 8;
	UInt32 LSP : 4;
	UInt32 Reserved : 3;
	UInt32 RAE : 1;
	UInt32 NUMDL : 16;

	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// Section 5.11
struct ADMIN_IDENTIFY {
	UInt32 CNS : 8;
	UInt32 Reserved : 8;
	UInt32 CNTID : 16;

	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

#define ADMIN_IDENTIFY_CNS_CONTROLLER      0x1

struct ADMIN_FEATURES_ARBITRATION
{
	UInt32   AB : 3;
	UInt32   Reserved : 5;
	UInt32   LPW : 8;
	UInt32   MPW : 8;
	UInt32   HPW : 8;
};

struct ADMIN_FEATURES_POWER_MANAGEMENT
{
	UInt32 PS : 5;
	UInt32 WH : 3;
	UInt32 Reserved : 24;
};

struct ADMIN_FEATURES_LBA_RANGE_TYPE_COMPLETION_CDW0
{
	UInt32 NUM : 6;
	UInt32 Reserved : 26;
};

union ADMIN_FEATURES_TEMPERATURE_THRESHOLD
{
	struct {
		UInt32 TMPTH : 16;     // Temperature Threshold
		UInt32 TMPSEL : 4;     // Threshold Temperature Select
		UInt32 THSEL : 2;      // Threshold Type Select
		UInt32 Reserved : 10;
	}a;
	UInt32 Raw;
};

struct ADMIN_FEATURES_ERROR_RECOVERY
{
	UInt32 TLER : 16;
	UInt32 DUBLE : 1;
	UInt32 Reserved : 15;
};

struct ADMIN_FEATURES_VOLATILE_WRITE_CACHE
{
	UInt32 WCE : 1;
	UInt32 Reserved : 31;
};

struct ADMIN_FEATURES_NUMBER_OF_QUEUES {
	UInt32   NCQR : 16;
	UInt32   NSQR : 16;
};

struct ADMIN_FEATURES_INTERRUPT_COALESCING
{
	UInt32 THR : 8;
	UInt32 Time : 8;
	UInt32 Reserved : 16;
};

struct ADMIN_FEATURES_INTERRUPT_VECTOR_CONFIGURATION
{
	UInt32 IV : 16;
	UInt32 CD : 1;
	UInt32 Reserved : 15;
};

struct ADMIN_FEATURES_WRITE_ATOMICITY
{
	UInt32  DN : 1;
	UInt32 Reserved : 31;
};

union ADMIN_FEATURES_ASYNCHRONOUS_EVENT_CONFIGURATION
{
	struct {
		UInt32 SMART : 8;
		UInt32 NamespaceAttributeNotices : 1;
		UInt32 FirmwareActivationNotices : 1;
		UInt32 Reserved : 22;
	}a;
	UInt32 Mask;
};

struct ADMIN_FEATURES_AUTONOMOUS_POWER_STATE_TRANSITION_CDW0
{
	UInt32 APSTE : 1;
	UInt32 Reserved : 31;
};

struct ADMIN_FEATURES_SOFTWARE_PROGRESS_MARKER
{
	UInt32 PBSLC : 8;
	UInt32 Reserved : 24;
};

struct ADMIN_FEATURES_HOST_MEMORY_BUFFER {
	UInt32   EHM : 1; // Enable Host Memory
	UInt32   MR : 1;  // Memory Return
	UInt32   Reserved : 30;
};

struct ADMIN_SET_FEATURES {
	UInt32 FID : 8;
	UInt32 Reserved : 23;
	UInt32 SV : 1;

	union {
		struct ADMIN_FEATURES_ARBITRATION CommandArbitration;
		struct ADMIN_FEATURES_POWER_MANAGEMENT PowerManagement;
		struct ADMIN_FEATURES_LBA_RANGE_TYPE_COMPLETION_CDW0 LbaRangeType;
		union ADMIN_FEATURES_TEMPERATURE_THRESHOLD TemperatureThreshold;
		struct ADMIN_FEATURES_ERROR_RECOVERY ErrorRecovery;
		struct ADMIN_FEATURES_VOLATILE_WRITE_CACHE VolatileWriteCache;
		struct ADMIN_FEATURES_NUMBER_OF_QUEUES NumberOfQueues;
		struct ADMIN_FEATURES_INTERRUPT_COALESCING InterruptCoalescing;
		struct ADMIN_FEATURES_INTERRUPT_VECTOR_CONFIGURATION InterruptVectorConfiguration;
		struct ADMIN_FEATURES_WRITE_ATOMICITY WriteAtomicity;
		union ADMIN_FEATURES_ASYNCHRONOUS_EVENT_CONFIGURATION AsynchronousEventConfiguration;
		struct ADMIN_FEATURES_AUTONOMOUS_POWER_STATE_TRANSITION_CDW0 AutonomousPowerStateTransition;
		struct ADMIN_FEATURES_SOFTWARE_PROGRESS_MARKER SoftwareProgressMarker;
		struct ADMIN_FEATURES_HOST_MEMORY_BUFFER HostMemoryBuffer;

		UInt32 CDW11;
	}a;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// Section 5.9
struct ADMIN_GET_FEATURES {
	UInt32 FID : 8;
	UInt32 SEL : 3;
	UInt32 Reserved : 21;

	union {
		struct ADMIN_FEATURES_ARBITRATION CommandArbitration;
		struct ADMIN_FEATURES_POWER_MANAGEMENT PowerManagement;
		struct ADMIN_FEATURES_LBA_RANGE_TYPE_COMPLETION_CDW0 LbaRangeType;
		union ADMIN_FEATURES_TEMPERATURE_THRESHOLD TemperatureThreshold;
		struct ADMIN_FEATURES_ERROR_RECOVERY ErrorRecovery;
		struct ADMIN_FEATURES_VOLATILE_WRITE_CACHE VolatileWriteCache;
		struct ADMIN_FEATURES_NUMBER_OF_QUEUES NumberOfQueues;
		struct ADMIN_FEATURES_INTERRUPT_COALESCING InterruptCoalescing;
		struct ADMIN_FEATURES_INTERRUPT_VECTOR_CONFIGURATION InterruptVectorConfiguration;
		struct ADMIN_FEATURES_WRITE_ATOMICITY WriteAtomicity;
		union ADMIN_FEATURES_ASYNCHRONOUS_EVENT_CONFIGURATION AsynchronousEventConfiguration;
		struct ADMIN_FEATURES_AUTONOMOUS_POWER_STATE_TRANSITION_CDW0 AutonomousPowerStateTransition;
		struct ADMIN_FEATURES_SOFTWARE_PROGRESS_MARKER SoftwareProgressMarker;

		UInt32 CDW11;
	}a;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

struct ADMIN_FORMAT_NVM_COMMAND {
	UInt32 LBAF : 4;
	UInt32 MS : 1;
	UInt32 PI : 3;
	UInt32 IPL : 1;
	UInt32 SES : 3;
	UInt32 Reserved : 20;

	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

struct ADMIN_SECURITY_SEND_COMMAND {
	UInt32 Reserved : 8;
	UInt32 SPSP : 16;
	UInt32 SECP : 8;

	UInt32 AL;

	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

struct ADMIN_SECURITY_RECEIVE_COMMAND {
	UInt32 Reserved : 8;
	UInt32 SPSP : 16;
	UInt32 SECP : 8;

	UInt32 AL;

	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// section 5.7
struct ADMIN_DEVICE_SELF_TEST_COMMAND {
	UInt32 STC : 4;
	UInt32 Reserved : 28;

	UInt32 CDW11;
	UInt32 CDW12;
	UInt32 CDW13;
	UInt32 CDW14;
	UInt32 CDW15;
};

// Section 4.2, Figure 11
typedef struct AdminCommand_s {
	CommandDword0       CDW0;
	UInt32              NSID;
	UInt64              Reserved;
	UInt64              MPTR;
	UInt64              PRP1;
	UInt64              PRP2;

	union {
		struct ADMIN_ABORT AdminAbort;
		struct ADMIN_ASYNCHRONOUS_EVENT_INFORMATION AsynchronousEventInformation;
		struct ADMIN_CREATE_IO_COMPLETION_QUEUE CreateIoCompletionQueue;
		struct ADMIN_CREATE_IO_SUBMISSION_QUEUE CreateIoSubmissionQueue;
		struct ADMIN_DELETE_IO_COMPLETION_QUEUE DeleteIoCompletionQueue;
		struct ADMIN_DELETE_IO_SUBMISSION_QUEUE DeleteIoSubmissionQueue;
		struct ADMIN_FIRMWARE_COMMIT FirmwareCommit;
		struct ADMIN_FIRMWARE_IMAGE_DOWNLOAD FirmwareImageDownload;
		struct ADMIN_GET_FEATURES GetFeatures;
		struct ADMIN_GET_LOG_PAGE GetLogPage;
		struct ADMIN_IDENTIFY Identify;
		struct ADMIN_SET_FEATURES SetFeatures;
		struct ADMIN_FORMAT_NVM_COMMAND FormatNvmCommand;
		struct ADMIN_SECURITY_SEND_COMMAND SecuritySendCommand;
		struct ADMIN_SECURITY_RECEIVE_COMMAND SecurityReceiveCommand;
		struct ADMIN_DEVICE_SELF_TEST_COMMAND DeviceSelfTest;

		struct {
			UInt32 CDW10;
			UInt32 CDW11;
			UInt32 CDW12;
			UInt32 CDW13;
			UInt32 CDW14;
			UInt32 CDW15;
		} Raw;
	}a;

}AdminCommand, *AdminCommandPtr;

struct COMPLETION_QUEUE_ENTRY {
	u32 DW0_Command_Specific;
	u32 DW1_Reserved;
	u32 DW2_SQuid_SQHead;
	u32 DW3_StatusField_CommmandID;
};

typedef struct AdminAsynchronousEventRequestCompletionDw0_s {
	union {
		struct {
			UInt32   AsynchronousEventType : 3;
			UInt32   Reserved1 : 5;
			UInt32   AsynchronousEventInformation : 8;
			UInt32   AssociatedLogPage : 8;
			UInt32   Reserved2 : 8;
		} AdminAsynchronousEventRequestCompletionDw0s;
		UInt32 Raw;
	} AdminAsynchronousEventRequestCompletionDw0u;
} AdminAsynchronousEventRequestCompletionDw0, *AdminAsynchronousEventRequestCompletionDw0Ptr;

typedef struct AdminIdentifyControllerData_s {
	UInt16  VID;
	UInt16  SSVID;
	UInt8   SN[20];
	UInt8   MN[40];
	UInt8   FR[8];
	UInt8   RAB;
	UInt8   IEEE[3];
	UInt8   Data[190];
	UInt16  WCTemp;
	UInt8   VS[3828];
}AdminIdentifyControllerData, *AdminIdentifyControllerDataPtr;

#pragma pack(pop)

typedef struct
{
	ATA_PASS_THROUGH_EX Apt;
	DWORD Filer;
	BYTE  Buf[512];
} ATA_PASS_THROUGH_EX_WITH_BUFFERS;

struct NVME_PASS_THROUGH_PARAMETERS {
	struct GENERIC_COMMAND Command;
	BOOLEAN IsIOCommandSet;
	struct COMPLETION_QUEUE_ENTRY Completion;
	ULONG DataBufferOffset;
	ULONG DataBufferLength;
	ULONG Reserved[10];
};

struct NVME_IOCTL_PASS_THROUGH {
	SRB_IO_CONTROL Header;
	UCHAR Version;
	UCHAR PathID;
	UCHAR TargetID;
	UCHAR Lun;
	struct NVME_PASS_THROUGH_PARAMETERS Parameters;
};

struct NVME_IOCTL_PASS_THROUGH_WITH_DATA {
	struct NVME_IOCTL_PASS_THROUGH Ioctl;
	AdminIdentifyControllerData Data;
	UCHAR Padding[4];
};

typedef struct AsynchronousNotificationEventInfo_s {
	HANDLE eventHandle;
	esif_string eventName;
	int scsiPort;
	int scbl;
	char *scsiPath;
}AsynchronousNotificationEventInfo, *AsynchronousNotificationEventInfoPtr;

typedef struct RaidportRegisterSharedEvent_s {
	UInt8  eventName[AEN_MAX_EVENT_NAME_LENGTH];
	UInt8  reserved;
	UInt64 eventMask;
} RaidportRegisterSharedEvent, *RaidportRegisterSharedEventPtr;

typedef struct NvmeIoctlRegisterAer_s {
	SRB_IO_CONTROL Header;
	RaidportRegisterSharedEvent EventData;
}NvmeIoctlRegisterAer, *NvmeIoctlRegisterAerPtr;

typedef struct NvmeAerData_s {
	UInt8  eventName[AEN_MAX_EVENT_NAME_LENGTH];
	UInt8  reserved;
	AdminAsynchronousEventRequestCompletionDw0 Completions[NVME_GET_AER_DATA_MAX_COMPLETIONS];
	UInt32 CompletionsCount;
}NvmeAerData, *NvmeAerDataPtr;

typedef struct NvmeIoctlGetAerData_s {
	SRB_IO_CONTROL Header;
	NvmeAerData Data;
}NvmeIoctlGetAerData, *NvmeIoctlGetAerDataPtr;

typedef struct CsmiSasDriverInfo_s {
	UInt8 szName[81];			/**< RST driver file name */
	UInt8 szDescription[81];	/**< RST driver description */
	UInt16 usMajorRevision;		/**< RST driver MAJOR version  - X.y.z.bbbb */
	UInt16 usMinorRevision;		/**< RST driver MINOR version  - x.Y.z.bbbb */
	UInt16 usBuildRevision;		/**< RST driver HOT FIX number - x.y.Z.bbbb */
	UInt16 usReleaseRevision;	/**< RST driver BUILD number   - x.y.z.BBBB */
	UInt16 usCSMIMajorRevision;	/**< RST driver supports this MAJOR revision of the CSMI specification - X.y */
	UInt16 usCSMIMinorRevision;	/**< RST driver supports this MINOR revision of the CSMI specification - x.Y */
}CsmiSasDriverInfo, *CsmiSasDriverInfoPtr;

typedef struct CsmiSasDriverInfoBuffer_s {
	SRB_IO_CONTROL Header;
	CsmiSasDriverInfo Information;
}CsmiSasDriverInfoBuffer, *CsmiSasDriverInfoBufferPtr;

typedef struct CsmiSasRaidInfo_s {
	UInt32 uNumRaidSets;		/**< Number of defined RST RAID volumes */
	UInt32 uMaxDrivesPerSet;	/**< Maximum number of drives in a RST volume */
	UInt8 bReserved[92];
}CsmiSasRaidInfo, *CsmiSasRaidInfoPtr;

typedef struct CsmiSasRaidInfoBuffer_s {
	SRB_IO_CONTROL Header;
	CsmiSasRaidInfo Information;
}CsmiSasRaidInfoBuffer, *CsmiSasRaidInfoBufferPtr;

typedef struct CsmiSasRaidDrives_s {
	UInt8 bModel[40];			/**< Device model number */
	UInt8 bFirmware[8];			/**< Device firmware revision */
	UInt8 bSerialNumber[40];	/**< Device serial number */
	UInt8 bSASAddress[8];		/**< [0] Device lun,
								[1] Device target ID,
								[2] Device bus ID,
								[3] Device bus ID (for legacy applications) */
	UInt8 bSASLun[8];			/**< NOT IMPLEMENTED */
	UInt8 bDriveStatus;			/**< CSMI_SAS_DRIVE_STATUS_OK, CSMI_SAS_DRIVE_STATUS_REBUILDING, CSMI_SAS_DRIVE_STATUS_DEGRADED or DISK_SMART_EVENT_TRIGGERED */
	UInt8 bDriveUsage;			/**< RST drive usage CSMI_SAS_DRIVE_CONFIG_NOT_USED,
								CSMI_SAS_DRIVE_CONFIG_MEMBER or
								CSMI_SAS_DRIVE_CONFIG_SPARE */
	UInt8 bReserved[30];
}CsmiSasRaidDrives, *CsmiSasRaidDrivesPtr;

typedef struct CsmiSasRaidConfig_s {
	UInt32 uRaidSetIndex;	/**< RST volume index requested/returned is zero based array */
	UInt32 uCapacity;		/**< RST volume capacity in MB */
	UInt32 uStripeSize;		/**< RST stripe size in KB */
	UInt8 bRaidType;		/**< CSMI_SAS_RAID_TYPE_0, 1, 5, 10 or CSMI_SAS_RAID_TYPE_OTHER */
	UInt8 bStatus;			/**< CSMI_SAS_RAID_SET_STATUS_OK         if RST volume == "Normal"
							CSMI_SAS_RAID_SET_STATUS_DEGRADED   if RST volume == "Degraded"
							CSMI_SAS_RAID_SET_STATUS_FAILED     if RST volume == "Failed"
							CSMI_SAS_RAID_SET_STATUS_REBUILDING if RST volume == "Initializing", "Rebuild", "Verifying" or "Migrating" */
	UInt8 bInformation;		/**< No data 0x00                                if RST volume == "Normal"
							Drive index drive causing the degradation   if RST volume == "Degraded"
							Drive index of the failed drive             if RST volume == "Failed"
							0-100 (percent done)                        if RST volume == "Initializing/Rebuilding" */
	UInt8 bDriveCount;		/**< RST number of disks in the volume */
	UInt8 bReserved[20];
	CsmiSasRaidDrives Drives[5];
}CsmiSasRaidConfig, *CsmiSasRaidConfigPtr;

typedef struct CsmiSasRaidConfigBuffer_s {
	SRB_IO_CONTROL Header;
	CsmiSasRaidConfig Configuration;
}CsmiSasRaidConfigBuffer, *CsmiSasRaidConfigBufferPtr;

typedef enum _AER_COMPLETION_EVENT_TYPES {
	AE_TYPE_ERROR_STATUS = 0,
	AE_TYPE_SMART_HEALTH_STATUS = 1,
	AE_TYPE_NOTICE = 2,
	AE_TYPE_IO_CMD_SET_SPECIFIC_STATUS = 6,
	AE_TYPE_VENDOR_SPECIFIC = 7
} AER_COMPLETION_EVENT_TYPE;

typedef enum _AER_COMPLETION_EVENT_INFO_FOR_SMARTS {
	AE_SMART_INFO_NVM_SUBSYSTEM_RELIABILITY = 0,
	AE_SMART_INFO_TEMPERATURE_EXCEEDED_THRESHOLD = 1,
	AE_SMART_INFO_SPARE_CAPACITY_BELOW_THRESHOLD = 2
} AER_COMPLETION_EVENT_INFO_FOR_SMART;

#endif // ESIF_ATTR_OS_WINDOWS



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
