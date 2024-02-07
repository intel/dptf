/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "ipf_ipc_codec.h"

#pragma warning(disable:4204)

/*
** EsifData Encoders
*/

size_t Irpc_Serialize_EsifData(
	IBinary *blob,
	const EsifData *data,
	Encoded_EsifData *encoded,
	const size_t varbinary)
{
	size_t bytes = 0;
	encoded->offset = Irpc_Cast_UInt32(Irpc_Uncast_UInt32(encoded->offset) + (UInt32)varbinary);
	if ((data->buf_ptr == NULL) || (data->data_len == 0) || ((data->data_len <= data->buf_len) && (IBinary_Append(blob, data->buf_ptr, data->data_len) != NULL))) {
		bytes = data->data_len;
	}
	return bytes;
}

void *Irpc_Deserialize_EsifData(
	void *into,
	const Encoded_EsifData *encoded,
	const UInt8 *offsetfrom)
{
	if (Irpc_Uncast_UInt32(encoded->buf_len)) {
		if (into == NULL) {
			into = esif_ccb_malloc(Irpc_Uncast_UInt32(encoded->buf_len));
		}
		if (into && Irpc_Uncast_UInt32(encoded->data_len) <= Irpc_Uncast_UInt32(encoded->buf_len)) {
			esif_ccb_memcpy(into, offsetfrom + Irpc_Uncast_UInt32(encoded->offset), Irpc_Uncast_UInt32(encoded->data_len));
		}
	}
	return into;
}

Encoded_EsifData *Irpc_Encode_EsifData(
	IBinary *blob,
	const EsifData *data)
{
	Encoded_EsifData *ret = NULL;
	if (data && blob) {
		Encoded_EsifData encoded = Irpc_Cast_EsifData(*data, Irpc_OffsetOf(*data, data_len));
		size_t offset = IBinary_GetLen(blob);
		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if ((data->buf_ptr == NULL) || (data->data_len == 0) || (data->data_len <= data->buf_len) || (IBinary_Append(blob, data->buf_ptr, data->data_len) != NULL)) {
				ret = (Encoded_EsifData *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			}
		}
	}
	return ret;
}

EsifData *Irpc_Decode_EsifData(
	EsifData *into,
	const Encoded_EsifData *encoded)
{
	if (encoded) {
		void *buf_ptr = NULL;
		if (into == NULL) {
			into = esif_ccb_malloc(sizeof(*into));
		}
		if (into && Irpc_Uncast_UInt32(encoded->buf_len)) {
			buf_ptr = Irpc_Deserialize_EsifData(NULL, encoded, Irpc_OffsetFrom(*encoded, offset));
			if (buf_ptr == NULL) {
				esif_ccb_free(into);
				into = NULL;
			}
		}
		if (into) {
			EsifData decoded = Irpc_Uncast_EsifData(encoded, buf_ptr);
			*into = decoded;
		}
	}
	return into;
}

size_t Irpc_Serialize_EsifDataPtr(
	IBinary *blob,
	const EsifDataPtr data,
	Encoded_EsifDataPtr *encoded,
	const size_t offsetof,
	const size_t varbinary)
{
	size_t bytes = 0;
	if (data && encoded) {
		Encoded_EsifData encodedData = Irpc_Cast_EsifData(*data, 0);
		encoded->offset = Irpc_Cast_UInt32((UInt32)(offsetof + varbinary));
		IBinary_Append(blob, &encodedData, sizeof(encodedData));
		bytes += sizeof(encodedData);
		bytes += Irpc_Serialize_EsifData(blob, data, &encodedData, 0);
	}
	return bytes;
}

EsifData *Irpc_Deserialize_EsifDataPtr(
	EsifData *into,
	const Encoded_EsifDataPtr *encoded,
	const UInt8 *offsetfrom)
{
	if (encoded && encoded->offset != IRPC_NULLVALUE) {
		if (into == NULL) {
			into = esif_ccb_malloc(sizeof(*into));
		}
		if (into) {
			Encoded_EsifData *encodedData = (Encoded_EsifData *)(offsetfrom + Irpc_Uncast_UInt32(encoded->offset));
			EsifData data = Irpc_Uncast_EsifData(encodedData, NULL);
			data.buf_ptr = Irpc_Deserialize_EsifData(data.buf_ptr, encodedData, Irpc_OffsetFrom(*encodedData, offset));
			*into = data;
		}
	}
	return into;
}

