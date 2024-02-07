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

#include "esif_ccb.h"
#include "esif_ccb_string.h"
#include "esif_ccb_thread.h"
#include "esif_ccb_timer.h"
#include "ipf_dll.h"
#include "ipf_apploader.h"
#include "ipf_trace.h"
#include "ipf_version.h"

#include <sys/file.h>
#include <signal.h>

#define PROGRAM_NAME		"ipfhostd"
#define PROGRAM_HEADER		PROGRAM_NAME " - Intel(R) Innovation Platform Framework Application Host Daemon, Version " IPF_APP_VERSION
#define PROGRAM_COPYRIGHT	"Copyright (c) 2013-2024 Intel Corporation All Rights Reserved"

typedef enum startmode {
	STARTMODE_BACKGROUND_DAEMON,
	STARTMODE_FOREGROUND_DAEMON,
	STARTMODE_FOREGROUND_CONSOLE,
} startmode_t;

static Bool g_exitApp = ESIF_FALSE;	// Signal to Exit AppLoader
static int g_tracelevel = IPF_TRACE_LEVEL_ERROR; // AppLoader Trace Level

/* Number of threads in thread pool: default to 1, but will
 * change to the actual number of processore cores if there
 * is support to the _SC_NPROCESSORS_ONLN system configuration
 * at run time.
 */
static long g_nproc = 1;

#define MAX_TIMER_THREADS_AUTO 16 // max number of timer work threads allowed from auto-detection

// Dedicated thread pool to handle SIGRTMIN timer signals
static esif_thread_t *g_sigrtmin_thread_pool;
static Bool g_rtmin_thread_quit = ESIF_FALSE; // global flag to supress KW flagging as while(1) alternative

#  include <syslog.h>
#  define ESIF_PRIORITY_FATAL   LOG_EMERG
#  define ESIF_PRIORITY_ERROR   LOG_ERR
#  define ESIF_PRIORITY_WARNING LOG_WARNING
#  define ESIF_PRIORITY_INFO    LOG_INFO
#  define ESIF_PRIORITY_DEBUG   LOG_DEBUG

// Local Trace Message Handler for AppLoader
esif_error_t ESIF_CALLCONV apploader_tracemessage(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr message,
	const eLogType logType
)
{
	UNREFERENCED_PARAMETER(esifHandle);
	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);

	if (message && message->buf_ptr) {
		static int priority[] = { ESIF_PRIORITY_FATAL, ESIF_PRIORITY_ERROR, ESIF_PRIORITY_WARNING, ESIF_PRIORITY_INFO, ESIF_PRIORITY_DEBUG };
		static char *levelname[] = { "FATAL", "ERROR", "WARNING", "INFO", "DEBUG" };
		int traceidx = (logType >= eLogTypeFatal && logType <= eLogTypeDebug ? logType : eLogTypeDebug);
		size_t buffer_len = message->data_len + 10;
		char *buffer = malloc(buffer_len);
		if (buffer) {
			esif_ccb_sprintf(buffer_len, buffer, "%s:%s", levelname[traceidx], (char *)message->buf_ptr);

			openlog(PROGRAM_NAME, LOG_PID, LOG_DAEMON);
			syslog(priority[traceidx], "%s", buffer);
			closelog();
			free(buffer);
		}
	}
	return ESIF_OK;
}

// Start AppLoader and Load Dynamic ESIF or Core API Library App using specified Server Address (Pipe)
static esif_error_t start_apploader(char *libname, char *serveraddr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (libname && serveraddr && ((rc = IpfTrace_Init()) == ESIF_OK)) {
		IpfTrace_SetConfig(PROGRAM_NAME, apploader_tracemessage, ESIF_FALSE);
		IpfTrace_SetTraceLevel(g_tracelevel);

		char appLibPath[MAX_PATH] = { 0 };
		IpfDll_GetFullPath(libname, appLibPath, sizeof(appLibPath));
		printf("Starting AppLoader: %s [%s]\n", appLibPath, serveraddr);

		rc = Apploader_Init(appLibPath, serveraddr);
		if (rc == ESIF_OK) {

			rc = Apploader_Start();
			if (rc == ESIF_OK) {
				// Sleep forever or until we get a signal to exit
				while (!g_exitApp) {
					esif_ccb_sleep_msec((UInt32)(-1));
				}
			}
		}
		Apploader_Stop();
		Apploader_Exit();
		IpfTrace_Exit();
		printf("Exiting AppLoader: %s (%d)\n", esif_rc_str(rc), rc);
	}
	return rc;
}

