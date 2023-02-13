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
#include "CommandDispatcher.h"
#include <iostream>
#include "WIRunCommand.h"
#include "esif_sdk_dcfg.h"
#include "ipf_sdk_version.h"
#include "ipf_sdk_version_check.h"
#include "EsifAppBroadcastProcessing.h"

// Minimum Required IPF SDK Version for this App to load (i.e., NULL, "1.0.0", IPF_SDK_VERSION, Specific Version, etc.)
#define IPFSDK_MINIMUM_VERSION_REQUIRED "1.0.0"

//
// Macros must be used to reduce the code and still allow writing out the file name, line number, and function name
//

#define RETURN_ERROR_IF_CONTEXT_DATA_NULL                                                                              \
	if (dptfManager == nullptr)                                                                                        \
	{                                                                                                                  \
		return ESIF_E_PARAMETER_IS_NULL;                                                                               \
	}

#define RETURN_ERROR_IF_WORK_ITEM_QUEUE_MANAGER_NOT_CREATED                                                            \
	if (dptfManager->isWorkItemQueueManagerCreated() == false)                                                         \
	{                                                                                                                  \
		if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeError)                                        \
		{                                                                                                              \
			ManagerMessage message = ManagerMessage(                                                                   \
				dptfManager, FLF, "Function call from ESIF ignored.  DPTF work item queue has not been created.");     \
			dptfManager->getEsifServices()->writeMessageError(message);                                                \
		}                                                                                                              \
		return ESIF_E_UNSPECIFIED;                                                                                     \
	}

#define RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED                                                                       \
	if (dptfManager->isDptfManagerCreated() == false)                                                                  \
	{                                                                                                                  \
		if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeError)                                        \
		{                                                                                                              \
			ManagerMessage message = ManagerMessage(                                                                   \
				dptfManager, FLF, "Function call from ESIF ignored.  DPTF manager creation is not complete.");         \
			dptfManager->getEsifServices()->writeMessageError(message);                                                \
		}                                                                                                              \
		return ESIF_E_UNSPECIFIED;                                                                                     \
	}

#define RETURN_WARNING_IF_DPTF_SHUTTING_DOWN                                                                           \
	if (dptfManager->isDptfShuttingDown() == true)                                                                     \
	{                                                                                                                  \
		if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeWarning)                                      \
		{                                                                                                              \
			ManagerMessage message =                                                                                   \
				ManagerMessage(dptfManager, FLF, "Function call from ESIF ignored.  DPTF manager is shutting down.");  \
			dptfManager->getEsifServices()->writeMessageWarning(message);                                              \
		}                                                                                                              \
		return ESIF_E_UNSPECIFIED;                                                                                     \
	}

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