size_t Irpc_Serialize_EsifDataArray(
	IBinary *blob,
	const EsifDataArray data,
	const UInt32 array_len,
	Encoded_EsifDataArray *encoded,
	const size_t offsetof,
	const size_t varbinary)
{
	size_t bytes = 0;
	if (array_len && array_len < IRPC_MAXARRAY && data && encoded) {
		for (UInt32 j = 0; j < array_len; j++) {
			if (j == 0) {
				encoded->offset = Irpc_Cast_UInt32((UInt32)(offsetof + varbinary));
			}
			Encoded_EsifData encodedData = Irpc_Cast_EsifData(data[j], 0);
			IBinary_Append(blob, &encodedData, sizeof(encodedData));
			bytes += sizeof(encodedData);
			bytes += Irpc_Serialize_EsifData(blob, &data[j], &encodedData, 0);
		}
	}
	return bytes;
}

EsifDataArray Irpc_Deserialize_EsifDataArray(
	EsifDataArray into,
	const UInt32 array_len,
	const Encoded_EsifDataArray *encoded,
	const UInt8 *offsetfrom)
{
	if (array_len && array_len < IRPC_MAXARRAY && encoded && encoded->offset != IRPC_NULLVALUE) {
		Bool dynamic = (into == NULL);
		if (into == NULL) {
			into = esif_ccb_malloc(sizeof(*into) * array_len);
		}
		if (into) {
			Encoded_EsifData *encodedData = (Encoded_EsifData *)(offsetfrom + Irpc_Uncast_UInt32(encoded->offset));
			for (UInt32 j = 0; j < array_len; j++) {
				EsifData data = Irpc_Uncast_EsifData(encodedData, NULL);
				data.buf_ptr = Irpc_Deserialize_EsifData(data.buf_ptr, encodedData, Irpc_OffsetFrom(*encodedData, offset));
				encodedData = (Encoded_EsifData *)(((UInt8 *)(encodedData + 1)) + data.data_len);
				into[j] = data;

				// Auto-destroy dynamic data if any memory allocations fail
				if (dynamic && data.buf_ptr == NULL && data.buf_len) {
					for (int k = (int)j; k >= 0; k--) {
						esif_ccb_free(into[k].buf_ptr);
					}
					esif_ccb_free(into);
					into = NULL;
					break;
				}
			}
		}
	}
	return into;
}

esif_error_t Irpc_Unmarshall_EsifDataPtr(
	EsifData *into,
	const Encoded_EsifDataPtr *encoded,
	const UInt8 *offsetfrom
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (into && encoded && offsetfrom) {
		EsifData *param = Irpc_Deserialize_EsifDataPtr(NULL, encoded, offsetfrom);
		if (param) {
			rc = ESIF_OK;
			if (param->data_len <= into->buf_len && into->buf_ptr && into->buf_len) {
				if (param->buf_ptr) {
					esif_ccb_memcpy(into->buf_ptr, param->buf_ptr, param->data_len);
				}
				else {
					rc = ESIF_E_NO_MEMORY;
				}
			}
			into->type = param->type;
			into->data_len = param->data_len;
			esif_ccb_free(param->buf_ptr);
			esif_ccb_free(param);
		}
	}
	return rc;
}


/*
** EsifIfaceHdr Encoders
*/

Encoded_EsifIfaceHdr *Irpc_Encode_EsifIfaceHdr(
	IBinary *blob,
	const EsifIfaceHdr *data)
{
	Encoded_EsifIfaceHdr *ret = NULL;
	if (data && blob) {
		Encoded_EsifIfaceHdr encoded = Irpc_Cast_EsifIfaceHdr(*data);
		size_t offset = IBinary_GetLen(blob);
		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			ret = (Encoded_EsifIfaceHdr *)((UInt8 *)IBinary_GetBuf(blob) + offset);
		}
	}
	return ret;
}

EsifIfaceHdr *Irpc_Decode_EsifIfaceHdr(
	EsifIfaceHdr *into,
	const Encoded_EsifIfaceHdr *encoded)
{
	if (encoded) {
		if (into == NULL) {
			into = esif_ccb_malloc(sizeof(*into));
		}
		if (into) {
			EsifIfaceHdr decoded = Irpc_Uncast_EsifIfaceHdr(*encoded);
			*into = decoded;
		}
	}
	return into;
}

/*
** AppData Encoders
*/

Encoded_AppData *Irpc_Encode_AppData(
	IBinary *blob,
	const AppData *data)
{
	Encoded_AppData *ret = NULL;
	if (data && blob) {
		Encoded_AppData encoded = {
			Irpc_Cast_EsifData(data->fPathHome, Irpc_OffsetOf(encoded, fPathHome)),
			Irpc_Cast_eLogType(data->fLogLevel),
		};
		size_t offset = IBinary_GetLen(blob);
		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if ((data->fPathHome.buf_ptr == NULL) || (data->fPathHome.data_len == 0) || (IBinary_Append(blob, data->fPathHome.buf_ptr, data->fPathHome.data_len) != NULL)) {
				ret = (Encoded_AppData *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			}
		}
	}
	return ret;
}

