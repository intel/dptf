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

#pragma once

#include "ipf_ipc_codec.h"

// IPF Transactions
IrpcTransaction *IrpcTransaction_Create(eIrpcMsgType msgType);
void IrpcTransaction_Destroy(IrpcTransaction *self);
void IrpcTransaction_GetRef(IrpcTransaction *self);
void IrpcTransaction_PutRef(IrpcTransaction *self);
void IrpcTransaction_Signal(IrpcTransaction *self);
esif_error_t IrpcTransaction_Wait(IrpcTransaction *self);

// IPF Transaction Manager
typedef struct IpfTrxMgr_s {
	esif_ccb_lock_t	lock;		// Transaction Manager Lock
	IrpcTransaction** trxPool;	// Active Transaction Pool
	size_t poolSize;			// Active Transaction Pool size
	atomic_t rpcTimeout;		// RPC Timeout in Seconds
} IpfTrxMgr;

// Use to Get Singleton Instance; Multiple Instances also supported
IpfTrxMgr *IpfTrxMgr_GetInstance(void);

// IPF Transaction Manager Multiple Instances
esif_error_t IpfTrxMgr_Init(IpfTrxMgr *self);
void IpfTrxMgr_Uninit(IpfTrxMgr *self);
void IpfTrxMgr_ExpireInactive(IpfTrxMgr *self);
void IpfTrxMgr_ExpireAll(IpfTrxMgr *self);
esif_error_t IpfTrxMgr_AddTransaction(IpfTrxMgr *self, IrpcTransaction *trx);
IrpcTransaction * IpfTrxMgr_GetTransaction(IpfTrxMgr *self, esif_handle_t ipfHandle, UInt64 trxId);
double IpfTrxMgr_GetMinTimeout(IpfTrxMgr *self);
size_t IpfTrxMgr_GetTimeout(IpfTrxMgr *self);
void IpfTrxMgr_SetTimeout(IpfTrxMgr *self, size_t timeout);
void IpfTrxMgr_ResetTimeouts(IpfTrxMgr *self);

#define IPFTRX_MATCHANY		0xffffffffffffffff	// Match Any Transaction ID for given IPF Handle
