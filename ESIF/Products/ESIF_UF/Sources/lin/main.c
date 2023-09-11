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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_LINUX
#include "esif_ccb.h"
#include "esif_uf.h"
#include "esif_uf_appmgr.h"
#include "esif_pm.h"
#include "esif_version.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_ccb_system.h"
#include "esif_uf_sensor_manager_os_lin.h"
#include "esif_uf_ccb_imp_spec.h"
#include "esif_uf_sysfs_os_lin.h"
#include "esif_uf_sensors.h"
#include "esif_sdk_data_misc.h"
#include "esif_ccb_cpuid.h"
#include <sys/socket.h>
#include <linux/netlink.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <poll.h>

#ifdef ESIF_FEAT_OPT_HAVE_EDITLINE
#include <editline/readline.h>
#endif

// Feature Flag to enable dynamic capability feature
#ifdef ESIF_FEAT_OPT_DYNAMIC_CAPABILITY 
#include <sys/capability.h>

eEsifError disable_all_capabilities()
{
	eEsifError rc = ESIF_OK;
	cap_t caps = NULL;

	// Get the current Capability set
	caps = cap_get_proc();
	if (caps == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	// Clear the Effective Capability set
	if (cap_clear_flag(caps, CAP_EFFECTIVE) == -1) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	// Set the new cleared effective capability set
	if (cap_set_proc(caps) == -1) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

exit:
	if ( caps != NULL) {
		cap_free(caps);
	}
	return rc;
}
#else // IF Feature flag is not set
eEsifError disable_all_capabilities()
{
	return ESIF_OK;
}
#endif // end of ESIF_FEAT_OPT_DYNAMIC_CAPABILITY
#define COPYRIGHT_NOTICE "Copyright (c) 2013-2023 Intel Corporation All Rights Reserved"

/* ESIF_UF Startup Script Defaults */
#define ESIF_STARTUP_SCRIPT_DAEMON_MODE	"arbitrator disable && appstart ipfsrv"
#define ESIF_STARTUP_SCRIPT_SERVER_MODE	"appstart ipfsrv"
#define TOTAL_PSY_PROPERTIES 10

static void esif_udev_start();
static void esif_udev_stop();
static void esif_udev_exit();
static Bool esif_udev_is_started();
static void *esif_udev_listen(void *ptr);
static void esif_process_udev_event(char *udev_target);
static int kobj_thermal_uevent_parse(char *buffer, int len, char **zone_name, int *temp, int *event);
static int kobj_wifi_uevent_parse(char *buffer, int len , char **wifi_action,int *wifi_event);
static eEsifError ProcessWifiEvents(UfPmIterator *upIterPtr, EsifUpPtr upPtr, int wifi_event);
static int kobj_rfkill_uevent_parse(char *buffer, int len, char **rfkill_type, int *rfkill_state);
static eEsifError ProcessRfkillEvents(UfPmIterator *upIterPtr, EsifUpPtr upPtr, int rfkill_state);

static esif_thread_t g_udev_thread;
static Bool g_udev_quit = ESIF_TRUE;
static char *g_udev_target = NULL;

static esif_thread_t gSysfsReadThread;
static Bool gSysfsReadQuit = ESIF_TRUE;

static Bool IsIntelCPU();
static Bool EsifSysfsReadIsStarted();
static void EsifSysfsReadStart();
static void EsifSysfsReadStop();
static void *EsifSysfsReadListen(void *ptr);
static void EsifSysfsReadExit();
static Bool EsifSysfsPollEvent();
static Bool EsifSysfsPeriodicPolling();
/* Dedicated thread pool to handle SIGRTMIN timer signals */
static esif_thread_t *g_sigrtmin_thread_pool;
static Bool g_os_quit = ESIF_FALSE; /* global flag to supress KW flagging as while(1) alternative */
static esif_ccb_sem_t g_sigquit; /* global semaphore to signal main thread to quit, called from shell exit or sigterm handler */

/* Number of threads in thread pool: default to 1, but will
 * change to the actual number of processore cores if there
 * is support to the _SC_NPROCESSORS_ONLN system configuration
 * at run time. This value can also be overriden by command
 * line argument for experimental purposes.
 */
static long g_nproc = 1;

#define MAX_TIMER_THREADS_AUTO 16 /* max number of timer work threads allowed from auto-detection */
#define MAX_TIMER_THREADS_CLI 256 /* max number of timer work threads allowed from command line */
#define MAX_PAYLOAD 1024 /* max message size*/
#define MAX_AUTO_REPOS 4 /* max Repos that can be loaded with -m option */
#define EVENT_INTERVAL_THRESHOLD 100 /* in ms, the threshold we process successive THERMAL_TABLE_CHANGED events */

static struct sockaddr_nl sock_addr_src, sock_addr_dest;
static struct nlmsghdr *netlink_msg = NULL;
static struct iovec msg_buf;
static int sock_fd;
static struct msghdr msg;

#ifdef ESIF_FEAT_OPT_DBUS
#include <dbus/dbus.h>
esif_thread_t g_dbus_thread;
DBusConnection *g_dbus_conn;
#endif

/* Instance Lock */
struct instancelock {
	char *lockfile; /* lock filename */
	int  lockfd;    /* lock file descriptor */
};
static struct instancelock g_instance = {"ipf_ufd.pid"};
static char thermal_device_path[] = "/devices/virtual/thermal/thermal_zone";
static char wifi_device_path[] = "/module/iwlwifi";

#define HOME_DIRECTORY	NULL /* use OS-specific default */

#ifdef ESIF_ATTR_64BIT
# define ARCHNAME "x64"
# define ARCHBITS "64"
#else
#define ARCHNAME "x86"
#  define ARCHBITS "32"
#endif

#define CPUID_LEAF_0_EBX	(0x756E6547)	//'Genu'
#define CPUID_LEAF_0_EDX	(0x49656E69)	//'ineI'
#define CPUID_LEAF_0_ECX	(0x6C65746E)	//'ntel'


// Default ESIF Paths for each OS
// Paths preceded by "$" are treated as system paths and are not auto-created or checked for symbolic links
static const esif_string ESIF_PATHLIST =
#if   defined(ESIF_ATTR_OS_CHROME)
	// Chromium
	"HOME=/var/log/dptf\n"
	"TEMP=$/tmp\n"
	"DV=/etc/dptf\n"
	"LOG=/var/log/dptf\n"
	"BIN=/usr/share/dptf/bin\n"
	"LOCK=$/var/run\n"
	"EXE=$/usr/bin\n"
	"DLL=$/usr/lib" ARCHBITS "\n"
	"DLLALT=$/usr/lib" ARCHBITS "\n"
	"DSP=/etc/dptf/dsp\n"
	"CMD=/etc/dptf/cmd\n"
	"DATA=/usr/share/dptf/ui\n"
#else
	// Generic Linux
	"HOME=/var/log/dptf\n"
	"TEMP=$/tmp\n"
	"DV=/etc/dptf\n"
	"LOG=/var/log/dptf\n"
	"BIN=/usr/share/dptf/bin\n"
	"LOCK=$/var/run\n"
	"EXE=/usr/share/dptf/uf" ARCHNAME "\n"
	"DLL=/usr/share/dptf/uf" ARCHNAME "\n"
	"DLLALT=/usr/share/dptf/uf" ARCHNAME "\n"
	"DSP=/usr/share/dptf/dsp\n"
	"CMD=/usr/share/dptf/cmd\n"
	"DATA=/usr/share/dptf/ui\n"
#endif
;

/* Enable Instance Lock? */
#define ESIF_ATTR_INSTANCE_LOCK

/*
** Not ALL OS Entries Suport All Of These
** Not Declared In Header File Intentionallly
*/
extern int g_dst;
extern int g_binary_buf_size;
extern int g_errorlevel;
extern int g_quit;
extern int g_repeat;
extern int g_repeat_delay;
extern int g_soe;
extern int g_shell_enabled;
extern int g_cmdshell_enabled;
extern char **g_autorepos;
/* Worker Thread in esif_uf */
Bool g_start_event_thread = ESIF_TRUE;
esif_thread_t g_thread;
void *esif_event_worker_thread (void *ptr);

/* IPC Resync */
extern enum esif_rc sync_lf_participants();
extern esif_os_handle_t g_ipc_handle;

u32 g_ipc_retry_msec	 = 100;		/* 100ms by default */
u32 g_ipc_retry_max_msec = 2000;	/* 2 sec by default */

/* Thermal notification reason */
enum thermal_notify_event {
	THERMAL_EVENT_UNDEFINED, /* Undefined event */
	THERMAL_EVENT_TEMP_SAMPLE, /* New Temperature sample */
	THERMAL_EVENT_TRIP_VIOLATED, /* TRIP Point violation */
	THERMAL_EVENT_TRIP_CHANGED, /* TRIP Point temperature changed */
	THERMAL_DEVICE_DOWN, /* Thermal device is down */
	THERMAL_DEVICE_UP, /* Thermal device is up after a down event */
	THERMAL_DEVICE_POWER_CAPABILITY_CHANGED, /* power capability changed */
	THERMAL_TABLE_CHANGED, /* Thermal table(s) changed */
	THERMAL_EVENT_KEEP_ALIVE, /* Request for user space handler to respond */
};

/* RF event notification */
enum rfkill_notify_event {
	RF_EVENT_UNDEFINED = -1,
	RF_EVENT_DISABLE = 0,
	RF_EVENT_ENABLE = 1,
} rfkill_event;

/* Wifi event notification */
enum wifi_notify_event {
	WIFI_EVENT_UNDEFINED,
	WIFI_EVENT_MODULE_ADDED,
	WIFI_EVENT_MODULE_REMOVED,
}wifi_event;

/* Signal Application to Quit */
static void signal_quit()
{
	static atomic_t signaled = ATOMIC_INIT(0);
	if (atomic_set(&signaled, 1) == 0) {
		g_quit = 1;
		esif_ccb_sem_up(&g_sigquit);
	}
}

enum power_supply_event {
	POWER_SUPPLY_NAME = 0,
	POWER_SUPPLY_CAPACITY,
	POWER_SUPPLY_STATUS,
};

/* Attempt IPC Connect and Sync for specified time if no IPC connection */
eEsifError ipc_resync()
{
#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	return ESIF_OK;
#else
	eEsifError rc = ESIF_OK;
	u32 elapsed_msec = 0;

	if (g_ipc_handle == INVALID_HANDLE_VALUE) {
		rc = ESIF_E_NO_LOWER_FRAMEWORK;

		while (!g_quit && elapsed_msec < g_ipc_retry_max_msec) {
			ipc_connect();
			if (g_ipc_handle != INVALID_HANDLE_VALUE) {
				sync_lf_participants();
				rc = ESIF_OK;
				break;
			}
			esif_ccb_sleep_msec(g_ipc_retry_msec);
			elapsed_msec += g_ipc_retry_msec;
		}
	}
	return rc;
#endif
}

static Int64 time_elapsed_in_ms(struct timeval *old, struct timeval *now)
{
	Int64 elapsed = 0;
	if (old && now) {
		elapsed = (now->tv_sec - old->tv_sec) * 1000;
		elapsed += (now->tv_usec - old->tv_usec) / 1000;
	}

	return elapsed;
}

static int kobj_rfkill_uevent_parse(char *buffer, int len, char **rfkill_type, int *rfkill_state)
{
	static const char type_eq[] = "RFKILL_TYPE=";
	static const char state_eq[] = "RFKILL_STATE=";
	int i = 0;
	int ret = 0;

	while (i < len) {
		if (esif_ccb_strlen(buffer + i, len - i) > esif_ccb_strlen(type_eq, sizeof(type_eq))
				&& esif_ccb_strncmp(buffer + i, type_eq, esif_ccb_strlen(type_eq, sizeof(type_eq))) == 0) {
				*rfkill_type = buffer + i + esif_ccb_strlen(type_eq, sizeof(type_eq));
		}
		else if (esif_ccb_strlen(buffer + i, len - i) > esif_ccb_strlen(state_eq, sizeof(state_eq))
				&& esif_ccb_strncmp(buffer + i, state_eq, esif_ccb_strlen(state_eq, sizeof(state_eq))) == 0) {
				*rfkill_state = esif_atoi(buffer + i + esif_ccb_strlen(state_eq, sizeof(state_eq)));
		}
		i += esif_ccb_strlen(buffer + i, len - i) + 1;
	}

	if (*rfkill_type != NULL) {
		ESIF_TRACE_INFO("rfkill_type:%s, rfkill_state: %d\n", *rfkill_type, *rfkill_state);
		ret = esif_ccb_strncmp(*rfkill_type, "wlan", sizeof("wlan")-1);
		if (*rfkill_state != -1 && ret == 0) {
			return 1;
		}
	}

	return 0;
}

static int kobj_thermal_uevent_parse(char *buffer, int len, char **zone_name, int *temp, int *event)
{
	static const char name_eq[] = "NAME=";
	static const char temp_eq[] = "TEMP=";
	static const char event_eq[] = "EVENT=";
	int i = 0;

	*temp = -1;
	*event = -1;

	while (i < len) {
		if (esif_ccb_strlen(buffer + i, len - i) > esif_ccb_strlen(name_eq, sizeof(name_eq))
				&& esif_ccb_strncmp(buffer + i, name_eq, esif_ccb_strlen(name_eq, sizeof(name_eq))) == 0) {
				*zone_name = buffer + i + esif_ccb_strlen(name_eq, sizeof(name_eq));
		}
		else if (esif_ccb_strlen(buffer + i, len - i) > esif_ccb_strlen(temp_eq, sizeof(temp_eq))
				&& esif_ccb_strncmp(buffer + i, temp_eq, esif_ccb_strlen(temp_eq, sizeof(temp_eq))) == 0) {
				*temp= esif_atoi(buffer + i + esif_ccb_strlen(temp_eq, sizeof(temp_eq)));
		}
		else if (esif_ccb_strlen(buffer + i, len - i) > esif_ccb_strlen(event_eq, sizeof(event_eq))
				&& esif_ccb_strncmp(buffer + i, event_eq, esif_ccb_strlen(event_eq, sizeof(event_eq))) == 0) {
				*event= esif_atoi(buffer + i + esif_ccb_strlen(event_eq, sizeof(event_eq)));
		}
		i += esif_ccb_strlen(buffer + i, len - i) + 1;
	}

	if (*temp != -1 && *event != -1)
		return 1;
	else
		return 0;
}

static int kobj_wifi_uevent_parse(char *buffer, int len, char **action, int *wifi_event)
{
	static const char action_eq[] = "ACTION=";

	int i = 0;
	int ret = 1;

	while (i < len) {
		if (esif_ccb_strlen(buffer + i, len - i) > esif_ccb_strlen(action_eq, sizeof(action_eq))
				&& esif_ccb_strncmp(buffer + i, action_eq, esif_ccb_strlen(action_eq, sizeof(action_eq))) == 0) {
				*action = buffer + i + esif_ccb_strlen(action_eq, sizeof(action_eq));
		}
		i += esif_ccb_strlen(buffer + i, len - i) + 1;
	}

	if (*action == NULL) {
		ret = 0;
		goto exit;
	}

	if (!esif_ccb_strncmp(*action, "add", sizeof("add")-1)) {
		*wifi_event = WIFI_EVENT_MODULE_ADDED;
	}
	else if (!esif_ccb_strncmp(*action, "remove", sizeof("remove")-1)) {
		*wifi_event = WIFI_EVENT_MODULE_REMOVED;
	}
	else {
		*wifi_event = WIFI_EVENT_UNDEFINED;
		ret = 0;
	}

	ESIF_TRACE_INFO("action:%s, wifi_event: %d\n", *action, *wifi_event);

exit:
	return ret;
}

static eEsifError ProcessRfkillEvents(UfPmIterator *upIterPtr, EsifUpPtr upPtr, int rfkill_state)
{
	eEsifError rc = ESIF_OK;

	static int prev_rfkill_state = RF_EVENT_UNDEFINED;

	if (prev_rfkill_state == rfkill_state) {
		goto exit;
	}
	prev_rfkill_state = rfkill_state;

	switch (rfkill_state) {
		case RF_EVENT_ENABLE:
			ESIF_TRACE_INFO("WIFI is enabled\n");
			rc = EsifUpPm_ResumeParticipantByType("INT3408", ESIF_DOMAIN_TYPE_WIRELESS);
			break;

		case RF_EVENT_DISABLE:
			ESIF_TRACE_INFO("WIFI is Disabled\n");
			rc = EsifUpPm_InitIterator(upIterPtr);
			if (rc == ESIF_E_PARAMETER_IS_NULL) {
				goto exit;
			}
			rc = EsifUpPm_GetNextUp(upIterPtr, &upPtr);

			while (ESIF_OK == rc) {
				if ( !esif_ccb_strcmp(upPtr->fMetadata.fAcpiDevice, "INT3408")
					&& upPtr->fMetadata.fAcpiType == ESIF_DOMAIN_TYPE_WIRELESS ) {
						EsifEventMgr_SignalSynchronousEvent(EsifUp_GetInstance(upPtr), EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_SUSPEND, NULL, EVENT_MGR_SYNCHRONOUS_EVENT_TIME_MAX);
						ESIF_TRACE_DEBUG(" The participant suspended fAcpidevice: %s, fAcpiType:%u \n", upPtr->fMetadata.fAcpiDevice, upPtr->fMetadata.fAcpiType);
				}
				rc = EsifUpPm_GetNextUp(upIterPtr, &upPtr);
			}
			if (rc != ESIF_E_ITERATION_DONE) {
				EsifUp_PutRef(upPtr);
			}
			break;
		default:
			ESIF_TRACE_INFO("RF_EVENT_UNDEFINED\n");
			break;
	}
exit:
	return rc;
}


static eEsifError ProcessWifiEvents(UfPmIterator *upIterPtr, EsifUpPtr upPtr, int wifi_event)
{
	eEsifError rc = ESIF_OK;
	static int prev_wifi_event = WIFI_EVENT_UNDEFINED;

	if (prev_wifi_event == wifi_event) {
		goto exit;
	}
	prev_wifi_event = wifi_event;

	switch (wifi_event) {
		case WIFI_EVENT_MODULE_ADDED:
			ESIF_TRACE_INFO("WIFI driver is loaded\n");
			rc = EsifUpPm_ResumeParticipantByType("INT3408", ESIF_DOMAIN_TYPE_WIRELESS);
			break;
		case WIFI_EVENT_MODULE_REMOVED:
			ESIF_TRACE_INFO("WIFI driver is unloaded\n");
			rc = EsifUpPm_InitIterator(upIterPtr);
			if (rc == ESIF_E_PARAMETER_IS_NULL) {
				goto exit;
			}
			rc = EsifUpPm_GetNextUp(upIterPtr, &upPtr);

			while (ESIF_OK == rc) {
				if ( !esif_ccb_strcmp(upPtr->fMetadata.fAcpiDevice, "INT3408")
					&& upPtr->fMetadata.fAcpiType == ESIF_DOMAIN_TYPE_WIRELESS ) {
						EsifEventMgr_SignalSynchronousEvent(EsifUp_GetInstance(upPtr), EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_SUSPEND, NULL, EVENT_MGR_SYNCHRONOUS_EVENT_TIME_MAX);
						ESIF_TRACE_DEBUG(" The participant suspended fAcpidevice: %s, fAcpiType:%u \n", upPtr->fMetadata.fAcpiDevice, upPtr->fMetadata.fAcpiType);
				}
				rc = EsifUpPm_GetNextUp(upIterPtr, &upPtr);
			}
			if (rc != ESIF_E_ITERATION_DONE) {
				EsifUp_PutRef(upPtr);
			}
			break;
		default:
			ESIF_TRACE_INFO("WIFI_EVENT_UNDEFINED\n");
			break;
	}
exit:
	return rc;
}



static int check_for_uevent(int fd) {
	eEsifError rc = ESIF_OK;
	int i = 0;
	int len;
	const char dev_path[] = "DEVPATH=";
	unsigned int dev_path_len = sizeof(dev_path) - 1;
	char buffer[MAX_PAYLOAD + 1] = {0};
	char thermal_path[MAX_PAYLOAD + 1] = {0};
	char *buf_ptr;
	char *zone_name;
	char *wifi_action;
	char *rfkill_type;
	int temp;
	int event;
	int wifi_event;
	int rfkill_state = RF_EVENT_UNDEFINED;
	static struct timeval last_event_time = {0};
	struct timeval cur_event_time = {0};
	Int64 interval = 0;
	UfPmIterator upIter = { 0 };
	EsifUpPtr upPtr = NULL;
	EsifData evtData = { 0 };

	len = recv(fd, buffer, sizeof(buffer), 0);
	if (len <= 0) {
		return 0;
	}

	while (i < len) {

		buf_ptr = buffer + i;

		if (esif_ccb_strlen(buf_ptr, sizeof(dev_path)) > dev_path_len
				&& esif_ccb_strncmp(buf_ptr, dev_path, dev_path_len) == 0) {
			if (esif_ccb_strstr(buf_ptr, "ieee80211") != NULL) {

				if (kobj_rfkill_uevent_parse(buffer, len, &rfkill_type, &rfkill_state)) {
					rc = ProcessRfkillEvents(&upIter, upPtr, rfkill_state);
					ESIF_TRACE_INFO(" ProcessRfKillEvents error code: %d .\n",rc);
				}
				else if (kobj_wifi_uevent_parse(buffer, len, &wifi_action, &wifi_event))
				{
					rc = ProcessWifiEvents(&upIter, upPtr, wifi_event);
					ESIF_TRACE_INFO("ProcessWiFiEvents error code: %d .\n",rc);
				}

			} else if (esif_ccb_strncmp(buf_ptr + dev_path_len, thermal_device_path, sizeof(thermal_device_path) - 1) == 0) {
				char *parsed = NULL;
				char *ctx = NULL;

				esif_ccb_strcpy(thermal_path, buf_ptr + dev_path_len, sizeof(thermal_path));
				parsed = esif_ccb_strtok(thermal_path, "/", &ctx);
				while (parsed != NULL) {
					esif_ccb_strcpy(g_udev_target,parsed,MAX_PAYLOAD);
					parsed = esif_ccb_strtok(NULL,"/",&ctx);
				}

				if (!kobj_thermal_uevent_parse(buffer, len, &zone_name, &temp, &event))
					break;

				switch (event) {
				case THERMAL_EVENT_UNDEFINED:
					ESIF_TRACE_INFO("THERMAL_EVENT_UNDEFINED\n");
					break;
				case THERMAL_EVENT_TEMP_SAMPLE:
					ESIF_TRACE_INFO("THERMAL_EVENT_TEMP_SAMPLE\n");
					break;
				case THERMAL_EVENT_TRIP_VIOLATED:
					ESIF_TRACE_INFO("THERMAL_EVENT_TRIP_VIOLATED\n");
					esif_process_udev_event(g_udev_target);
					break;
				case THERMAL_EVENT_TRIP_CHANGED:
					ESIF_TRACE_INFO("THERMAL_EVENT_TRIP_CHANGED\n");

					rc = EsifUpPm_InitIterator(&upIter);
					rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
					while (ESIF_OK == rc) {

						EsifEventMgr_SignalEvent(EsifUp_GetInstance(upPtr), EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED, NULL);
						rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
					}
					if (rc != ESIF_E_ITERATION_DONE) {
						EsifUp_PutRef(upPtr);
					}
					break;
				case THERMAL_TABLE_CHANGED:
					ESIF_TRACE_INFO("THERMAL_TABLE_CHANGED\n");
					gettimeofday(&cur_event_time, NULL);
					/*
					 * Some old BIOS has a bug that any reset to fan speed will generate a new THERMAL_TABLE_CHANGED
					 * notification, and this will cause an infinite loop if the corresponding DTT_ACTIVE_RELATIONSHIP_TABLE_CHANGED
					 * event is blindly sent to DPTF, as DPTF will always reset fan speed upon receiving such an event.
					 * To break from the loop, we send thermal table changed event only if the time elaspsed between two
					 * successive events exceeds a certain threshold.
					 */
					interval = time_elapsed_in_ms(&last_event_time, &cur_event_time);
					if (interval > EVENT_INTERVAL_THRESHOLD) {
						EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_DTT_THERMAL_RELATIONSHIP_TABLE_CHANGED, NULL);
						EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_DTT_ACTIVE_RELATIONSHIP_TABLE_CHANGED, NULL);
					} else {
						ESIF_TRACE_DEBUG("Recevied spurious THERMAL_TABLE_CHANGED event\n");
					}
					last_event_time = cur_event_time;
					break;
				case THERMAL_EVENT_KEEP_ALIVE:
					ESIF_TRACE_INFO("THERMAL_EVENT_KEEP_ALIVE");
					EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_DTT_ALIVE_REQUEST, NULL);
					break;
				case THERMAL_DEVICE_POWER_CAPABILITY_CHANGED:
					ESIF_TRACE_INFO("THERMAL_DEVICE_POWER_CAPABILITY_CHANGED");
					EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_OEM_VARS_CHANGED, NULL);
					break;
				default:
					break;
				}
				return 1;
			}
		}
		i += esif_ccb_strlen(buffer + i, len - i) + 1;
	}

	return 0;
}