AppData *Irpc_Decode_AppData(
	AppData *into,
	Encoded_AppData *data)
{
	if (data) {
		if (into == NULL) {
			into = esif_ccb_malloc(sizeof(*into));
		}
		if (into) {
			AppData decoded = {
				Irpc_Uncast_EsifData(&data->fPathHome, Irpc_Deserialize_EsifData(NULL, &data->fPathHome, Irpc_OffsetFrom(*data, fPathHome))),
				Irpc_Uncast_eLogType(data->fLogLevel),
			};
			*into = decoded;
		}
	}
	return into;
}

/*
** AppParticipantData Encoders
*/

Encoded_AppParticipantData *Irpc_Encode_AppParticipantData(IBinary *blob, AppParticipantDataPtr data)
{
	Encoded_AppParticipantData *ret = NULL;
	if (data && blob) {
		Encoded_AppParticipantData encoded = {
			Irpc_Cast_UInt8(data->fVersion),
			{ 0 },
			Irpc_Cast_EsifData(data->fDriverType, Irpc_OffsetOf(encoded, fDriverType)),
			Irpc_Cast_EsifData(data->fDeviceType, Irpc_OffsetOf(encoded, fDeviceType)),
			Irpc_Cast_EsifData(data->fName, Irpc_OffsetOf(encoded, fName)),
			Irpc_Cast_EsifData(data->fDesc, Irpc_OffsetOf(encoded, fDesc)),
			Irpc_Cast_EsifData(data->fDriverName, Irpc_OffsetOf(encoded, fDriverName)),
			Irpc_Cast_EsifData(data->fDeviceName, Irpc_OffsetOf(encoded, fDeviceName)),
			Irpc_Cast_EsifData(data->fDevicePath, Irpc_OffsetOf(encoded, fDevicePath)),
			Irpc_Cast_UInt8(data->fDomainCount),
			{ 0 },
			Irpc_Cast_eParticipantBus(data->fBusEnumerator),
			Irpc_Cast_EsifData(data->fAcpiDevice, Irpc_OffsetOf(encoded, fAcpiDevice)),
			Irpc_Cast_EsifData(data->fAcpiScope, Irpc_OffsetOf(encoded, fAcpiScope)),
			Irpc_Cast_EsifData(data->fAcpiUID, Irpc_OffsetOf(encoded, fAcpiUID)),
			Irpc_Cast_eDomainType(data->fAcpiType),

			Irpc_Cast_UInt16(data->fPciVendor),
			Irpc_Cast_UInt16(data->fPciDevice),
			Irpc_Cast_UInt8(data->fPciBus),
			Irpc_Cast_UInt8(data->fPciBusDevice),
			Irpc_Cast_UInt8(data->fPciFunction),
			Irpc_Cast_UInt8(data->fPciRevision),
			Irpc_Cast_UInt8(data->fPciClass),
			Irpc_Cast_UInt8(data->fPciSubClass),
			Irpc_Cast_UInt8(data->fPciProgIf),
			{ 0 },
		};
		Irpc_Init_Array(Irpc_Cast_UInt8, encoded, data, fReserved);
		Irpc_Init_Array(Irpc_Cast_UInt8, encoded, data, fReserved2);
		Irpc_Init_Array(Irpc_Cast_UInt8, encoded, data, fReserved3);
		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			varbinary += Irpc_Serialize_EsifData(blob, &data->fDriverType, &encoded.fDriverType, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fDeviceType, &encoded.fDeviceType, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fName, &encoded.fName, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fDesc, &encoded.fDesc, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fDriverName, &encoded.fDriverName, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fDeviceName, &encoded.fDeviceName, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fDevicePath, &encoded.fDevicePath, varbinary);

			varbinary += Irpc_Serialize_EsifData(blob, &data->fAcpiDevice, &encoded.fAcpiDevice, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fAcpiScope, &encoded.fAcpiScope, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fAcpiUID, &encoded.fAcpiUID, varbinary);

			ret = (Encoded_AppParticipantData *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			esif_ccb_memcpy(ret, &encoded, sizeof(encoded));
		}
	}
	return ret;
}

