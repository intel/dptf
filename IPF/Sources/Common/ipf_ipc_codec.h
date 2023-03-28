/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#pragma once

#include "esif_ccb_memory.h"
#include "esif_ccb_sem.h"
#include "esif_ccb_string.h"
#include "esif_ccb_socket.h"
#include "esif_ccb_thread.h"
#include "esif_ccb_time.h"
#include "esif_sdk_iface_app.h"
#include "esif_sdk_message.h"
#include "ipf_common.h"
#include "ipf_ibinary.h"
#include "ipf_ipc_clisrv.h"

#define	IPFCONSOLE(fmt, ...)	fprintf(stderr, fmt, ##__VA_ARGS__)

#ifdef ESIF_ATTR_DEBUG
# define IPFDEBUG(fmt, ...)	(0)
//# define IPFDEBUG(fmt, ...)	IPFCONSOLE(fmt, ##__VA_ARGS__)
#else
# define IPFDEBUG(fmt, ...)	(0)
#endif

typedef enum _t_IrpcFunction {
	IrpcFunc_None = 0,

	/* TODO: IpfInterface = 0x01-0x1F (31 max) i.e, Authentication, Controls, Signals, etc. */

	/* EsifInterface = 0x20-0x3F (32 max) */
	IrpcFunc_EsifGetConfig = 0x20,
	IrpcFunc_EsifSetConfig,

	IrpcFunc_EsifPrimitive,
	IrpcFunc_EsifWriteLog,

	IrpcFunc_EsifEventRegister,
	IrpcFunc_EsifEventUnregister,

	IrpcFunc_EsifSendEvent,
	IrpcFunc_EsifSendCommand,

	/* AppInterface = 0x40-0x5F (32 max) */
	IrpcFunc_AppCreate = 0x40,
	IrpcFunc_AppDestroy,
	IrpcFunc_AppSuspend,
	IrpcFunc_AppResume,
	IrpcFunc_AppGetVersion,
	IrpcFunc_AppCommand,
	IrpcFunc_AppGetIntro,
	IrpcFunc_AppGetDescription,
	IrpcFunc_AppGetName,
	IrpcFunc_AppGetStatus,

	IrpcFunc_AppParticipantCreate,
	IrpcFunc_AppParticipantDestroy,

	IrpcFunc_AppDomainCreate,
	IrpcFunc_AppDomainDestroy,

	IrpcFunc_AppEvent,

	/* Reserved for Future Interaces (WebServer, Conjure, Action, Transport, etc) */
	/* Unused: 0x60-0x7F (32 max) */
	/* Unused: 0x80-0x9F (32 max) */
	/* Unused: 0xA0-0xBF (32 max) */
	/* Unused: 0xC0-0xDF (32 max) */
	/* Unused: 0xE0-0xFF (32 max) - Reserved for After-Market IRPC Extensions */
} eIrpcFunction;

/*
** Template Macros for creating Encoders for Simple Data Types
*/
#define IRPC_DECLARE_ENCODER(TYPENAME, NATIVETYPE, SERIALTYPE, TRANSFORM) \
\
typedef SERIALTYPE Encoded_##TYPENAME/*, *Encoded_##TYPENAME##Ptr*/; \
\
static ESIF_INLINE SERIALTYPE Irpc_Cast_##TYPENAME(NATIVETYPE data) \
{ \
	return ((SERIALTYPE)TRANSFORM((SERIALTYPE)data)); \
} \
\
static ESIF_INLINE NATIVETYPE Irpc_Uncast_##TYPENAME(SERIALTYPE data) \
{ \
	return ((NATIVETYPE)TRANSFORM(data)); \
} \
\
static SERIALTYPE *Irpc_Encode_##TYPENAME(IBinary *blob, NATIVETYPE data) \
{ \
	SERIALTYPE encoded = Irpc_Cast_##TYPENAME(data); \
	return (SERIALTYPE *)IBinary_Append(blob, &encoded, sizeof(encoded)); \
} \
static NATIVETYPE *Irpc_Decode_##TYPENAME(NATIVETYPE *into, SERIALTYPE *encoded) \
{ \
	if (encoded) { \
		NATIVETYPE decoded = Irpc_Uncast_##TYPENAME(*(encoded)); \
		if (into == NULL) { \
			into = esif_ccb_malloc(sizeof(*into)); \
			if (into == NULL) { \
				return NULL; \
			} \
		} \
		*into = decoded; \
	} \
	return into; \
}

typedef enum IrpcMsgType_e {
	IrpcMsg_None = 0,
	IrpcMsg_ProcRequest,	// RPC Remote Procedure Call
	IrpcMsg_ProcResponse,	// RPC Remote Procedure Call Response
	IrpcMsg_AuthRequest,	// Authentication Request [Placeholder]
	IrpcMsg_AuthResponse,	// Authentiction Response [Placeholder]
} eIrpcMsgType;

// IPF RPC Transaction
typedef struct IrpcTransaction_s {
	esif_handle_t		ipfHandle;	// Unique IPF Client Session Handle
	atomic_t			refCount;	// Reference Count
	UInt64				trxId;		// Unique Transaction ID
	esif_ccb_realtime_t	timestamp;	// Real-Time Timestamp that Transaction was Created
	signal_t			sync;		// Synchronization Object for Inter-Thread Signals
	IBinary				*request;	// Serialized Request Message, if any
	IBinary				*response;	// Serialized Response Message, if any
	esif_error_t		result;		// RPC Request Result
} IrpcTransaction;

/* Definition of these compiler options control how simple native types are encoded in RPC calls
**
** IRPC_ENUM_COMPRESS		= Compress encoded enum types to use fewer bytes (1-4 bytes) [default = Serialze enums as 4-byte integers]
** IRPC_SERIALIZE_64BIT		= Serialize all 1-8 byte integers as 8-bytes [default = Serialize using native size]
** IRPC_NETWORK_BYTE_ORDER	= Serialize 2-8 byte integers using network byte order (Big-Endian) [default=Little Endian]
**
** NOTE: EnumHuge values are not supported on Linux since enum values > 0xFFFFFFFF are unsupported
*/

//#define IRPC_ENUM_COMPRESS