extern "C"
{
	static Bool DptfIsIpfClient = ESIF_FALSE;		// Running as an out-of-process IPF Client
	static char DptfIpfSdkVersion[ESIF_NAME_LEN];	// IPF Server SDK Version (possibly higher or lower than IPF_SDK_VERSION)

	/* Check that this App is allowed to run according to specified rules:
	** 1. Check Required Server SDK Version (if any)
	** 2. Check DCFG when running In-Process (enforced by IPFSRV when Out-of-Process)
	*/
	static esif_error_t DptfVerifyAppEnabled(AppInterfaceSetPtr ifaceSetPtr, const esif_handle_t esifHandle)
	{
		esif_error_t rc = ESIF_OK;

		// 1. Check that Server is using the Minimum Required IPF_SDK_VERSION (if specified) even if Client passes Server version check.
		if (IPFSDK_MINIMUM_VERSION_REQUIRED && ifaceSetPtr && esifHandle != ESIF_INVALID_HANDLE)
		{
			rc = IpfSdk_VersionCheckEsifIface(
				esifHandle,
				ifaceSetPtr->esifIface,
				DptfIpfSdkVersion,
				sizeof(DptfIpfSdkVersion)
			);
			if (rc == ESIF_OK && IpfSdk_VersionFromString(DptfIpfSdkVersion) < IpfSdk_VersionFromString(IPFSDK_MINIMUM_VERSION_REQUIRED))
			{
				rc = ESIF_E_NOT_SUPPORTED;
			}
		}
		
		// 2. Check whether App is disabled in DCFG. Change to just verify Out-of-Process if In-Process support ever removed.
		if (rc == ESIF_OK && ifaceSetPtr && esifHandle != ESIF_INVALID_HANDLE && !DptfIsIpfClient)
		{
			DCfgOptions dcfg = {{ 0 }};
			EsifData request = { ESIF_DATA_VOID };
			EsifData response = { ESIF_DATA_UINT32 };
			response.buf_ptr = &dcfg.asU32;
			response.buf_len = (u32)sizeof(dcfg);
			esif_error_t dcfgrc = ifaceSetPtr->esifIface.fPrimitiveFuncPtr(
				esifHandle,
				ESIF_HANDLE_PRIMARY_PARTICIPANT,
				ESIF_INVALID_HANDLE,
				&request,
				&response,
				GET_CONFIG_ACCESS_CONTROL_SUR,
				ESIF_INSTANCE_INVALID
			);
			if (dcfgrc == ESIF_OK && dcfg.opt.DttAccessControl)
			{
				rc = ESIF_E_DISABLED;
			}
		}
		return rc;
	}

	static eEsifError GetDptfDescription(EsifDataPtr dataPtr)
	{
		return FillDataPtrWithString(dataPtr, "Intel(R) Dynamic Tuning Technology Manager");
	}

	static eEsifError GetDptfName(EsifDataPtr dataPtr)
	{
		char client_prefix[] = "ipfcli-";
		if (esif_ccb_strncmp((const char *)dataPtr->buf_ptr, client_prefix, sizeof(client_prefix) - 1) == 0) {
			DptfIsIpfClient = ESIF_TRUE;
		}
		return FillDataPtrWithString(dataPtr, "DPTF");
	}

	static eEsifError GetDptfVersion(EsifDataPtr dataPtr)
	{
		return FillDataPtrWithString(dataPtr, VERSION_STR);
	}

	static eEsifError DptfAllocateHandle(esif_handle_t* appHandleLocation)
	{
		DEBUG_MEMORY_LEAK_INIT();
		DptfManagerInterface* dptfManager = nullptr;

		try
		{
			dptfManager = new DptfManager();
			*appHandleLocation = reinterpret_cast<esif_handle_t>(dptfManager);
		}
		catch (...)
		{
			dptfManager = nullptr;
			*appHandleLocation = reinterpret_cast<esif_handle_t>(nullptr);
		}

		return (dptfManager != nullptr) ? ESIF_OK : ESIF_E_UNSPECIFIED;
	}

	static eEsifError DptfCreate(
		AppInterfaceSetPtr ifaceSetPtr,
		const esif_handle_t esifHandle,
		esif_handle_t* appHandlePtr,
		const AppDataPtr appData,
		const eAppState appInitialState)
	{
		eEsifError rc = ESIF_OK;
		esif_handle_t appHandle = ESIF_INVALID_HANDLE;

		// Make sure we received what we are expecting from ESIF.  We can't start the application if we don't have
		// the ESIF interface pointers.  In this case we will check everything manually here instead of in a
		// constructor. If this fails we can't throw an exception or log a message since the infrastructure isn't up.
		// All we can do is return an error.
		if (ifaceSetPtr == nullptr || appHandlePtr == nullptr || ifaceSetPtr->hdr.fIfaceType != eIfaceTypeApplication
			|| ifaceSetPtr->hdr.fIfaceVersion != APP_INTERFACE_VERSION
			|| ifaceSetPtr->hdr.fIfaceSize != (UInt16)sizeof(*ifaceSetPtr)
			|| ifaceSetPtr->esifIface.fGetConfigFuncPtr == nullptr
			|| ifaceSetPtr->esifIface.fSetConfigFuncPtr == nullptr
			|| ifaceSetPtr->esifIface.fPrimitiveFuncPtr == nullptr
			|| ifaceSetPtr->esifIface.fWriteLogFuncPtr == nullptr
			|| ifaceSetPtr->esifIface.fRegisterEventFuncPtr == nullptr
			|| ifaceSetPtr->esifIface.fUnregisterEventFuncPtr == nullptr
			|| ifaceSetPtr->esifIface.fSendEventFuncPtr == nullptr
			|| ifaceSetPtr->esifIface.fSendCommandFuncPtr == nullptr
			|| (appData == nullptr))
		{
			rc = ESIF_E_CALLBACK_IS_NULL;
		}
		// Verify that this app is not Disabled or Incompatible before proceeding
		else if (ESIF_OK != (rc = DptfVerifyAppEnabled(ifaceSetPtr, esifHandle)))
		{
			*appHandlePtr = ESIF_INVALID_HANDLE;
		}
		else if (ESIF_OK == (rc = DptfAllocateHandle(&appHandle)))
		{
			// The app handle must be returned to ESIF prior to any calls back into the ESIF service interface
			*appHandlePtr = appHandle;

			try
			{
				eLogType currentLogVerbosityLevel = appData->fLogLevel;

				if (eLogType::eLogTypeInfo <= currentLogVerbosityLevel)
				{
					ifaceSetPtr->esifIface.fWriteLogFuncPtr(
						esifHandle,
						ESIF_INVALID_HANDLE,
						ESIF_INVALID_HANDLE,
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
					esifHandle, &ifaceSetPtr->esifIface, dptfHomeDirectoryPath, currentLogVerbosityLevel, enabled);

				if (eLogType::eLogTypeInfo <= currentLogVerbosityLevel)
				{
					ifaceSetPtr->esifIface.fWriteLogFuncPtr(
						esifHandle,
						0,
						ESIF_INVALID_HANDLE,
						EsifDataString("DptfCreate: Initialization completed."),
						eLogType::eLogTypeInfo);
				}
			}
			catch (...)
			{
				ifaceSetPtr->esifIface.fWriteLogFuncPtr(
					esifHandle,
					0,
					ESIF_INVALID_HANDLE,
					EsifDataString("DptfCreate: Initialization failed."),
					eLogType::eLogTypeFatal);
				rc = ESIF_E_UNSPECIFIED;
			}
		}

		return rc;
	}

	static eEsifError DptfDestroy(const esif_handle_t appHandle)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeInfo)
		{
			ManagerMessage message = ManagerMessage(dptfManager, FLF, "Function execution beginning.");
			dptfManager->getEsifServices()->writeMessageInfo(message);
		}

		eEsifError rc = ESIF_OK;

		try
		{
			DELETE_MEMORY(dptfManager);
		}
		catch (...)
		{
			rc = ESIF_E_UNSPECIFIED;
		}

		return rc;
	}

	static eEsifError DptfSuspend(esif_handle_t appHandle)
	{
		return ESIF_OK;

		// FIXME:
		//
		// Problem:
		//  We've temporarily commented out the code below to prevent a deadlock.  We added both the DptfSuspend and
		//  DptfResume functions specifically for the case of Critical policy needing to process temperature
		//  threshold events on D0 Entry.  However, we have a deadlock.  When Critical policy (using a work item
		//  thread) calls to shut down the system, we automatically get a call to DptfSuspend.  But the
		//  WIDptfSuspend work item never executes because it is waiting for the thread to return from the call to
		//  shut down the system.
		//
		// Solution:
		//  Update the policy services sleep(), hibernate(), and shutdown() functions.  The Manager should create a
		//  separate thread for these calls so we don't have the deadlock on the work item thread.
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

	static eEsifError DptfResume(const esif_handle_t appHandle)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		eEsifError rc = ESIF_OK;

		try
		{
			auto workItem = std::make_shared<WIDptfResume>(dptfManager);
			dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
		}
		catch (...)
		{
			rc = ESIF_E_UNSPECIFIED;
		}

		return rc;
	}

	static eEsifError GetDptfBanner(const esif_handle_t appHandle, EsifDataPtr dataPtr)
	{
		return FillDataPtrWithString(dataPtr, "Intel(R) Dynamic Tuning Technology Manager v" VERSION_STR " [" ESIF_ATTR_OS " " ESIF_PLATFORM_TYPE " " ESIF_BUILD_TYPE "]");
	}

	esif_error_t checkCommandParameters(
		const esif_handle_t appHandle,
		const UInt32 argc,
		const EsifDataArray argv,
		const EsifDataPtr response)
	{
		if (appHandle == ESIF_INVALID_HANDLE || argv == NULL || response == NULL)
		{
			return ESIF_E_PARAMETER_IS_NULL;
		}
		else
		{
			return ESIF_OK;
		}
	}

	static eEsifError DptfCommand(
		const esif_handle_t appHandle,
		const UInt32 argc,
		const EsifDataArray argv,
		EsifDataPtr response)
	{
		eEsifError rc = checkCommandParameters(appHandle, argc, argv, response);
		if (rc != ESIF_OK)
		{
			return rc;
		}

		// argv is an array of EsifData objects of length argc (1 or greater)
		// Current implementation assumes an array of STRING types but future versions could support other types
		// First argument is DPTF command and remaining arguments, if any, are parameters to that DPTF Command
		// <appcmd> [arg] [...]

		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		try
		{
			auto arguments = CommandArguments::parse(argc, argv);
			auto wiCommand = std::make_shared<WIRunCommand>(dptfManager, arguments);
			dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(wiCommand);
			rc = FillDataPtrWithString(response, wiCommand->getLastMessageWithNewline());
		}
		catch (...)
		{
			FillDataPtrWithString(response, "An unknown error occurred when processing app command.");
		}
		return rc;
	}

	static eEsifError GetDptfStatus(
		const esif_handle_t appHandle,
		const eAppStatusCommand command,
		const UInt32 appStatusIn,
		EsifDataPtr appStatusOut)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		eEsifError rc = ESIF_E_UNSPECIFIED;

		return rc;
	}

	static eEsifError ParticipantCreate(
		const esif_handle_t appHandle,
		esif_handle_t participantHandle,
		const AppParticipantDataPtr participantDataPtr,
		const eParticipantState participantInitialState)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		UIntN newParticipantIndex = Constants::Invalid;
		Bool participantCreated = false;

		try
		{
			auto allocWorkItem = std::make_shared<WIParticipantAllocate>(dptfManager, &newParticipantIndex);
			dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(allocWorkItem);

			if (newParticipantIndex != Constants::Esif::NoParticipant)
			{
				Bool participantEnabled = (participantInitialState == eParticipantState::eParticipantStateEnabled);

				dptfManager->getIndexContainer()->insertHandle(
					newParticipantIndex, Constants::Invalid, participantHandle, ESIF_INVALID_HANDLE);

				auto workItem = std::make_shared<WIParticipantCreate>(
					dptfManager, newParticipantIndex, participantDataPtr, participantEnabled, &participantCreated);
				dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
			}
		}
		catch (...)
		{
			dptfManager->getIndexContainer()->removeHandle(participantHandle, ESIF_INVALID_HANDLE);
			participantCreated = false;
		}

		return (participantCreated == true) ? ESIF_OK : ESIF_E_UNSPECIFIED;
	}

	eEsifError ParticipantDestroy(const esif_handle_t appHandle, const esif_handle_t participantHandle)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		eEsifError rc = ESIF_OK;

		try
		{
			auto workItem = std::make_shared<WIParticipantDestroy>(
				dptfManager, dptfManager->getIndexContainer()->getParticipantIndex(participantHandle));
			dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
		}
		catch (...)
		{
			rc = ESIF_E_UNSPECIFIED;
		}

		dptfManager->getIndexContainer()->removeHandle(participantHandle, ESIF_INVALID_HANDLE);

		return rc;
	}

	static eEsifError DomainCreate(
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle,
		const AppDomainDataPtr domainDataPtr,
		const eDomainState domainInitialState)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		UIntN newDomainIndex = Constants::Esif::NoDomain;
		Bool domainCreated = false;

		try
		{
			auto allocWorkItem = std::make_shared<WIDomainAllocate>(
				dptfManager, dptfManager->getIndexContainer()->getParticipantIndex(participantHandle), &newDomainIndex);
			dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(allocWorkItem);

			if (newDomainIndex != Constants::Esif::NoDomain)
			{
				Bool domainEnabled = (domainInitialState == eDomainState::eDomainStateEnabled);

				dptfManager->getIndexContainer()->insertHandle(
					dptfManager->getIndexContainer()->getParticipantIndex(participantHandle),
					newDomainIndex,
					participantHandle,
					domainHandle);

				auto workItem = std::make_shared<WIDomainCreate>(
					dptfManager,
					dptfManager->getIndexContainer()->getParticipantIndex(participantHandle),
					newDomainIndex,
					domainDataPtr,
					domainEnabled,
					&domainCreated);
				dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
			}
		}
		catch (...)
		{
			dptfManager->getIndexContainer()->removeHandle(participantHandle, domainHandle);
			domainCreated = false;
		}

		return (domainCreated == true) ? ESIF_OK : ESIF_E_UNSPECIFIED;
	}

	static eEsifError DomainDestroy(
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle)
	{
		DptfManagerInterface* dptfManager = (DptfManagerInterface*)appHandle;
		RETURN_ERROR_IF_CONTEXT_DATA_NULL;
		RETURN_ERROR_IF_DPTF_MANAGER_NOT_CREATED;
		RETURN_WARNING_IF_DPTF_SHUTTING_DOWN;

		eEsifError rc = ESIF_OK;

		try
		{
			auto workItem = std::make_shared<WIDomainDestroy>(
				dptfManager,
				dptfManager->getIndexContainer()->getParticipantIndex(participantHandle),
				dptfManager->getIndexContainer()->getDomainIndex(participantHandle, domainHandle));
			dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);

			dptfManager->getIndexContainer()->removeHandle(participantHandle, domainHandle);
		}
		catch (...)
		{
			rc = ESIF_E_UNSPECIFIED;
		}

		dptfManager->getIndexContainer()->removeHandle(participantHandle, domainHandle);
		return rc;
	}

	static eEsifError DptfEvent(
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle,
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

			if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeWarning)
			{
				dptfManager->getEsifServices()->writeMessageWarning(message);
			}
			return ESIF_E_UNSPECIFIED;
		}

		IndexContainerInterface* idx = dptfManager->getIndexContainer();
		UIntN participantIndex = idx->getParticipantIndex(participantHandle);
		UIntN domainIndex = idx->getDomainIndex(participantHandle, domainHandle);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
		// Added to help debug issue with missing temperature threshold events
		if (frameworkEvent == FrameworkEvent::DomainTemperatureThresholdCrossed)
		{
			ManagerMessage temperatureMessage =
				ManagerMessage(dptfManager, FLF, "Received temperature threshold crossed event");
			temperatureMessage.setParticipantAndDomainIndex(participantIndex, domainIndex);
			temperatureMessage.setFrameworkEvent(frameworkEvent);
			if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeDebug)
			{
				dptfManager->getEsifServices()->writeMessageDebug(
					temperatureMessage, MessageCategory::TemperatureThresholds);
			}
		}
