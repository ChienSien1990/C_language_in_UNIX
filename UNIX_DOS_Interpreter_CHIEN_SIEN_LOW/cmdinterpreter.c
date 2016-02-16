/************************************************************************
NAME: CHIEN SIEN LOW
ID: 01436482
CIS 570
STEPS: MUST INCLUDE cmdinterpreter.c AND smallsh.h IN THE SAME DIRECTORY.
	LOCATE TO THE DIRECTORY AND START COMPILE AND RUN.
COMPILE: gcc cmdinterpreter.c
RUN: ./a.out

**************************************************************************/
#include "smallsh.h"

static char inpbuf[MAXBUF], tokbuf[2*MAXBUF], *ptr = inpbuf, *tok = tokbuf;

int userin (char *p)
{ 
  	int c, count;

	ptr = inpbuf;
	tok = tokbuf;

	printf("%s" ,  p);
	count = 0;
	while(1)
   	{
	     if ((c = getchar()) == EOF)
		   return(EOF);

	     if (count < MAXBUF)
		  inpbuf[count++]  =   c;

	     if (c == '\n' && count < MAXBUF)
		{
		    inpbuf[count] = '\0';
		    return count;
		}   

             if(c=='\n') 
	 	{
 	   	    printf("smallsh: input line too long \n");
	            count = 0;
  	            printf("%s",p);
 	  	}
        }
}
//shorter version of detecting the error
int fatal(char *s)

  {
    perror(s);
    exit(1);
  }
static char special [] = {' ', '\t', '&', ';', '\n', '\0'};
static int inarg(char c)
{ 
 char *wrk;
 for (wrk = special; *wrk; wrk++)

 {
    if (c == *wrk)
        return (0);
  }
return (1);
}

//getting the token
int gettok(char **outptr)
{
  	int type;

	*outptr = tok;
	while ( *ptr == ' ' || *ptr == '\t')
	      ptr++;

	*tok++ = *ptr;
	switch (*ptr++)
	{
	case '|':
		type=PIPE;
		break;
	case '\n':
	       type = EOL;
	       break;

	case '&':
	     type = AMPERSAND;
	    break;

	case ';':
	     type = SEMICOLON;
	     break;

	default:
	     type = ARG;
	   while (inarg(*ptr))
		  *tok++ = *ptr++;
	}
	*tok++ = '\0';
	return type;
}

static int runcommand(char **cline, int where)
{
	  int p[2];
	  pid_t pid;
	  int status;

	  switch (pid = fork())
	   {
	    case -1:
		     perror("smallsh");
		     return (-1);
	    case 0:
		// commands for translate the dos command to unix command
		// REF -- concept from Stackoverflow.com 
		if(strcmp(cline[0],"del")==0)
			cline[0]="rm";
		else if(strcmp(cline[0],"copy")==0)
			cline[0]="cp";
		else if(strcmp(cline[0],"move")==0)
			cline[0]="mv";
		else if(strcmp(cline[0],"rename")==0)
			cline[0]="mv";
		else if(strcmp(cline[0],"type")==0)
			cline[0]="cat";
		else if(strcmp(cline[0],"md")==0)
			cline[0]="mkdir";
		else if(strcmp(cline[0],"rd")==0)
			cline[0]="rmdir";
		else if(strcmp(cline[0],"cls")==0)
			cline[0]="clear";
		execvp (*cline,cline);
		perror (*cline);
		    if(where == BACKGROUND)
			{
			 signal(SIGINT, SIG_IGN);
			 signal(SIGQUIT, SIG_IGN);
			}
		    else
			{
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT,SIG_DFL);
			}
			execvp(*cline,cline);
			perror(*cline);
			exit(1);
		    default:
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_DFL);

		      }
	    // code for parent
	    // if background process print pid and exit
	    if(where == BACKGROUND){
		printf("[Process id %d]\n",pid);
		return (0);
	    }
	  
	  
		
	/* wait  until process pid exits */
	  if (waitpid(pid, &status, 0)  == -1)
	      return (-1);
	  else 
	      return (status);
}

char *arg1[10][MAXARG+1];

int procline(void)
{  //process input line

int p[8][2];
   char *arg[MAXARG + 1];  //pointer array for runcommand
    int toktype;        // type of token in command
    int narg;           // number of arguments so far
    int type;   //type =  FOREGROUND or BACKGROUND
    int npipe=0;
    int type1;
    int m,n;
    narg = 0;

    for (m=0; m<10; m++)
      {
        for(n=0;n<MAXARG+1; n++)
          arg1[m][n]=0;
      }
   type1=0;

    for(;;)
   {           // loop forever
        // take action according to token type
        switch(toktype = gettok(&arg[narg]))

    {

       case PIPE:  //get pipe commands

       for (m=0;m<narg;m++)
           arg1[npipe][m]=arg[m]; //get pipe string
           arg1[npipe][narg]=NULL;
           npipe++;
           narg=0;
           type1=toktype;
           break;

        case ARG:
            if(narg<MAXARG)
                narg++;
            break;

        case EOL:
        case SEMICOLON:
        case PONDKEY:
        case AMPERSAND:
            if(toktype == AMPERSAND)
                type = BACKGROUND;
            else
                type = FOREGROUND;

            if(narg != 0)
              {

                int j=0;
                int i=1,i_p=1;
                int fd1,fd2;
                pid_t pid;
                char **temp;
                arg[narg] = NULL;

                /* find if there is a "<" or ">" in the tokens */
                  temp=arg;

                while((i<=narg)&&((strcmp(temp[i-1],"<")*
                             strcmp(temp[i-1],">"))!=0)) i++;

                /*solve change working directory case*/
                if (strcmp(arg[0],"cd") == 0 )
                {  if (narg != 1)
                        {if (chdir(arg[1])==-1)
                       printf("No directory: %s to change\n", arg[1]);}
                   else
                     chdir(getenv("HOME"));
                }

                /* solve redirection case*/
                else if(i<narg) redirect(temp,i,type);

                /*pipe case*/
                else if(type1 == PIPE)
                  {
                    for(m=0; m<narg; m++)
                     arg1[npipe][m] = arg[m];
                    pipes(npipe,type);
                  }

                /* regular commands*/
                 else
                  {
			runcommand(arg,type);
	        }
            }

            if((toktype == EOL)||(toktype ==PONDKEY))
                return;
            narg = 0;
            break;
        }
   }
}

