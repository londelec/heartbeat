/*
 ============================================================================
 Name        : heartbeat.c
 Author      : AK
 Version     : V1.00
 Copyright   : Property of Londelec UK Ltd
 Description : iMX287 heartbeat agent

  Change log  :

  *********V1.00 04/03/2015**************
  Initial revision

 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// strcat, strcpy
//#include <math.h>			// pow
//#include <types.h>
//#include <socket.h>
//#include <sys/ioctl.h>
#include <errno.h>			// Error number

#include <fcntl.h>			// File controls
#include <unistd.h>			// getcwd, access
//#include <sys/types.h>
#include <sys/stat.h>		// Open function constants
#include <time.h>			// clock_nanosleep


#include "heartbeat.h"
#include "../../libleiodc/include/libleiodchw.h"




//const lechar *MainVersion = " heartbeatVersion=1.00 ";
const lechar *FirmwareVersion = " FirmwareVersion=1.00 ";
#include "builddate.txt"




#ifdef GLOBAL_DEBUG
#endif	// GLOBAL_DEBUG


#define	LIBLEIODC_MIN_VER	100					// Minimal required libleiodc version

#define	LED_ON_NSEC			50000000			// LED on period in nanoseconds (0.05sec)
#define	LED_OFF_SEC			1					// LED off period in seconds (1sec)


// Global variables
uint8_t					fwarguments = ARGF_QUIET; // Initialize with quite argument


// Dynamic variables
lechar 					Output_string[OUTPUTSTR_LENGTH];


const ArgumentTableStr ArgTable[] =
{
	{"-h",					arghelp},
	{"-help",				arghelp},
	{"--help",				arghelp},
	{"?",					arghelp},
	{"-v",					argVersion},
	{"--version",			argVersion},
	{"-m",					argModules},
	{"--modules",			argModules},
};


const lechar *consHelpConst = "iMX287 heartbeat agent\n\n"
		"Run without arguments for continuous heartbeat generation\n"
		"Available arguments:\n\n"
		//"  -d, --debug\t\trun in debug mode\n"
		//"  -m, \t\t\tdisplay module version information and exit\n"
		"  -v, \t\t\tdisplay version information and exit\n"
		"   ?  --help\t\tdisplay help and exit\n";


// Macros
// For expanding system logger

// Critical error messages in both modes
#define SYSLOG_CONSOLE printf("%s\n", Output_string);
#define SYSLOGC_CRITICALERR(mstrconst) printf("%s\n", mstrconst);




int main(int argc, char *argv[]) {
	uint8_t					cnt, argcnt;


/***************************************************************************
* Read and process command line arguments
* [04/03/2015]
***************************************************************************/
	for (argcnt = 1; argcnt < argc; argcnt++) {
		for (cnt = 0; cnt < (ARRAY_SIZE(ArgTable)); cnt++) {
			if (strcasecmp(ArgTable[cnt].string, argv[argcnt]) == 0) {
				switch (ArgTable[cnt].entry) {

				/*case argModules:
					strcpy(Output_string, MainVersion);
					strcat(Output_string, "\n");
					printf("%s", Output_string);
					//SYSLOG_USERARGS
					exit(EXIT_SUCCESS);
					break;*/

				case argVersion:
					strcpy(Output_string, FirmwareVersion);
					strcat(Output_string, "\n");
					strcat(Output_string, FirmwareDate);
					SYSLOG_CONSOLE
					exit(EXIT_SUCCESS);
					break;


				case arghelp:
					//SYSLOGC_CRITICALERR(consHelpConst)
					printf(consHelpConst);
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
* Initialize gpio files and states
* [11/03/2015]
***************************************************************************/
	if (leiodc_libverchk(LIBLEIODC_MIN_VER) == EXIT_FAILURE) {
		SYSLOGC_CRITICALERR(LibErrorString);
		exit(EXIT_FAILURE);
	}


	if (leiodc_pininit(NULL, lepin_count) == EXIT_FAILURE) {
		SYSLOGC_CRITICALERR(LibErrorString);
		exit(EXIT_FAILURE);
	}


	if (leiodc_pinoutstate(lepin_heartbeat, 1) == EXIT_FAILURE) {
		SYSLOGC_CRITICALERR(LibErrorString);
		return EXIT_FAILURE;
	}


/***************************************************************************
* Main loop
* [11/03/2015]
***************************************************************************/
	while(1) {
		nanotimedef		 		definedtimeout;


		if (leiodc_pinstate(lepin_heartbeat, 0) == EXIT_FAILURE) {
			SYSLOGC_CRITICALERR(LibErrorString);
			exit(EXIT_FAILURE);
		}


		definedtimeout.tv_sec = 0;
		definedtimeout.tv_nsec = LED_ON_NSEC;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &definedtimeout, NULL);


		if (leiodc_pinstate(lepin_heartbeat, 1) == EXIT_FAILURE) {
			SYSLOGC_CRITICALERR(LibErrorString);
			exit(EXIT_FAILURE);
		}


		definedtimeout.tv_sec = LED_OFF_SEC;
		definedtimeout.tv_nsec = 0;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &definedtimeout, NULL);

	}	// Close main loop
	exit(EXIT_SUCCESS);
}


