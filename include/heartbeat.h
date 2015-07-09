/*
 ============================================================================
 Name        : heartbeat.h
 Author      : AK
 Version     : V1.00
 Copyright   : Property of Londelec UK Ltd
 Description : Header file for iMX287 heartbeat agent

  Change log  :

  *********V1.00 04/03/2015**************
  Initial revision

 ============================================================================
 */

#ifndef HEARTBEAT_H_
#define HEARTBEAT_H_


#include "ledefs.h"


// General constants
//#define	ABSOLUTE_PATH_LENGTH			500				// Absolute path including filename length
#define OUTPUTSTR_LENGTH				512				// Output string length


// Timing Constants
#define MULT10USEC						100000			// Multiplier to convert from sec to 10usec
//#define MAINSLEEP						100000			// Main loop sleep in nanoseconds, default 0.1ms


// Command line argument flags
#define	ARGF_QUIET						0x01			// Suppress all output to console
//#define	ARGF_DEBUG						0x02



// Command line argument specification
typedef enum {
	arghelp						= 1,
	argVersion,
	argModules,
} LEOPACK ArgEnum;



// Table structures
typedef struct ArgumentTableStr_ {
	lechar					*string;
	ArgEnum					entry;
} ArgumentTableStr;


// Always define global variables in C source file
//extern lechar 			*AbsoultePathPtr;


//extern lechar 			Output_string[OUTPUTSTR_LENGTH];
//extern lechar			*StdErrorPtr;


#endif /* HEARTBEAT_H_ */
