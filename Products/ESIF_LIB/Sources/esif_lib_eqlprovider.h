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
#ifndef _EQLPROVIDER_H
#define _EQLPROVIDER_H

#include "esif_lib.h"

// Provider Callback Function
typedef eEsifError (*ProviderCallback)(void *param);

struct Provider_s;
typedef struct Provider_s Provider, *ProviderPtr, **ProviderPtrLocation, **ProviderPtrArray;

struct ProviderAction_s;
typedef struct ProviderAction_s ProviderAction, *ProviderActionPtr, **ProviderActionPtrLocation, **ProviderActionPtrArray;

struct Adapter_s;
typedef struct Adapter_s Adapter, *AdapterPtr, **AdapterPtrLocation, **AdapterPtrArray;

struct ProviderRegistry_s;
typedef struct ProviderRegistry_s ProviderRegistry, *ProviderRegistryPtr, **ProviderRegistryLocation, **ProviderRegistryArray;

#ifdef _EQLPROVIDER_CLASS

struct ProviderAction_s {
	StringPtr  action;
	ProviderCallback fnInterface;
};

struct Provider_s {
	StringPtr  name;
	ProviderActionPtr actions;
};

struct Adapter_s {
	StringPtr  name;
	int provider;
	void       *context;
};

struct ProviderRegistry_s {
	AdapterPtr   adapters;
	int nAdapters;
	ProviderPtr  providers;
	int nProviders;
};

#endif

// Adapter Class
eEsifError Adapter_SetCallback (StringPtr adapter, StringPtr action, ProviderCallback fnProviderCallback, void *context);	// Set Callback Function

#endif