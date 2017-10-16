/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "Dptf.h"
#include "DptfVer.h"
#include "esif_sdk_iface_app.h"
#include "esif_sdk_iface_esif.h"
#include "PolicyManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIAll.h"
#include "EsifDataString.h"
#include "EsifServicesInterface.h"
#include "EsifDataGuid.h"
#include "EsifDataUInt32.h"
#include "CommandHandler.h"
#include "Commands.h"
#include "CommandDispatch.h"
#include "Diag.h"
#include <iostream>

using namespace std;

//
// Macros must be used to reduce the code and still allow writing out the file name, line number, and function name
//

#define RETURN_ERROR_IF_CONTEXT_DATA_NULL                                                                              \
	if (dptfManager == nullptr)                                                                                        \
	\
{                                                                                                                 \
		return ESIF_E_PARAMETER_IS_NULL;                                                                               \
	\
}

#define RETURN_ERROR_IF_WORK_ITEM_QUEUE_MANAGER_NOT_CREATED                                                            \
	if (dptfManager->isWorkItemQueueManagerCreated() == false)                                                         \
	\
{                                                                                                                 \
		ManagerMessage message = ManagerMessage(                                                                       \
			dptfManager, FLF, "Function call from ESIF ignored.  DPTF work item queue has not been created.");         \
		dptfManager->getEsifServices()->writeMessageError(message);                                                    \
		return ESIF_E_UNSPECIFIED;                                                                                     \
	\
}

#define RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED                                                                       \
	if (dptfManager->isDptfManagerCreated() == false)                                                                  \
	\
{                                                                                                                 \
		ManagerMessage message = ManagerMessage(                                                                       \
			dptfManager, FLF, "Function call from ESIF ignored.  DPTF manager creation is not complete.");             \
		dptfManager->getEsifServices()->writeMessageError(message);                                                    \
		return ESIF_E_UNSPECIFIED;                                                                                     \
	\
}

#define RETURN_WARNING_IF_DPTF_SHUTTING_DOWN                                                                           \
	if (dptfManager->isDptfShuttingDown() == true)                                                                     \
	\
{                                                                                                                 \
		ManagerMessage message =                                                                                       \
			ManagerMessage(dptfManager, FLF, "Function call from ESIF ignored.  DPTF manager is shutting down.");      \
		dptfManager->getEsifServices()->writeMessageWarning(message);                                                  \
		return ESIF_E_UNSPECIFIED;                                                                                     \
	\
}

CommandDispatch* dispatch;

static const Guid DptfAppGuid(
	0x8f,
	0x0d,
	0x8c,
	0x59,
	0xad,
	0x8d,
	0x4d,
	0x82,
	0xaa,
	0x25,
	0x46,
	0xd3,
	0xc0,
	0x83,
	0x30,
	0x5b);

//
// Implement the required interface
//