#if defined(IRPC_ENUM_COMPRESS)
typedef UInt8	Encoded_EnumTiny;	// Enum Types <= 0xFF
typedef UInt16	Encoded_EnumShort;	// Enum Types <= 0xFFFF
typedef UInt32	Encoded_EnumLong;	// Enum Types <= 0xFFFFFFFF
typedef UInt64	Encoded_EnumHuge;	// Enum Types <= 0xFFFFFFFFFFFFFFFF
#define ByteOrderEnumTiny(i)		(UInt8)(i)
#define ByteOrderEnumShort(i)		esif_ccb_htons((UInt16)(i))
#define ByteOrderEnumLong(i)		esif_ccb_htonl((UInt32)(i))
#define ByteOrderEnumHuge(i)		esif_ccb_htonll((UInt64)(i))
#elif defined(IRPC_SERIALIZE_64BIT)
typedef UInt64	Encoded_EnumTiny;	// Enum Types <= 0xFF
typedef UInt64	Encoded_EnumShort;	// Enum Types <= 0xFFFF
typedef UInt64	Encoded_EnumLong;	// Enum Types <= 0xFFFFFFFF
typedef UInt64	Encoded_EnumHuge;	// Enum Types <= 0xFFFFFFFFFFFFFFFF
#define ByteOrderEnumTiny(i)		esif_ccb_htonll((UInt64)(i))
#define ByteOrderEnumShort(i)		esif_ccb_htonll((UInt64)(i))
#define ByteOrderEnumLong(i)		esif_ccb_htonll((UInt64)(i))
#define ByteOrderEnumHuge(i)		esif_ccb_htonll((UInt64)(i))
#else
typedef UInt32	Encoded_EnumTiny;	// Enum Types <= 0xFF
typedef UInt32	Encoded_EnumShort;	// Enum Types <= 0xFFFF
typedef UInt32	Encoded_EnumLong;	// Enum Types <= 0xFFFFFFFF
typedef UInt64	Encoded_EnumHuge;	// Enum Types <= 0xFFFFFFFFFFFFFFFF
#define ByteOrderEnumTiny(i)		esif_ccb_htonl((UInt32)(i))
#define ByteOrderEnumShort(i)		esif_ccb_htonl((UInt32)(i))
#define ByteOrderEnumLong(i)		esif_ccb_htonl((UInt32)(i))
#define ByteOrderEnumHuge(i)		esif_ccb_htonll((UInt64)(i))
#endif

// Must cast (void*)->(size_t)->(UInt64) to avoid x86 conversion errors and compiler warnings.
// Also do not do network byte ordering conversion on pointers.
#define XformPointer(i)		((UInt64)(size_t)(i))

#ifdef IRPC_NETWORK_BYTE_ORDER
#define XformBool(i)		(Bool)(i)
#define XformUInt8(i)		(UInt8)(i)
#define XformUInt16(i)		esif_ccb_htons((UInt16)(i))
#define XformUInt32(i)		esif_ccb_htonl((UInt32)(i))
#define XformUInt64(i)		esif_ccb_htonll((UInt64)(i))
#define XformHandle(i)		XformUInt64(i)
#define XformEnumTiny(i)	ByteOrderEnumTiny(i)
#define XformEnumShort(i)	ByteOrderEnumShort(i)
#define XformEnumLong(i)	ByteOrderEnumLong(i)
#define XformEnumHuge(i)	ByteOrderEnumHuge(i)
#else
#define XformBool(i)		(i)
#define XformUInt8(i)		(i)
#define XformUInt16(i)		(i)
#define XformUInt32(i)		(i)
#define XformUInt64(i)		(i)
#define XformHandle(i)		(i)
#define XformEnumTiny(i)	(i)
#define XformEnumShort(i)	(i)
#define XformEnumLong(i)	(i)
#define XformEnumHuge(i)	(i)
#endif

