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

/* Friend */
extern EsifAppMgr g_appMgr;

#ifdef ESIF_ATTR_OS_LINUX_HAVE_READLINE
	#include <readline/readline.h>
	#include <readline/history.h>
#endif

// Not ALL OS Entries Suport All Of These
// Not Declared In Header File Intentionallly
extern FILE *g_debuglog;
extern int g_dst;
extern int g_autocpc;
extern int g_autocpc;
extern int g_binary_buf_size;
extern int g_errorlevel;
extern int g_quit;
extern int g_quit2;
extern int g_repeat;
extern int g_repeat_delay;
extern int g_soe;
esif_thread_t g_thread;

// Worker Thread in esif_uf
void*esif_event_worker_thread (void *ptr);

// Emulate Windows Function
static int kbhit (void)
{
	struct termios save_t = {0};
	struct termios new_t  = {0};
	int key     = 0;
	int save_fc = 0;

	// Get Terminal Attribute
	tcgetattr(STDIN_FILENO, &save_t);
	new_t = save_t;
	new_t.c_lflag &= ~(ICANON | ECHO);

	// Set Terminal Attribute
	tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
	save_fc = fcntl(STDIN_FILENO, F_GETFL, 0);

	// Add NONBLOCK To Existing File Control
	fcntl(STDIN_FILENO, F_SETFL, save_fc | O_NONBLOCK);

	// If No Character No Problem
	key = getchar();

	// Remove NONBLOCK Attribute
	tcsetattr(STDIN_FILENO, TCSANOW, &save_t);
	fcntl(STDIN_FILENO, F_SETFL, save_fc);

	if (key != EOF) {
		return 1;
	}
	return 0;
}


#ifdef ESIF_ATTR_DAEMON
int main (
	int arg,
	char * *argv
	)
{
	FILE *fp = stdin;
	char *ptr;
	char line[MAX_LINE + 1];

	char *prompt = NULL;
	char prompt_buf[PROMPT_LEN];
	ESIF_DATA(data_prompt, ESIF_DATA_STRING, prompt_buf, PROMPT_LEN);

	// Init ESIF
	esif_uf_init("./");

	// PARENT PROCESS. Need to kill it.
	pid_t process_id = fork();
	if (process_id > 0) {
		printf("\nESIF Daemon/Loader (c) Intel Corp. Ver x1.0.0.1\n");
		printf("Spawn Daemon ESIF Child Process %d \n", process_id);
		printf("Manage With /tmp/esifd.cmd e.g. cat > /tmp/esifd.cmd\n");
		exit(0);
	}

	esif_ccb_thread_create(&g_thread, esif_event_worker_thread, NULL);
	esif_ccb_sleep(1);

	// Child
	int r = mkfifo("/tmp/esifd.cmd", S_IROTH);
	printf("\nWaiting For ESIF Command On INODE /tmp/esif.cmd\n");
	esif_ccb_sleep(1);
	cmd_app_subsystem(SYSTEM_ESIF);	/* ESIF */

	while (!g_quit) {
		fp = fopen("/tmp/esifd.cmd", "r");
		if (NULL == fp) {
			break;
		}

		if (fgets(line, MAX_LINE, fp) == NULL) {
			// Do nothing
		}
		ptr = line;
		while (*ptr != '\0') {
			if (*ptr == '\r' || *ptr == '\n' || *ptr == '#') {
				*ptr = '\0';
			}
			ptr++;
		}
		EsifAppMgrGetPrompt(&data_prompt);
		prompt = (esif_string)data_prompt.buf_ptr;

		printf("%s ", prompt);
		parse_cmd(line, ESIF_FALSE);
	}

	if (fp) {
		fclose(fp);
	}
	while (!g_quit2)
		sleep(1);
	printf("ESIF Daemon Exiting...\n");

	// Exit ESIF
	esif_uf_exit();
	exit(errorlevel);
}


#else
int main (
	int argc,
	char * *argv
	)
{
	FILE *fp = stdin;
	char *ptr;
	char line[MAX_LINE + 1];
	char line2[MAX_LINE + 1];
	char full_prompt[MAX_LINE + 1];
	char command[MAX_LINE + 1] = {0};
	int quit_after_command     = 0;
	int c = 0;
	g_debuglog = stdin;
		#define PROMPT_LEN 64

	char *prompt = NULL;
	char prompt_buf[PROMPT_LEN];
	int rc;
	ESIF_DATA(data_prompt, ESIF_DATA_STRING, prompt_buf, PROMPT_LEN);

	// Init ESIF
	rc = chdir("..");
	esif_uf_init("./");

	optind = 1;	// Rest To 1 Restart Vector Scan

	while ((c = getopt(argc, argv, "d:f:c:b:nxhq?")) != -1) {
		switch (c) {
		case 'd':
			g_dst = (u8)esif_atoi(optarg);
			break;

		case 'n':
			g_autocpc = 0;
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
			quit_after_command = 1;
			break;

		case 'h':
		case '?':
			printf(
				"EsiF Eco-System Independent Framework Shell\n"
				"(c) 2013 Intel Corp\n\n"
				"-d [*id]            Set Destination\n"
				"-f [*filename]      Load Filename\n"
				"-n                  No Auto CPC Assignment\n"
				"-x                  XML Output Data Format\n"
				"-c [*command]       Issue Shell Command\n"
				"-q                  Quit After Command\n"
				"-b [*size]          Set Binary Buffer Size\n"
				"-h or -?            This Help\n\n");
			exit(0);
			break;

		default:
			break;
		}
	}
	esif_ccb_thread_create(&g_thread, esif_event_worker_thread, NULL);
	esif_ccb_sleep(1);

	if (NULL == fp) {
		fp = stdin;
	}
	cmd_app_subsystem(SUBSYSTEM_ESIF);	// ESIF

	while (!g_quit) {
		int count = 0;
		// Startup Command?
		if (command) {
			parse_cmd(command, ESIF_FALSE);
			if (quit_after_command) {
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
		ptr = readline(full_prompt);
		// Add To History NO NUL's
		if (ptr[0] != 0) {
			add_history(ptr);
		}
		strcpy(line, ptr);
		free(ptr);
#else
		// No History So Sorry
		printf("%s ", prompt);
		if (fgets(line, MAX_LINE, fp) == NULL) {
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

		if (1 == g_repeat || !strncmp(line, "repeat", 6)) {
			parse_cmd(line, ESIF_FALSE);
		} else {
			for (count = 0; count < g_repeat; count++) {
				strcpy(line2, line);
				parse_cmd(line2, ESIF_FALSE);	/* parse destroys command line */
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
	if (fp && fp != stdin) {
		fclose(fp);
	}

	/* NICE Wait For Worker Thread To Exit */
	printf("Waiting For EVENT Thread To Exit...\n");
	while (!g_quit2)
		esif_ccb_sleep(1);

	printf("Errorlevel Returned: %d\n", g_errorlevel);

	// Exit ESIF
	esif_uf_exit();
	exit(g_errorlevel);
}


#endif

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
