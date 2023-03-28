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

#include "ipf_ipc_codec.h"

// IPF Transactions
IrpcTransaction *IrpcTransaction_Create(eIrpcMsgType msgType);
void IrpcTransaction_Destroy(IrpcTransaction *self);
void IrpcTransaction_GetRef(IrpcTransaction *self);
void IrpcTransaction_PutRef(IrpcTransaction *self);
void IrpcTransaction_Signal(IrpcTransaction *self);
esif_error_t IrpcTransaction_Wait(IrpcTransaction *self);

// IPF RPC Transaction Manager
esif_error_t IpfTrxMgr_Init(void);
void IpfTrxMgr_Uninit(void);
void IpfTrxMgr_ExpireInactive(void);
void IpfTrxMgr_ExpireAll(void);
esif_error_t IpfTrxMgr_AddTransaction(IrpcTransaction *trx);
IrpcTransaction *IpfTrxMgr_GetTransaction(esif_handle_t ipfHandle, UInt64 trxId);
double IpfTrxMgr_GetMinTimeout();
size_t IpfTrxMgr_GetTimeout();
void IpfTrxMgr_SetTimeout(size_t timeout);
void IpfTrxMgr_ResetTimeouts();

#define IPFTRX_MATCHANY		0xffffffffffffffff	// Match Any Transaction ID for given IPF Handle