// Declare Encoders for Simple Data Types
#ifdef IRPC_SERIALIZE_64BIT
IRPC_DECLARE_ENCODER(Bool, Bool, UInt64, XformUInt64)
IRPC_DECLARE_ENCODER(UInt8, UInt8, UInt64, XformUInt64)
IRPC_DECLARE_ENCODER(UInt16, UInt16, UInt64, XformUInt64)
IRPC_DECLARE_ENCODER(UInt32, UInt32, UInt64, XformUInt64)
IRPC_DECLARE_ENCODER(UInt64, UInt64, UInt64, XformUInt64)
IRPC_DECLARE_ENCODER(EsifHandle, esif_handle_t, UInt64, XformHandle)
IRPC_DECLARE_ENCODER(EsifFlags, esif_flags_t, UInt64, XformUInt64)
IRPC_DECLARE_ENCODER(eEsifError, esif_error_t, Encoded_EnumShort, XformEnumShort)
IRPC_DECLARE_ENCODER(eIfaceType, eIfaceType, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eDataType, esif_data_type_t, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(ePrimitiveType, esif_primitive_type_t, Encoded_EnumShort, XformEnumShort)
IRPC_DECLARE_ENCODER(eLogType, eLogType, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eParticipantBus, eParticipantBus, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eAppState, eAppState, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eAppStatusCommand, eAppStatusCommand, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eParticipantState, eParticipantState, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eDomainState, eDomainState, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eDomainType, eDomainType, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eIrpcFunction, eIrpcFunction, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eIrpcMsgType, eIrpcMsgType, Encoded_EnumTiny, XformEnumTiny)
#else
IRPC_DECLARE_ENCODER(Bool, Bool, Bool, XformUInt8)
IRPC_DECLARE_ENCODER(UInt8, UInt8, UInt8, XformUInt8)
IRPC_DECLARE_ENCODER(UInt16, UInt16, UInt16, XformUInt16)
IRPC_DECLARE_ENCODER(UInt32, UInt32, UInt32, XformUInt32)
IRPC_DECLARE_ENCODER(UInt64, UInt64, UInt64, XformUInt64)
IRPC_DECLARE_ENCODER(EsifHandle, esif_handle_t, UInt64, XformHandle)
IRPC_DECLARE_ENCODER(esif_handle_t, esif_handle_t, UInt64, XformHandle)
IRPC_DECLARE_ENCODER(EsifFlags, esif_flags_t, UInt32, XformUInt32)
IRPC_DECLARE_ENCODER(esif_flags_t, esif_flags_t, UInt32, XformUInt32)
IRPC_DECLARE_ENCODER(eEsifError, esif_error_t, Encoded_EnumShort, XformEnumShort)
IRPC_DECLARE_ENCODER(esif_error_t, esif_error_t, Encoded_EnumShort, XformEnumShort)
IRPC_DECLARE_ENCODER(eIfaceType, eIfaceType, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eDataType, esif_data_type_t, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(esif_data_type_t, esif_data_type_t, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(ePrimitiveType, esif_primitive_type_t, Encoded_EnumShort, XformEnumShort)
IRPC_DECLARE_ENCODER(eLogType, eLogType, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eParticipantBus, eParticipantBus, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eAppState, eAppState, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eAppStatusCommand, eAppStatusCommand, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eParticipantState, eParticipantState, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eDomainState, eDomainState, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eDomainType, eDomainType, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eIrpcFunction, eIrpcFunction, Encoded_EnumTiny, XformEnumTiny)
IRPC_DECLARE_ENCODER(eIrpcMsgType, eIrpcMsgType, Encoded_EnumTiny, XformEnumTiny)
#endif

/*
** IRPC Helper Macros
*/

// TODO: Refactor these so that "offset" is offset from *start* of field, not end of field. That way, 0=NULL
#define Irpc_OffsetOf(obj, field)	(UInt32)((ptrdiff_t)((UInt8 *)(&(obj) + 1) - (UInt8 *)&((obj).field)) - sizeof((obj).field))
#define Irpc_OffsetFrom(obj, field)	((UInt8 *)&((obj).field) + sizeof((obj).field))

#define Irpc_Init_Array(cast, target, source, field) \
	do { \
		for (size_t j = 0; j < (sizeof((target).field)/sizeof((target).field[0])); j++) { \
			(target).field[j] = cast((source)->field[j]); \
		} \
	} while (0)

#define Irpc_Cast_EsifRealtime(data) \
	{ \
		.clockticks		= Irpc_Cast_UInt64((data).clockticks), \
		.clocktime		= Irpc_Cast_UInt64((data).clocktime), \
	}
#define Irpc_Uncast_EsifRealtime(data) \
	{ \
		.clockticks		= Irpc_Uncast_UInt64((data).clockticks), \
		.clocktime		= Irpc_Uncast_UInt64((data).clocktime), \
	}

#define Irpc_Cast_EsifData(data, offsetof) \
	{ \
		.type		= Irpc_Cast_eDataType((data).type), \
		.buf_len	= Irpc_Cast_UInt32((data).buf_len), \
		.data_len	= Irpc_Cast_UInt32((data).data_len), \
		.offset		= Irpc_Cast_UInt32(offsetof) \
	}
#define Irpc_Uncast_EsifData(data, ptr) \
	{ \
		.type		= Irpc_Uncast_eDataType((data)->type), \
		.buf_ptr	= ptr, \
		.buf_len	= Irpc_Uncast_UInt32((data)->buf_len), \
		.data_len	= Irpc_Uncast_UInt32((data)->data_len), \
	}

#define Irpc_Cast_EsifIfaceHdr(data) \
	{ \
		.fIfaceType		= Irpc_Cast_eIfaceType((data).fIfaceType), \
		.fIfaceVersion	= Irpc_Cast_UInt16((data).fIfaceVersion), \
		.fIfaceSize		= Irpc_Cast_UInt16((data).fIfaceSize), \
	}
#define Irpc_Uncast_EsifIfaceHdr(data) \
	{ \
		.fIfaceType		= Irpc_Uncast_eIfaceType((data).fIfaceType), \
		.fIfaceVersion	= Irpc_Uncast_UInt16((data).fIfaceVersion), \
		.fIfaceSize		= Irpc_Uncast_UInt16((data).fIfaceSize), \
	}

#pragma warning(disable:4152) // Temp - needed for IrpcId2Func
#define IrpcFuncPtr2Id(ptr, id)		((ptr) == NULL ? IrpcFunc_None : (id))
#define IrpcFuncId2Ptr(id, ptr)		((id) == IrpcFunc_None ? NULL : (ptr))
#define IrpcId2Func(id)				((void *)((id) == IrpcFunc_None ? (UInt64)0 : ((UInt64)(id) | 0x8000000000000000)))

#define Irpc_Cast_AppInterface(data) \
	{ \
		.fAppCreateFuncPtr = IrpcFuncPtr2Id((data).fAppCreateFuncPtr, IrpcFunc_AppCreate), \
		.fAppDestroyFuncPtr = IrpcFuncPtr2Id((data).fAppDestroyFuncPtr, IrpcFunc_AppDestroy), \
		.fAppSuspendFuncPtr = IrpcFuncPtr2Id((data).fAppSuspendFuncPtr, IrpcFunc_AppSuspend), \
		.fAppResumeFuncPtr = IrpcFuncPtr2Id((data).fAppResumeFuncPtr, IrpcFunc_AppResume), \
		.fAppGetVersionFuncPtr = IrpcFuncPtr2Id((data).fAppGetVersionFuncPtr, IrpcFunc_AppGetVersion), \
		.fAppCommandFuncPtr = IrpcFuncPtr2Id((data).fAppCommandFuncPtr, IrpcFunc_AppCommand), \
		.fAppGetIntroFuncPtr = IrpcFuncPtr2Id((data).fAppGetIntroFuncPtr, IrpcFunc_AppGetIntro), \
		.fAppGetDescriptionFuncPtr = IrpcFuncPtr2Id((data).fAppGetDescriptionFuncPtr, IrpcFunc_AppGetDescription), \
		.fAppGetNameFuncPtr = IrpcFuncPtr2Id((data).fAppGetNameFuncPtr, IrpcFunc_AppGetName), \
		.fAppGetStatusFuncPtr = IrpcFuncPtr2Id((data).fAppGetStatusFuncPtr, IrpcFunc_AppGetStatus), \
		.fParticipantCreateFuncPtr = IrpcFuncPtr2Id((data).fParticipantCreateFuncPtr, IrpcFunc_AppParticipantCreate), \
		.fParticipantDestroyFuncPtr = IrpcFuncPtr2Id((data).fParticipantDestroyFuncPtr, IrpcFunc_AppParticipantDestroy), \
		.fDomainCreateFuncPtr = IrpcFuncPtr2Id((data).fDomainCreateFuncPtr, IrpcFunc_AppDomainCreate), \
		.fDomainDestroyFuncPtr = IrpcFuncPtr2Id((data).fDomainDestroyFuncPtr, IrpcFunc_AppDomainDestroy), \
		.fAppEventFuncPtr = IrpcFuncPtr2Id((data).fAppEventFuncPtr, IrpcFunc_AppEvent), \
	}

#define Irpc_Uncast_AppInterface(data) \
	{ \
		.fAppCreateFuncPtr = IrpcFuncId2Ptr((data).fAppCreateFuncPtr, IrpcId2Func(IrpcFunc_AppCreate)), \
		.fAppDestroyFuncPtr = IrpcFuncId2Ptr((data).fAppDestroyFuncPtr, IrpcId2Func(IrpcFunc_AppDestroy)), \
		.fAppSuspendFuncPtr = IrpcFuncId2Ptr((data).fAppSuspendFuncPtr, IrpcId2Func(IrpcFunc_AppSuspend)), \
		.fAppResumeFuncPtr = IrpcFuncId2Ptr((data).fAppResumeFuncPtr, IrpcId2Func(IrpcFunc_AppResume)), \
		.fAppGetVersionFuncPtr = IrpcFuncId2Ptr((data).fAppGetVersionFuncPtr, IrpcId2Func(IrpcFunc_AppGetVersion)), \
		.fAppCommandFuncPtr = IrpcFuncId2Ptr((data).fAppCommandFuncPtr, IrpcId2Func(IrpcFunc_AppCommand)), \
		.fAppGetIntroFuncPtr = IrpcFuncId2Ptr((data).fAppGetIntroFuncPtr, IrpcId2Func(IrpcFunc_AppGetIntro)), \
		.fAppGetDescriptionFuncPtr = IrpcFuncId2Ptr((data).fAppGetDescriptionFuncPtr, IrpcId2Func(IrpcFunc_AppGetDescription)), \
		.fAppGetNameFuncPtr = IrpcFuncId2Ptr((data).fAppGetNameFuncPtr, IrpcId2Func(IrpcFunc_AppGetName)), \
		.fAppGetStatusFuncPtr = IrpcFuncId2Ptr((data).fAppGetStatusFuncPtr, IrpcId2Func(IrpcFunc_AppGetStatus)), \
		.fParticipantCreateFuncPtr = IrpcFuncId2Ptr((data).fParticipantCreateFuncPtr, IrpcId2Func(IrpcFunc_AppParticipantCreate)), \
		.fParticipantDestroyFuncPtr = IrpcFuncId2Ptr((data).fParticipantDestroyFuncPtr, IrpcId2Func(IrpcFunc_AppParticipantDestroy)), \
		.fDomainCreateFuncPtr = IrpcFuncId2Ptr((data).fDomainCreateFuncPtr, IrpcId2Func(IrpcFunc_AppDomainCreate)), \
		.fDomainDestroyFuncPtr = IrpcFuncId2Ptr((data).fDomainDestroyFuncPtr, IrpcId2Func(IrpcFunc_AppDomainDestroy)), \
		.fAppEventFuncPtr = IrpcFuncId2Ptr((data).fAppEventFuncPtr, IrpcId2Func(IrpcFunc_AppEvent)), \
	}

#define Irpc_Cast_EsifInterface(data) \
	{ \
		.fGetConfigFuncPtr = IrpcFuncPtr2Id((data).fGetConfigFuncPtr, IrpcFunc_EsifGetConfig), \
		.fSetConfigFuncPtr = IrpcFuncPtr2Id((data).fSetConfigFuncPtr, IrpcFunc_EsifSetConfig), \
		.fPrimitiveFuncPtr = IrpcFuncPtr2Id((data).fPrimitiveFuncPtr, IrpcFunc_EsifPrimitive), \
		.fWriteLogFuncPtr = IrpcFuncPtr2Id((data).fWriteLogFuncPtr, IrpcFunc_EsifWriteLog), \
		.fRegisterEventFuncPtr = IrpcFuncPtr2Id((data).fRegisterEventFuncPtr, IrpcFunc_EsifEventRegister), \
		.fUnregisterEventFuncPtr = IrpcFuncPtr2Id((data).fUnregisterEventFuncPtr, IrpcFunc_EsifEventUnregister), \
		.fSendEventFuncPtr = IrpcFuncPtr2Id((data).fSendEventFuncPtr, IrpcFunc_EsifSendEvent), \
		.fSendCommandFuncPtr = IrpcFuncPtr2Id((data).fSendCommandFuncPtr, IrpcFunc_EsifSendCommand), \
	}
#define Irpc_Uncast_EsifInterface(data) \
	{ \
		.fGetConfigFuncPtr = IrpcFuncId2Ptr((data).fGetConfigFuncPtr, IrpcId2Func(IrpcFunc_EsifGetConfig)), \
		.fSetConfigFuncPtr = IrpcFuncId2Ptr((data).fSetConfigFuncPtr, IrpcId2Func(IrpcFunc_EsifSetConfig)), \
		.fPrimitiveFuncPtr = IrpcFuncId2Ptr((data).fPrimitiveFuncPtr, IrpcId2Func(IrpcFunc_EsifPrimitive)), \
		.fWriteLogFuncPtr = IrpcFuncId2Ptr((data).fWriteLogFuncPtr, IrpcId2Func(IrpcFunc_EsifWriteLog)), \
		.fRegisterEventFuncPtr = IrpcFuncId2Ptr((data).fRegisterEventFuncPtr, IrpcId2Func(IrpcFunc_EsifEventRegister)), \
		.fUnregisterEventFuncPtr = IrpcFuncId2Ptr((data).fUnregisterEventFuncPtr, IrpcId2Func(IrpcFunc_EsifEventUnregister)), \
		.fSendEventFuncPtr = IrpcFuncId2Ptr((data).fSendEventFuncPtr, IrpcId2Func(IrpcFunc_EsifSendEvent)), \
		.fSendCommandFuncPtr = IrpcFuncId2Ptr((data).fSendCommandFuncPtr, IrpcId2Func(IrpcFunc_EsifSendCommand)), \
	}

#define IRPC_REVISION	0
#define IRPC_NULLVALUE	((UInt32)(-1))	// NULL Offset Indicator for Encoded_<ObjType>Ptr objects
#define IRPC_MAXARRAY	1024			// Max Length of Array Parameters

#pragma pack(push, 1)
typedef struct Encoded_EsifHandlePtr_s {
	Encoded_UInt32		offset;		// Offset of Encoded_EsifHandle from this struct or IRPC_NULLVALUE
} Encoded_EsifHandlePtr;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifRealtime_s {
	Encoded_UInt64		clockticks;	// OS-specific high resolution clock counter
	Encoded_UInt64		clocktime;	// Standard time_t clock seconds since 1/1/1970
} Encoded_EsifRealtime;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifData_s {
	Encoded_eDataType	type;		// Data Type
	Encoded_UInt32		buf_len;	// Buffer Length or 0
	Encoded_UInt32		data_len;	// Data Length or 0; Always <= buf_len unless ESIF_E_NEED_LARGER_BUFFER response
	Encoded_UInt32		offset;		// Offset of Variable Length Data from this struct if data_len > 0
	/* Variable Length Data follows */
} Encoded_EsifData;
typedef struct Encoded_EsifDataPtr_s {
	Encoded_UInt32		offset;		// Offset of Encoded_EsifData from this struct or IRPC_NULLVALUE
} Encoded_EsifDataPtr;
typedef struct Encoded_EsifDataArray_s {
	Encoded_UInt32		offset;		// Offset of Encoded_EsifData Array from this struct or IRPC_NULLVALUE
} Encoded_EsifDataArray;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct Encoded_IrpcMsg_s {
	Encoded_eIrpcMsgType	msgtype;	// IRPC Message Type (Request, Response, etc)
	Encoded_UInt16			revision;	// Message Type Revision Number
	Encoded_eIrpcFunction	funcId;		// Function ID (IrpcFunc_None = NULL)
	Encoded_UInt64			trxId;		// Caller Unique Transaction ID
	Encoded_EsifRealtime	timestamp;	// Transaction Creation Real-Time Timestamp
	/* Variable Length Function Parameters Follow */
} Encoded_IrpcMsg;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifIfaceHdr_s {
	Encoded_eIfaceType  fIfaceType;		// Inteface type: App, ESIF, action, etc.
	Encoded_UInt16      fIfaceVersion;	// Version of the given interface type
	Encoded_UInt16      fIfaceSize;		// Size of the whole structure including this header
} Encoded_EsifIfaceHdr;
#pragma pack(pop)

/*
** AppInterfaceSet Encoders
*/

#pragma pack(push, 1)
typedef struct Encoded_AppInterface_s {
	Encoded_eIrpcFunction	fAppCreateFuncPtr;
	Encoded_eIrpcFunction	fAppDestroyFuncPtr;
	Encoded_eIrpcFunction	fAppSuspendFuncPtr;
	Encoded_eIrpcFunction	fAppResumeFuncPtr;
	Encoded_eIrpcFunction	fAppGetVersionFuncPtr;
	Encoded_eIrpcFunction	fAppCommandFuncPtr;
	Encoded_eIrpcFunction	fAppGetIntroFuncPtr;
	Encoded_eIrpcFunction	fAppGetDescriptionFuncPtr;
	Encoded_eIrpcFunction	fAppGetNameFuncPtr;
	Encoded_eIrpcFunction	fAppGetStatusFuncPtr;
	Encoded_eIrpcFunction	fParticipantCreateFuncPtr;
	Encoded_eIrpcFunction	fParticipantDestroyFuncPtr;
	Encoded_eIrpcFunction	fDomainCreateFuncPtr;
	Encoded_eIrpcFunction	fDomainDestroyFuncPtr;
	Encoded_eIrpcFunction	fAppEventFuncPtr;
} Encoded_AppInterface;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifInterface_s {
	Encoded_eIrpcFunction	fGetConfigFuncPtr;
	Encoded_eIrpcFunction	fSetConfigFuncPtr;
	Encoded_eIrpcFunction	fPrimitiveFuncPtr;
	Encoded_eIrpcFunction	fWriteLogFuncPtr;
	Encoded_eIrpcFunction	fRegisterEventFuncPtr;
	Encoded_eIrpcFunction	fUnregisterEventFuncPtr;
	Encoded_eIrpcFunction	fSendEventFuncPtr;
	Encoded_eIrpcFunction	fSendCommandFuncPtr;
} Encoded_EsifInterface;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppInterfaceSet_s {
	Encoded_EsifIfaceHdr	hdr;
	Encoded_AppInterface	appIface;
	Encoded_EsifInterface	esifIface;
} Encoded_AppInterfaceSet;
typedef struct Encoded_AppInterfaceSetPtr_s {
	Encoded_UInt32		offset;		// Offset of Encoded_AppInterfaceSet from this struct or IRPC_NULLVALUE
} Encoded_AppInterfaceSetPtr;
#pragma pack(pop)

/*
** AppData Encoders
*/

#pragma pack(push, 1)
typedef struct Encoded_AppData_s {
	Encoded_EsifData	fPathHome;
	Encoded_eLogType	fLogLevel;
	/* Variable Length Data follows */
} Encoded_AppData;
typedef struct Encoded_AppDataPtr_s {
	Encoded_UInt32		offset;		// Offset of Encoded_AppData from this struct or IRPC_NULLVALUE
} Encoded_AppDataPtr;
#pragma pack(pop)

/*
** AppParticipantData Encoders
*/

#pragma pack(push, 1)
typedef struct Encoded_AppParticipantData_s {
	Encoded_UInt8		fVersion;
	Encoded_UInt8		fReserved[3];
	Encoded_EsifData	fDriverType;
	Encoded_EsifData	fDeviceType;
	Encoded_EsifData	fName;
	Encoded_EsifData	fDesc;
	Encoded_EsifData	fDriverName;
	Encoded_EsifData	fDeviceName;
	Encoded_EsifData	fDevicePath;
	Encoded_UInt8		fDomainCount;
	Encoded_UInt8		fReserved2[3];
	Encoded_eParticipantBus fBusEnumerator;

	/* ACPI */
	Encoded_EsifData    fAcpiDevice;
	Encoded_EsifData    fAcpiScope;
	Encoded_EsifData    fAcpiUID;
	Encoded_eDomainType fAcpiType;

	/* PCI */
	Encoded_UInt16  fPciVendor;
	Encoded_UInt16  fPciDevice;
	Encoded_UInt8   fPciBus;
	Encoded_UInt8   fPciBusDevice;
	Encoded_UInt8   fPciFunction;
	Encoded_UInt8   fPciRevision;
	Encoded_UInt8   fPciClass;
	Encoded_UInt8   fPciSubClass;
	Encoded_UInt8   fPciProgIf;
	Encoded_UInt8   fReserved3[1];
	/* Variable Length Data follows */
} Encoded_AppParticipantData;
typedef struct Encoded_AppParticipantDataPtr_s {
	Encoded_UInt32		offset;		// Offset of Encoded_AppParticipantData from this struct or IRPC_NULLVALUE
} Encoded_AppParticipantDataPtr;
#pragma pack(pop)

/*
** AppDomainData Encoders
*/

#pragma pack(push, 1)
typedef struct Encoded_t_AppDomainData {
	Encoded_UInt8		fVersion;
	Encoded_UInt8		fReserved[3];
	Encoded_EsifData	fName;
	Encoded_EsifData	fDescription;
	Encoded_EsifData	fGuid;
	Encoded_eDomainType	fType;
	Encoded_EsifFlags	fCapability;
	Encoded_UInt8		fCapabilityBytes[32];
	/* Variable Length Data follows */
} Encoded_AppDomainData;
typedef struct Encoded_AppDomainDataPtr_s {
	Encoded_UInt32		offset;		// Offset of Encoded_AppDomainData from this struct or IRPC_NULLVALUE
} Encoded_AppDomainDataPtr;
#pragma pack(pop)

/*
** Encoded RPC Function Calls
*/

#pragma pack(push, 1)
typedef struct Encoded_EsifGetConfigFunction_s {
	Encoded_IrpcMsg			irpcHdr;		// IRPC Message Header
	Encoded_eEsifError		result;			// Function Result
	Encoded_EsifHandle		esifHandle;		// const
	Encoded_EsifDataPtr		nameSpace;		// const
	Encoded_EsifDataPtr		elementPath;	// const
	Encoded_EsifDataPtr		elementValue;
	/* Variable Length Data Follows */
} Encoded_EsifGetConfigFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifSetConfigFunction_s {
	Encoded_IrpcMsg			irpcHdr;		// IRPC Message Header
	Encoded_eEsifError		result;			// Function Result
	Encoded_EsifHandle		esifHandle;		// const
	Encoded_EsifDataPtr		nameSpace;		// const
	Encoded_EsifDataPtr		elementPath;	// const
	Encoded_EsifDataPtr		elementValue;	// const
	Encoded_EsifFlags		elementFlags;	// const
	/* Variable Length Data Follows */
} Encoded_EsifSetConfigFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifPrimitiveFunction_s {
	Encoded_IrpcMsg			irpcHdr;			// IRPC Message Header
	Encoded_eEsifError		result;				// Function Result
	Encoded_EsifHandle		esifHandle;			// const
	Encoded_EsifHandle		participantHandle;	// const
	Encoded_EsifHandle		domainHandle;		// const
	Encoded_EsifDataPtr		request;			// const
	Encoded_EsifDataPtr		response;
	Encoded_ePrimitiveType	primitive;			// const
	Encoded_UInt8			instance;			// const
	/* Variable Length Data Follows */
} Encoded_EsifPrimitiveFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifWriteLogFunction_s {
	Encoded_IrpcMsg			irpcHdr;			// IRPC Message Header
	Encoded_eEsifError		result;				// Function Result
	Encoded_EsifHandle		esifHandle;			// const
	Encoded_EsifHandle		participantHandle;	// const
	Encoded_EsifHandle		domainHandle;		// const
	Encoded_EsifDataPtr		message;			// const
	Encoded_eLogType		logType;			// const
	/* Variable Length Data Follows */
} Encoded_EsifWriteLogFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifEventActionFunction_s {
	Encoded_IrpcMsg			irpcHdr;			// IRPC Message Header
	Encoded_eEsifError		result;				// Function Result
	Encoded_EsifHandle		esifHandle;			// const
	Encoded_EsifHandle		participantHandle;	// const
	Encoded_EsifHandle		domainHandle;		// const
	Encoded_EsifDataPtr		eventGuid;			// const
	/* Variable Length Data Follows */
} Encoded_EsifEventActionFunction;
// Event Register and Unregister Functions use same prototype
typedef	Encoded_EsifEventActionFunction	Encoded_EsifEventRegisterFunction;
typedef	Encoded_EsifEventActionFunction	Encoded_EsifEventUnregisterFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifSendEventFunction_s {
	Encoded_IrpcMsg			irpcHdr;			// IRPC Message Header
	Encoded_eEsifError		result;				// Function Result
	Encoded_EsifHandle		esifHandle;			// const
	Encoded_EsifHandle		participantHandle;	// const
	Encoded_EsifHandle		domainHandle;		// const
	Encoded_EsifDataPtr		eventData;			// const
	Encoded_EsifDataPtr		eventGuid;			// const
	/* Variable Length Data Follows */
} Encoded_EsifSendEventFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_EsifSendCommandFunction_s {
	Encoded_IrpcMsg			irpcHdr;	// IRPC Message Header
	Encoded_eEsifError		result;		// Function Result
	Encoded_EsifHandle		esifHandle;	// const
	Encoded_UInt32			argc;		// const
	Encoded_EsifDataArray	argv;		// const
	Encoded_EsifDataPtr		response;
	/* Variable Length Data Follows */
} Encoded_EsifSendCommandFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppGetStringFunction_s {
	Encoded_IrpcMsg			irpcHdr;	// IRPC Message Header
	Encoded_eEsifError		result;		// Function Result
	Encoded_EsifHandle		appHandle;	// const [GetIntro only]
	Encoded_EsifDataPtr		stringData;
	/* Variable Length Data Follows */
} Encoded_AppGetStringFunction;
//
// AppGetStringFunction is used by multiple AppInterface functions (AppGetName, AppGetDescription, AppGetVersion,  AppGetIntro)
//
typedef Encoded_AppGetStringFunction Encoded_AppGetDescriptionFunction;
typedef Encoded_AppGetStringFunction Encoded_AppGetNameFunction;
typedef Encoded_AppGetStringFunction Encoded_AppGetVersionFunction;
typedef Encoded_AppGetStringFunction Encoded_AppGetIntroFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppCreateFunction_s {
	Encoded_IrpcMsg				irpcHdr;		// IRPC Message Header
	Encoded_eEsifError			result;			// Function Result
	Encoded_AppInterfaceSetPtr	ifaceSetPtr;
	Encoded_EsifHandle			esifHandle;		// const
	Encoded_EsifHandlePtr		appHandlePtr;
	Encoded_AppDataPtr			appData;		// const
	Encoded_eAppState			initialAppState;// const
	/* Variable Length Data Follows */
} Encoded_AppCreateFunction;
#pragma pack(pop)

// Functions that only take appHandle as parameter
#pragma pack(push, 1)
typedef struct Encoded_AppHandleFunction_s {
	Encoded_IrpcMsg				irpcHdr;		// IRPC Message Header
	Encoded_eEsifError			result;			// Function Result
	Encoded_EsifHandle			appHandle;		// const
} Encoded_AppHandleFunction;
#pragma pack(pop)
typedef Encoded_AppHandleFunction	Encoded_AppDestroyFunction;
typedef Encoded_AppHandleFunction	Encoded_AppSuspendFunction;
typedef Encoded_AppHandleFunction	Encoded_AppResumeFunction;

#pragma pack(push, 1)
typedef struct Encoded_AppCommandFunction_s {
	Encoded_IrpcMsg			irpcHdr;	// IRPC Message Header
	Encoded_eEsifError		result;		// Function Result
	Encoded_EsifHandle		appHandle;	// const
	Encoded_UInt32			argc;		// const
	Encoded_EsifDataArray	argv;		// const
	Encoded_EsifDataPtr		response;
	/* Variable Length Data Follows */
} Encoded_AppCommandFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppGetStatusFunction_s {
	Encoded_IrpcMsg				irpcHdr;		// IRPC Message Header
	Encoded_eEsifError			result;			// Function Result
	Encoded_EsifHandle			appHandle;		// const
	Encoded_eAppStatusCommand	command;		// const
	Encoded_UInt32				appStatusIn;	// const
	Encoded_EsifDataPtr			appStatusOut;
	/* Variable Length Data Follows */
} Encoded_AppGetStatusFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppParticipantCreateFunction_s {
	Encoded_IrpcMsg					irpcHdr;				// IRPC Message Header
	Encoded_eEsifError				result;					// Function Result
	Encoded_EsifHandle				appHandle;				// const
	Encoded_EsifHandle				participantHandle;		// const
	Encoded_AppParticipantDataPtr	participantData;		// const
	Encoded_eParticipantState		participantInitialState;// const
	/* Variable Length Data Follows */
} Encoded_AppParticipantCreateFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppParticipantDestroyFunction_s {
	Encoded_IrpcMsg					irpcHdr;				// IRPC Message Header
	Encoded_eEsifError				result;					// Function Result
	Encoded_EsifHandle				appHandle;				// const
	Encoded_EsifHandle				participantHandle;		// const
} Encoded_AppParticipantDestroyFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppDomainCreateFunction_s {
	Encoded_IrpcMsg				irpcHdr;			// IRPC Message Header
	Encoded_eEsifError			result;				// Function Result
	Encoded_EsifHandle			appHandle;			// const
	Encoded_EsifHandle			participantHandle;	// const
	Encoded_EsifHandle			domainHandle;		// const
	Encoded_AppDomainDataPtr	domainDataPtr;		// const
	Encoded_eDomainState		domainInitialState;	// const
	/* Variable Length Data Follows */
} Encoded_AppDomainCreateFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppDomainDestroyFunction_s {
	Encoded_IrpcMsg					irpcHdr;				// IRPC Message Header
	Encoded_eEsifError				result;					// Function Result
	Encoded_EsifHandle				appHandle;				// const
	Encoded_EsifHandle				participantHandle;		// const
	Encoded_EsifHandle				domainHandle;			// const
} Encoded_AppDomainDestroyFunction;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct Encoded_AppEventFunction_s {
	Encoded_IrpcMsg					irpcHdr;				// IRPC Message Header
	Encoded_eEsifError				result;					// Function Result
	Encoded_EsifHandle				appHandle;				// const
	Encoded_EsifHandle				participantHandle;		// const
	Encoded_EsifHandle				domainHandle;			// const
	Encoded_EsifDataPtr				eventData;				// const
	Encoded_EsifDataPtr				eventGuid;				// const
	/* Variable Length Data Follows */
} Encoded_AppEventFunction;
#pragma pack(pop)

/* Deprecate Use of ObjectPtr typedefs to distinguish between encoded struct pointers and pointers to encoded structs:

EsifData d;					// struct
EsifDataPtr pd;				// struct *

Encoded_EsifData e;			// encoded struct
Encoded_EsifDataPtr ep;		// encoded (struct *)

Encoded_EsifData *p;		// *(encoded struct)
Encoded_EsifDataPtr *pp;	// *(encoded (struct *))

// Note that Encoded_EsifDataPtr != (Encoded_EsifData *)
*/

size_t Irpc_Serialize_EsifData(
	IBinary *blob,
	const EsifData *data,
	Encoded_EsifData *encoded,
	const size_t varbinary);
void *Irpc_Deserialize_EsifData(
	void *into,
	const Encoded_EsifData *encoded,
	const UInt8 *offsetfrom);
Encoded_EsifData *Irpc_Encode_EsifData(
	IBinary *blob,
	const EsifData *data);
EsifData *Irpc_Decode_EsifData(
	EsifData *into,
	const Encoded_EsifData *encoded);
size_t Irpc_Serialize_EsifDataPtr(
	IBinary *blob,
	const EsifDataPtr data,
	Encoded_EsifDataPtr *encoded,
	const size_t offsetof,
	const size_t varbinary);
EsifData *Irpc_Deserialize_EsifDataPtr(
	EsifData *into,
	const Encoded_EsifDataPtr *encoded,
	const UInt8 *offsetfrom);
size_t Irpc_Serialize_EsifDataArray(
	IBinary *blob,
	const EsifDataArray data,
	const UInt32 array_len,
	Encoded_EsifDataArray *encoded,
	const size_t offsetof,
	const size_t varbinary);
EsifDataArray Irpc_Deserialize_EsifDataArray(
	EsifDataArray into,
	const UInt32 array_len,
	const Encoded_EsifDataArray *encoded,
	const UInt8 *offsetfrom);

esif_error_t Irpc_Unmarshall_EsifDataPtr(
	EsifData *into,
	const Encoded_EsifDataPtr *encoded,
	const UInt8 *offsetfrom);

Encoded_EsifIfaceHdr *Irpc_Encode_EsifIfaceHdr(
	IBinary *blob,
	const EsifIfaceHdr *data);
EsifIfaceHdr *Irpc_Decode_EsifIfaceHdr(
	EsifIfaceHdr *into,
	const Encoded_EsifIfaceHdr *encoded);

Encoded_AppData *Irpc_Encode_AppData(
	IBinary *blob,
	const AppData *data);
AppData *Irpc_Decode_AppData(
	AppData *into,
	Encoded_AppData *data);

Encoded_AppParticipantData *Irpc_Encode_AppParticipantData(
	IBinary *blob,
	AppParticipantDataPtr data);
AppParticipantData *Irpc_Decode_AppParticipantData(
	AppParticipantData *into,
	Encoded_AppParticipantData *encoded);

Encoded_AppDomainData *Irpc_Encode_AppDomainData(
	IBinary *blob,
	AppDomainDataPtr data);
AppDomainData *Irpc_Decode_AppDomainData(
	AppDomainData *into,
	Encoded_AppDomainData *encoded);

/* IRPC EsifInterface Functions */

Encoded_EsifGetConfigFunction *Irpc_Encode_EsifGetConfigFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	EsifDataPtr elementValue);

Encoded_EsifSetConfigFunction *Irpc_Encode_EsifSetConfigFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	const EsifDataPtr elementValue,
	const EsifFlags elementFlags);

Encoded_EsifPrimitiveFunction *Irpc_Encode_EsifPrimitiveFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr request,
	EsifDataPtr response,
	const ePrimitiveType primitive,
	const UInt8 instance);

