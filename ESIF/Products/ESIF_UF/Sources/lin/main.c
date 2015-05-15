/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#include <termios.h>
#include <errno.h>
#include <fcntl.h>

#include "esif_uf.h"
#include "esif_uf_appmgr.h"
#include "esif_version.h"
#include "esif_uf_eventmgr.h"

#define COPYRIGHT_NOTICE "Copyright (c) 2013-2015 Intel Corporation All Rights Reserved"

/* ESIF_UF Startup Script Defaults */
#define LF "\n"
#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
/* Place holder for SysFs model that may require different startup script */
#define ESIF_STARTUP_SCRIPT_DAEMON_MODE		"appstart dptf"
#else 
#define ESIF_STARTUP_SCRIPT_DAEMON_MODE		"appstart dptf"
#endif

#define ESIF_STARTUP_SCRIPT_SERVER_MODE		NULL

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

#define HOME_DIRECTORY	NULL /* use OS-specific default */

#ifdef ESIF_ATTR_64BIT
#define ARCHNAME "x64"
#define ARCHBITS "64"
#else
#define ARCHNAME "x86"
#define ARCHBITS ""
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
	"LOCK=/mnt/dptf\n"
	"EXE=#/system/bin\n"
	"DLL=#/system/lib\n"
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
	"EXE=/usr/bin\n"
	"DLL=/usr/lib" ARCHBITS "\n"
	"DPTF=/usr/lib" ARCHBITS "\n"
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
extern char *g_DataVaultStartScript;

/* Worker Thread in esif_uf */
Bool g_start_event_thread = ESIF_TRUE;
esif_thread_t g_thread;
void *esif_event_worker_thread (void *ptr);

/* IPC Resync */
extern enum esif_rc sync_lf_participants();
extern esif_handle_t g_ipc_handle;

u32 g_ipc_retry_msec	 = 100;		/* 100ms by default */
u32 g_ipc_retry_max_msec = 2000;	/* 2 sec by default */

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
	if ((g_instance.lockfd = open(fullpath, O_CREAT | O_RDWR, 0644)) == 0 || flock(g_instance.lockfd, LOCK_EX | LOCK_NB) != 0) {
		int error = errno;
		close(g_instance.lockfd);
		g_instance.lockfd = 0;
		errno = error;
		return ESIF_FALSE;
	}
	sprintf(pid_buffer, "%d\n", getpid());
	rc = write(g_instance.lockfd, pid_buffer, strlen(pid_buffer));
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