AppParticipantData *Irpc_Decode_AppParticipantData(AppParticipantData *into, Encoded_AppParticipantData *encoded)
{
	if (encoded) {
		if (into == NULL) {
			into = esif_ccb_malloc(sizeof(*into));
		}
		if (into) {
			AppParticipantData decoded = {
				Irpc_Uncast_UInt8(encoded->fVersion),
				{ 0 },
				Irpc_Uncast_EsifData(&encoded->fDriverType, Irpc_Deserialize_EsifData(NULL, &encoded->fDriverType, Irpc_OffsetFrom(*encoded, fDriverType))),
				Irpc_Uncast_EsifData(&encoded->fDeviceType, Irpc_Deserialize_EsifData(NULL, &encoded->fDeviceType, Irpc_OffsetFrom(*encoded, fDeviceType))),
				Irpc_Uncast_EsifData(&encoded->fName,		Irpc_Deserialize_EsifData(NULL, &encoded->fName, Irpc_OffsetFrom(*encoded, fName))),
				Irpc_Uncast_EsifData(&encoded->fDesc,		Irpc_Deserialize_EsifData(NULL, &encoded->fDesc, Irpc_OffsetFrom(*encoded, fDesc))),
				Irpc_Uncast_EsifData(&encoded->fDriverName, Irpc_Deserialize_EsifData(NULL, &encoded->fDriverName, Irpc_OffsetFrom(*encoded, fDriverName))),
				Irpc_Uncast_EsifData(&encoded->fDeviceName, Irpc_Deserialize_EsifData(NULL, &encoded->fDeviceName, Irpc_OffsetFrom(*encoded, fDeviceName))),
				Irpc_Uncast_EsifData(&encoded->fDevicePath, Irpc_Deserialize_EsifData(NULL, &encoded->fDevicePath, Irpc_OffsetFrom(*encoded, fDevicePath))),
				Irpc_Uncast_UInt8(encoded->fDomainCount),
				{ 0 },
				Irpc_Uncast_eParticipantBus(encoded->fBusEnumerator),

				Irpc_Uncast_EsifData(&encoded->fAcpiDevice, Irpc_Deserialize_EsifData(NULL, &encoded->fAcpiDevice, Irpc_OffsetFrom(*encoded, fAcpiDevice))),
				Irpc_Uncast_EsifData(&encoded->fAcpiScope,	Irpc_Deserialize_EsifData(NULL, &encoded->fAcpiScope, Irpc_OffsetFrom(*encoded, fAcpiScope))),
				Irpc_Uncast_EsifData(&encoded->fAcpiUID,	Irpc_Deserialize_EsifData(NULL, &encoded->fAcpiUID, Irpc_OffsetFrom(*encoded, fAcpiUID))),
				Irpc_Uncast_eDomainType(encoded->fAcpiType),

				Irpc_Uncast_UInt16(encoded->fPciVendor),
				Irpc_Uncast_UInt16(encoded->fPciDevice),
				Irpc_Uncast_UInt8(encoded->fPciBus),
				Irpc_Uncast_UInt8(encoded->fPciBusDevice),
				Irpc_Uncast_UInt8(encoded->fPciFunction),
				Irpc_Uncast_UInt8(encoded->fPciRevision),
				Irpc_Uncast_UInt8(encoded->fPciClass),
				Irpc_Uncast_UInt8(encoded->fPciSubClass),
				Irpc_Uncast_UInt8(encoded->fPciProgIf),
				{ 0 },
			};
			Irpc_Init_Array(Irpc_Uncast_UInt8, decoded, encoded, fReserved);
			Irpc_Init_Array(Irpc_Uncast_UInt8, decoded, encoded, fReserved2);
			Irpc_Init_Array(Irpc_Uncast_UInt8, decoded, encoded, fReserved3);
			*into = decoded;
		}
	}
	return into;
}

/*
** AppDomainData Encoders
*/

Encoded_AppDomainData *Irpc_Encode_AppDomainData(IBinary *blob, AppDomainDataPtr data)
{
	Encoded_AppDomainData *ret = NULL;
	if (data && blob) {
		Encoded_AppDomainData encoded = {
			Irpc_Cast_UInt8(data->fVersion),
			{ 0 },
			Irpc_Cast_EsifData(data->fName, Irpc_OffsetOf(encoded, fName)),
			Irpc_Cast_EsifData(data->fDescription, Irpc_OffsetOf(encoded, fDescription)),
			Irpc_Cast_EsifData(data->fGuid, Irpc_OffsetOf(encoded, fGuid)),
			Irpc_Cast_eDomainType(data->fType),
			Irpc_Cast_EsifFlags(data->fCapability),
			{ 0 },
		};
		Irpc_Init_Array(Irpc_Cast_UInt8, encoded, data, fReserved);
		Irpc_Init_Array(Irpc_Cast_UInt8, encoded, data, fCapabilityBytes);

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		// Standard method - Append incomplete struct and variable-length data, then Update completed struct at beginning
		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			varbinary += Irpc_Serialize_EsifData(blob, &data->fName, &encoded.fName, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fDescription, &encoded.fDescription, varbinary);
			varbinary += Irpc_Serialize_EsifData(blob, &data->fGuid, &encoded.fGuid, varbinary);
			ret = (Encoded_AppDomainData *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			esif_ccb_memcpy(ret, &encoded, sizeof(encoded));
		}
	}
	return ret;
}