Encoded_EsifWriteLogFunction *Irpc_Encode_EsifWriteLogFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr message,
	const eLogType logType);

// EventRegister and EventUnregister Encoders use same prototype
Encoded_EsifEventActionFunction *Irpc_Encode_EsifEventActionFunc(
	IrpcTransaction *trx,
	const eIrpcFunction funcId,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid);

#define Irpc_Encode_EsifEventRegisterFunc(session, esifHandle, participantHandle, domainHandle, eventGuid) \
		(Encoded_EsifEventRegisterFunction *)Irpc_Encode_EsifEventActionFunc(session, IrpcFunc_EsifEventRegister, esifHandle, participantHandle, domainHandle, eventGuid)
#define Irpc_Encode_EsifEventUnregisterFunc(session, esifHandle, participantHandle, domainHandle, eventGuid) \
		(Encoded_EsifEventUnregisterFunction *)Irpc_Encode_EsifEventActionFunc(session, IrpcFunc_EsifEventUnregister, esifHandle, participantHandle, domainHandle, eventGuid)

Encoded_EsifSendEventFunction *Irpc_Encode_EsifSendEventFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid);

Encoded_EsifSendCommandFunction *Irpc_Encode_EsifSendCommandFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response);

/* IRPC AppInterface Functions */

// AppGetName, AppGetDescription, and AppGetIntro use same prototype
Encoded_AppGetStringFunction *Irpc_Encode_AppGetStringFunc(
	IrpcTransaction *trx,
	const eIrpcFunction funcId,
	const esif_handle_t appHandle,
	EsifDataPtr stringData);

