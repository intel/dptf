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
#ifndef _EQLPARSER_H
#define _EQLPARSER_H

#include "esif.h"

#include "esif_lib.h"
#include "esif_lib_istringlist.h"
#include "esif_lib_esifdata.h"
#include "esif_lib_esifdatalist.h"
#include "esif_lib_eqlcmd.h"

//////////////////////////////////////////////////////////////////////////////
// EqlParser class

#define EQLPARSER_MAXVALUE  256		// Max length of any individual token or value

struct EqlParser_s;
typedef struct EqlParser_s EqlParser, *EqlParserPtr, **EqlParserPtrLocation;

#ifdef _EQLPARSER_CLASS
// struct EqlParser_s definition is in esif_uf_eqlparser.c due to enum type dependencies
#endif

// object management
EqlParserPtr EqlParser_Create ();
void EqlParser_Destroy (EqlParserPtr self);

// methods
void EqlParser_Init ();
void EqlParser_Cleanup ();
EqlCmdPtr EqlParser_Parse (EqlParserPtr self, const StringPtr buffer);

// static methods
eEsifError EqlParser_ExecuteEql (esif_string eql, EsifDataPtr *arguments);

#endif
