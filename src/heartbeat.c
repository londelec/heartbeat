/*
 ============================================================================
 Name        : heartbeat.c
 Author      : AK
 Version     : V1.01
 Copyright   : Property of Londelec UK Ltd
 Description : iMX287 heartbeat agent

  Change log :

  *********V1.01 30/03/2018**************
  Term signal handler added

  *********V1.00 04/03/2015**************
  Initial revision

 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// strcat, strcpy
#include <stdarg.h>
//#include <types.h>
//#include <socket.h>
//#include <sys/ioctl.h>
#include <errno.h>			// Error number

#include <fcntl.h>			// File controls
#include <unistd.h>			// getcwd, access
//#include <sys/types.h>
#include <sys/stat.h>		// Open function constants
#include <signal.h>			// Serial port signaling
#include <time.h>			// clock_nanosleep


#include "ledefs.h"
#include "../../libleiodc/include/libleiodchw.h"


static const lechar FirmwareVersion[] = " HeartbeatVersion=1.01 ";
#include "hbdate.txt"


#ifdef GLOBAL_DEBUG
#define DEBUG_FUNCTIONS
//#define DEBUG_DRYRUN
#endif	// GLOBAL_DEBUG


// General constants
#define LOGLEN_FULL			1024				// Full logfile entry string length
#define LOGLEN_ARGBUF		512					// Argument buffer string length


#define	LIBLEIODC_MIN_VER	100					// Minimal required libleiodc version

#define	LED_ON_NSEC			50000000			// LED on period in nanoseconds (0.05sec)
#define	LED_OFF_SEC			1					// LED off period in seconds (1sec)


// Global variables
static lesigaction sa_term;					// Signal action for Termination (kill) and Ctrl + C


// Command line argument specification
typedef enum {
	arghelp						= 1,
	argVersion,
} LEOPACK ArgEnum;


static const struct ArgumentTable_s {
	const lechar			*string;
	ArgEnum					entry;
} ArgTable[] =
{
	{"-h",					arghelp},
	{"-help",				arghelp},
	{"--help",				arghelp},
	{"?",					arghelp},
	{"-v",					argVersion},
	{"--version",			argVersion},
};


static const lechar consHelpConst[] = "iMX287 heartbeat agent\n\n"
		"Run without arguments for continuous heartbeat\n"
		"Available arguments:\n\n"
		//"  -d, --debug\t\trun in debug mode\n"
		"  -v, \t\t\tdisplay version information and exit\n"
		"   ?  --help\t\tdisplay help and exit\n";


// Macros
// For expanding system logger
#define SYSLOG_CONSOLE(...) syslogger(1, __func__, __FILE__, __LINE__, __VA_ARGS__);




/***************************************************************************
* Formated logger for system debug information
* [30/03/2018]
***************************************************************************/
static void syslogger(int llevel, const lechar *cfunc, const lechar *cfile, int lineno, const lechar *format, ...) {
	lechar 			argbuf[LOGLEN_ARGBUF];
	lechar 			outstring[LOGLEN_FULL];
	int 			retstat;
	va_list 		ap;


	outstring[0] = 0;
#ifdef DEBUG_FUNCTIONS
	if (cfunc) {
		strcat(outstring, cfunc);
		strcat(outstring, "() ");
	}
#endif

	va_start(ap, format);
	retstat = vsnprintf(argbuf, sizeof(argbuf) - 1, format, ap); // vnsprintf returns what it would have written, even if truncated
	va_end(ap);

	if (retstat > sizeof(argbuf) - 1)
		retstat = sizeof(argbuf) - 1;

	if (retstat > 0) {
		argbuf[retstat] = 0;
		strcat(outstring, argbuf);
	}
	printf("%s\n", outstring);
}