/* SIGTERM Signal Handler */
static void sigterm_handler(int signum)
{
	signal_quit(); /* Attempt Graceful ESIF Shutdown */
}

/* Enable or Disable SIGTERM Handler */
void sigterm_enable()
{
	struct sigaction action={0};
	action.sa_handler = sigterm_handler;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT,  &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
}

// The SIGUSR1 handler below is the alternative
// solution for Android where the pthread_cancel()
// is not implemented in Android NDK

/* SIGUSR1 Signal Handler */
static void sigusr1_handler(int signum)
{
	pthread_exit(0);
}

/* Enable or Disable SIGTERM Handler */
static void sigusr1_enable()
{
	struct sigaction action={0};
	action.sa_handler = sigusr1_handler;
	sigaction(SIGUSR1, &action, NULL);
}

static void sigrtmin_block(void)
{
	/* Block SIGRTMIN; other threads created from the main thread
	 * will inherit a copy of the signal mask.
	 */
	sigset_t set = {0};
	sigemptyset(&set);
	sigaddset(&set, SIGRTMIN);
	if (pthread_sigmask(SIG_BLOCK, &set, NULL)) {
		ESIF_TRACE_ERROR("Fail to block SIGRTMIN\n");
	}
}

static void *sigrtmin_worker_thread(void *ptr)
{
	sigset_t set = {0};
	siginfo_t info = {0};
	struct esif_tmrm_cb_obj *cb_object_ptr = NULL;

	UNREFERENCED_PARAMETER(ptr);
	sigemptyset(&set);
	sigaddset(&set, SIGRTMIN);

	while (!g_os_quit) { /* This thread will be canceled/killed after esif_uf_exit() */
		if (-1 == sigwaitinfo(&set, &info)) { /* sigwaitinfo is a cancellation point */
			/* When there are multiple worker threads waiting for the same signal,
			 * only one thread will get the signal and its associated info structure,
			 * and the rest will get an errno of EINTR. In this case, simply continue
			 * the loop and wait again.
			 */
			continue;
		}

		if (SIGRTMIN != info.si_signo) {
			ESIF_TRACE_ERROR("Received signal is not SIGRTMIN, ignore\n");
			continue;
		}

		cb_object_ptr = (struct esif_tmrm_cb_obj *) info.si_value.sival_ptr;
		if (cb_object_ptr && cb_object_ptr->func) {
			cb_object_ptr->func(cb_object_ptr->cb_handle);
			cb_object_ptr->func = NULL;
			/* Use native free function instead of esif_ccb_free(). For explanation,
			 * please see the comments under esif_ccb_timer_obj_enable_timer() in
			 * file esif_ccb_timer_lin_user.h.
			 */
			free(cb_object_ptr);
		} else {
			ESIF_TRACE_ERROR("Invalid timer callback object: (obj=%p, func=%p)\n",
				cb_object_ptr, (cb_object_ptr ? cb_object_ptr->func : NULL));
		}
	}

	return NULL;
}

