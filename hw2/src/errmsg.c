/*********************/
/* errmsg.c          */
/* for Par 3.20      */
/* Copyright 1993 by */
/* Adam M. Costello  */
/*********************/

/* This is ANSI C code. */


#include "errmsg.h"  /* Makes sure we're consistent with the declarations. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char * errmsg;

char * outofmem = "Out of memory.\n";

/**
 * @brief  Set an error indication, with a specified error message.
 * @param msg Pointer to the error message.  The string passed by the caller
 * will be copied.
 */
void set_error(char *msg) {
	errmsg = strdup(msg);
}

/**
 * @brief  Test whether there is currently an error indication.
 * @return 1 if an error indication currently exists, 0 otherwise.
 */
int is_error() {
	if (errmsg)
		return 1;
	return 0;
}

/**
 * @brief  Issue any existing error message to the specified output stream.
 * @param file  Stream to which the error message is to be issued.  
 * @return 0 if either there was no existing error message, or else there
 * was an existing error message and it was successfully output.
 * Return non-zero if the attempt to output an existing error message
 * failed.
 */
int report_error(FILE *file) {
	if (!errmsg)
		return 0;
	fprintf(file, "%s", errmsg);
	return 1;
}

/**
 * Clear any existing error indication and free storage occupied by
 * any existing error message.
 */
void clear_error() {
	if (errmsg)
		free(errmsg);
}