/***************************************************************************
* Termination signal handler
* [30/03/2018]
***************************************************************************/
static void term_signal(int signalnum, siginfo_t *info, void *ptr) {

	switch (signalnum) {
	case SIGINT:
		SYSLOG_CONSOLE("Agent received 'Ctrl + C' (signal %u)", signalnum);
		break;

	case SIGTERM:
		SYSLOG_CONSOLE("Agent received TERM (signal %u)", signalnum);
		break;

	default:
		SYSLOG_CONSOLE("Agent received unknown kill (signal %u)", signalnum);
		break;
	}


#ifdef DEBUG_DRYRUN
	SYSLOG_CONSOLE("Turning HB LED off");
#else
	if (leiodc_pinstate(lepin_heartbeat, 1) == EXIT_FAILURE) {
		SYSLOG_CONSOLE(LibErrorString);
	}
#endif


	SYSLOG_CONSOLE("-------> Heartbeat agent terminated (by pid %u) <-------", info->si_pid);
	exit(EXIT_SUCCESS);
}


/***************************************************************************
* Read and process command line arguments
* [04/03/2015]
***************************************************************************/
int main(int argc, char *argv[]) {
	uint8_t		cnt, argcnt;


	for (argcnt = 1; argcnt < argc; argcnt++) {
		for (cnt = 0; cnt < (ARRAY_SIZE(ArgTable)); cnt++) {
			if (strcasecmp(ArgTable[cnt].string, argv[argcnt]) == 0) {
				switch (ArgTable[cnt].entry) {
				case argVersion:
					SYSLOG_CONSOLE(FirmwareVersion)
					SYSLOG_CONSOLE(hbDate);
					exit(EXIT_SUCCESS);
					break;


				case arghelp:
					SYSLOG_CONSOLE(consHelpConst);
					exit(EXIT_SUCCESS);
					break;

				default:
					// Don't parse unknown arguments
					break;
				}
				break;
			}
		}
	}


/***************************************************************************
* Install Signal handlers for Termination
* [30/03/2018]
***************************************************************************/
	memset(&sa_term, 0, sizeof(sa_term));
	sa_term.sa_sigaction = term_signal;			// Handler function
	sa_term.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &sa_term, NULL);			// Ctrl + C action handler
	sigaction(SIGTERM, &sa_term, NULL);			// Terminate (kill) action handler


/***************************************************************************
* Initialize gpio files and states
* [11/03/2015]
***************************************************************************/
	if (leiodc_libverchk(LIBLEIODC_MIN_VER) == EXIT_FAILURE) {
		SYSLOG_CONSOLE(LibErrorString);
		exit(EXIT_FAILURE);
	}


	if (leiodc_pininit(NULL, lepin_count) == EXIT_FAILURE) {
		SYSLOG_CONSOLE(LibErrorString);
		exit(EXIT_FAILURE);
	}


	if (leiodc_pinoutstate(lepin_heartbeat, 1) == EXIT_FAILURE) {
		SYSLOG_CONSOLE(LibErrorString);
		return EXIT_FAILURE;
	}


/***************************************************************************
* Main loop
* [11/03/2015]
***************************************************************************/
	while(1) {
		nanotimedef		definedtimeout;


#ifdef DEBUG_DRYRUN
		//SYSLOG_CONSOLE("HB LED on");
#else
		if (leiodc_pinstate(lepin_heartbeat, 0) == EXIT_FAILURE) {
			SYSLOG_CONSOLE(LibErrorString);
			exit(EXIT_FAILURE);
		}
#endif


		definedtimeout.tv_sec = 0;
		definedtimeout.tv_nsec = LED_ON_NSEC;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &definedtimeout, NULL);


#ifdef DEBUG_DRYRUN
		//SYSLOG_CONSOLE("HB LED off");
#else
		if (leiodc_pinstate(lepin_heartbeat, 1) == EXIT_FAILURE) {
			SYSLOG_CONSOLE(LibErrorString);
			exit(EXIT_FAILURE);
		}
#endif


		definedtimeout.tv_sec = LED_OFF_SEC;
		definedtimeout.tv_nsec = 0;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &definedtimeout, NULL);

	}	// Close main loop
	exit(EXIT_SUCCESS);
}
