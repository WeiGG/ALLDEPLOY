/* client.c */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "daemon.c"

//#define MAXLINE   10
#define SERVS_PORT  9526
#define SERVS_IP    "127.0.0.1"

int main(int argc, char *argv[])
{

    //StartDaemon();    

    struct sockaddr_in servaddr;
    char buf[1];
    int sockfd; //, i
    char ch = 'a';

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVS_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVS_PORT);
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    while (1)
    {
        buf[0] = ch++;
        write(sockfd, buf, sizeof(char));
        printf("%c\n",buf[0]);
        sleep(1);
    }

    close(sockfd);

    return 0;
}