extern "C" {
static eEsifError GetDptfAbout(EsifDataPtr dataPtr)
{
	return FillDataPtrWithString(dataPtr, "About DPTF");
}

static eEsifError GetDptfDescription(EsifDataPtr dataPtr)
{
	return FillDataPtrWithString(dataPtr, "DPTF Manager");
}

static eEsifError GetDptfGuid(EsifDataPtr appGuidPtr)
{
	appGuidPtr->data_len = ESIF_GUID_LEN;
	if (appGuidPtr->buf_len >= ESIF_GUID_LEN)
	{
		appGuidPtr->type = ESIF_DATA_GUID;
		esif_ccb_memcpy(appGuidPtr->buf_ptr, DptfAppGuid, ESIF_GUID_LEN);
		return ESIF_OK;
	}
	else
	{
		return ESIF_E_NEED_LARGER_BUFFER;
	}
}

static eEsifError GetDptfName(EsifDataPtr dataPtr)
{
	return FillDataPtrWithString(dataPtr, "DPTF");
}

static eEsifError GetDptfVersion(EsifDataPtr dataPtr)
{
	return FillDataPtrWithString(dataPtr, VERSION_STR);
}

static eEsifError DptfAllocateHandle(void** appHandleLocation)
{
	DptfManagerInterface* dptfManager = nullptr;

	try
	{
		dptfManager = new DptfManager();
		*appHandleLocation = static_cast<void*>(dptfManager);
	}
	catch (...)
	{
		dptfManager = nullptr;
		*appHandleLocation = nullptr;
	}

	return (dptfManager != nullptr) ? ESIF_OK : ESIF_E_UNSPECIFIED;
}

static eEsifError DptfCreate(
	EsifInterfacePtr esifInterfacePtr,
	const void* esifHandle,
	const void* appHandle,
	const AppDataPtr appData,
	const eAppState appInitialState)
{
	eEsifError rc = ESIF_OK;

	// Make sure we received what we are expecting from EISF.  We can't start the application if we don't have
	// the ESIF interface pointers.  In this case we will check everything manually here instead of in a constructor.
	// If this fails we can't throw an exception or log a message since the infrastructure isn't up.  All we can do
	// is return an error.
	if (esifInterfacePtr == nullptr || esifInterfacePtr->fIfaceType != eIfaceTypeEsifService
		|| ((esifInterfacePtr->fIfaceVersion != ESIF_INTERFACE_VERSION_1)
			&& (esifInterfacePtr->fIfaceVersion != ESIF_INTERFACE_VERSION_2)
			&& (esifInterfacePtr->fIfaceVersion != ESIF_INTERFACE_VERSION_3))
		|| esifInterfacePtr->fIfaceSize != (UInt16)sizeof(EsifInterface)
		|| esifInterfacePtr->fGetConfigFuncPtr == nullptr || esifInterfacePtr->fSetConfigFuncPtr == nullptr
		|| esifInterfacePtr->fPrimitiveFuncPtr == nullptr || esifInterfacePtr->fWriteLogFuncPtr == nullptr
		|| esifInterfacePtr->fRegisterEventFuncPtr == nullptr || esifInterfacePtr->fUnregisterEventFuncPtr == nullptr
		|| (((esifInterfacePtr->fIfaceVersion == ESIF_INTERFACE_VERSION_2)
			 || (esifInterfacePtr->fIfaceVersion == ESIF_INTERFACE_VERSION_3))
			&& (esifInterfacePtr->fSendEventFuncPtr == nullptr))
		|| ((esifInterfacePtr->fIfaceVersion == ESIF_INTERFACE_VERSION_3)
			&& (esifInterfacePtr->fSendCommandFuncPtr == nullptr))
		|| appHandle == nullptr || appData == nullptr)
	{
		rc = ESIF_E_UNSPECIFIED;
	}
	else
	{
		try
		{
			eLogType currentLogVerbosityLevel = appData->fLogLevel;

			if (eLogType::eLogTypeInfo <= currentLogVerbosityLevel)
			{
				esifInterfacePtr->fWriteLogFuncPtr(
					esifHandle,
					appHandle,
					nullptr,
					nullptr,
					EsifDataString("DptfCreate:  Initialization starting."),
					eLogType::eLogTypeInfo);
			}

			// Creating the DptfManager will start the framework.  When this call returns the work item queue
			// manager is up and running and the polices have been created.  All future work will execute in the
			// context of a work item and will only take place on the work item thread.
			Bool enabled = (appInitialState == eAppState::eAppStateEnabled);
			std::string dptfHomeDirectoryPath = EsifDataString(&appData->fPathHome);
			DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
			dptfManager->createDptfManager(
				esifHandle, esifInterfacePtr, dptfHomeDirectoryPath, currentLogVerbosityLevel, enabled);

			dispatch = new CommandDispatch();

			if (eLogType::eLogTypeInfo <= currentLogVerbosityLevel)
			{
				esifInterfacePtr->fWriteLogFuncPtr(
					esifHandle,
					appHandle,
					nullptr,
					nullptr,
					EsifDataString("DptfCreate: Initialization completed."),
					eLogType::eLogTypeInfo);
			}
		}
		catch (...)
		{
			esifInterfacePtr->fWriteLogFuncPtr(
				esifHandle,
				appHandle,
				nullptr,
				nullptr,
				EsifDataString("DptfCreate: Initialization failed."),
				eLogType::eLogTypeFatal);
			rc = ESIF_E_UNSPECIFIED;
		}
	}

	return rc;
}

static eEsifError DptfDestroy(void* appHandle)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	ManagerMessage message = ManagerMessage(dptfManager, FLF, "Function execution beginning.");
	dptfManager->getEsifServices()->writeMessageInfo(message);

	eEsifError rc = ESIF_OK;

	try
	{
		DELETE_MEMORY(dptfManager);
		DELETE_MEMORY(dispatch);
	}
	catch (...)
	{
		rc = ESIF_E_UNSPECIFIED;
	}

	return rc;
}