// SIGTERM Signal Handler
static void sigterm_handler(int signum)
{
	// Signal Main AppLoader Thread to Stop
	g_exitApp = ESIF_TRUE;
}

// Enable or Disable SIGTERM Handler
void sigterm_enable()
{
	struct sigaction action={0};
	action.sa_handler = sigterm_handler;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT,  &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
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
		printf("Fail to block SIGRTMIN\n");
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
	// This thread will be canceled/killed after apploader_exit 
	while (!g_rtmin_thread_quit) {
		/* sigwaitinfo is a cancellation point */
		if (-1 == sigwaitinfo(&set, &info)) {
			/* When there are multiple worker threads waiting for the same signal,
			 * only one thread will get the signal and its associated info structure,
			 * and the rest will get an errno of EINTR. In this case, simply continue
			 * the loop and wait again.
			 */
			continue;
		}
		if (SIGRTMIN != info.si_signo) {
			printf("Received signal is not SIGRTMIN, ignore\n");
			continue;
		}

		cb_object_ptr = (struct esif_tmrm_cb_obj *) info.si_value.sival_ptr;
		if (cb_object_ptr && cb_object_ptr->func) {
			printf("Calling callback function...\n");
			cb_object_ptr->func(cb_object_ptr->cb_handle);
			cb_object_ptr->func = NULL;
			/* Use native free function instead of esif_ccb_free(). For explanation,
			 * please see the comments under esif_ccb_timer_obj_enable_timer() in
			 * file esif_ccb_timer_lin_user.h.
			 */
			free(cb_object_ptr);
		} else {
			printf("Invalid timer callback object: (obj=%p, func=%p)\n",
				cb_object_ptr, (cb_object_ptr ? cb_object_ptr->func : NULL));
		}
	}

	return NULL;
}

static enum esif_rc create_timer_thread_pool(void)
{
	int i = 0;
	enum esif_rc rc = ESIF_OK;

	// Use native malloc instead of esif_ccb_malloc
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

static long get_nproc(long old, long new, long max)
{
	if (new > 0) {
		return (new > max) ? max : new;
	} else {
		return old;
	}
}

// Start Daemon (Foreground or Background) or as a Foreground Console App
static esif_error_t start_daemon(startmode_t startup_mode, char *libname, char *serveraddr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	/* Find number of online CPU cores if the feature is available,
	 * and then assign it to a global which we will use later to
	 * create the equivalent number of timer worker threads to
	 * serve timer expiration events most efficiently.
	 */
	g_nproc = get_nproc(g_nproc, sysconf(_SC_NPROCESSORS_ONLN), MAX_TIMER_THREADS_AUTO);

	// Run as a Foreground Console App and Exit
	if (startup_mode == STARTMODE_FOREGROUND_CONSOLE) {
		printf(
			PROGRAM_HEADER "\n"
			PROGRAM_COPYRIGHT "\n"
		);
		// Block SIGRTMIN (will re-enable it in dedicated thread pool)
		sigrtmin_block();
		// Start the thread pool that handles SIGRTMIN
		rc = create_timer_thread_pool();
		if (ESIF_OK != rc) {
			printf("ipfhostd: Time Thread pool creation failed. Error #%d\n", errno);
			return rc;
		}
		sigterm_enable();
		rc = start_apploader(libname, serveraddr);
		goto exit;
	}

	// Run as a Foreground or Background Daemon

	// 1. Call Fork (if background daemon)
	pid_t process_id = (startup_mode == STARTMODE_BACKGROUND_DAEMON ? fork() : getpid());

	if (-1 == process_id) {
		printf("ipfhostd: %s failed. Error #%d\n", (startup_mode == STARTMODE_BACKGROUND_DAEMON ? "fork" : "getpid"), errno);
		return ESIF_E_NOT_INITIALIZED;
	}
	else if (process_id != 0) {
		printf( "\n"
				PROGRAM_HEADER "\n"
				PROGRAM_COPYRIGHT "\n"
				"%s Daemon Process: %d\n"
			, (startup_mode == STARTMODE_BACKGROUND_DAEMON ? "Background" : "Foreground")
			, process_id
		);
		// 2. Exit From Parent (if background daemon)
		if (startup_mode == STARTMODE_BACKGROUND_DAEMON) {
			exit(EXIT_SUCCESS);
		}
	}

	// 3. Call setsid (if background daemon)
	if (startup_mode == STARTMODE_BACKGROUND_DAEMON && -1 == setsid()) {
		printf("ipfhostd: setsid failed. Error #%d\n", errno);
		return ESIF_E_NOT_INITIALIZED;
	}

	// 4. Block SIGRTMIN (will re-enable it in dedicated thread pool)
	sigterm_enable();
	sigrtmin_block();
	// Start the thread pool that handles SIGRTMIN
	rc = create_timer_thread_pool();
	if (ESIF_OK != rc) {
		printf("ipfhostd: Time Thread pool creation failed. Error #%d\n", errno);
		return rc;
	}

	// 5. Close all file descriptors incuding stdin, stdout, stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// 6. Open file descriptors 0, 1, and 2 and redirect stdin, stdout, stderr
	int stdinfd = open("/dev/null", O_RDWR);
	if (dup(0) != -1) {
		setvbuf(stdout, NULL, _IONBF, 0);
	}
	if (dup(0) != -1) {
		setvbuf(stderr, NULL, _IONBF, 0);
	}

	// Start the AppLoader using the specified Pipe
	rc = start_apploader(libname, serveraddr);

	if (stdinfd != -1) {
		close(stdinfd);
	}

exit:
	/* Stop and join the SIGRTMIN worker thread */
	g_rtmin_thread_quit = ESIF_TRUE;
	release_timer_thread_pool();
	return rc;
}

// Main Entry Point
int main (int argc, char **argv)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	startmode_t startup_mode = STARTMODE_BACKGROUND_DAEMON;
	char *libname = NULL;
	char *serveraddr = DEFAULT_SERVERADDR;

