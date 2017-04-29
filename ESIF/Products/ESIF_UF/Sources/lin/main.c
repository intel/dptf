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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_LINUX
#include <sys/socket.h>
#include <linux/netlink.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>

#include "esif_uf.h"
#include "esif_uf_appmgr.h"
#include "esif_pm.h"
#include "esif_version.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_ccb_system.h"
#include "esif_uf_event_broadcast.h"
#include "esif_uf_sensor_manager_os_lin.h"

#define COPYRIGHT_NOTICE "Copyright (c) 2013-2017 Intel Corporation All Rights Reserved"

/* ESIF_UF Startup Script Defaults */
#ifdef ESIF_ATTR_OS_ANDROID
#define ESIF_STARTUP_SCRIPT_DAEMON_MODE		"conjure upe_java && appstart dptf"
#else
#define ESIF_STARTUP_SCRIPT_DAEMON_MODE		"appstart dptf"
#endif
#define ESIF_STARTUP_SCRIPT_SERVER_MODE		NULL

static void esif_udev_start();
static void esif_udev_stop();
static void esif_udev_exit();
static Bool esif_udev_is_started();
static void *esif_udev_listen(void *ptr);
static void esif_process_udev_event(char *udev_target);
static int kobj_uevent_parse(char *buffer, int len, char **zone_name, int *temp, int *event);

static esif_thread_t g_udev_thread;
static Bool g_udev_quit = ESIF_TRUE;
static char *g_udev_target = NULL;

#define MAX_PAYLOAD 1024 /* max message size*/

static struct sockaddr_nl sock_addr_src, sock_addr_dest;
static struct nlmsghdr *netlink_msg = NULL;
static struct iovec msg_buf;
static int sock_fd;
static struct msghdr msg;


/* Friend */
extern EsifAppMgr g_appMgr;

#ifdef ESIF_ATTR_OS_LINUX_HAVE_READLINE
	#include <readline/readline.h>
	#include <readline/history.h>
#endif

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
static struct instancelock g_instance = {"esif_ufd.pid"};
static char device_path[] = "/devices/virtual/thermal/thermal_zone";

#define HOME_DIRECTORY	NULL /* use OS-specific default */

#ifdef ESIF_ATTR_64BIT
# define ARCHNAME "x64"
# define ARCHBITS "64"
#else
#define ARCHNAME "x86"
# ifdef ESIF_ATTR_OS_ANDROID
#  define ARCHBITS ""	// Android Libs go in /system/vendor/lib64 or lib, not lib32
# else
#  define ARCHBITS "32"
# endif
#endif

// Default ESIF Paths for each OS
// Paths preceded by "#" indicate that full path is not specified when loading the binary
static const esif_string ESIF_PATHLIST =
#if defined(ESIF_ATTR_OS_ANDROID)
	// Android
	"HOME=/etc/dptf\n"
	"TEMP=/data/misc/dptf/tmp\n"
	"DV=/etc/dptf/dv\n"
	"LOG=/data/misc/dptf/log\n"
	"BIN=/etc/dptf/bin\n"
	"LOCK=/data/misc/dptf/lock\n"
	"EXE=#/system/vendor/bin\n"
	"DLL=#/system/vendor/lib" ARCHBITS "\n"
	"DPTF=/etc/dptf/bin\n"
	"DSP=/etc/dptf/dsp\n"
	"CMD=/etc/dptf/cmd\n"
	"UI=/etc/dptf/ui\n"
#elif defined(ESIF_ATTR_OS_CHROME)
	// Chromium
	"HOME=/usr/share/dptf\n"
	"TEMP=/tmp\n"
	"DV=/etc/dptf\n"
	"LOG=/usr/local/var/log/dptf\n"
	"BIN=/usr/share/dptf/bin\n"
	"LOCK=/var/run\n"
	"EXE=#/usr/bin\n"
	"DLL=#/usr/lib" ARCHBITS "\n"
	"DPTF=/usr/share/dptf\n"
	"DSP=/etc/dptf/dsp\n"
	"CMD=/etc/dptf/cmd\n"
	"UI=/usr/share/dptf/ui\n"
#else
	// Generic Linux
	"HOME=/usr/share/dptf\n"
	"TEMP=/tmp\n"
	"DV=/etc/dptf\n"
	"LOG=/usr/share/dptf/log\n"
	"BIN=/usr/share/dptf/bin\n"
	"LOCK=/var/run\n"
	"EXE=/usr/share/dptf/uf" ARCHNAME "\n"
	"DLL=/usr/share/dptf/uf" ARCHNAME "\n"
	"DPTF=/usr/share/dptf/uf" ARCHNAME "\n"
	"DSP=/usr/share/dptf/dsp\n"
	"CMD=/usr/share/dptf/cmd\n"
	"UI=/usr/share/dptf/ui\n"
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