static long get_nproc(long old, long new, long max)
{
	if (new > 0) {
		return (new > max) ? max : new;
	} else {
		return old;
	}
}

static enum esif_rc create_timer_thread_pool(void)
{
	int i = 0;
	enum esif_rc rc = ESIF_OK;

	/* Use native malloc instead of esif_ccb_malloc
	 * because the poll will be freed after esif_uf_exit().
	 */
	g_sigrtmin_thread_pool = (esif_thread_t *) malloc(g_nproc * sizeof(esif_thread_t));
	if (g_sigrtmin_thread_pool) {
		pthread_attr_t attr;

		for (i = 0; i < g_nproc; ++i) {
			pthread_attr_init(&attr);
			pthread_create(&g_sigrtmin_thread_pool[i], &attr, sigrtmin_worker_thread, NULL);
			pthread_attr_destroy(&attr);
		}
	} else {
		rc = ESIF_E_NO_MEMORY;
	}

	return rc;
}

static void release_timer_thread_pool(void)
{
	int i = 0;

	if (g_sigrtmin_thread_pool) {
		/* All ESIF timers should have been canceled
		 * after esif_uf_exit() call, so it is safe
		 * to terminate the g_sigrtmin_thread_pool now.
		 */
		for (i = 0; i < g_nproc; ++i) {
			pthread_cancel(g_sigrtmin_thread_pool[i]);
		}
		for (i = 0; i < g_nproc; ++i) {
			pthread_join(g_sigrtmin_thread_pool[i], NULL);
		}
		free(g_sigrtmin_thread_pool);
	}
}