static eEsifError DptfSuspend(void* appHandle)
{
	return ESIF_OK;

	// FIXME:
	//
	// Problem:
	//  We've temporarily commented out the code below to prevent a deadlock.  We added both the DptfSuspend and
	//  DptfResume functions specifically for the case of Critical policy needing to process temperature threshold
	//  events on D0 Entry.  However, we have a deadlock.  When Critical policy (using a work item thread) calls
	//  to shut down the system, we automatically get a call to DptfSuspend.  But the WIDptfSuspend work item
	//  never executes because it is waiting for the thread to return from the call to shut down the system.
	//
	// Solution:
	//  Update the policy services sleep(), hibernate(), and shutdown() functions.  The Manager should create a separate
	//  thread for these calls so we don't have the deadlock on the work item thread.
	//

	// DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	// RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	// RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	// RETURN_ERROR_IF_DPTF_SHUTTING_DOWN;

	// eEsifError rc = ESIF_OK;

	// try
	//{
	//    WorkItem* workItem = new WIDptfSuspend(dptfManager);
	//    dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	//}
	// catch (...)
	//{
	//    rc = ESIF_E_UNSPECIFIED;
	//}

	// return rc;
}

static eEsifError DptfResume(void* appHandle)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	eEsifError rc = ESIF_OK;

	try
	{
		WorkItem* workItem = new WIDptfResume(dptfManager);
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		rc = ESIF_E_UNSPECIFIED;
	}

	return rc;
}

static eEsifError GetDptfBanner(const void* appHandle, EsifDataPtr dataPtr)
{
	return FillDataPtrWithString(dataPtr, "DPTF Manager Loaded");
}

static eEsifError GetDptfPrompt(const void* appHandle, EsifDataPtr dataPtr)
{
	// FIXME:  Remove this from ESIF interface.
	return FillDataPtrWithString(dataPtr, "DPTF application prompt [not supported]");
}

static eEsifError DptfCommand(
	const void* appHandle,
	const UInt32 argc,
	const EsifDataPtr argv,
	const EsifDataPtr response)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	pair<esif_error_t, string> result;

	// argv is an array of EsifData objects of length argc (1 or greater)
	// Current implementation assumes an array of STRING types but future versions could support other types
	// First argument is DPTF command and remaining arguments, if any, are parameters to that DPTF Command

	// <appcmd> [arg] [...]
	if (argc >= 1 && argv != NULL && response != NULL && argv[0].type == ESIF_DATA_STRING)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		size_t optarg = 0;
		std::string appcmd = (const char*)argv[optarg++].buf_ptr;
		rc = ESIF_OK;
		response->type = ESIF_DATA_STRING;
		result = dispatch->dispatchCommand(appcmd, argc, argv, dptfManager);
		FillDataPtrWithString(response, result.second);
	}
	return rc;
}

static eEsifError SetDptfState(const void* appHandle, const eAppState appState)
{
	// FIXME:  Remove this from ESIF interface.
	return ESIF_OK;
}

static eEsifError GetDptfStatus(
	const void* appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	eEsifError rc = ESIF_E_UNSPECIFIED;

	try
	{
		WorkItem* workItem = new WIDptfGetStatus(dptfManager, command, appStatusIn, appStatusOut, &rc);
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
	}

	return rc;
}