	// Parse Command Line Options
	for (int j = 1; j < argc; j++) {
		char *param = "";
		if (esif_ccb_strncmp(argv[j], "--", 2) == 0) {
			param = &argv[j][2];
		}
		else if (argv[j][0] == '-' || argv[j][0] == '/') {
			param = &argv[j][1];
		}
		char *opt = esif_ccb_strpbrk(param, ":=");

		// Individual Options
		if (esif_ccb_strnicmp(param, "load", 4) == 0 && opt) {
			libname = ++opt;
		}
		else if (esif_ccb_strnicmp(param, "srv", 3) == 0 && opt) {
			serveraddr = ++opt;
		}
		else if (esif_ccb_strnicmp(param, "trace", 5) == 0 && opt) {
			int level = atoi(++opt);
			if (level >= IPF_TRACE_LEVEL_FATAL && level <= IPF_TRACE_LEVEL_DEBUG) {
				g_tracelevel = level;
			}
		}
		else if (esif_ccb_strnicmp(param, "daemon", 6) == 0) {
			startup_mode = STARTMODE_BACKGROUND_DAEMON;
		}
		else if (esif_ccb_strnicmp(param, "foreground", 4) == 0) {
			startup_mode = STARTMODE_FOREGROUND_DAEMON;
		}
		else if (esif_ccb_strnicmp(param, "console", 7) == 0) {
			startup_mode = STARTMODE_FOREGROUND_CONSOLE;
		}
		else {
			libname = serveraddr = NULL;
			break;
		}
	}
	
	// Help Text
	if (libname == NULL || serveraddr == NULL) {
		printf(
			PROGRAM_HEADER "\n"
			PROGRAM_COPYRIGHT "\n"
			"Usage: ipfhostd --load:libname.so [options]\n"
			"Options:\n"
			"  --load:libname.so Load Dynamic Library ESIF or Core API App\n"
			"  --srv:serveraddr  Connect to Server Address [Default=%s]\n"
			"  --trace:level     Set Trace Level [Default=%d]\n"
			"  --daemon          Run in Background as Daemon [Default]\n"
			"  --foreground      Run in Foreground as Daemon\n"
			"  --console         Run in Foreground as Console App\n"
			, DEFAULT_SERVERADDR
			, IPF_TRACE_LEVEL_ERROR
		);
		exit(ESIF_E_INVALID_ARGUMENT_COUNT);
	}

	// Start Daemon in specified mode
	rc = start_daemon(startup_mode, libname, serveraddr);

	exit(rc);
}