/* Worker Thread in esif_uf */
Bool g_start_event_thread = ESIF_TRUE;
esif_thread_t g_thread;
void *esif_event_worker_thread (void *ptr);

/* IPC Resync */
extern enum esif_rc sync_lf_participants();
extern esif_handle_t g_ipc_handle;

u32 g_ipc_retry_msec	 = 100;		/* 100ms by default */
u32 g_ipc_retry_max_msec = 2000;	/* 2 sec by default */

/* Thermal notification reason */
enum thermal_notify_event {
	THERMAL_EVENT_UNDEFINED, /* Undefined event */
	THERMAL_EVENT_TEMP_SAMPLE, /* New Temperature sample */
	THERMAL_EVENT_TRIP_VIOLATED, /* TRIP Point violation */
	THERMAL_EVENT_TRIP_CHANGED, /* TRIP Point temperature changed */
};

/* Attempt IPC Connect and Sync for specified time if no IPC connection */
eEsifError ipc_resync()
{
#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	return ESIF_OK;
#else
	eEsifError rc = ESIF_OK;
	u32 elapsed_msec = 0;

	if (g_ipc_handle == ESIF_INVALID_HANDLE) {
		rc = ESIF_E_NO_LOWER_FRAMEWORK;

		while (!g_quit && elapsed_msec < g_ipc_retry_max_msec) {
			ipc_connect();
			if (g_ipc_handle != ESIF_INVALID_HANDLE) {
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

/* Emulate Windows kbhit Function */
static int kbhit (void)
{
	struct termios save_t = {0};
	struct termios new_t  = {0};
	int key     = 0;
	int save_fc = 0;

	/* Get Terminal Attribute */
	tcgetattr(STDIN_FILENO, &save_t);
	new_t = save_t;
	new_t.c_lflag &= ~(ICANON | ECHO);

	/* Set Terminal Attribute */
	tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
	save_fc = fcntl(STDIN_FILENO, F_GETFL, 0);

	/* Add NONBLOCK To Existing File Control */
	fcntl(STDIN_FILENO, F_SETFL, save_fc | O_NONBLOCK);

	/* If No Character No Problem */
	key = getchar();

	/* Remove NONBLOCK Attribute */
	tcsetattr(STDIN_FILENO, TCSANOW, &save_t);
	fcntl(STDIN_FILENO, F_SETFL, save_fc);

	if (key != EOF) {
		return 1;
	}
	return 0;
}

static int kobj_uevent_parse(char *buffer, int len, char **zone_name, int *temp, int *event)
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
				*temp= atoi(buffer + i + esif_ccb_strlen(temp_eq, sizeof(temp_eq)));
		}
		else if (esif_ccb_strlen(buffer + i, len - i) > esif_ccb_strlen(event_eq, sizeof(event_eq))
				&& esif_ccb_strncmp(buffer + i, event_eq, esif_ccb_strlen(event_eq, sizeof(event_eq))) == 0) {
				*event= atoi(buffer + i + esif_ccb_strlen(event_eq, sizeof(event_eq)));
		}
		i += esif_ccb_strlen(buffer + i, len - i) + 1;
	}

	if (*temp != -1 && *event != -1)
		return 1;
	else
		return 0;
}

static int check_for_uevent(int fd) {
	int i = 0, j;
	int len;
	const char dev_path[] = "DEVPATH=";
	unsigned int dev_path_len = sizeof(dev_path) - 1;
	char buffer[MAX_PAYLOAD + 1];
	char thermal_path[MAX_PAYLOAD + 1];
	char *buf_ptr;
	char *zone_name;
	int temp;
	int event;

	len = recv(fd, buffer, sizeof(buffer), 0);
	if (len <= 0) {
		return 0;
	}

	while (i < len) {

		buf_ptr = buffer + i;
		if (!buf_ptr)
			break;

		if (esif_ccb_strlen(buf_ptr, sizeof(dev_path)) > dev_path_len
				&& esif_ccb_strncmp(buf_ptr, dev_path, dev_path_len) == 0) {

			if (esif_ccb_strncmp(buf_ptr + dev_path_len, device_path, sizeof(device_path) - 1) == 0) {
				char *parsed = NULL;
				char *ctx = NULL;

				esif_ccb_strncpy(thermal_path, buf_ptr + dev_path_len, MAX_PAYLOAD);
				parsed = esif_ccb_strtok(thermal_path, "/", &ctx);
				while (parsed != NULL) {
					esif_ccb_strcpy(g_udev_target,parsed,MAX_PAYLOAD);
					parsed = esif_ccb_strtok(NULL,"/",&ctx);
				}

				if (!kobj_uevent_parse(buffer, len, &zone_name, &temp, &event))
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
					for (j = 0; j < MAX_PARTICIPANT_ENTRY; j++)
						EsifEventMgr_SignalEvent(j, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED, NULL);
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
	g_quit = 1; /* Attempt Graceful ESIF Shutdown */
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
		unlink(fullpath);
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

static void esif_udev_exit()
{
	g_udev_quit = ESIF_TRUE;
#ifdef ESIF_ATTR_OS_ANDROID
	// Android NDK does not support pthread_cancel()
	// Use pthread_kill() to emualte
	pthread_kill(g_udev_thread, SIGUSR1);
#else
	pthread_cancel(g_udev_thread);
#endif
	esif_ccb_thread_join(&g_udev_thread);
	esif_ccb_free(g_udev_target);
}

static Bool esif_udev_is_started()
{
	return !g_udev_quit;
}


static void *esif_udev_listen(void *ptr)
{
	UNREFERENCED_PARAMETER(ptr);
#ifdef ESIF_ATTR_OS_ANDROID
	sigusr1_enable();
#endif
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
	if (sock_fd < 0) {
		goto exit;
	}

	memset(&sock_addr_src, 0, sizeof(sock_addr_src));
	sock_addr_src.nl_family = AF_NETLINK;
	sock_addr_src.nl_pid = getpid();
	sock_addr_src.nl_groups = -1;

	bind(sock_fd, (struct sockaddr *)&sock_addr_src, sizeof(sock_addr_src));

	memset(&sock_addr_dest, 0, sizeof(sock_addr_dest));
	memset(&sock_addr_dest, 0, sizeof(sock_addr_dest));
	sock_addr_dest.nl_family = AF_NETLINK;
	sock_addr_dest.nl_pid = 0;
	sock_addr_dest.nl_groups = 0;

	netlink_msg = (struct nlmsghdr *)esif_ccb_malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (netlink_msg == NULL) {
		goto exit;
	}
	memset(netlink_msg, 0, NLMSG_SPACE(MAX_PAYLOAD));
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
		EsifEventMgr_SignalEvent(participant_id, domain_index++, ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED, NULL);

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
	int message_type = dbus_message_get_type (message);
	int i = 0;

	if (DBUS_MESSAGE_TYPE_SIGNAL == message_type) {
		if (!esif_ccb_strcmp(dbus_message_get_member(message), "SuspendImminent")) {
			ESIF_TRACE_INFO("D-Bus: received SuspendImminent signal\n");
			for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++)
				EsifEventMgr_SignalEvent(i, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_SUSPEND, NULL);
		} else if (!esif_ccb_strcmp(dbus_message_get_member(message), "SuspendDone")) {
			ESIF_TRACE_INFO("D-Bus: received SuspendDone signal\n");
			for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++)
				EsifEventMgr_SignalEvent(i, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_PARTICIPANT_RESUME, NULL);
		}
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

void* dbus_listen()
{
	DBusError err = {0};

#ifdef ESIF_ATTR_OS_ANDROID
	sigusr1_enable();
#endif

	dbus_error_init(&err);
	g_dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
		return NULL;
	}

	dbus_bus_add_match(g_dbus_conn, "type='signal'", &err);
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
**  Buiild Supports Deaemon?
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
	char line2[MAX_LINE + 1] = {0};
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

	/* 4. Change to known directory.  Performed in main */
	/* 5. Close all file descriptors incuding stdin, stdout, stderr */
	close(STDIN_FILENO); /* stdin */
	close(STDOUT_FILENO); /* stdout */
	close(STDERR_FILENO); /* stderr */

	/* 6. Open file descriptors 0, 1, and 2 and redirect */
	/* stdin */
	if (ESIF_TRUE == start_with_log) {
		unlink(cmd_out);
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
			unlink(cmd_in);
		}
	}

	/* Process Input ? */
	if (ESIF_TRUE == start_with_pipe) {

		/* Open Input Pipe using FIFO PIPE? */
		if (!g_shell_enabled) {
			start_with_pipe = ESIF_FALSE;
		}
		else {
			unlink(cmd_in);
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
	}
	if (stdinfd != -1)
		close(stdinfd);
	return ESIF_TRUE;
}
#endif

static int run_as_server(FILE* input, char* command, int quit_after_command)
{
	char *ptr = NULL;
	char line[MAX_LINE + 1]  = {0};
	char line2[MAX_LINE + 1] = {0};

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
		return ESIF_FALSE;
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
		if (command) {
				parse_cmd(command, ESIF_FALSE, ESIF_TRUE);
				if (ESIF_TRUE == quit_after_command) {
						g_quit = 1;
						continue;
				}
		}

		// Get User Input
		g_appMgr.GetPrompt(&g_appMgr, &data_prompt);
		prompt = (esif_string)data_prompt.buf_ptr;

#ifdef ESIF_ATTR_OS_LINUX_HAVE_READLINE
		// Use Readline With History
		esif_ccb_sprintf(sizeof(full_prompt), full_prompt, "%s ", prompt);
		CMD_LOGFILE("%s ", prompt);
		ptr = readline(full_prompt);

		// Skip command and wait for graceful shutdown if readline returns NULL due to SIGTERM
		if (NULL == ptr)
			continue;

		// Add To History NO NUL's
		if (ptr[0] != 0) {
				add_history(ptr);
		}
		esif_ccb_strcpy(line, ptr, sizeof(line));
		free(ptr);
#else
		// No History So Sorry
		CMD_OUT("%s ", prompt);
		if (esif_ccb_fgets(line, MAX_LINE, input) == NULL) {
				break;
		}
		ptr = line;
		while (*ptr != '\0') {
				if (*ptr == '\r' || *ptr == '\n' || *ptr == '#') {
						*ptr = '\0';
				}
				ptr++;
		}
#endif
		CMD_LOGFILE("%s\n", line);
		esif_shell_execute(line);
	}
	return ESIF_TRUE;
}

int main (int argc, char **argv)
{
	int c = 0;
	FILE *fp = NULL;
	char command[MAX_LINE + 1] = {0};
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

	// Init ESIF
	esif_main_init(ESIF_PATHLIST);

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	g_start_event_thread = ESIF_FALSE;
#endif

	optind = 1;	// Rest To 1 Restart Vector Scan

	while ((c = getopt(argc, argv, "d:f:c:b:r:i:xhq?tsnzpl")) != -1) {
		switch (c) {
		case 'd':
			g_dst = (u8)esif_atoi(optarg);
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
		case '?':
			CMD_DEBUG(
			"ESIF Eco-System Independent Framework Shell\n"
			COPYRIGHT_NOTICE "\n"
			"-d [*id]            Set Destination\n"
			"-f [*filename]      Load Filename\n"
			"-x                  XML Output Data Format\n"
			"-c [*command]       Issue Shell Command\n"
			"-q                  Quit After Command\n"
			"-b [*size]          Set Binary Buffer Size\n"
			"-r [*msec]          Set IPC Retry Timeout in msec\n"
			"-i [*msec]          Set IPC Retry Interval in msec\n"
#if defined (ESIF_ATTR_DAEMON)
			"-t or 'reload'      Terminate and Reload Daemon or Server\n"
			"-s or 'server'      Run As Server [Interactive]\n"
			"-z                  Run As Daemon in Background [Default]\n"
			"-n                  Run As Daemon in Foreground\n"
			"-p                  Use Pipe For Input\n"
			"-l                  Use Log For Output\n"
#endif
			"-h or -?            This Help\n\n");
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
	// Gracefully Terminate all esif_ufd processes (including this process)
	// This will automatically reload esif_ufd if configured in /etc/init/dptf.conf [enabled for Chrome & Android]
	if (terminate_daemon) {
		printf("Reloading esif_ufd daemon...\n");
		sigterm_enable();
		esif_ccb_system("killall esif_ufd");
		exit(0);
	}

	// Start as Server or Daemon
	if (start_as_server) {
		run_as_server(fp, command, quit_after_command);
	} else {
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

	/* Wait For Worker Thread To Exit if started, otherwise wait for ESIF to exit */
	if (g_start_event_thread) {
		CMD_DEBUG("Stopping EVENT Thread...\n");
		esif_ccb_thread_join(&g_thread);
	}
	else {
		while (!g_quit) {
			esif_ccb_sleep_msec(250);
		}
	}
	CMD_DEBUG("Errorlevel Returned: %d\n", g_errorlevel);

#ifdef ESIF_FEAT_OPT_DBUS
	CMD_DEBUG("Stopping D-Bus listener thread...\n");
#ifdef ESIF_ATTR_OS_ANDROID
	// Android NDK does not support pthread_cancel()
	// Use pthread_kill() to emualte
	pthread_kill(g_dbus_thread, SIGUSR1);
#else
	pthread_cancel(g_dbus_thread);
#endif
	esif_ccb_thread_join(&g_dbus_thread);
	if (g_dbus_conn)
		dbus_connection_unref(g_dbus_conn);
#endif


	/* Exit ESIF */
	esif_uf_exit();

	/* Release Instance Lock and exit*/
	instance_unlock();
	esif_main_exit();
	exit(g_errorlevel);
}

eEsifError esif_uf_os_init ()
{
	/* Start sensor manager thread */
	EsifEventBroadcast_MotionSensorEnable(ESIF_TRUE);
	EsifSensorMgr_Init();

	return ESIF_OK;
}


void esif_uf_os_exit ()
{
	/* Stop uevent listener */
	esif_udev_stop();

	/* Stop sensor manager thread */
	EsifEventBroadcast_MotionSensorEnable(ESIF_FALSE);
	EsifSensorMgr_Exit();
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