AppDomainData *Irpc_Decode_AppDomainData(AppDomainData *into, Encoded_AppDomainData *encoded)
{
	if (encoded) {
		if (into == NULL) {
			into = esif_ccb_malloc(sizeof(*into));
		}
		if (into) {
			AppDomainData decoded = {
				Irpc_Uncast_UInt8(encoded->fVersion),
				{ 0 },
				Irpc_Uncast_EsifData(&encoded->fName,		Irpc_Deserialize_EsifData(NULL, &encoded->fName, Irpc_OffsetFrom(*encoded, fName))),
				Irpc_Uncast_EsifData(&encoded->fDescription,Irpc_Deserialize_EsifData(NULL, &encoded->fDescription, Irpc_OffsetFrom(*encoded, fDescription))),
				Irpc_Uncast_EsifData(&encoded->fGuid,		Irpc_Deserialize_EsifData(NULL, &encoded->fGuid, Irpc_OffsetFrom(*encoded, fGuid))),
				Irpc_Uncast_eDomainType(encoded->fType),
				Irpc_Uncast_EsifFlags(encoded->fCapability),
				{ 0 },
			};
			Irpc_Init_Array(Irpc_Uncast_UInt8, decoded, encoded, fReserved);
			Irpc_Init_Array(Irpc_Uncast_UInt8, decoded, encoded, fCapabilityBytes);
			*into = decoded;
		}
	}
	return into;
}


/*
** EsifGetConfig and EsifSetConfig Encoders
*/