static eEsifError ParticipantAllocateHandle(const void* appHandle, void** participantHandleLocation)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	eEsifError rc = ESIF_E_UNSPECIFIED;
	UIntN newParticipantIndex = Constants::Esif::NoParticipant;

	try
	{
		WorkItem* workItem = new WIParticipantAllocate(dptfManager, &newParticipantIndex);
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		newParticipantIndex = Constants::Esif::NoParticipant;
	}

	if (newParticipantIndex != Constants::Esif::NoParticipant)
	{
		try
		{
			*participantHandleLocation = (void*)(dptfManager->getIndexContainer()->getIndexPtr(newParticipantIndex));
			rc = ESIF_OK;
		}
		catch (...)
		{
		}
	}

	return rc;
}

static eEsifError ParticipantCreate(
	const void* appHandle,
	const void* participantHandle,
	const AppParticipantDataPtr participantDataPtr,
	const eParticipantState particiapntInitialState)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	Bool participantCreated = false;

	try
	{
		Bool participantEnabled = (particiapntInitialState == eParticipantState::eParticipantStateEnabled);

		WorkItem* workItem = new WIParticipantCreate(
			dptfManager,
			dptfManager->getIndexContainer()->getIndex((IndexStructPtr)participantHandle),
			participantDataPtr,
			participantEnabled,
			&participantCreated);
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		participantCreated = false;
	}

	return (participantCreated == true) ? ESIF_OK : ESIF_E_UNSPECIFIED;
}

eEsifError ParticipantDestroy(const void* appHandle, const void* participantHandle)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	eEsifError rc = ESIF_OK;

	try
	{
		WorkItem* workItem = new WIParticipantDestroy(
			dptfManager, dptfManager->getIndexContainer()->getIndex((IndexStructPtr)participantHandle));
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		rc = ESIF_E_UNSPECIFIED;
	}

	return rc;
}

static eEsifError ParticipantSetState(
	const void* appHandle,
	const void* participantHandle,
	eParticipantState participantState)
{
	// FIXME:  Remove this from ESIF interface.
	return ESIF_OK;
}

static eEsifError DomainAllocateHandle(
	const void* appHandle,
	const void* participantHandle,
	void** domainHandleLocation)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	eEsifError rc = ESIF_E_UNSPECIFIED;
	UIntN newDomainIndex = Constants::Esif::NoDomain;

	try
	{
		WorkItem* workItem = new WIDomainAllocate(
			dptfManager,
			dptfManager->getIndexContainer()->getIndex((IndexStructPtr)participantHandle),
			&newDomainIndex);
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		newDomainIndex = Constants::Esif::NoDomain;
	}

	if (newDomainIndex != Constants::Esif::NoDomain)
	{
		try
		{
			*domainHandleLocation = (void*)(dptfManager->getIndexContainer()->getIndexPtr(newDomainIndex));
			rc = ESIF_OK;
		}
		catch (...)
		{
		}
	}

	return rc;
}

static eEsifError DomainCreate(
	const void* appHandle,
	const void* participantHandle,
	const void* domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	Bool domainCreated = false;

	try
	{
		Bool domainEnabled = (domainInitialState == eDomainState::eDomainStateEnabled);

		WorkItem* workItem = new WIDomainCreate(
			dptfManager,
			dptfManager->getIndexContainer()->getIndex((IndexStructPtr)participantHandle),
			dptfManager->getIndexContainer()->getIndex((IndexStructPtr)domainHandle),
			domainDataPtr,
			domainEnabled,
			&domainCreated);
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		domainCreated = false;
	}

	return (domainCreated == true) ? ESIF_OK : ESIF_E_UNSPECIFIED;
}

static eEsifError DomainDestroy(const void* appHandle, const void* participantHandle, const void* domainHandle)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	eEsifError rc = ESIF_OK;

	try
	{
		WorkItem* workItem = new WIDomainDestroy(
			dptfManager,
			dptfManager->getIndexContainer()->getIndex((IndexStructPtr)participantHandle),
			dptfManager->getIndexContainer()->getIndex((IndexStructPtr)domainHandle));
		dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		rc = ESIF_E_UNSPECIFIED;
	}

	return rc;
}