static int run_as_daemon(int start_with_pipe, int start_with_log)
{
	FILE *fp = NULL;
	char *ptr = NULL;
	char line[MAX_LINE + 1] = {0};
	char line2[MAX_LINE + 1] = {0};
	char cmd_in[MAX_PATH] = {0};
	char cmd_out[MAX_PATH] = {0};

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

	/* 1. Call Fork */
	pid_t process_id = fork();

	if (-1 == process_id) {
		return ESIF_FALSE;
	} else if (process_id != 0) {
		/* 2. Exit From Parent */
		CMD_DEBUG("\nESIF Daemon/Loader Version %s\n", ESIF_VERSION);
		CMD_DEBUG(COPYRIGHT_NOTICE "\n");
		CMD_DEBUG("Spawn Daemon ESIF Child Process: %d\n", process_id);
		if (ESIF_TRUE == start_with_pipe) {
			CMD_DEBUG("Command Input:  %s\n", cmd_in);
		}

		if (ESIF_TRUE == start_with_log) {
			CMD_DEBUG("Command Output: %s\n", cmd_out);
		}
		exit(EXIT_SUCCESS);
	}

	/* 3. Call setsid */
	if (-1 == setsid()) {
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
		open (cmd_out, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	}
	else {
		open ("/dev/null", O_RDWR);
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
	g_DataVaultStartScript = ESIF_STARTUP_SCRIPT_DAEMON_MODE;
	esif_uf_init();
	ipc_resync();
	if (g_start_event_thread) {
		esif_ccb_thread_create(&g_thread, esif_event_worker_thread, "Daemon");
	}
	cmd_app_subsystem(SUBSYSTEM_ESIF);

	/* Start D-Bus listener thread if necessary */
#ifdef ESIF_FEAT_OPT_DBUS
	esif_ccb_thread_create(&g_dbus_thread, dbus_listen, "D-Bus Listener");
#endif

	/* UF Startup Script may have enabled/disabled ESIF Shell */
	if (!g_shell_enabled) {
		CMD_OUT("Shell Unavailable. Closing Command Pipes.\n");

		/* Redirect stdout, stderr to NULL and close input pipe */
		if (ESIF_TRUE == start_with_log) {
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			open("/dev/null", O_RDWR);
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
			int count = 0;
			fp = fopen(cmd_in, "r");
			if (NULL == fp) {
				break;
			}

			/* Grab line from FIFO PIPE */
			if (fgets(line, MAX_LINE, fp) == NULL) {
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
		}

		if (NULL != fp) {
			fclose(fp);
		}
	}
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

	/* Guarantee a single instance is running */
	if (!instance_lock()) {
		return ESIF_FALSE;
	}
	sigterm_enable();

	g_DataVaultStartScript = ESIF_STARTUP_SCRIPT_SERVER_MODE;
	esif_uf_init();
	ipc_resync();
	if (g_start_event_thread) {
		esif_ccb_thread_create(&g_thread, esif_event_worker_thread, "Server");
		esif_ccb_sleep_msec(10);
	}
	cmd_app_subsystem(SUBSYSTEM_ESIF);

	/* Start D-Bus listener thread if necessary */
#ifdef ESIF_FEAT_OPT_DBUS
	esif_ccb_thread_create(&g_dbus_thread, dbus_listen, "D-Bus Listener");
#endif

 	while (!g_quit) {
		int count = 0;
		// Startup Command?
		if (command) {
				parse_cmd(command, ESIF_FALSE);
				if (ESIF_TRUE == quit_after_command) {
						g_quit = 1;
						continue;
				}
		}

		// Exit if Shell disabled
		if (!g_shell_enabled) {
			CMD_OUT("Shell Unavailable.\n");
			g_quit = 1;
			continue;
		}

		// Get User Input
		g_appMgr.GetPrompt(&g_appMgr, &data_prompt);
		prompt = (esif_string)data_prompt.buf_ptr;

#ifdef ESIF_ATTR_OS_LINUX_HAVE_READLINE
		// Use Readline With History
		sprintf(full_prompt, "%s ", prompt);
		CMD_LOGFILE("%s ", prompt);
		ptr = readline(full_prompt);

		// Skip command and wait for graceful shutdown if readline returns NULL due to SIGTERM
		if (NULL == ptr)
			continue;

		// Add To History NO NUL's
		if (ptr[0] != 0) {
				add_history(ptr);
		}
		strcpy(line, ptr);
		free(ptr);
#else
		// No History So Sorry
		CMD_OUT("%s ", prompt);
		if (fgets(line, MAX_LINE, input) == NULL) {
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
	FILE *fp = stdin;
	char command[MAX_LINE + 1] = {0};
	int quit_after_command = ESIF_FALSE;
#if defined(ESIF_ATTR_DAEMON)
	int start_as_server = ESIF_FALSE;
	int start_with_pipe = ESIF_FALSE;
	int start_with_log = ESIF_FALSE;
#else
	int start_as_server = ESIF_TRUE;
#endif

	// Init ESIF
	esif_main_init(ESIF_PATHLIST);

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	g_start_event_thread = ESIF_FALSE;
#endif

	optind = 1;	// Rest To 1 Restart Vector Scan

	while ((c = getopt(argc, argv, "d:f:c:b:r:i:xhq?spl")) != -1) {
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
			fp = fopen(optarg, "r");
			break;

		case 'c':
			sprintf(command, "%s", optarg);
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

		case 's':
			start_as_server = ESIF_TRUE;
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
			"-s                  Run As Server\n"
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

#if defined (ESIF_ATTR_DAEMON)
	if (start_as_server) {
		run_as_server(fp, command, quit_after_command);
	} else {
		run_as_daemon(start_with_pipe, start_with_log);
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

	if (fp && fp != stdin) {
		fclose(fp);
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
	pthread_cancel(g_dbus_thread);
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
	return ESIF_OK;
}


void esif_uf_os_exit ()
{
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