int SysfsGetString(const char *path, const char *filename, char *str, size_t buf_len)
{
	FILE *fd = NULL;
	int ret = -1;
	char filepath[MAX_PATH] = { 0 };
	char scanf_fmt[IIO_STR_LEN] = { 0 };

	if (str == NULL) {
		return ret;
	}

	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		return ret;
	}

	// Use dynamic format width specifier to avoid scanf buffer overflow
	esif_ccb_sprintf(sizeof(scanf_fmt), scanf_fmt, "%%%ds", (int)buf_len - 1);
	ret = esif_ccb_fscanf(fd, scanf_fmt, str);
	esif_ccb_fclose(fd);

	return ret;
}

int SysfsGetStringMultiline(const char *path, const char *filename, char *str)
{
	FILE *fp = NULL;
	int rc = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };
	char lineStr[MAX_STR_LINE_LEN + 1] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fp = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		goto exit;
	}

	*str = 0; // Initialize first character to ensure that concat works properly
	while (esif_ccb_fgets(lineStr, MAX_STR_LINE_LEN, fp)) {
		esif_ccb_sprintf_concat(MAX_SYSFS_STRING, str, "%s\n", lineStr);
		rc++;
	}

	esif_ccb_fclose(fp);

exit:
	return rc;
}

int SysfsGetInt64(const char *path, const char *filename, Int64 *p64)
{
	FILE *fd = NULL;
	int rc = 0;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	if (path == NULL || filename == NULL) {
		goto exit;
	}

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		goto exit;
	}
	rc = esif_ccb_fscanf(fd, "%lld", p64);
	esif_ccb_fclose(fd);

exit:
	// Klocwork bounds check. Should depend on context
	if (rc > 0 && (*p64 < MIN_INT64 || *p64 > MAX_INT64)) {
		rc = 0;
	}
	return rc;
}

int SysfsGetInt64Direct(int fd, Int64 *p64)
{
	int rc = 0;
	char buf[MAX_SYSFS_STRING] = {0};

	lseek(fd, 0, SEEK_SET);
	if (read(fd, buf, MAX_SYSFS_STRING) > 0) {
		rc = esif_ccb_sscanf(buf, "%lld", p64);
		if (rc < 1) {
			ESIF_TRACE_WARN("Failed to get file scan. Error code: %d .\n",rc);
		}
	}
	else {
		ESIF_TRACE_WARN("Error on context file read: %s\n", strerror(errno));
	}

	return rc;
}

eEsifError SysfsGetInt(const char *path, const char *filename, int *pInt)
{
	FILE *fp = NULL;
	eEsifError rc = ESIF_OK;
	char filepath[MAX_PATH] = { 0 };
	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", path, filename);

	if ((fp = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}
	if (esif_ccb_fscanf(fp, "%d", pInt) <= 0) {
		rc = ESIF_E_INVALID_HANDLE;
	}

	esif_ccb_fclose(fp);

exit:
	return rc;
}

eEsifError SysfsGetIntDirect(int fd, int *pInt)
{
	eEsifError rc = ESIF_OK;
	char buf[IIO_STR_LEN] = {0};

	lseek(fd, 0, SEEK_SET);
	if (read(fd, buf, IIO_STR_LEN) > 0) {
		if (esif_ccb_sscanf(buf, "%d", pInt) <= 0) {
			rc = ESIF_E_INVALID_HANDLE;
			ESIF_TRACE_WARN("Failed to get file scan. Error code: %d .\n",rc);
		}
	}
	else {
		rc = ESIF_E_INVALID_HANDLE;
		ESIF_TRACE_WARN("Error on context file read: %s\n", strerror(errno));
	}

	return rc;
}

eEsifError SysfsGetFloat(const char *path, const char *filename, float *pFloat)
{
	FILE *fp = NULL;
	eEsifError rc = ESIF_OK;
	char filepath[MAX_PATH] = { 0 };
	esif_ccb_sprintf(MAX_PATH, filepath, "%s/%s", path, filename);

	if ((fp = esif_ccb_fopen(filepath, "r", NULL)) == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}
	if (esif_ccb_fscanf(fp, "%f", pFloat) <= 0) {
		rc = ESIF_E_INVALID_HANDLE;
	}

	esif_ccb_fclose(fp);

exit:
	return rc;
}

eEsifError SysfsGetBinaryData(const char *path, const char *fileName, UInt8 *buffer, size_t bufferLength)
{
	FILE *fp = NULL;
	eEsifError rc = 0;
	char filePath[MAX_SYSFS_PATH] = { 0 };

	ESIF_ASSERT(NULL != path);
	ESIF_ASSERT(NULL != fileName);
	ESIF_ASSERT(NULL != buffer);

	esif_ccb_sprintf(MAX_SYSFS_PATH, filePath, "%s/%s", path, fileName);

	if ((fp = esif_ccb_fopen(filePath, "rb", NULL)) == NULL) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (esif_ccb_fread(buffer, bufferLength, 1, bufferLength, fp) != bufferLength) {
		ESIF_TRACE_ERROR("Error reading the binary data from %s \n",filePath);
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

exit:
	if ( fp != NULL ) {
		esif_ccb_fclose(fp);
	}
	return rc;
}

eEsifError SysfsGetFileSize(const char *path, const char *fileName, size_t *fileSize)
{
	FILE *fp = NULL;
	eEsifError rc = ESIF_OK;
	Int32 errorNumber = 0;
	struct stat st = { 0 };
	char filePath[MAX_SYSFS_PATH] = { 0 };

	ESIF_ASSERT(NULL != path);
	ESIF_ASSERT(NULL != fileName);
	ESIF_ASSERT(NULL != fileSize);

	esif_ccb_sprintf(MAX_SYSFS_PATH, filePath, "%s/%s", path, fileName);
	//Initialize the file size to 0
	*fileSize = 0;
	if((errorNumber = esif_ccb_stat(filePath, &st)) != 0) {
		ESIF_TRACE_WARN("Retrieving file size for %s failed with Error Code : %d\n",filePath, errorNumber);
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	//Set the filesize before returning
	*fileSize = st.st_size;
exit:
	return rc;
}

int SysfsSetInt64(char *path, char *filename, Int64 val)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "w", NULL)) == NULL) {
		goto exit;
	}

	esif_ccb_fprintf(fd, "%lld", val);
	esif_ccb_fclose(fd);
	rc = 0;

exit:
	return rc;
}
int SysfsSetInt64Direct(int fd, const Int32 val)
{
	int rc = 0;
	char buf[32] = {0};

	lseek(fd, 0, SEEK_SET);
	esif_ccb_memset(buf,0, sizeof(buf));
	esif_ccb_sprintf(sizeof(buf),buf,"%d",val);
	if ((rc = write(fd, buf, sizeof(buf))) < 0) {
		ESIF_TRACE_WARN("Failed to write the value: error code rc: %ld\n", rc);
	}
	return rc;
}

int SysfsSetString(const char *path, const char *filename, char *val)
{
	FILE *fd = NULL;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if ((fd = esif_ccb_fopen(filepath, "w", NULL)) == NULL) {
		goto exit;
	}

	esif_ccb_fprintf(fd, "%s", val);
	esif_ccb_fclose(fd);
	rc =0;

exit:
	return rc;
}

int SysfsSetStringWithError(const char *path, const char *filename, char *buffer, unsigned int length)
{
	int fd = -1;
	int rc = -1;
	char filepath[MAX_SYSFS_PATH] = { 0 };

	esif_ccb_sprintf(MAX_SYSFS_PATH, filepath, "%s/%s", path, filename);

	if (!length || ((fd = open(filepath, O_WRONLY)) == -1)) {
		goto exit;
	}

	if (write(fd, buffer, length) != length) {
		goto exit;
	}
	rc = 0;

exit:
	if (fd != -1) {
		close(fd);
	}
	return rc;
}