#endif

		ManagerMessage startMessage = ManagerMessage(dptfManager, FLF, "Received event notification from ESIF");
		startMessage.setParticipantAndDomainIndex(participantIndex, domainIndex);
		startMessage.setFrameworkEvent(frameworkEvent);
		if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeInfo)
		{
			dptfManager->getEsifServices()->writeMessageInfo(startMessage);
		}

		eEsifError rc = ESIF_OK;
		try
		{
			std::shared_ptr<WorkItem> wi;
			UInt32 uint32param = Constants::Invalid;

			switch (frameworkEvent)
			{
			case FrameworkEvent::DptfAppBroadcastUnprivileged:
			case FrameworkEvent::DptfAppBroadcastPrivileged:
				wi = EsifAppBroadcastProcessing::FindAppBroadcastIdAndCreateWorkItem(
					dptfManager, esifEventDataPtr);
				break;
			// FIXME:  DptfConnectedStandbyEntry/DptfConnectedStandbyExit aren't used today so this isn't a high
			// priority.
			//        Should these return synchronously?  If so they don't belong here.
			case FrameworkEvent::DptfConnectedStandbyEntry:
				wi = std::make_shared<WIDptfConnectedStandbyEntry>(dptfManager);
				break;
			case FrameworkEvent::DptfConnectedStandbyExit:
				wi = std::make_shared<WIDptfConnectedStandbyExit>(dptfManager);
				break;
			case FrameworkEvent::DptfLogVerbosityChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				dptfManager->setCurrentLogVerbosityLevel((eLogType)uint32param);
				break;
			case FrameworkEvent::DptfParticipantActivityLoggingEnabled:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIDptfParticipantActivityLoggingEnabled>(
					dptfManager, participantIndex, domainIndex, uint32param);
				break;
			case FrameworkEvent::DptfParticipantActivityLoggingDisabled:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIDptfParticipantActivityLoggingDisabled>(
					dptfManager, participantIndex, domainIndex, uint32param);
				break;
			case FrameworkEvent::DptfPolicyActivityLoggingEnabled:
				wi = std::make_shared<WIDptfPolicyActivityLoggingEnabled>(dptfManager);
				break;
			case FrameworkEvent::DptfPolicyActivityLoggingDisabled:
				wi = std::make_shared<WIDptfPolicyActivityLoggingDisabled>(dptfManager);
				break;
			case FrameworkEvent::ParticipantSpecificInfoChanged:
				wi = std::make_shared<WIParticipantSpecificInfoChanged>(dptfManager, participantIndex);
				break;
			case FrameworkEvent::DomainCoreControlCapabilityChanged:
				wi = std::make_shared<WIDomainCoreControlCapabilityChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainDisplayControlCapabilityChanged:
				wi = std::make_shared<WIDomainDisplayControlCapabilityChanged>(
					dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainDisplayStatusChanged:
				wi = std::make_shared<WIDomainDisplayStatusChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainPerformanceControlCapabilityChanged:
				wi = std::make_shared<WIDomainPerformanceControlCapabilityChanged>(
					dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainPerformanceControlsChanged:
				wi = std::make_shared<WIDomainPerformanceControlsChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainPowerControlCapabilityChanged:
				wi =
					std::make_shared<WIDomainPowerControlCapabilityChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainPriorityChanged:
				wi = std::make_shared<WIDomainPriorityChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainRadioConnectionStatusChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIDomainRadioConnectionStatusChanged>(
					dptfManager, participantIndex, domainIndex, (RadioConnectionStatus::Type)uint32param);
				break;
			case FrameworkEvent::DomainRfProfileChanged:
				wi = std::make_shared<WIDomainRfProfileChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainTemperatureThresholdCrossed:
				wi = std::make_shared<WIDomainTemperatureThresholdCrossed>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainVirtualSensorRecalcChanged:
				wi = std::make_shared<WIDomainVirtualSensorRecalcChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainBatteryStatusChanged:
				wi = std::make_shared<WIDomainBatteryStatusChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainBatteryInformationChanged:
				wi = std::make_shared<WIDomainBatteryInformationChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainBatteryHighFrequencyImpedanceChanged:
				wi = std::make_shared<WIDomainBatteryHighFrequencyImpedanceChanged>(
					dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainBatteryNoLoadVoltageChanged:
				wi = std::make_shared<WIDomainBatteryNoLoadVoltageChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainPlatformPowerSourceChanged:
				wi = std::make_shared<WIDomainPlatformPowerSourceChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainAdapterPowerRatingChanged:
				wi = std::make_shared<WIDomainAdapterPowerRatingChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainChargerTypeChanged:
				wi = std::make_shared<WIDomainChargerTypeChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainPlatformRestOfPowerChanged:
				wi = std::make_shared<WIDomainPlatformRestOfPowerChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainMaxBatteryPowerChanged:
				wi = std::make_shared<WIDomainMaxBatteryPowerChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainPlatformBatterySteadyStateChanged:
				wi = std::make_shared<WIDomainPlatformBatterySteadyStateChanged>(
					dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainACNominalVoltageChanged:
				wi = std::make_shared<WIDomainACNominalVoltageChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainACOperationalCurrentChanged:
				wi = std::make_shared<WIDomainACOperationalCurrentChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainAC1msPercentageOverloadChanged:
				wi = std::make_shared<WIDomainAC1msPercentageOverloadChanged>(
					dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainAC2msPercentageOverloadChanged:
				wi = std::make_shared<WIDomainAC2msPercentageOverloadChanged>(
					dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainAC10msPercentageOverloadChanged:
				wi = std::make_shared<WIDomainAC10msPercentageOverloadChanged>(
					dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainEnergyThresholdCrossed:
				wi = std::make_shared<WIDomainEnergyThresholdCrossed>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainFanCapabilityChanged:
				wi = std::make_shared<WIDomainFanCapabilityChanged>(dptfManager, participantIndex, domainIndex);
				break;
			case FrameworkEvent::DomainSocWorkloadClassificationChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIDomainSocWorkloadClassificationChanged>(
					dptfManager, participantIndex, domainIndex, (SocWorkloadClassification::Type)uint32param);
				break;
			case FrameworkEvent::DomainEppSensitivityHintChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIDomainEppSensitivityHintChanged>(
					dptfManager, participantIndex, domainIndex, (MbtHint::Type)uint32param);
				break;
			case FrameworkEvent::DomainExtendedWorkloadPredictionChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIDomainExtendedWorkloadPredictionChanged>(
					dptfManager, participantIndex, domainIndex, (ExtendedWorkloadPrediction::Type)uint32param);
				break;
			case FrameworkEvent::PolicyCoolingModePolicyChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyCoolingModePolicyChanged>(dptfManager, (CoolingMode::Type)uint32param);
				break;
			case FrameworkEvent::PolicyForegroundApplicationChanged:
				wi = std::make_shared<WIPolicyForegroundApplicationChanged>(
					dptfManager, EsifDataString(esifEventDataPtr));
				break;
			case FrameworkEvent::PolicyOperatingSystemPowerSourceChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemPowerSourceChanged>(
					dptfManager, (OsPowerSource::Type)uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemLidStateChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemLidStateChanged>(
					dptfManager, (OsLidState::Type)uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemBatteryPercentageChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemBatteryPercentageChanged>(dptfManager, uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemPlatformTypeChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemPlatformTypeChanged>(
					dptfManager, (OsPlatformType::Type)uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemDockModeChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemDockModeChanged>(
					dptfManager, (OsDockMode::Type)uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemMobileNotification:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemMobileNotification>(dptfManager, uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemMixedRealityModeChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemMixedRealityModeChanged>(
					dptfManager, OnOffToggle::toType(uint32param));
				break;
			case FrameworkEvent::PolicyOperatingSystemBatteryCountChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemBatteryCountChanged>(dptfManager, uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemPowerSliderChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemPowerSliderChanged>(
					dptfManager, OsPowerSlider::toType(uint32param));
				break;
			case FrameworkEvent::PolicySensorOrientationChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicySensorOrientationChanged>(
					dptfManager, (SensorOrientation::Type)uint32param);
				break;
			case FrameworkEvent::PolicySensorMotionChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicySensorMotionChanged>(dptfManager, OnOffToggle::toType(uint32param));
				break;
			case FrameworkEvent::PolicySensorSpatialOrientationChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicySensorSpatialOrientationChanged>(
					dptfManager, (SensorSpatialOrientation::Type)uint32param);
				break;
			case FrameworkEvent::PolicyOperatingSystemPowerSchemePersonalityChanged:
			{
				EsifDataGuid guidParam(esifEventDataPtr);
				uint32param = OsPowerSchemePersonality::toType(Guid(guidParam));
				wi = std::make_shared<WIPolicyOperatingSystemPowerSchemePersonalityChanged>(
					dptfManager, (OsPowerSchemePersonality::Type)uint32param);
				break;
			}
			case FrameworkEvent::PolicyOperatingSystemUserPresenceChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemUserPresenceChanged>(
					dptfManager, OsUserPresence::toType(uint32param));
				break;
			case FrameworkEvent::PolicyOperatingSystemSessionStateChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemSessionStateChanged>(
					dptfManager, OsSessionState::toType(uint32param));
				break;
			case FrameworkEvent::PolicyOperatingSystemScreenStateChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemScreenStateChanged>(
					dptfManager, OnOffToggle::toType(uint32param));
				break;
			case FrameworkEvent::PolicyEmergencyCallModeTableChanged:
				wi = std::make_shared<WIPolicyEmergencyCallModeTableChanged>(dptfManager);
				break;
			case FrameworkEvent::PolicyWorkloadHintConfigurationChanged:
				wi = std::make_shared<WIPolicyWorkloadHintConfigurationChanged>(dptfManager);
				break;
			case FrameworkEvent::DptfAppUnloading:
				break;
			case FrameworkEvent::DptfSupportedPoliciesChanged:
				wi = std::make_shared<WIPolicySupportedListChanged>(dptfManager);
				waitForEventToProcess = true;
				break;
			case FrameworkEvent::DptfAppAliveRequest:
				wi = std::make_shared<WIApplicationAliveRequest>(dptfManager);
				break;
			case FrameworkEvent::PolicyPlatformUserPresenceChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyPlatformUserPresenceChanged>(
					dptfManager, SensorUserPresence::toType(uint32param));
				break;
			case FrameworkEvent::PolicyExternalMonitorStateChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyExternalMonitorStateChanged>(dptfManager, Bool(uint32param));
				break;
			case FrameworkEvent::PolicyOperatingSystemGameModeChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyOperatingSystemGameModeChanged>(
					dptfManager, OnOffToggle::toType(uint32param));
				break;
			case FrameworkEvent::PolicyUserInteractionChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyUserInteractionChanged>(
					dptfManager, UserInteraction::toType(uint32param));
				break;
			case FrameworkEvent::PolicyForegroundRatioChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyForegroundRatioChanged>(dptfManager, uint32param);
				break;
			case FrameworkEvent::PolicyCollaborationChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyCollaborationChanged>(dptfManager, OnOffToggle::toType(uint32param));
				break;
			case FrameworkEvent::PolicyThirdPartyGraphicsPowerStateChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyThirdPartyGraphicsPowerStateChanged>(dptfManager, uint32param);
				break;
			case FrameworkEvent::DomainFanOperatingModeChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIDomainFanOperatingModeChanged>(
					dptfManager, participantIndex, domainIndex, FanOperatingMode::toType(uint32param));
				break;
			case FrameworkEvent::PolicyOemVariablesChanged:
				wi = std::make_shared<WIPolicyOemVariablesChanged>(dptfManager);
				break;
			case FrameworkEvent::PolicyThirdPartyGraphicsTPPLimitChanged:
				uint32param = EsifDataUInt32(esifEventDataPtr);
				wi = std::make_shared<WIPolicyThirdPartyGraphicsTPPLimitChanged>(dptfManager, (OsPowerSource::Type)uint32param);
				break;
			default:
			{
				ManagerMessage message = ManagerMessage(dptfManager, FLF, "Received unexpected event");
				message.setParticipantAndDomainIndex(participantIndex, domainIndex);
				message.setFrameworkEvent(frameworkEvent);
				if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeWarning)
				{
					dptfManager->getEsifServices()->writeMessageWarning(message);
				}
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
		catch (duplicate_work_item& ex)
		{
			ManagerMessage message =
				ManagerMessage(dptfManager, FLF, "Discarding duplicate event.  Already in immediate queue.");
			message.setParticipantAndDomainIndex(participantIndex, domainIndex);
			message.setFrameworkEvent(frameworkEvent);
			message.setExceptionCaught("WorkItemQueueManager::enqueueImmediateWorkItemAndReturn", ex.what());
			if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeDebug)
			{
				dptfManager->getEsifServices()->writeMessageDebug(message);
			}
			rc = ESIF_OK;
		}
		catch (std::exception& ex)
		{
			ManagerMessage message = ManagerMessage(
				dptfManager, FLF, "Error while trying to create work item for event received from ESIF.");
			message.setParticipantAndDomainIndex(participantIndex, domainIndex);
			message.setFrameworkEvent(frameworkEvent);
			message.setExceptionCaught("DptfEvent", ex.what());
			if (dptfManager->getEsifServices()->getLoggingLevel() >= eLogTypeWarning)
			{
				dptfManager->getEsifServices()->writeMessageWarning(message);
			}
			rc = ESIF_E_UNSPECIFIED;
		}

		return rc;
	}

	dptf_public_export eEsifError GetApplicationInterfaceV2(AppInterfaceSetPtr ifaceSetPtr)
	{
		eEsifError rc = ESIF_OK;
		/*
		 * First verify the size and version of the passed in structure in the
		 * header before supplying pointers.
		 */
		if ((ifaceSetPtr->hdr.fIfaceType != eIfaceTypeApplication)
			|| (ifaceSetPtr->hdr.fIfaceSize != (UInt16)sizeof(*ifaceSetPtr))
			|| (ifaceSetPtr->hdr.fIfaceVersion != APP_INTERFACE_VERSION))
		{
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}

		/*
		 * FIll in the application interface pointers at this time.
		 * NOTE: The ESIF pointers should not be saved or used at this time,
		 * and should only be used when the creation function is called.
		 */
		ifaceSetPtr->appIface.fAppCreateFuncPtr = DptfCreate;
		ifaceSetPtr->appIface.fAppDestroyFuncPtr = DptfDestroy;
		ifaceSetPtr->appIface.fAppSuspendFuncPtr = DptfSuspend;
		ifaceSetPtr->appIface.fAppResumeFuncPtr = DptfResume;

		ifaceSetPtr->appIface.fAppCommandFuncPtr = DptfCommand;
		ifaceSetPtr->appIface.fAppGetIntroFuncPtr = GetDptfBanner;
		ifaceSetPtr->appIface.fAppGetDescriptionFuncPtr = GetDptfDescription;
		ifaceSetPtr->appIface.fAppGetNameFuncPtr = GetDptfName;
		ifaceSetPtr->appIface.fAppGetVersionFuncPtr = GetDptfVersion;
		ifaceSetPtr->appIface.fAppGetStatusFuncPtr = GetDptfStatus;

		/* Participant */
		ifaceSetPtr->appIface.fParticipantCreateFuncPtr = ParticipantCreate;
		ifaceSetPtr->appIface.fParticipantDestroyFuncPtr = ParticipantDestroy;

		/* Domain */
		ifaceSetPtr->appIface.fDomainCreateFuncPtr = DomainCreate;
		ifaceSetPtr->appIface.fDomainDestroyFuncPtr = DomainDestroy;

		/* Event */
		ifaceSetPtr->appIface.fAppEventFuncPtr = DptfEvent;
	exit:
		return rc;
	}
} // extern "C"