int redirect(char *temp[],int i, int type)
  {
        char *temp1[MAXARG],*temp2[MAXARG];
        int j;
        int fd1,fd2;
        pid_t pid;
                 if(strcmp(temp[i-1],"<")==0)
                     {
                        for(j=0;j<=i-2;j++)
                          temp1[j]=temp[j]; //arg[j];
                        temp1[j+1]=NULL;
                        temp2[0]=temp[i];
                        temp2[1]=NULL;
                        fd1=open(temp2[0],O_RDONLY);
                        if((pid=fork())==0)
                        {
                            close(0);
                            fd2=fcntl(fd1,F_DUPFD,0);

                        execvp(*temp1, temp1);
                        perror(*temp1);
                        exit(1);}
                     }

                /* output redirection*/
                    else
                     {
                        for (j=0;j<=i-2;j++)
                          temp1[j]=temp[j];//arg[j];
                        temp1[j+1]=NULL;
                        temp2[0]=temp[i];
                        temp2[1]=NULL;
                        fd1=open(temp2[0],O_WRONLY|O_CREAT|O_TRUNC,0777);
                        if((pid=fork())==0)
                        {

                            close(1);
                            fd2=fcntl(fd1,F_DUPFD,1);
                            close(fd1);
			    execvp(*temp1, temp1);
		            perror(*temp1);
			    exit(1);
                        }
                     }

     }
//REF - ANGELFIRE.com
int pipes(int npipe,int where)
{
    pid_t pid[npipe+1];
    int i=0,j;
    int p[npipe][2], status;

 // create npipe pipes
    for(i=0;i<npipe;i++)
    {
        if(pipe(p[i])==-1)
           fatal("pipe call in join");
    }

 // fork npipe+1 children, this is the number of command line commands
    for(i=0;i<=npipe;i++)
    {
        switch(pid[i]=fork()){
        case -1:
              fatal("fork children failed");
        case 0:
              if(where==BACKGROUND)
              {// back ground process will ignore signal SIGINT, SIGQUIT
                 signal(SIGINT,SIG_IGN);
                 signal(SIGQUIT,SIG_IGN);
              }
              else
              {
                 signal(SIGINT,SIG_DFL);
                 signal(SIGQUIT,SIG_DFL);
              }

              if(i==0)
              {
  // for first command, make its standard outpout go to first pipe
                dup2(p[i][1],1);
                for(j=0;j<npipe;j++)
                {// save file descripters
                    close(p[j][1]);
                    close(p[j][0]);
                }
                execvp(arg1[i][0], arg1[i]);
                fatal(arg1[i][0]);
             }

  // for the last command, make its standard input from the last pipe
             else if (i==npipe)
             {
                dup2(p[i-1][0],0);
                for(j=0;j<npipe;j++)
                {// save file descriptor
                    close(p[j][0]);
                    close(p[j][1]);
                }
                execvp(arg1[npipe][0],arg1[npipe]);
                fatal(arg1[npipe][0]);
            }

  // for other commands, make its std. input from apropriate pipe
  // std. output go to the next pipe
            else
            {
               dup2(p[i-1][0],0);
               dup2(p[i][1],1);
               for(j=0;j<npipe;j++)
               {
                   close(p[j][0]);
                   close(p[j][1]);
               }
               execvp(arg1[i][0],arg1[i]);
               fatal(arg1[i][0]);
            }

         } // end switch

     }//  end for

    // parent code here
    for(j=0;j<npipe;j++)
    {// save the file descriptors
       close(p[j][0]);
       close(p[j][1]);
    }

 // if background, print the children id and return
    if (where==BACKGROUND)
    {
  // wait for parent having forked all the children
       for(j=0;j<=npipe;j++)
       {
            if (pid[j]>0)
                printf("[Process id %d]\n",pid[j]);
            else
                 sleep(1);
       }
       return(0);
   }

 //parent wait here
   while(waitpid(pid[npipe],&status,WNOHANG)==0)
         sleep(1);

       return(0);

}  // end of join
int main()
{	
	char buff[PATH_MAX + 1];
	getcwd( buff, PATH_MAX + 1 );
	strcat(buff,"->");
	while (userin(buff) != EOF)
	procline();
}
// Reference: Lecture Slides: Unix and Pipes, get ampersands concepts from angelfire.com, interpreter commands from Stackoverflow.com 