#if !defined(ESIF_ATTR_INSTANCE_LOCK)
// Instance Checking Disabled in Android for now

int instance_lock()
{
	return ESIF_TRUE;
}

void instance_unlock()
{
}

int instance_islocked()
{
	return ESIF_TRUE;
}

#else

// Prevent multiple instances from running by creating and locking a .pid file
int instance_lock()
{
	int rc=0;
	char pid_buffer[12]={0};
	char fullpath[MAX_PATH]={0};

	if (g_instance.lockfd) {
		return ESIF_TRUE;
	}

	esif_build_path(fullpath, sizeof(fullpath), ESIF_PATHTYPE_LOCK, g_instance.lockfile, NULL);
	if ((g_instance.lockfd = open(fullpath, O_CREAT | O_RDWR, 0644)) < 0 || flock(g_instance.lockfd, LOCK_EX | LOCK_NB) < 0) {
		if (g_instance.lockfd >= 0) {
			close(g_instance.lockfd);
		}
		g_instance.lockfd = 0;
		return ESIF_FALSE;
	}
	esif_ccb_sprintf(sizeof(pid_buffer), pid_buffer, "%d\n", getpid());
	rc = write(g_instance.lockfd, pid_buffer, esif_ccb_strlen(pid_buffer, sizeof(pid_buffer)));
	return ESIF_TRUE;
}

void instance_unlock()
{
	char fullpath[MAX_PATH]={0};

	if (g_instance.lockfd) {
		close(g_instance.lockfd);
		g_instance.lockfd = 0;
		esif_build_path(fullpath, sizeof(fullpath), ESIF_PATHTYPE_LOCK, g_instance.lockfile, NULL);
		esif_ccb_unlink(fullpath);
	}
}

int instance_islocked()
{
	return (g_instance.lockfd == 0 ? 0 : 1);
}

#endif

static void esif_udev_start()
{
	if (!esif_udev_is_started()) {
		g_udev_quit = ESIF_FALSE;
		g_udev_target = esif_ccb_malloc(MAX_PAYLOAD);
		if (g_udev_target != NULL) {
			esif_ccb_thread_create(&g_udev_thread, esif_udev_listen, NULL);
		}
		else {
			CMD_DEBUG("Unable to start ESIF udev listener (no memory)");
		}
	}
}

static void esif_udev_stop()
{
	if (esif_udev_is_started()) {
		esif_udev_exit();
	}
}
static void EsifSysfsReadStart()
{
	if (!EsifSysfsReadIsStarted()) {
		gSysfsReadQuit = ESIF_FALSE;
		esif_ccb_thread_create(&gSysfsReadThread, EsifSysfsReadListen, NULL);
	}
}

static void EsifSysfsReadStop()
{
	if (esif_udev_is_started()) {
		EsifSysfsReadExit();
	}
}

void enumerate_available_uf_participants(EnumerableUFParticipants typeOfUFParticipantsToEnumerate)
{
	// currently not used for linux
}

void register_events_for_available_uf_participants(EnumerableUFParticipants typeOfUFParticipantsToEnumerate)
{
	// currently not used for linux
}

esif_error_t CreateEnumeratedParticipants()
{
	// currently not used for linux
	return ESIF_OK;
}

esif_error_t CreateActionAssociatedParticipants(esif_action_type_t actionType)
{
	// currently not used for linux
	UNREFERENCED_PARAMETER(actionType);
	return ESIF_OK;
}

static void esif_udev_exit()
{
	g_udev_quit = ESIF_TRUE;
	pthread_cancel(g_udev_thread);
	esif_ccb_thread_join(&g_udev_thread);
	esif_ccb_free(g_udev_target);
}

static void EsifSysfsReadExit()
{
	gSysfsReadQuit = ESIF_TRUE;

	pthread_cancel(gSysfsReadThread);
	esif_ccb_thread_join(&gSysfsReadThread);
}

static Bool esif_udev_is_started()
{
	return !g_udev_quit;
}

static Bool EsifSysfsReadIsStarted()
{
	return !gSysfsReadQuit;
}

#ifdef ESIF_FEAT_SYSFS_POLL
static Bool EsifSysfsPeriodicPolling()
{
	//Open the Fds
	for (UInt32 i = 0; i < gNumberOfSysfsReadEntries; i++) {
		/* The IsValidFilePath() filters out all the invalid sysfs path */
		Int32 fd = open(gSysfsReadTable[i].sysfsPath, O_RDONLY);
		if (fd < 0) {
			CMD_DEBUG("Failed to open sysfs attribute : %s\n", gSysfsReadTable[i].sysfsPath);
			return ESIF_FALSE;
		}
		gSysfsReadTable[i].fd = fd;
	}

	/* Poll the FDs for change in values */
	while(!gSysfsReadQuit) {
		for ( UInt32 i = 0 ; i < gNumberOfSysfsReadEntries ; i++) {
			if (gSysfsReadTable[i].fd > 0 ) {
				Int64 value = 0;
				if (SysfsGetInt64Direct(gSysfsReadTable[i].fd, &value) <= 0) {
					ESIF_TRACE_ERROR("Failed to get sysfs attribute value : %s\n", gSysfsReadTable[i].sysfsPath);
					return ESIF_FALSE;
				}
				if (value != gSysfsReadTable[i].value) {
					ESIF_TRACE_DEBUG("\n Attribute changed(%s) : %s",gSysfsReadTable[i].participantName, gSysfsReadTable[i].sysfsPath);
					ESIF_TRACE_DEBUG("\n Old Value : %d New Value : %d",gSysfsReadTable[i].value, value);
					EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByName(gSysfsReadTable[i].participantName);
					if ( upPtr != NULL ) {
						EsifEventMgr_SignalEvent(
							EsifUp_GetInstance(upPtr), 
							EVENT_MGR_DOMAIN_NA, 
							gSysfsReadTable[i].eventType, 
							NULL
							);
						EsifUp_PutRef(upPtr);
					}
					gSysfsReadTable[i].value = value; 
				}
			}
		}
		// Poll for every 1s interval
		esif_ccb_sleep_msec(1000);
	}
	// Close the fds
	for ( UInt32 i = 0 ; i < gNumberOfSysfsReadEntries ; i++) {
		if (gSysfsReadTable[i].fd > 0 ) {
			close(gSysfsReadTable[i].fd);
			esif_ccb_memset(&gSysfsReadTable[i], 0 , sizeof(gSysfsReadTable[i]));
		}
	}
	return ESIF_TRUE;
}
#endif
static Bool EsifSysfsPollEvent()
{

	Bool oneTime = ESIF_TRUE;
	struct pollfd *sysfs_fds = (struct pollfd *)esif_ccb_malloc((gNumberOfSysfsReadEntries * sizeof(struct pollfd)));
	if (sysfs_fds == NULL) {
		CMD_DEBUG(" Failed to allocate memory for sysfs_fds");
		goto exit;
	}

	while(!gSysfsReadQuit) {
		for (UInt32 i = 0; i < gNumberOfSysfsReadEntries; i++) {
			/* The IsValidFilePath() filters out all the invalid sysfs path */
			gSysfsReadTable[i].fd = open(gSysfsReadTable[i].sysfsPath, O_RDONLY);
			if (gSysfsReadTable[i].fd < 0) {
				CMD_DEBUG("Failed to open sysfs attribute : %s\n", gSysfsReadTable[i].sysfsPath);
				goto exit;
			}
			sysfs_fds[i].fd = gSysfsReadTable[i].fd;
			sysfs_fds[i].events = POLLPRI|POLLERR;
			if (SysfsGetInt64Direct(gSysfsReadTable[i].fd, &gSysfsReadTable[i].value) <= 0) {
				CMD_DEBUG("Failed to get sysfs attribute value for %s\n", gSysfsReadTable[i].sysfsPath);
				goto exit;
			}
			if (oneTime) {
				EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByName(gSysfsReadTable[i].participantName);
				if ( upPtr != NULL ) {
					EsifEventMgr_SignalEvent(
					EsifUp_GetInstance(upPtr),
					EVENT_MGR_DOMAIN_NA,
					gSysfsReadTable[i].eventType,
					NULL
					);
					EsifUp_PutRef(upPtr);
				}
			}
		}

		oneTime = ESIF_FALSE;
		// poll wait for an event to happen on all the sysfs fds.
		if (poll(sysfs_fds, gNumberOfSysfsReadEntries, -1) < 0 ) {
			ESIF_TRACE_ERROR("Failed attempt on sysfs poll wait\n");
			goto exit;
		}

		for (UInt32 i = 0; i < gNumberOfSysfsReadEntries; i++) {
			if (sysfs_fds[i].revents != 0 && gSysfsReadTable[i].fd > 0) {
				Int64 value = 0;
				if (SysfsGetInt64Direct(gSysfsReadTable[i].fd, &value) <= 0) {
					ESIF_TRACE_ERROR("Failed to get sysfs attribute value : %s\n", gSysfsReadTable[i].sysfsPath);
					goto exit;
				}
				ESIF_TRACE_DEBUG("\n Attribute changed(%s) : %s",gSysfsReadTable[i].participantName, gSysfsReadTable[i].sysfsPath);
				ESIF_TRACE_DEBUG("\n Old Value : %d New Value : %d",gSysfsReadTable[i].value, value);
				gSysfsReadTable[i].value = value;
				EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByName(gSysfsReadTable[i].participantName);
				if ( upPtr != NULL ) {
					EsifEventMgr_SignalEvent(
					EsifUp_GetInstance(upPtr),
					EVENT_MGR_DOMAIN_NA,
					gSysfsReadTable[i].eventType,
					NULL
					);
					EsifUp_PutRef(upPtr);
				}
			}
		}

		// Close the fds
		for (UInt32 i = 0 ; i < gNumberOfSysfsReadEntries ; i++) {
			if (gSysfsReadTable[i].fd > 0) {
				close(gSysfsReadTable[i].fd);
			}
		}
	}
	esif_ccb_free(sysfs_fds);
	return ESIF_TRUE;
exit:
	esif_ccb_free(sysfs_fds);
	return ESIF_FALSE;
}

