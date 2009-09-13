/*
 * common.c
 *
 *  Created on: Sep 9, 2009
 *      Author:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void
fatal(char *s)
{
	perror(s);
	exit(1);
}