Encoded_EsifGetConfigFunction *Irpc_Encode_EsifGetConfigFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	EsifDataPtr elementValue)
{
	Encoded_EsifGetConfigFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_EsifGetConfigFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_EsifGetConfig),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.esifHandle = Irpc_Cast_EsifHandle(esifHandle),
			.nameSpace = IRPC_NULLVALUE,
			.elementPath = IRPC_NULLVALUE,
			.elementValue = IRPC_NULLVALUE,
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataPtr(blob, nameSpace, &encoded.nameSpace, Irpc_OffsetOf(encoded, nameSpace), varbinary);
				varbinary += Irpc_Serialize_EsifDataPtr(blob, elementPath, &encoded.elementPath, Irpc_OffsetOf(encoded, elementPath), varbinary);
			}
			varbinary += Irpc_Serialize_EsifDataPtr(blob, elementValue, &encoded.elementValue, Irpc_OffsetOf(encoded, elementValue), varbinary);
			ret = (Encoded_EsifGetConfigFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

Encoded_EsifSetConfigFunction *Irpc_Encode_EsifSetConfigFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	const EsifDataPtr elementValue,
	const EsifFlags elementFlags)
{
	Encoded_EsifSetConfigFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_EsifSetConfigFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_EsifSetConfig),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.esifHandle = Irpc_Cast_EsifHandle(esifHandle),
			.nameSpace = IRPC_NULLVALUE,
			.elementPath = IRPC_NULLVALUE,
			.elementValue = IRPC_NULLVALUE,
			.elementFlags = Irpc_Cast_EsifFlags(elementFlags)
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataPtr(blob, nameSpace, &encoded.nameSpace, Irpc_OffsetOf(encoded, nameSpace), varbinary);
				varbinary += Irpc_Serialize_EsifDataPtr(blob, elementPath, &encoded.elementPath, Irpc_OffsetOf(encoded, elementPath), varbinary);
				varbinary += Irpc_Serialize_EsifDataPtr(blob, elementValue, &encoded.elementValue, Irpc_OffsetOf(encoded, elementValue), varbinary);
			}
			ret = (Encoded_EsifSetConfigFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

/*
** EsifPrimitive Encoders
*/

Encoded_EsifPrimitiveFunction *Irpc_Encode_EsifPrimitiveFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr request,
	EsifDataPtr response,
	const ePrimitiveType primitive,
	const UInt8 instance)
{
	Encoded_EsifPrimitiveFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_EsifPrimitiveFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_EsifPrimitive),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.esifHandle = Irpc_Cast_EsifHandle(esifHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.domainHandle = Irpc_Cast_EsifHandle(domainHandle),
			.request = IRPC_NULLVALUE,
			.response = IRPC_NULLVALUE,
			.primitive = Irpc_Cast_ePrimitiveType(primitive),
			.instance = Irpc_Cast_UInt8(instance),
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataPtr(blob, request, &encoded.request, Irpc_OffsetOf(encoded, request), varbinary);
			}
			varbinary += Irpc_Serialize_EsifDataPtr(blob, response, &encoded.response, Irpc_OffsetOf(encoded, response), varbinary);
			ret = (Encoded_EsifPrimitiveFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

Encoded_EsifWriteLogFunction *Irpc_Encode_EsifWriteLogFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr message,
	const eLogType logType)
{
	Encoded_EsifWriteLogFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_EsifWriteLogFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_EsifWriteLog),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.esifHandle = Irpc_Cast_EsifHandle(esifHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.domainHandle = Irpc_Cast_EsifHandle(domainHandle),
			.message = IRPC_NULLVALUE,
			.logType = Irpc_Cast_eLogType(logType)
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataPtr(blob, message, &encoded.message, Irpc_OffsetOf(encoded, message), varbinary);
			}
			ret = (Encoded_EsifWriteLogFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

/*
** EsifEventRegister and EsifEventUnregister Encoders
*/

// Event Register and Unregister Functions use same prototype
Encoded_EsifEventActionFunction *Irpc_Encode_EsifEventActionFunc(
	IrpcTransaction *trx,
	const eIrpcFunction funcId,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid)
{
	Encoded_EsifEventActionFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_EsifEventActionFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(funcId),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.esifHandle = Irpc_Cast_EsifHandle(esifHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.domainHandle = Irpc_Cast_EsifHandle(domainHandle),
			.eventGuid = IRPC_NULLVALUE,
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataPtr(blob, eventGuid, &encoded.eventGuid, Irpc_OffsetOf(encoded, eventGuid), varbinary);
			}
			ret = (Encoded_EsifEventActionFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

Encoded_EsifSendEventFunction *Irpc_Encode_EsifSendEventFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid)
{
	Encoded_EsifSendEventFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_EsifSendEventFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_EsifSendEvent),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.esifHandle = Irpc_Cast_EsifHandle(esifHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.domainHandle = Irpc_Cast_EsifHandle(domainHandle),
			.eventData = IRPC_NULLVALUE,
			.eventGuid = IRPC_NULLVALUE,
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataPtr(blob, eventData, &encoded.eventData, Irpc_OffsetOf(encoded, eventData), varbinary);
				varbinary += Irpc_Serialize_EsifDataPtr(blob, eventGuid, &encoded.eventGuid, Irpc_OffsetOf(encoded, eventGuid), varbinary);
			}
			ret = (Encoded_EsifSendEventFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

Encoded_EsifSendCommandFunction *Irpc_Encode_EsifSendCommandFunc(
	IrpcTransaction *trx,
	const esif_handle_t esifHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response)
{
	Encoded_EsifSendCommandFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_EsifSendCommandFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_EsifSendCommand),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.esifHandle = Irpc_Cast_EsifHandle(esifHandle),
			.argc = Irpc_Cast_UInt32(0),
			.argv = IRPC_NULLVALUE,
			.response = IRPC_NULLVALUE
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataArray(blob, argv, argc, &encoded.argv, Irpc_OffsetOf(encoded, argv), varbinary);
				if (encoded.argv.offset != IRPC_NULLVALUE) {
					encoded.argc = Irpc_Cast_UInt32(argc);
				}
			}
			varbinary += Irpc_Serialize_EsifDataPtr(blob, response, &encoded.response, Irpc_OffsetOf(encoded, response), varbinary);
			ret = (Encoded_EsifSendCommandFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

/*
** AppGetString Encoders (AppGetName, AppGetDescription, AppGetVersion, AppGetIntro)
*/

Encoded_AppGetStringFunction *Irpc_Encode_AppGetStringFunc(
	IrpcTransaction *trx,
	const eIrpcFunction funcId,
	const esif_handle_t appHandle,
	EsifDataPtr stringData)
{
	Encoded_AppGetStringFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppGetStringFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(funcId),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.stringData = IRPC_NULLVALUE,
		};
		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			varbinary += Irpc_Serialize_EsifDataPtr(blob, stringData, &encoded.stringData, Irpc_OffsetOf(encoded, stringData), varbinary);
			ret = (Encoded_AppGetStringFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

/*
** AppCreate Encoders
*/

Encoded_AppCreateFunction *Irpc_Encode_AppCreateFunc(
	IrpcTransaction *trx,
	AppInterfaceSetPtr ifaceSetPtr,
	const esif_handle_t esifHandle,
	esif_handle_t *appHandlePtr,
	const AppDataPtr appData,
	const eAppState initialAppState)
{
	Encoded_AppCreateFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		esif_error_t result = ESIF_OK;
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppCreateFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppCreate),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(result),
			.ifaceSetPtr = IRPC_NULLVALUE,
			.esifHandle = esifHandle,
			.appHandlePtr = IRPC_NULLVALUE,
			.appData = IRPC_NULLVALUE,
			.initialAppState = Irpc_Cast_eAppState(initialAppState),
		};
		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (ifaceSetPtr) {
				Encoded_AppInterfaceSet encoded_ifaceSet = {
					.hdr = Irpc_Cast_EsifIfaceHdr(ifaceSetPtr->hdr),
					.appIface = Irpc_Cast_AppInterface(ifaceSetPtr->appIface),
					.esifIface = Irpc_Cast_EsifInterface(ifaceSetPtr->esifIface),
				};
				if (IBinary_Append(blob, &encoded_ifaceSet, sizeof(encoded_ifaceSet))) {
					encoded.ifaceSetPtr.offset = Irpc_Cast_UInt32((UInt32)(Irpc_OffsetOf(encoded, ifaceSetPtr) + varbinary));
					varbinary += sizeof(encoded_ifaceSet);
				}
			}
			if (appHandlePtr) {
				Encoded_EsifHandle encoded_appHandle = Irpc_Cast_EsifHandle(*appHandlePtr);
				if (IBinary_Append(blob, &encoded_appHandle, sizeof(encoded_appHandle))) {
					encoded.appHandlePtr.offset = Irpc_Cast_UInt32((UInt32)(Irpc_OffsetOf(encoded, appHandlePtr) + varbinary));
					varbinary += sizeof(encoded_appHandle);
				}
			}
			if (appData && msgtype == IrpcMsg_ProcRequest) { // const
				size_t appData_offset = IBinary_GetLen(blob);
				Encoded_AppData *encoded_appData = Irpc_Encode_AppData(blob, appData);
				if (encoded_appData) {
					encoded.appData.offset = Irpc_Cast_UInt32((UInt32)(Irpc_OffsetOf(encoded, appData) + varbinary));
					varbinary += IBinary_GetLen(blob) - appData_offset;
				}
			}

			ret = (Encoded_AppCreateFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

/*
** AppHandle Function Encoders: AppDestroy, AppSuspend, AppResume
*/

Encoded_AppHandleFunction *Irpc_Encode_AppHandleFunc(
	IrpcTransaction *trx,
	const eIrpcFunction funcId,
	const esif_handle_t appHandle)
{
	Encoded_AppHandleFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		esif_error_t result = ESIF_OK;
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppHandleFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(funcId),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
		};

		size_t offset = IBinary_GetLen(blob);
		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			ret = (Encoded_AppHandleFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
		}
	}
	return ret;
}

/*
** AppCommand Encoders
*/

Encoded_AppCommandFunction *Irpc_Encode_AppCommandFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response)
{
	Encoded_AppCommandFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppCommandFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppCommand),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.argc = Irpc_Cast_UInt32(0),
			.argv = IRPC_NULLVALUE,
			.response = IRPC_NULLVALUE
		};
		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) {	// const
				varbinary += Irpc_Serialize_EsifDataArray(blob, argv, argc, &encoded.argv, Irpc_OffsetOf(encoded, argv), varbinary);
				if (encoded.argv.offset != IRPC_NULLVALUE) {
					encoded.argc = Irpc_Cast_UInt32(argc);
				}
			}
			varbinary += Irpc_Serialize_EsifDataPtr(blob, response, &encoded.response, Irpc_OffsetOf(encoded, response), varbinary);
			ret = (Encoded_AppCommandFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}

