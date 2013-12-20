/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#ifndef _EQLCMD_H
#define _EQLCMD_H

#include "esif.h"

#include "esif_lib.h"
#include "esif_lib_istringlist.h"
#include "esif_lib_esifdata.h"
#include "esif_lib_esifdatalist.h"
#include "esif_lib_eqlcmd.h"
#include "esif_lib_eqlprovider.h"

///////////////////////////////////////////////////////
// EqlCmd Class

struct EqlCmd_s;
typedef struct EqlCmd_s EqlCmd, *EqlCmdPtr, **EqlCmdPtrLocation;

#ifdef _EQLCMD_CLASS
struct EqlCmd_s {
	StringPtr  adapter;
	StringPtr  subtype;
	StringPtr  action;
	StringListPtr     messages;
	StringListPtr     parameters;
	StringListPtr     datatypes;
	StringListPtr     values;
	StringListPtr     options;
	EsifDataListPtr   results;
	ProviderCallback  handler;

	// TODO:
	// int				instance; // or context or replace subtype with context
	// ProviderCallback	handler;
};

#endif

// object management
EqlCmdPtr EqlCmd_Create ();						// new operator
void EqlCmd_Destroy (EqlCmdPtr self);	// delete operator

// methods
eEsifError EqlCmd_Dispatch (EqlCmdPtr self, EsifDataPtr *arguments);		// Bind and Execute an EQL Command and optional arguments
void EqlCmd_Reset (EqlCmdPtr self);			// Reset EqlCmd Object for Reuse
eEsifError EqlCmd_BindArguments (EqlCmdPtr self, EsifDataPtr *arguments);	// Bind optional Arguments to a Parsed EqlCmd
eEsifError EqlCmd_Execute (EqlCmdPtr self);			// Execute a Parsed and Bound EQL Command
void EqlCmd_DebugDump (EqlCmdPtr self);		// Dump object to console
void EqlCmd_DisplayResults (EqlCmdPtr self);// Display output of command

#endif