static void *EsifSysfsReadListen(void *ptr)
{
	UNREFERENCED_PARAMETER(ptr);

#ifdef ESIF_FEAT_SYSFS_POLL
	if(!EsifSysfsPeriodicPolling())
		goto exit;
#else
	if(!EsifSysfsPollEvent())
		goto exit;
#endif
	ESIF_TRACE_INFO("Exiting thread EsifSysfsReadListen...\n");
	return NULL;
exit:
	ESIF_TRACE_INFO("Exiting thread EsifSysfsReadListen with Error...\n");
	return NULL;
}

static void *esif_udev_listen(void *ptr)
{
	UNREFERENCED_PARAMETER(ptr);
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
	if (sock_fd < 0) {
		goto exit;
	}

	esif_ccb_memset(&sock_addr_src, 0, sizeof(sock_addr_src));
	sock_addr_src.nl_family = AF_NETLINK;
	sock_addr_src.nl_pid = getpid();
	sock_addr_src.nl_groups = -1;

	if ( bind(sock_fd, (struct sockaddr *)&sock_addr_src, sizeof(sock_addr_src)) != ESIF_OK) {
		goto exit;
	}

	esif_ccb_memset(&sock_addr_dest, 0, sizeof(sock_addr_dest));
	sock_addr_dest.nl_family = AF_NETLINK;
	sock_addr_dest.nl_pid = 0;
	sock_addr_dest.nl_groups = 0;

	netlink_msg = (struct nlmsghdr *)esif_ccb_malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (netlink_msg == NULL) {
		goto exit;
	}
	esif_ccb_memset(netlink_msg, 0, NLMSG_SPACE(MAX_PAYLOAD));
	netlink_msg->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	netlink_msg->nlmsg_pid = getpid();
	netlink_msg->nlmsg_flags = 0;

	msg_buf.iov_base = (void *)netlink_msg;
	msg_buf.iov_len = netlink_msg->nlmsg_len;
	msg.msg_name = (void *)&sock_addr_dest;
	msg.msg_namelen = sizeof(sock_addr_dest);
	msg.msg_iov = &msg_buf;
	msg.msg_iovlen = 1;

	/* Read message from kernel */
	while(!g_udev_quit) {
		check_for_uevent(sock_fd);
	}

exit:
	if (sock_fd >= 0) {
		close(sock_fd);
	}
	esif_ccb_free(netlink_msg);
	netlink_msg = NULL;
	return NULL;
}

static void esif_domain_signal_and_stop_poll(EsifUpPtr up_ptr, UInt8 participant_id, char *udev_target)
{
	eEsifError iter_rc = ESIF_OK;
	EsifUpDomainPtr domainPtr = NULL;
	UpDomainIterator udIter = { 0 };
	UInt32 domain_index = 0;

	iter_rc = EsifUpDomain_InitIterator(&udIter, up_ptr);
	if (ESIF_OK != iter_rc) {
		return;
	}

	iter_rc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
	while (ESIF_OK == iter_rc) {
		if (NULL == domainPtr) {
			iter_rc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
			domain_index++;
			continue;
		}

		ESIF_TRACE_INFO("Udev Event: THRESHOLD CROSSED in thermal zone: %s\n",udev_target);
		EsifEventMgr_SignalEvent(participant_id, domain_index++, ESIF_EVENT_TEMP_THRESHOLD_CROSSED, NULL);

		// Find a valid domain, check if it is being polled
		if (domainPtr->tempPollType != ESIF_POLL_NONE) {
			EsifUpDomain_StopTempPoll(domainPtr);
		}
		iter_rc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
	}

	if (iter_rc != ESIF_E_ITERATION_DONE) {
		EsifUp_PutRef(up_ptr);
	}
}

static void esif_process_udev_event(char *udev_target)
{
	char *participant_device_path = NULL;
	EsifUpPtr up_ptr = NULL;
	EsifUpPtr up_proc_ptr = NULL;
	char *parsed = NULL;
	char *ctx = NULL;
	UInt8 participant_id = 0;
	UInt8 processor_id = 0;
	eEsifError iter_rc = ESIF_OK;
	UfPmIterator up_iter = { 0 };
	Bool target_tz_found = ESIF_FALSE;

	participant_device_path = esif_ccb_malloc(MAX_PAYLOAD);
	if (participant_device_path == NULL) {
		goto exit;
	}

	iter_rc = EsifUpPm_InitIterator(&up_iter);
	if (iter_rc != ESIF_OK) {
		goto exit;
	}

	iter_rc = EsifUpPm_GetNextUp(&up_iter, &up_ptr);
	while (ESIF_OK == iter_rc) {
		char *path_to_compare = esif_ccb_strdup(up_ptr->fMetadata.fDevicePath);
		participant_id = EsifUp_GetInstance(up_ptr);
		if (!esif_ccb_strcmp(up_ptr->fMetadata.fAcpiDevice, "INT3401")) {
			processor_id = participant_id;
			up_proc_ptr = up_ptr;
		}

		parsed = esif_ccb_strtok(path_to_compare,"/",&ctx);
		while (parsed != NULL) {
			esif_ccb_strcpy(participant_device_path,parsed,MAX_PAYLOAD);
			parsed = esif_ccb_strtok(NULL,"/",&ctx);
		}
		if (!esif_ccb_strcmp(udev_target, participant_device_path)) {
			target_tz_found = ESIF_TRUE;
			esif_domain_signal_and_stop_poll(up_ptr, participant_id, udev_target);
			esif_ccb_free(path_to_compare);
			break;
		}

		esif_ccb_free(path_to_compare);
		iter_rc = EsifUpPm_GetNextUp(&up_iter, &up_ptr);
	}

	if (ESIF_E_ITERATION_DONE != iter_rc) {
		EsifUp_PutRef(up_ptr);
	}

exit:
	esif_ccb_free(participant_device_path);
}

#ifdef ESIF_FEAT_OPT_DBUS
static DBusHandlerResult
s3_callback(DBusConnection *conn, DBusMessage *message, void *user_data)
{
	eEsifError rc = ESIF_OK;
	int message_type = dbus_message_get_type (message);
	UfPmIterator upIter = { 0 };
	EsifUpPtr upPtr = NULL;

	if (DBUS_MESSAGE_TYPE_SIGNAL == message_type) {
		if (!esif_ccb_strcmp(dbus_message_get_member(message), "SuspendImminent")) {
			ESIF_TRACE_INFO("D-Bus: received SuspendImminent signal\n");
			rc = EsifUpPm_InitIterator(&upIter);
			if (ESIF_OK == rc) {
				rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
				while (ESIF_OK == rc) {
					// This event should be synchronous
					EsifEventMgr_SignalSynchronousEvent(EsifUp_GetInstance(upPtr), EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_SUSPEND, NULL, EVENT_MGR_SYNCHRONOUS_EVENT_TIME_MAX);
					rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
				}
				if (rc != ESIF_E_ITERATION_DONE) {
					EsifUp_PutRef(upPtr);
				}
			}
			ESIF_TRACE_DEBUG("Suspend Completed\n");
		} 
		else if (!esif_ccb_strcmp(dbus_message_get_member(message), "SuspendDone")) {
			ESIF_TRACE_INFO("D-Bus: received SuspendDone signal\n");
			// Send resume event to Manager to resume all the apps as well
			EsifEventMgr_SignalSynchronousEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_RESUME, NULL, EVENT_MGR_SYNCHRONOUS_EVENT_TIME_MAX);
			EsifUpPm_ResumeParticipants();
			ESIF_TRACE_DEBUG("Resume Completed\n");
		}
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

void* dbus_listen()
{
	DBusError err = {0};


	dbus_error_init(&err);
	g_dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
		return NULL;
	}

	dbus_bus_add_match(g_dbus_conn, "type='signal', sender='org.chromium.PowerManager'", &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
		return NULL;
	}

	if (!dbus_connection_add_filter(g_dbus_conn, s3_callback, NULL, NULL)) {
		fprintf(stderr, "Couldn't add filter!\n");
		return NULL;
	}

	while (dbus_connection_read_write_dispatch(g_dbus_conn, -1));

	return NULL;
}
#endif

/*
**  Build Supports Deaemon?
*/
#if defined (ESIF_ATTR_DAEMON)

/*
** To Daemonize
**
** 1.) Call Fork.
** 2.) Call Exit From The Parent.
** 3.) Call setsid().  Assign new process group and session.
** 4.) Change working directory with chdir()
** 5.) Close all file descriptors
** 6.) Open file descriptors 0, 1, and 2 and redirect as necessary
*/

static char *cmd_infile  = "esifd.cmd"; /* -p parameter, created in temp path */
static char *cmd_outfile = "esifd.log"; /* -l parameter, created in temp path */

