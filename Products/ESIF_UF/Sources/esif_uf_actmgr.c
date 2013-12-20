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

#define ESIF_TRACE_DEBUG_DISABLED

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_actmgr.h"	/* Action Manager */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Friend */
EsifActMgr g_actMgr = {0};

static EsifActPtr GetActionFromName (
	EsifActMgr *THIS,
	EsifString lib_name
	)
{
	u8 i = 0;
	EsifActPtr a_action_ptr = NULL;

	for (i = 0; i < ESIF_MAX_ACTIONS; i++) {
		a_action_ptr = &THIS->fEnrtries[i];

		if (NULL == a_action_ptr->fLibNamePtr) {
			continue;
		}

		if (!strcmp(a_action_ptr->fLibNamePtr, lib_name)) {
			return a_action_ptr;
		}
	}
	return NULL;
}


/*
    ACTION MANAGER
 */
#define ACT_VERSION "x1.0.0.1"
#define IS_KERNEL 1
#define IS_PLUGIN 0
#define PAD 0
#define GUID 0
#define OS "ALL"
#define OS_WINDOWS "WINDOWS"

/* Kernel Actions */
static EsifActType g_kernelActions[] = {
	{0, ESIF_ACTION_ACPI,       {PAD}, "ACPI",       "Advanced Config Power Interface", OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_CODE,       {PAD}, "CODE",       "Programmed Action",               OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
// JDH TODO Fix This!
#ifdef ESIF_ATTR_OS_WINDOWS
	{0, ESIF_ACTION_DDIGFXDISP, {PAD}, "DDIGFXDISP", "D2D Graphics Display",            OS_WINDOWS, ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_DDIGFXPERF, {PAD}, "DDIGFXPERF", "D2D Graphics Performance",        OS_WINDOWS, ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
#endif
	{0, ESIF_ACTION_KONST,      {PAD}, "KONST",      "Kernel Constant",                 OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_IOSF,       {PAD}, "IOSF",       "IO Sideband Fabric",              OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_MMIO,       {PAD}, "MMIO",       "Memory Mapped IO",                OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_MSR,        {PAD}, "MSR",        "Model Specific Register",         OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_SYSTEMIO,   {PAD}, "SYSTEMIO",   "System IO",                       OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_VAR,        {PAD}, "VAR",        "Persisted State Management",      OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},
	{0, ESIF_ACTION_VIRTUAL,    {PAD}, "VIRTUAL",    "Virtual Action/Sensor",           OS,         ACT_VERSION,
	  {GUID},
	  IS_KERNEL, IS_PLUGIN, {PAD}, NULL,
	  NULL},

	{0, 0}	/* Must be NULL terminated */
};

static EsifActTypePtr GetActionType (
	EsifActMgrPtr THIS,
	UInt8 type
	)
{
	EsifActTypePtr found_ptr = NULL;
	struct esif_link_list_node *curr_ptr = THIS->fActTypes->head_ptr;

	while (curr_ptr) {
		EsifActTypePtr cur_actiontype_ptr = (EsifActTypePtr)curr_ptr->data_ptr;
		if (cur_actiontype_ptr != NULL) {
			if (type == cur_actiontype_ptr->fType) {
				found_ptr = cur_actiontype_ptr;
				break;
			}
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	return found_ptr;
}


/* Insert Action Into List */
static eEsifError AddAction (
	EsifActMgrPtr THIS,
	EsifActTypePtr actionPtr
	)
{
	esif_link_list_node_add(THIS->fActTypes, esif_link_list_create_node(actionPtr));
	ESIF_TRACE_DEBUG("%s: item %p", ESIF_FUNC, actionPtr);
	return ESIF_OK;
}


static eEsifError RemoveAction (
	EsifActMgrPtr THIS,
	EsifActTypePtr type
	)
{
	UNREFERENCED_PARAMETER(THIS);
	UNREFERENCED_PARAMETER(type);

	return ESIF_E_NOT_IMPLEMENTED;
}


enum esif_rc EsifActMgrInit ()
{
	enum esif_rc rc = ESIF_OK;
	u8 i = 0;

	ESIF_TRACE_DEBUG("%s: Init Action Manager (ACTMGR)", ESIF_FUNC);

	g_actMgr.fActTypes = esif_link_list_create();
	if (NULL == g_actMgr.fActTypes) {
		return ESIF_E_NO_MEMORY;
	}

	g_actMgr.GetActType     = GetActionType;
	g_actMgr.GetActFromName = GetActionFromName;
	g_actMgr.AddActType     = AddAction;
	g_actMgr.RemoveActType  = RemoveAction;

	/* Add static Kernel Entries */
	for (i = 0; g_kernelActions[i].fType; i++)
		g_actMgr.AddActType(&g_actMgr, &g_kernelActions[i]);

	/* Action manager must be initialized */
	EsifActInit();

	return rc;
}


void EsifActMgrExit ()
{
	/* Call before destroying action manager */
	EsifActExit();

	if (NULL != g_actMgr.fActTypes) {
		esif_link_list_destroy(g_actMgr.fActTypes);
	}

	ESIF_TRACE_DEBUG("%s: Exit Action Manager (ACTMGR)", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