Encoded_AppCreateFunction *Irpc_Encode_AppCreateFunc(
	IrpcTransaction *trx,
	AppInterfaceSetPtr ifaceSetPtr,
	const esif_handle_t esifHandle,
	esif_handle_t *appHandlePtr,
	const AppDataPtr appData,
	const eAppState initialAppState);

Encoded_AppDestroyFunction *Irpc_Encode_AppHandleFunc(
	IrpcTransaction *trx,
	const eIrpcFunction funcId,
	const esif_handle_t appHandle);

#define Irpc_Encode_AppDestroyFunc(session, handle)	Irpc_Encode_AppHandleFunc(session, IrpcFunc_AppDestroy, handle)
#define Irpc_Encode_AppSuspendFunc(session, handle)	Irpc_Encode_AppHandleFunc(session, IrpcFunc_AppSuspend, handle)
#define Irpc_Encode_AppResumeFunc(session, handle)	Irpc_Encode_AppHandleFunc(session, IrpcFunc_AppResume, handle)

Encoded_AppCommandFunction *Irpc_Encode_AppCommandFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response);
	
Encoded_AppGetStatusFunction *Irpc_Encode_AppGetStatusFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut);

Encoded_AppParticipantCreateFunction *Irpc_Encode_AppParticipantCreateFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const AppParticipantDataPtr participantData,
	const eParticipantState participantInitialState
	);