/*
** AppGetStatus Encoders
*/

Encoded_AppGetStatusFunction *Irpc_Encode_AppGetStatusFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut)
{
	Encoded_AppGetStatusFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppGetStatusFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppGetStatus),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(trx->result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.command = Irpc_Cast_eAppStatusCommand(command),
			.appStatusIn = Irpc_Cast_UInt32(appStatusIn),
			.appStatusOut = IRPC_NULLVALUE,
		};
		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			varbinary += Irpc_Serialize_EsifDataPtr(blob, appStatusOut, &encoded.appStatusOut, Irpc_OffsetOf(encoded, appStatusOut), varbinary);
			ret = (Encoded_AppGetStatusFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			*ret = encoded;
		}
	}
	return ret;
}


/*
** AppParticipantCreate Encoders
*/

Encoded_AppParticipantCreateFunction *Irpc_Encode_AppParticipantCreateFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const AppParticipantDataPtr participantData,
	const eParticipantState participantInitialState)
{
	Encoded_AppParticipantCreateFunction *ret = NULL;
	if (trx && (trx->request || trx->response)) {
		esif_error_t result = ESIF_OK;
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppParticipantCreateFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppParticipantCreate),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.participantData = IRPC_NULLVALUE,
			.participantInitialState = Irpc_Cast_eParticipantState(participantInitialState)
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (participantData && msgtype == IrpcMsg_ProcRequest) { // const
				size_t participantData_offset = IBinary_GetLen(blob);
				Encoded_AppParticipantData *encoded_participantData = Irpc_Encode_AppParticipantData(blob, participantData);
				if (encoded_participantData) {
					encoded.participantData.offset = Irpc_Cast_UInt32((UInt32)(Irpc_OffsetOf(encoded, participantData) + varbinary));
					varbinary += IBinary_GetLen(blob) - participantData_offset;
				}
			}
			ret = (Encoded_AppParticipantCreateFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			esif_ccb_memcpy(ret, &encoded, sizeof(encoded));
		}
	}
	return ret;
}

