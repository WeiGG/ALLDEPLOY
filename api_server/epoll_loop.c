/*
########################################


########################################
*/


#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_EVENTS 	1024
#define BUFLEN 		128
#define SERV_PORT	9526

struct myevent_s
{
	//file descriptor
	int fd;

	//events
	int events;

	//Point to oneself
	void *arg;
	
	void (*call_back)(int fd, int events, void *arg);

	//status 1 is Monitor , 0 not
	int status;

	char buf[BUFLEN];
	int len;

	//last_avtive: Record the last response time, do overtime
	long last_active;
};

// epoll_create return handle
int g_efd;

// +1 is a last fd
struct myevent_s g_events[MAX_EVENTS+1];

void recvdata(int fd, int events, void *arg);
void senddata(int fd, int events, void *arg);
void eventdel(int efd, struct myevent_s *ev);
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg);
void eventadd(int efd, int events, struct myevent_s *ev);

//delete red-block tree node
void eventdel(int efd, struct myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};

    if (ev->status != 1)
        return;

    epv.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

    return;
}


//senddata
void senddata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = send(fd, ev->buf, ev->len, 0);
    /*
    printf("fd=%d\tev->buf=%s\ttev->len=%d\n", fd, ev->buf, ev->len);
    printf("send len = %d\n", len);
    */

    if (len > 0)
	{
        printf("send[fd=%d], [%d]%s\n", fd, len, ev->buf);
        eventdel(g_efd, ev);
        eventset(ev, fd, recvdata, ev);
        eventadd(g_efd, EPOLLIN, ev);
    }
    else
	{
        close(ev->fd);
        eventdel(g_efd, ev);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }

    return;
}

//recvdata
void recvdata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = recv(fd, ev->buf, sizeof(ev->buf), 0);
    eventdel(g_efd, ev);

    if (len > 0)
	{
        ev->len = len;
        ev->buf[len] = '\0';
        printf("C[%d]:%s\n", fd, ev->buf);
        //chenage send events
        eventset(ev, fd, senddata, ev);
        eventadd(g_efd, EPOLLOUT, ev);
    }
    else if (len == 0)
	{
        close(ev->fd);
        
		//The address is obtained by subtracting the offset position
		//ev - g_events
        printf("[fd=%d] pos[%d], closed\n", fd, (int)(ev-g_events));
    }
    else
	{
        close(ev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }

    return;
}


//init assignment
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg)
{
	ev->fd = fd;    //lfd
	ev->call_back = call_back;      //acceptconn（struct myevent_s）    //acceptconn(int lfd, int events, void *arg)
	ev->events = 0;
	ev->arg = arg;
	ev->status = 0;
	//memset(ev->buf, 0, sizeof(ev->buf));
	//ev->len = 0;

	//set time is a last_active
	ev->last_active = time(NULL);
	
	return;
}

//add red-black tree nodes
void eventadd(int efd, int events, struct myevent_s *ev)
{
	//create new node
	struct epoll_event epv = {0, {0}};
	int op;
	epv.data.ptr = ev;
	
	//events  = EPOLLIN
	epv.events = ev->events = events;

	if (ev->status == 1)
	{   
		//if in listen, changes to do
		op = EPOLL_CTL_MOD;
	}
	else
	{
		//if not in listen, add to do, set status = 1
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	//node operation, if new node,then add, if old node, then chenage
	if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
		printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
	else
		printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);
	
	return;
}

void acceptconn(int lfd, int events, void *arg)
{
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);
    int cfd, i;

	//accept
    if ((cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1)
	{
        if (errno != EAGAIN && errno != EINTR) {
            /* 暂时不做出错处理 */
        }
        printf("%s: accept, %s\n", __func__, strerror(errno));
        return;
    }

    do
	{
        for (i = 0; i < MAX_EVENTS; i++)
		{   
			//#define MAX_EVENTS  1024   client node num

			//if status not in listen, break
            if (g_events[i].status == 0)
                break;
        }

        if (i == MAX_EVENTS)
		{
			//if max node num, break
            printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;
        }

        int flag = 0;

		//if cfd set non blocking err
        if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0)
        {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }
		
		//set node read
        eventset(&g_events[i], cfd, recvdata, &g_events[i]);
        eventadd(g_efd, EPOLLIN, &g_events[i]);
    } while(0);

    printf("new connect [%s:%d][time:%ld], pos[%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i);

    return;
}


//init server socket ip and port, init red-black tree
void initlistensocket(int efd, short port)
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	
	//set non blocking
	fcntl(lfd, F_SETFL, O_NONBLOCK);

	//init assignment
	eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
	
	//add red-black tree nodes
	eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);

	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

	listen(lfd, 20);

	return;
}

int main(int argc,char *argv[])
{
	//init port
	unsigned short port = SERV_PORT;
	
	//input port
	if (argc == 2)
		port = atoi(argv[1]);
	
	g_efd = epoll_create(MAX_EVENTS+1);

	//create error
	if (g_efd <= 0)
		printf("%s: create efd in %s err\n",__func__,strerror(errno));

	//init server fd, red-black tree
	initlistensocket(g_efd,port);

	// while events
	struct epoll_event events[MAX_EVENTS+1];

    printf("server running:port[%d]\n", port);
    int checkpos = 0, i;
    while (1)
	{
		//Timeout verification, 100 links for each test, not test listenfd, if client 60seconds not accept server, then close client and server to link
        long now = time(NULL);
        for (i = 0; i < 100; i++, checkpos++)
		{
            if (checkpos == MAX_EVENTS)
                checkpos = 0;
            if (g_events[checkpos].status != 1)
                continue;
            long duration = now - g_events[checkpos].last_active;
            if (duration >= 60)
			{
                close(g_events[checkpos].fd);
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                eventdel(g_efd, &g_events[checkpos]);
            }
        }
		//wait for events
        int nfd = epoll_wait(g_efd, events, MAX_EVENTS+1, 1000);
        if (nfd < 0)
		{
            printf("epoll_wait error, exit\n");
            break;
        }
        for (i = 0; i < nfd; i++)
		{
            struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
			{
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
			{
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
        }
    }

    /* 退出前释放所有资源 */

	return 0;
}