Encoded_AppParticipantDestroyFunction *Irpc_Encode_AppParticipantDestroyFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle);

Encoded_AppDomainCreateFunction *Irpc_Encode_AppDomainCreateFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState
);

Encoded_AppDomainDestroyFunction *Irpc_Encode_AppDomainDestroyFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t domainHandle,
	const esif_handle_t participantHandle);

Encoded_AppEventFunction *Irpc_Encode_AppEventFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid);

#define ESIF_NULL_THREAD_ID	((esif_thread_id_t)(0))
#define ESIF_INTRO_LEN		1024

// IPF Server Remote Client Session
typedef struct AppSession_s {
	esif_ccb_lock_t		lock;							// Thread Lock
	atomic_t			refCount;						// Reference Count
	atomic_t			connected;						// Active IPC Connection?
	esif_ccb_realtime_t	connectTime;					// IPC Connection Time
	esif_ccb_realtime_t	updateTime;						// IPC Last Send/Receive
	esif_handle_t		ipfHandle;						// Unique IPF Client Session Handle, exposed to ESIF as appHandle & RPC Client as esifHandle
	esif_handle_t		esifHandle;						// ESIF Handle for RPC Client, Generated by ESIF
	esif_handle_t		appHandle;						// App Handle returned by RPC Client
	char				appName[ESIF_NAME_LEN];			// App Name returned by RPC Client
	char				appVersion[ESIF_VERSION_LEN];	// App Version returned by RPC Client
	char				appDescription[ESIF_DESC_LEN];	// App Description returned by RPC Client
	char				appIntro[ESIF_INTRO_LEN];		// App Intro returned by RPC Client
	char				esifName[ESIF_NAME_LEN];		// Unique App Name exposed to ESIF, may be Renamed after AppGetName
	esif_thread_id_t	threadId;						// Used to Lookup Session during AppCreate; Requires that AppGetName be called first on same thread
	AppInterfaceSet		ifaceSet;						// App Interface Set for RPC Client (ESIF=ESIF, App=IpfCli_App* or NULL until AppCreate)
	esif_ccb_sockaddr_t	sockaddr;						// Client IPC Socket Address
	esif_handle_t		authHandle;						// Client Authorization Role Handle
} AppSession;