/*
** AppParticipantDestroy Encoders
*/

Encoded_AppParticipantDestroyFunction *Irpc_Encode_AppParticipantDestroyFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle)
{
	Encoded_AppParticipantDestroyFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		esif_error_t result = ESIF_OK;
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppParticipantDestroyFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppParticipantDestroy),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
		};

		size_t offset = IBinary_GetLen(blob);
		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			ret = (Encoded_AppParticipantDestroyFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
		}
	}
	return ret;
}

/*
** AppDomainCreate Encoders
*/

Encoded_AppDomainCreateFunction *Irpc_Encode_AppDomainCreateFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState)
{
	Encoded_AppDomainCreateFunction *ret = NULL;
	if (trx && (trx->request || trx->response)) {
		esif_error_t result = ESIF_OK;
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppDomainCreateFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppDomainCreate),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.domainHandle = Irpc_Cast_EsifHandle(domainHandle),
			.domainDataPtr = IRPC_NULLVALUE,
			.domainInitialState = Irpc_Cast_eDomainState(domainInitialState)
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (domainDataPtr && msgtype == IrpcMsg_ProcRequest) { // const
				size_t domainDataPtr_offset = IBinary_GetLen(blob);
				Encoded_AppDomainData *encoded_domainDataPtr = Irpc_Encode_AppDomainData(blob, domainDataPtr);
				if (encoded_domainDataPtr) {
					encoded.domainDataPtr.offset = Irpc_Cast_UInt32((UInt32)(Irpc_OffsetOf(encoded, domainDataPtr) + varbinary));
					varbinary += IBinary_GetLen(blob) - domainDataPtr_offset;
				}
			}
			ret = (Encoded_AppDomainCreateFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			esif_ccb_memcpy(ret, &encoded, sizeof(encoded));
		}
	}
	return ret;
}

/*
** AppDomainDestroy Encoders
*/

Encoded_AppDomainDestroyFunction *Irpc_Encode_AppDomainDestroyFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle)
{
	Encoded_AppDomainDestroyFunction *ret = NULL;

	if (trx && (trx->request || trx->response)) {
		esif_error_t result = ESIF_OK;
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppDomainDestroyFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppDomainDestroy),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.domainHandle = Irpc_Cast_EsifHandle(domainHandle),
		};

		size_t offset = IBinary_GetLen(blob);
		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			ret = (Encoded_AppDomainDestroyFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
		}
	}
	return ret;
}

/*
** AppEvent Encoders
*/

Encoded_AppEventFunction *Irpc_Encode_AppEventFunc(
	IrpcTransaction *trx,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid)
{
	Encoded_AppEventFunction *ret = NULL;
	if (trx && (trx->request || trx->response)) {
		esif_error_t result = ESIF_OK;
		eIrpcMsgType msgtype = IrpcMsg_ProcRequest;
		IBinary *blob = trx->request;

		if (trx->response) {
			msgtype = IrpcMsg_ProcResponse;
			blob = trx->response;
		}

		Encoded_AppEventFunction encoded = {
			.irpcHdr = {
				.msgtype = Irpc_Cast_eIrpcMsgType(msgtype),
				.revision = Irpc_Cast_UInt16(IRPC_REVISION),
				.funcId = Irpc_Cast_eIrpcFunction(IrpcFunc_AppEvent),
				.trxId = Irpc_Cast_UInt64(trx->trxId),
				.timestamp = Irpc_Cast_EsifRealtime(trx->timestamp),
			},
			.result = Irpc_Cast_eEsifError(result),
			.appHandle = Irpc_Cast_EsifHandle(appHandle),
			.participantHandle = Irpc_Cast_EsifHandle(participantHandle),
			.domainHandle = Irpc_Cast_EsifHandle(domainHandle),
			.eventData = IRPC_NULLVALUE,
			.eventGuid = IRPC_NULLVALUE
		};

		size_t offset = IBinary_GetLen(blob);
		size_t varbinary = 0;

		if (IBinary_Append(blob, &encoded, sizeof(encoded)) != NULL) {
			if (msgtype == IrpcMsg_ProcRequest) { // const
				varbinary += Irpc_Serialize_EsifDataPtr(blob, eventData, &encoded.eventData, Irpc_OffsetOf(encoded, eventData), varbinary);
				varbinary += Irpc_Serialize_EsifDataPtr(blob, eventGuid, &encoded.eventGuid, Irpc_OffsetOf(encoded, eventGuid), varbinary);
			}
			ret = (Encoded_AppEventFunction *)((UInt8 *)IBinary_GetBuf(blob) + offset);
			esif_ccb_memcpy(ret, &encoded, sizeof(encoded));
		}
	}
	return ret;
}
