#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <strings.h>
#include <signal.h>
#define  EOL   		1     	 /* end of line */
#define  ARG 		2    	/* normal arguments  */
#define  AMPERSAND  	3	
#define SEMICOLON 	4
#define PONDKEY   5   /*for pondkey*/
#define PIPE      6   /*for pipe*/
#define MAXARG		512	/* max. no. command args */
#define MAXBUF		512	/* max length input line */

#define FOREGROUND	0
#define BACKGROUND	1