esif_error_t IrpcTransaction_EsifRequest(IrpcTransaction *trx);

// IPF RPC Requests (Server Side) App Interface Functions
esif_error_t ESIF_CALLCONV Irpc_Request_AppGetName(EsifDataPtr appNamePtr);
esif_error_t ESIF_CALLCONV Irpc_Request_AppGetDescription(EsifDataPtr appDescriptionPtr);
esif_error_t ESIF_CALLCONV Irpc_Request_AppGetVersion(EsifDataPtr appVersionPtr);

esif_error_t ESIF_CALLCONV Irpc_Request_AppCreate(
	AppInterfaceSetPtr ifaceSetPtr,
	const esif_handle_t esifHandle,
	esif_handle_t *appHandlePtr,
	const AppDataPtr appDataPtr,
	const eAppState appInitialState);

esif_error_t ESIF_CALLCONV Irpc_Request_AppGetIntro(
	const esif_handle_t appHandle,
	EsifDataPtr appIntroPtr);

esif_error_t ESIF_CALLCONV Irpc_Request_AppDestroy(const esif_handle_t appHandle);
esif_error_t ESIF_CALLCONV Irpc_Request_AppSuspend(const esif_handle_t appHandle);
esif_error_t ESIF_CALLCONV Irpc_Request_AppResume(const esif_handle_t appHandle);

esif_error_t ESIF_CALLCONV Irpc_Request_AppCommand(
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response);

esif_error_t ESIF_CALLCONV Irpc_Request_AppGetStatus(
	const esif_handle_t appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut);

esif_error_t ESIF_CALLCONV Irpc_Request_AppParticipantCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const AppParticipantDataPtr participantData,
	const eParticipantState participantInitialState);

esif_error_t ESIF_CALLCONV Irpc_Request_AppParticipantDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle);

esif_error_t ESIF_CALLCONV Irpc_Request_AppDomainCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState);

esif_error_t ESIF_CALLCONV Irpc_Request_AppDomainDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle);

esif_error_t ESIF_CALLCONV Irpc_Request_AppEvent(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid);