static int run_as_daemon(int start_with_pipe, int start_with_log, int start_in_background)
{
	char *ptr = NULL;
	char line[MAX_LINE + 1] = {0};
	char cmd_in[MAX_PATH] = {0};
	char cmd_out[MAX_PATH] = {0};
	int  stdinfd = -1;

	/* Check to see if another instance is running */
	if (!instance_lock()) {
		return ESIF_FALSE;
	}
	instance_unlock();

	/* Compute full paths for -p -l parameters */
	if (ESIF_TRUE == start_with_pipe) {
		esif_build_path(cmd_in,  sizeof(cmd_in),  ESIF_PATHTYPE_TEMP, cmd_infile, NULL);
	}
	if (ESIF_TRUE == start_with_log) {
		esif_build_path(cmd_out, sizeof(cmd_out), ESIF_PATHTYPE_TEMP, cmd_outfile, NULL);
	}

	/* 1. Call Fork (if background daemon) */
	pid_t process_id = (start_in_background ? fork() : getpid());

	if (-1 == process_id) {
		printf("ESIF_UFD: %s failed. Error #%d\n", (start_in_background ? "fork" : "getpid"), errno);
		return ESIF_FALSE;
	}
	else if (process_id != 0) {
		/* 2. Exit From Parent (if background daemon) */
		CMD_DEBUG("\nESIF Daemon/Loader Version %s\n", ESIF_VERSION);
		CMD_DEBUG(COPYRIGHT_NOTICE "\n");
		CMD_DEBUG("%s: %d\n", (start_in_background ? "Spawn Daemon ESIF Child Process" : "Foreground Daemon Process"), process_id);
		if (ESIF_TRUE == start_with_pipe) {
			CMD_DEBUG("Command Input:  %s\n", cmd_in);
		}

		if (ESIF_TRUE == start_with_log) {
			CMD_DEBUG("Command Output: %s\n", cmd_out);
		}
		if (start_in_background)
			exit(EXIT_SUCCESS);
	}

	/* 3. Call setsid (if background daemon) */
	if (start_in_background && -1 == setsid()) {
		printf("ESIF_UFD: setsid failed. Error #%d\n", errno);
		return ESIF_FALSE;
	}

	/* Guarantee a single instance is running */
	if (!instance_lock()) {
		return ESIF_FALSE;
	}
	sigterm_enable();

	/* 4. Block SIGRTMIN (will re-enable it in dedicated thread pool) */
	sigrtmin_block();
	/* Start the thread pool that handles SIGRTMIN */
	if (ESIF_OK != create_timer_thread_pool()) {
		return ESIF_FALSE;
	}

	/* 5. Change to known directory.  Performed in main */
	/* 6. Close all file descriptors incuding stdin, stdout, stderr */
	close(STDIN_FILENO); /* stdin */
	close(STDOUT_FILENO); /* stdout */
	close(STDERR_FILENO); /* stderr */

	/* 7. Open file descriptors 0, 1, and 2 and redirect */
	/* stdin */
	if (ESIF_TRUE == start_with_log) {
		esif_ccb_unlink(cmd_out);
		stdinfd = open (cmd_out, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	}
	else {
		stdinfd = open ("/dev/null", O_RDWR);
	}

	/* stdout, stderr */
	if (dup(0) != -1) {
		setvbuf(stdout, NULL, _IONBF, 0);
	}
	if (dup(0) != -1) {
		setvbuf(stderr, NULL, _IONBF, 0);
	}


	/*
	** Start The Daemon
	*/
	g_cmdshell_enabled = 0;
	esif_shell_set_start_script(ESIF_STARTUP_SCRIPT_DAEMON_MODE);
	esif_uf_init();
	ipc_resync();
	if (g_start_event_thread) {
		esif_ccb_thread_create(&g_thread, esif_event_worker_thread, "Daemon");
	}

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	/* uevent listener */
	esif_udev_start();
#endif
	EsifSysfsReadStart();

	/* Start D-Bus listener thread if necessary */
#ifdef ESIF_FEAT_OPT_DBUS
	esif_ccb_thread_create(&g_dbus_thread, dbus_listen, "D-Bus Listener");
#endif

	/* UF Startup Script may have enabled/disabled ESIF Shell */
	if (!g_shell_enabled) {
		CMD_OUT("Shell Unavailable. Closing Command Pipes.\n");

		/* Redirect stdout, stderr to NULL and close input pipe */
		if (ESIF_TRUE == start_with_log) {
			close(stdinfd != -1 ? stdinfd : STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			stdinfd = open("/dev/null", O_RDWR);
			if (dup(0) != -1) {
				setvbuf(stdout, NULL, _IONBF, 0);
			}
			if (dup(0) != -1) {
				setvbuf(stderr, NULL, _IONBF, 0);
			}
		}
		if (ESIF_TRUE == start_with_pipe) {
			esif_ccb_unlink(cmd_in);
		}
	}

	/* Process Input ? */
	if (ESIF_TRUE == start_with_pipe) {

		/* Open Input Pipe using FIFO PIPE? */
		if (!g_shell_enabled) {
			start_with_pipe = ESIF_FALSE;
		}
		else {
			esif_ccb_unlink(cmd_in);
			if (-1 == mkfifo(cmd_in, S_IROTH)) {
				start_with_pipe = ESIF_FALSE;
			}
		}

		/* Wait for input from pipe until exit */
		while (start_with_pipe && !g_quit) {
			FILE *fp = NULL;
			if ((fp = esif_ccb_fopen(cmd_in, "r", NULL)) == NULL) {
				break;
			}

			/* Grab line from FIFO PIPE */
			if (esif_ccb_fgets(line, MAX_LINE, fp) == NULL) {
				/* do nothing */
			}
			ptr = line;
			while (*ptr != '\0') {
				if (*ptr == '\r' || *ptr == '\n' || *ptr == '#') {
					*ptr = '\0';
				}
				ptr++;
			}

			/* execute command */
			esif_shell_execute(line);
			esif_ccb_fclose(fp);
		}
		if (start_with_pipe) {
			signal_quit();
		}
	}
	if (stdinfd != -1)
		close(stdinfd);
	return ESIF_TRUE;
}
#endif

static int run_as_server(FILE* input, char* command, int quit_after_command)
{
	int rc = ESIF_FALSE;
	char *ptr = NULL;
#ifdef ESIF_FEAT_OPT_HAVE_EDITLINE
	char *line = NULL;
#else
	char line[MAX_LINE + 1]  = {0};
#endif

	/* Prompt */
	#define PROMPT_LEN 64
	char *prompt = NULL;
	char full_prompt[MAX_LINE + 1] = {0};
	char prompt_buf[PROMPT_LEN] = {0};
	ESIF_DATA(data_prompt, ESIF_DATA_STRING, prompt_buf, PROMPT_LEN);

	if (input == NULL)
		input = stdin;

	/* Guarantee a single instance is running */
	if (!instance_lock()) {
		goto exit;
	}
	sigrtmin_block(); /* Block SIGRTMIN (will re-enable it in dedicated thread pool) */
	/* Start the thread pool that handles SIGRTMIN */
	if (ESIF_OK != create_timer_thread_pool()) {
		CMD_OUT("Cannot create timer thread pool, exiting\n");
		goto exit;
	}
	sigterm_enable();
	esif_shell_set_start_script(ESIF_STARTUP_SCRIPT_SERVER_MODE);
	esif_uf_init();
	ipc_resync();
	if (g_start_event_thread) {
		esif_ccb_thread_create(&g_thread, esif_event_worker_thread, "Server");
		esif_ccb_sleep_msec(10);
	}

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	/* uevent listener */
	esif_udev_start();
#endif
	EsifSysfsReadStart();

	/* Start D-Bus listener thread if necessary */
#ifdef ESIF_FEAT_OPT_DBUS
	esif_ccb_thread_create(&g_dbus_thread, dbus_listen, "D-Bus Listener");
#endif

	while (!g_quit) {
		// Exit if Shell disabled
		if (!g_shell_enabled) {
			CMD_OUT("Shell Unavailable.\n");
			g_quit = 1;
			continue;
		}

		// Startup Command?
		if (command && *command) {
				parse_cmd(command, ESIF_FALSE, ESIF_TRUE);
				command = NULL;
				if (ESIF_TRUE == quit_after_command) {
						g_quit = 1;
						continue;
				}
		}

		// Get User Input
		EsifAppMgr_GetPrompt(&data_prompt);
		prompt = (esif_string)data_prompt.buf_ptr;

#ifdef ESIF_FEAT_OPT_HAVE_EDITLINE
		CMD_LOGFILE("%s ", prompt);
		line = readline(prompt);
		if (line == NULL) {
				if (input == stdin || quit_after_command) {
					break;
				}
				input = stdin;
		}
		else {
			ptr = line;
			while (*ptr != '\0') {
				if (*ptr == '\r' || *ptr == '\n' || *ptr == '#') {
					*ptr = '\0';
				}
				ptr++;
			}

			CMD_LOGFILE("%s\n", line);
			add_history(line);
			esif_shell_execute(line);
			esif_ccb_free(line);
		}
#else
		// No History So Sorry
		CMD_OUT("%s ", prompt);
		if (esif_ccb_fgets(line, MAX_LINE, input) == NULL) {
				if (input == stdin || quit_after_command) {
					break;
				}
				input = stdin;
		}
		ptr = line;
		while (*ptr != '\0') {
				if (*ptr == '\r' || *ptr == '\n' || *ptr == '#') {
						*ptr = '\0';
				}
				ptr++;
		}

		CMD_LOGFILE("%s\n", line);
		esif_shell_execute(line);
#endif
	}
	rc = ESIF_TRUE;

exit:
	signal_quit();
	return rc;
}

int main (int argc, char **argv)
{
	int c = 0;
	FILE *fp = NULL;
	char command[MAX_LINE + 1] = {0};
	char *repolist[MAX_AUTO_REPOS + 1] = {0};
	int repolistlen = 0;
	int quit_after_command = ESIF_FALSE;
#if defined(ESIF_ATTR_DAEMON)
	int terminate_daemon = ESIF_FALSE;
	int start_as_server = ESIF_FALSE;
	int start_with_pipe = ESIF_FALSE;
	int start_with_log = ESIF_FALSE;
	int start_in_background = ESIF_TRUE;
#else
	int start_as_server = ESIF_TRUE;
#endif

	if(IsIntelCPU() == ESIF_FALSE) {
		CMD_DEBUG("Not Supported\n");
		exit(EXIT_FAILURE);
	}

	if (disable_all_capabilities() != 0 ) {
		// Best case effort. No need to exit even if it fails
		CMD_DEBUG("Error in disabling Caps");
	}

	// Init ESIF
	esif_main_init(ESIF_PATHLIST);
	esif_ccb_sem_init(&g_sigquit);

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	// Do not start kernel driver IPC event thread for sysfs builds
	g_start_event_thread = ESIF_FALSE;
#endif

	/* Find number of online CPU cores if the feature is available,
	 * and then assign it to a global which we will use later to
	 * create the equivalent number of timer worker threads to
	 * serve timer expiration events most efficiently.
	 */
	g_nproc = get_nproc(g_nproc, sysconf(_SC_NPROCESSORS_ONLN), MAX_TIMER_THREADS_AUTO);

	optind = 1;	// Rest To 1 Restart Vector Scan

	while ((c = getopt(argc, argv, "d:f:c:b:r:i:g:a:xqtsnzplhv?")) != -1) {
		switch (c) {
		case 'd':
			g_dst = (esif_handle_t)esif_atoi64(optarg);
			break;

		case 'x':
			g_format = FORMAT_XML;
			break;

		case 'b':
			g_binary_buf_size = (int)esif_atoi(optarg);
			break;

		case 'f':
			if (fp)
				esif_ccb_fclose(fp);
			fp = esif_ccb_fopen(optarg, "r", NULL);
			if (fp == NULL) {
				fprintf(stderr, "Unable to open: %s\n", optarg);
				exit(0);
			}
			break;

		case 'c':
			esif_ccb_sprintf(sizeof(command), command, "%s", optarg);
			break;

		case 'q':
			quit_after_command = ESIF_TRUE;
			break;

		case 'r':
			g_ipc_retry_max_msec = esif_atoi(optarg);
			break;

		case 'i':
			g_ipc_retry_msec = esif_atoi(optarg);
			break;

		case 'g':
			g_nproc = get_nproc(g_nproc, esif_atoi(optarg), MAX_TIMER_THREADS_CLI);
			break;

		case 'a':
			if (repolistlen < MAX_AUTO_REPOS) {
				repolist[repolistlen++] = optarg;
			}
			g_autorepos = repolist;
			break;

#if defined (ESIF_ATTR_DAEMON)

		case 't':
			terminate_daemon = ESIF_TRUE;
			break;

		case 's':
			start_as_server = ESIF_TRUE;
			break;

		case 'n':
			start_in_background = ESIF_FALSE;
			break;

		case 'z':
			start_in_background = ESIF_TRUE;
			break;

		case 'p':
			start_with_pipe = ESIF_TRUE;
			break;

		case 'l':
			start_with_log = ESIF_TRUE;
			break;
#endif

		case 'h':
		case 'v':
		case '?':
			CMD_DEBUG(
			"Intel(R) Innovation Platform Framework (IPF), Version " ESIF_UF_VERSION "\n"
			COPYRIGHT_NOTICE "\n"
			"-d [*id]            Set Destination\n"
			"-f [*filename]      Load Filename\n"
			"-x                  XML Output Data Format\n"
			"-c [*command]       Issue Shell Command\n"
			"-q                  Quit After Command\n"
			"-b [*size]          Set Binary Buffer Size\n"
			"-r [*msec]          Set IPC Retry Timeout in msec\n"
			"-i [*msec]          Set IPC Retry Interval in msec\n"
			"-g [*pool size]     Set Number of Threads to Handle Timer Functions\n"
			"-a [*filename]      Automatically Load Data Repository File on Startup\n"
#if defined (ESIF_ATTR_DAEMON)
			"-t or 'reload'      Terminate and Reload Daemon or Server\n"
			"-s or 'server'      Run As Server [Interactive]\n"
			"-z                  Run As Daemon in Background [Default]\n"
			"-n                  Run As Daemon in Foreground\n"
			"-p                  Use Pipe For Input\n"
			"-l                  Use Log For Output\n"
#endif
			"-h or -v or -?      Display This Help\n\n");
			exit(0);
			break;

		default:
			break;
		}
	}

	// For compatibility with Windows command line options
	while (optind < argc) {
#if defined(ESIF_ATTR_DAEMON)
		if (esif_ccb_stricmp(argv[optind], "server") == 0) {
			start_as_server = ESIF_TRUE;
		}
		if (esif_ccb_stricmp(argv[optind], "reload") == 0) {
			terminate_daemon = ESIF_TRUE;
		}
#endif
		optind++;
	}

#if defined (ESIF_ATTR_DAEMON)
	// Gracefully Terminate all ipf_ufd processes (including this process)
	// This will automatically reload ipf_ufd if configured in /etc/init/dptf.conf [enabled for Chrome & Android]
	if (terminate_daemon) {
		printf("Reloading ipf_ufd daemon...\n");
		sigterm_enable();
		esif_ccb_system("killall ipf_ufd");
		exit(0);
	}

	// Start as Server or Daemon
	if (start_as_server) {
		run_as_server(fp, command, quit_after_command);
	}
	else {
		if (fp) {
			esif_ccb_fclose(fp);
			fp = NULL;
		}
		run_as_daemon(start_with_pipe, start_with_log, start_in_background);
	}
#else
	run_as_server(fp, command, quit_after_command);
#endif

	if (!instance_islocked()) {
		if (EWOULDBLOCK == errno)
			CMD_DEBUG("Aborting: Another Instance is already running\n");
		else
			CMD_DEBUG("Aborting: Unable to Obtain Instance Lock, Error #%d\n", errno);
		exit(EXIT_FAILURE);
	}

	if (fp) {
		esif_ccb_fclose(fp);
	}

	/* Wait For Worker Thread To Exit if started, otherwise wait for ESIF shell to exit or daemon to terminate */
	if (g_start_event_thread) {
		CMD_DEBUG("Stopping EVENT Thread...\n");
		esif_ccb_thread_join(&g_thread);
	}
	else {
		esif_ccb_sem_down(&g_sigquit);
	}
	CMD_DEBUG("Errorlevel Returned: %d\n", g_errorlevel);

#ifdef ESIF_FEAT_OPT_DBUS
	CMD_DEBUG("Stopping D-Bus listener thread...\n");
	pthread_cancel(g_dbus_thread);
	esif_ccb_thread_join(&g_dbus_thread);
	if (g_dbus_conn)
		dbus_connection_unref(g_dbus_conn);
#endif

	/* Exit ESIF */
	esif_uf_exit();

	/* Stop and join the SIGRTMIN worker thread */
	g_os_quit = ESIF_TRUE;
	release_timer_thread_pool();
	esif_ccb_sem_uninit(&g_sigquit);

	/* Release Instance Lock and exit*/
	instance_unlock();
	esif_main_exit();
	exit(g_errorlevel);
}

eEsifError esif_uf_os_init ()
{
	/* Start sensor manager thread */
	EsifSensorMgr_Init();

	return ESIF_OK;
}

static Bool IsIntelCPU()
{
	Bool intelCPUDetected = ESIF_FALSE;
	esif_ccb_cpuid_t cpuInfo = { 0 };

	cpuInfo.leaf = 0;
	esif_ccb_cpuid(&cpuInfo);
	//
	// Check for CPUID support and verify its an Intel processor
	//
	if (cpuInfo.eax < 1) {
		CMD_DEBUG("CPUID not supported\n");
		goto exit;
	}
	//
	// Check for "GenuineIntel"
	//
	if (cpuInfo.ebx != CPUID_LEAF_0_EBX ||
	    cpuInfo.ecx != CPUID_LEAF_0_ECX ||
	    cpuInfo.edx != CPUID_LEAF_0_EDX) {
		CMD_DEBUG("Not a genuine Intel processor\n");
		goto exit;
	}

	intelCPUDetected = ESIF_TRUE;
exit:
	return intelCPUDetected;
}

void esif_uf_os_exit ()
{
	/* Stop uevent listener */
	esif_udev_stop();
	EsifSysfsReadStop();

	/* If uevent listener thread is canceled, the netlink_msg may not be freed yet */
	esif_ccb_free(netlink_msg);

	/* Stop sensor manager thread */
	EsifSensorMgr_Exit();
}

eEsifError esif_uf_os_shell_enable()
{
	return ESIF_OK;
}

void esif_uf_os_shell_disable()
{
}
