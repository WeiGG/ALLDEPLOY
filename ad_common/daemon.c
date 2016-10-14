
/*
########################################


########################################
*/


#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#define MAXLINE 10
#define SERV_PORT 8000

void StartDaemon(void)
{
    pid_t Daemon_pid;
    //create child process, father process quit
    Daemon_pid = fork();
    if(Daemon_pid < 0)
    {
        perror("Daemon start error\n");
        exit(-1);
    }
    else if(Daemon_pid != 0)
        exit(0);

    //get pid
    printf("pid is a %d\n",getpid());

    //create session
    setsid();

    //change dir for /
    if(chdir("/") < 0)
    {
        perror("chdir error\n");
        exit(1);
    }

    //again umask
    umask(0);

    //dup2 file descriptor STDIN,STDOUT,STDERR > /dev/null
    close(0);

    open("/dev/null", O_RDWR);
    dup2(0, 1);
    dup2(0, 2);
}

