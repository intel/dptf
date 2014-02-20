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

#include "esif_uf.h"
#include "esif_uf_appmgr.h"
#include <termios.h>
#include <errno.h>

/* ESIF_UF Startup Script Defaults */
#define ESIF_STARTUP_SCRIPT_DAEMON_MODE		"appstart dptf"
#define ESIF_STARTUP_SCRIPT_SERVER_MODE		NULL

/* Friend */
extern EsifAppMgr g_appMgr;

#ifdef ESIF_ATTR_OS_LINUX_HAVE_READLINE
	#include <readline/readline.h>
	#include <readline/history.h>
#endif

/* Instance Lock */
struct instancelock {
	char *lockfile; /* lock filename */
	int  lockfd;    /* lock file descriptor */
};
static struct instancelock g_instance = {"/var/run/esif_ufd.pid"};

#define HOME_DIRECTORY	NULL /* use OS-specific default */

/* 
** Not ALL OS Entries Suport All Of These
** Not Declared In Header File Intentionallly
*/
extern int g_dst;
extern int g_autocpc;
extern int g_binary_buf_size;
extern int g_errorlevel;
extern int g_quit;
extern int g_quit2;
extern int g_repeat;
extern int g_repeat_delay;
extern int g_soe;
extern int g_cmdshell_enabled;

/* Worker Thread in esif_uf */
esif_thread_t g_thread;
void*esif_event_worker_thread (void *ptr);

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

#if defined(ESIF_ATTR_OS_ANDROID)
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

	if (g_instance.lockfd) {
		return ESIF_TRUE;
	}
	if ((g_instance.lockfd = open(g_instance.lockfile, O_CREAT | O_RDWR, 0644)) == 0 || flock(g_instance.lockfd, LOCK_EX | LOCK_NB) != 0) {
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
	if (g_instance.lockfd) {
		close(g_instance.lockfd);
		g_instance.lockfd = 0;
		unlink(g_instance.lockfile);
	}
}

int instance_islocked()
{
	return (g_instance.lockfd == 0 ? 0 : 1);
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

static const char *cmd_in  = "/tmp/esifd.cmd";
static const char *cmd_out = "/tmp/esifd.log"; 

static int run_as_daemon(int start_with_pipe, int start_with_log)
{
	FILE *fp = NULL;
	char *ptr = NULL;
	char line[MAX_LINE + 1] = {0};
	char line2[MAX_LINE + 1] = {0};

	/* Check to see if another instance is running */
	if (!instance_lock()) {
		return ESIF_FALSE;
	}
	instance_unlock();

	/* 1. Call Fork */ 
	pid_t process_id = fork();

	if (-1 == process_id) {
		return ESIF_FALSE;
	} else if (process_id != 0) {
		/* 2. Exit From Parent */
		CMD_DEBUG("\nESIF Daemon/Loader (c) 2013-2014 Intel Corp. Ver x1.0.0.1\n");
		CMD_DEBUG("Spawn Daemon ESIF Child Process: %d\n", process_id);
		if (ESIF_TRUE == start_with_pipe) {
			unlink(cmd_in);
			CMD_DEBUG("Command Input:  %s\n", cmd_in);
		}

		if (ESIF_TRUE == start_with_log) {
			unlink(cmd_out);
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

	/* Child will receive input from FIFO PIPE? */
	if (ESIF_TRUE == start_with_pipe) {
		if (-1 == mkfifo(cmd_in, S_IROTH)) {
		}
	}

	/* 4. Change to known directory.  Performed in main */
	/* 5. Close all file descriptors incuding stdin, stdout, stderr */
	close(STDIN_FILENO); /* stdin */
	close(STDOUT_FILENO); /* stdout */
	close(STDERR_FILENO); /* stderr */

	/* 6. Open file descriptors 0, 1, and 2 and redirect */
	/* stdin */
	if (ESIF_TRUE == start_with_log) {
		open (cmd_out, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	} else {
		open ("/dev/null", O_RDWR);
	}

	/* stdout */
	dup(0);
	setvbuf(stdout, NULL, _IONBF, 0);
	/* stderr */
	dup(0);
	setvbuf(stderr, NULL, _IONBF, 0);

	/*
	** Start The Daemon
	*/
	g_cmdshell_enabled = 0;
	esif_uf_init(HOME_DIRECTORY);
	esif_ccb_thread_create(&g_thread, esif_event_worker_thread, "Daemon");
	cmd_app_subsystem(SUBSYSTEM_ESIF);

	/* Process Input ? */
	if (ESIF_TRUE == start_with_pipe) {
		while (!g_quit) {
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

			/* Parse and execute the command */
			if (1 == g_repeat || !strncmp(line, "repeat", 6)) {
					parse_cmd(line, ESIF_FALSE);
			} else {
					for (count = 0; count < g_repeat; count++) {
							strcpy(line2, line);
							parse_cmd(line2, ESIF_FALSE);   /* parse destroys command line */
							
							if (g_soe && g_errorlevel != 0) {
									break;
							}
							if (g_repeat_delay && count + 1 < g_repeat) {
									esif_ccb_sleep(g_repeat_delay / 1000);
							}
					}
					g_repeat = 1;
			}
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

	esif_uf_init(HOME_DIRECTORY);

	esif_ccb_thread_create(&g_thread, esif_event_worker_thread, "Server");
	sleep(1); 
	cmd_app_subsystem(SUBSYSTEM_ESIF);
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

		// Get User Input
		g_appMgr.GetPrompt(&g_appMgr, &data_prompt);
		prompt = (esif_string)data_prompt.buf_ptr;

#ifdef ESIF_ATTR_OS_LINUX_HAVE_READLINE
		// Use Readline With History
		sprintf(full_prompt, "%s ", prompt);
		CMD_LOGFILE("%s ", prompt);
		ptr = readline(full_prompt);
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

		if (1 == g_repeat || !strncmp(line, "repeat", 6)) {
					parse_cmd(line, ESIF_FALSE);
			} else {
					for (count = 0; count < g_repeat; count++) {
							strcpy(line2, line);
							parse_cmd(line2, ESIF_FALSE);   /* parse destroys command line */
							if (kbhit()) {
									break;
							}
							if (g_soe && g_errorlevel != 0) {
									break;
							}
							if (g_repeat_delay && count + 1 < g_repeat) {
									esif_ccb_sleep(g_repeat_delay / 1000);
							}
					}
					g_repeat = 1;
			}
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
	int rc = chdir("..");

	optind = 1;	// Rest To 1 Restart Vector Scan

	while ((c = getopt(argc, argv, "d:f:c:b:nxhq?spl")) != -1) {
		switch (c) {
		case 'd':
			g_dst = (u8)esif_atoi(optarg);
			break;

		case 'n':
			g_autocpc = ESIF_FALSE;
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
			"(c) 2013-2014 Intel Corp\n\n"
			"-d [*id]            Set Destination\n"
			"-f [*filename]      Load Filename\n"
			"-n                  No Auto CPC Assignment\n"
			"-x                  XML Output Data Format\n"
			"-c [*command]       Issue Shell Command\n"
			"-q                  Quit After Command\n"
			"-b [*size]          Set Binary Buffer Size\n"
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

	/* NICE Wait For Worker Thread To Exit */
	CMD_DEBUG("Waiting For EVENT Thread To Exit...\n");
	while (!g_quit2) {
		esif_ccb_sleep(1);
	}
	CMD_DEBUG("Errorlevel Returned: %d\n", g_errorlevel);

	/* Exit ESIF */
	esif_uf_exit();

	/* Release Instance Lock and exit*/
	instance_unlock();
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