static eEsifError DomainSetState(
	const void* appHandle,
	const void* participantHandle,
	const void* domainHandle,
	const eDomainState domainState)
{
	// FIXME:  Remove this from ESIF interface.
	return ESIF_OK;
}

static eEsifError DptfEvent(
	const void* appHandle,
	const void* participantHandle,
	const void* domainHandle,
	const EsifDataPtr esifEventDataPtr,
	const EsifDataPtr eventGuid)
{
	DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
	RETURN_ERROR_IF_CONTEXT_DATA_NULL;
	RETURN_ERROR_IF_WORK_ITEM_QUEUE_MANAGER_NOT_CREATED;
	RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

	Guid guid = Guid::createInvalid();
	FrameworkEvent::Type frameworkEvent = FrameworkEvent::Max;
	Bool waitForEventToProcess = false;

	try
	{
		guid = EsifDataGuid(eventGuid);
		frameworkEvent = FrameworkEventInfo::instance()->getFrameworkEventType(guid);
	}
	catch (...)
	{
		ManagerMessage message =
			ManagerMessage(dptfManager, FLF, "Error while trying to convert event guid to DPTF framework event.");
		if (guid.isValid())
		{
			message.setEsifEventGuid(guid);
		}
		dptfManager->getEsifServices()->writeMessageError(message);
		return ESIF_E_UNSPECIFIED;
	}

	IndexContainerInterface* idx = dptfManager->getIndexContainer();
	UIntN participantIndex = idx->getIndex((IndexStructPtr)participantHandle);
	UIntN domainIndex = idx->getIndex((IndexStructPtr)domainHandle);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	// Added to help debug issue with missing temperature threshold events
	if (frameworkEvent == FrameworkEvent::DomainTemperatureThresholdCrossed)
	{
		ManagerMessage temperatureMessage =
			ManagerMessage(dptfManager, FLF, "Received temperature threshold crossed event");
		temperatureMessage.setParticipantAndDomainIndex(participantIndex, domainIndex);
		temperatureMessage.setFrameworkEvent(frameworkEvent);
		dptfManager->getEsifServices()->writeMessageDebug(temperatureMessage, MessageCategory::TemperatureThresholds);
	}
#endif

	ManagerMessage startMessage = ManagerMessage(dptfManager, FLF, "Received event notification from ESIF");
	startMessage.setParticipantAndDomainIndex(participantIndex, domainIndex);
	startMessage.setFrameworkEvent(frameworkEvent);
	dptfManager->getEsifServices()->writeMessageInfo(startMessage);

	eEsifError rc = ESIF_OK;

	try
	{
		WorkItem* wi = nullptr;
		UInt32 uint32param = Constants::Invalid;

		switch (frameworkEvent)
		{
			// FIXME:  DptfConnectedStandbyEntry/DptfConnectedStandbyExit aren't used today so this isn't a high
			// priority.
			//        Should these return synchronously?  If so they don't belong here.
		case FrameworkEvent::DptfConnectedStandbyEntry:
			wi = new WIDptfConnectedStandbyEntry(dptfManager);
			break;
		case FrameworkEvent::DptfConnectedStandbyExit:
			wi = new WIDptfConnectedStandbyExit(dptfManager);
			break;
		case FrameworkEvent::DptfLogVerbosityChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			dptfManager->getEsifServices()->setCurrentLogVerbosityLevel((eLogType)uint32param);
			break;
		case FrameworkEvent::DptfParticipantActivityLoggingEnabled:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIDptfParticipantActivityLoggingEnabled(dptfManager, participantIndex, domainIndex, uint32param);
			break;
		case FrameworkEvent::DptfParticipantActivityLoggingDisabled:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIDptfParticipantActivityLoggingDisabled(dptfManager, participantIndex, domainIndex, uint32param);
			break;
		case FrameworkEvent::DptfPolicyActivityLoggingEnabled:
			wi = new WIDptfPolicyActivityLoggingEnabled(dptfManager);
			break;
		case FrameworkEvent::DptfPolicyActivityLoggingDisabled:
			wi = new WIDptfPolicyActivityLoggingDisabled(dptfManager);
			break;
		case FrameworkEvent::ParticipantSpecificInfoChanged:
			wi = new WIParticipantSpecificInfoChanged(dptfManager, participantIndex);
			break;
		case FrameworkEvent::DomainConfigTdpCapabilityChanged:
			wi = new WIDomainConfigTdpCapabilityChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainCoreControlCapabilityChanged:
			wi = new WIDomainCoreControlCapabilityChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainDisplayControlCapabilityChanged:
			wi = new WIDomainDisplayControlCapabilityChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainDisplayStatusChanged:
			wi = new WIDomainDisplayStatusChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainPerformanceControlCapabilityChanged:
			wi = new WIDomainPerformanceControlCapabilityChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainPerformanceControlsChanged:
			wi = new WIDomainPerformanceControlsChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainPowerControlCapabilityChanged:
			wi = new WIDomainPowerControlCapabilityChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainPriorityChanged:
			wi = new WIDomainPriorityChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainRadioConnectionStatusChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIDomainRadioConnectionStatusChanged(
				dptfManager, participantIndex, domainIndex, (RadioConnectionStatus::Type)uint32param);
			break;
		case FrameworkEvent::DomainRfProfileChanged:
			wi = new WIDomainRfProfileChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainTemperatureThresholdCrossed:
			wi = new WIDomainTemperatureThresholdCrossed(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainVirtualSensorCalibrationTableChanged:
			wi = new WIDomainVirtualSensorCalibrationTableChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainVirtualSensorPollingTableChanged:
			wi = new WIDomainVirtualSensorPollingTableChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainVirtualSensorRecalcChanged:
			wi = new WIDomainVirtualSensorRecalcChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainBatteryStatusChanged:
			wi = new WIDomainBatteryStatusChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainBatteryInformationChanged:
			wi = new WIDomainBatteryInformationChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainPlatformPowerSourceChanged:
			wi = new WIDomainPlatformPowerSourceChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainAdapterPowerRatingChanged:
			wi = new WIDomainAdapterPowerRatingChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainChargerTypeChanged:
			wi = new WIDomainChargerTypeChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainPlatformRestOfPowerChanged:
			wi = new WIDomainPlatformRestOfPowerChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainMaxBatteryPowerChanged:
			wi = new WIDomainMaxBatteryPowerChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainPlatformBatterySteadyStateChanged:
			wi = new WIDomainPlatformBatterySteadyStateChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainACNominalVoltageChanged:
			wi = new WIDomainACNominalVoltageChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainACOperationalCurrentChanged:
			wi = new WIDomainACOperationalCurrentChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainAC1msPercentageOverloadChanged:
			wi = new WIDomainAC1msPercentageOverloadChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainAC2msPercentageOverloadChanged:
			wi = new WIDomainAC2msPercentageOverloadChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainAC10msPercentageOverloadChanged:
			wi = new WIDomainAC10msPercentageOverloadChanged(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::DomainEnergyThresholdCrossed:
			wi = new WIDomainEnergyThresholdCrossed(dptfManager, participantIndex, domainIndex);
			break;
		case FrameworkEvent::PolicyActiveRelationshipTableChanged:
			wi = new WIPolicyActiveRelationshipTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyCoolingModePolicyChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyCoolingModePolicyChanged(dptfManager, (CoolingMode::Type)uint32param);
			break;
		case FrameworkEvent::PolicyForegroundApplicationChanged:
			wi = new WIPolicyForegroundApplicationChanged(dptfManager, EsifDataString(esifEventDataPtr));
			break;
		case FrameworkEvent::PolicyOperatingSystemConfigTdpLevelChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyOperatingSystemConfigTdpLevelChanged(dptfManager, uint32param);
			break;
		case FrameworkEvent::PolicyOperatingSystemPowerSourceChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyOperatingSystemPowerSourceChanged(dptfManager, (OsPowerSource::Type)uint32param);
			break;
		case FrameworkEvent::PolicyOperatingSystemLidStateChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyOperatingSystemLidStateChanged(dptfManager, (OsLidState::Type)uint32param);
			break;
		case FrameworkEvent::PolicyOperatingSystemBatteryPercentageChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyOperatingSystemBatteryPercentageChanged(dptfManager, uint32param);
			break;
		case FrameworkEvent::PolicyOperatingSystemPlatformTypeChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyOperatingSystemPlatformTypeChanged(dptfManager, (OsPlatformType::Type)uint32param);
			break;
		case FrameworkEvent::PolicyOperatingSystemDockModeChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyOperatingSystemDockModeChanged(dptfManager, (OsDockMode::Type)uint32param);
			break;
		case FrameworkEvent::PolicyOperatingSystemMobileNotification:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicyOperatingSystemMobileNotification(dptfManager, uint32param);
			break;
		case FrameworkEvent::PolicyPassiveTableChanged:
			wi = new WIPolicyPassiveTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicySensorOrientationChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicySensorOrientationChanged(dptfManager, (SensorOrientation::Type)uint32param);
			break;
		case FrameworkEvent::PolicySensorMotionChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicySensorMotionChanged(dptfManager, (OnOffToggle::Type)uint32param);
			break;
		case FrameworkEvent::PolicySensorSpatialOrientationChanged:
			uint32param = EsifDataUInt32(esifEventDataPtr);
			wi = new WIPolicySensorSpatialOrientationChanged(dptfManager, (SensorSpatialOrientation::Type)uint32param);
			break;
		case FrameworkEvent::PolicyThermalRelationshipTableChanged:
			wi = new WIPolicyThermalRelationshipTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyAdaptivePerformanceConditionsTableChanged:
			wi = new WIPolicyAdaptivePerformanceConditionsTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyAdaptivePerformanceParticipantConditionTableChanged:
			wi = new WIPolicyAdaptivePerformanceParticipantConditionTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyAdaptivePerformanceActionsTableChanged:
			wi = new WIPolicyAdaptivePerformanceActionsTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyOemVariablesChanged:
			wi = new WIPolicyOemVariablesChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyPowerBossConditionsTableChanged:
			wi = new WIPolicyPowerBossConditionsTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyPowerBossActionsTableChanged:
			wi = new WIPolicyPowerBossActionsTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyPowerBossMathTableChanged:
			wi = new WIPolicyPowerBossMathTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyOperatingSystemPowerSchemePersonalityChanged:
		{
			EsifDataGuid guidParam(esifEventDataPtr);
			uint32param = OsPowerSchemePersonality::toType(Guid(guidParam));
			wi = new WIPolicyOperatingSystemPowerSchemePersonalityChanged(
				dptfManager, (OsPowerSchemePersonality::Type)uint32param);
			break;
		}
		case FrameworkEvent::PolicyEmergencyCallModeTableChanged:
			wi = new WIPolicyEmergencyCallModeTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyPidAlgorithmTableChanged:
			wi = new WIPolicyPidAlgorithmTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyActiveControlPointRelationshipTableChanged:
			wi = new WIPolicyActiveControlPointRelationshipTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyPowerShareAlgorithmTableChanged:
			wi = new WIPolicyPowerShareAlgorithmTableChanged(dptfManager);
			break;
		case FrameworkEvent::PolicyWorkloadHintConfigurationChanged:
			wi = new WIPolicyWorkloadHintConfigurationChanged(dptfManager);
			break;
		case FrameworkEvent::DptfAppUnloading:
			break;
		case FrameworkEvent::DptfSupportedPoliciesChanged:
			wi = new WIPolicySupportedListChanged(dptfManager);
			waitForEventToProcess = true;
			break;
		case FrameworkEvent::DptfAppAliveRequest:
			wi = new WIApplicationAliveRequest(dptfManager);
			break;
		default:
		{
			ManagerMessage message = ManagerMessage(dptfManager, FLF, "Received unexpected event");
			message.setParticipantAndDomainIndex(participantIndex, domainIndex);
			message.setFrameworkEvent(frameworkEvent);
			dptfManager->getEsifServices()->writeMessageWarning(message);
			rc = ESIF_E_NOT_SUPPORTED;
			break;
		}
		}

		if (wi != nullptr)
		{
			if (waitForEventToProcess)
			{
				dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(wi);
			}
			else
			{
				dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);
			}
		}
	}
	catch (duplicate_work_item ex)
	{
		ManagerMessage message =
			ManagerMessage(dptfManager, FLF, "Discarding duplicate event.  Already in immediate queue.");
		message.setParticipantAndDomainIndex(participantIndex, domainIndex);
		message.setFrameworkEvent(frameworkEvent);
		message.setExceptionCaught("WorkItemQueueManager::enqueueImmediateWorkItemAndReturn", ex.what());
		dptfManager->getEsifServices()->writeMessageDebug(message);
		rc = ESIF_OK;
	}
	catch (std::exception& ex)
	{
		ManagerMessage message =
			ManagerMessage(dptfManager, FLF, "Error while trying to create work item for event received from ESIF.");
		message.setParticipantAndDomainIndex(participantIndex, domainIndex);
		message.setFrameworkEvent(frameworkEvent);
		message.setExceptionCaught("DptfEvent", ex.what());
		dptfManager->getEsifServices()->writeMessageError(message);
		rc = ESIF_E_UNSPECIFIED;
	}

	return rc;
}

dptf_export eEsifError GetApplicationInterface(AppInterfacePtr appInterfacePtr)
{
	// header
	appInterfacePtr->fIfaceType = eIfaceTypeApplication;
	appInterfacePtr->fIfaceSize = (UInt16)sizeof(AppInterface);
	appInterfacePtr->fIfaceVersion = APP_INTERFACE_VERSION;

	/* Application */
	appInterfacePtr->fAppAllocateHandleFuncPtr = DptfAllocateHandle;
	appInterfacePtr->fAppCreateFuncPtr = DptfCreate;
	appInterfacePtr->fAppDestroyFuncPtr = DptfDestroy;
	appInterfacePtr->fAppSuspendFuncPtr = DptfSuspend;
	appInterfacePtr->fAppResumeFuncPtr = DptfResume;

	appInterfacePtr->fAppCommandFuncPtr = DptfCommand;
	appInterfacePtr->fAppGetAboutFuncPtr = GetDptfAbout;
	appInterfacePtr->fAppGetBannerFuncPtr = GetDptfBanner;
	appInterfacePtr->fAppGetDescriptionFuncPtr = GetDptfDescription;
	appInterfacePtr->fAppGetGuidFuncPtr = GetDptfGuid;
	appInterfacePtr->fAppGetNameFuncPtr = GetDptfName;
	appInterfacePtr->fAppGetPromptFuncPtr = GetDptfPrompt;
	appInterfacePtr->fAppGetStatusFuncPtr = GetDptfStatus;
	appInterfacePtr->fAppGetVersionFuncPtr = GetDptfVersion;
	appInterfacePtr->fAppSetStateFuncPtr = SetDptfState;

	/* Participant */
	appInterfacePtr->fParticipantAllocateHandleFuncPtr = ParticipantAllocateHandle;
	appInterfacePtr->fParticipantCreateFuncPtr = ParticipantCreate;
	appInterfacePtr->fParticipantDestroyFuncPtr = ParticipantDestroy;
	appInterfacePtr->fParticipantSetStateFuncPtr = ParticipantSetState;

	/* Domain */
	appInterfacePtr->fDomainAllocateHandleFuncPtr = DomainAllocateHandle;
	appInterfacePtr->fDomainCreateFuncPtr = DomainCreate;
	appInterfacePtr->fDomainDestroyFuncPtr = DomainDestroy;
	appInterfacePtr->fDomainSetStateFuncPtr = DomainSetState;

	/* Event */
	appInterfacePtr->fAppEventFuncPtr = DptfEvent;

	return ESIF_OK;
}
} // extern "C"
