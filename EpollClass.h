/*
 * epollClass.h
 * to handle network by epoll
 * contains:
 *		Constants
 *			BUFMAXSIZE:	buf size
 *			IPADDRESS:	default ip address, the localhost
 *			PORT:		default port
 *		Client class
 *		Server class
 */

#ifndef EPOLLCLASS_H
#define EPOLLCLASS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "Basics.h"
#include "MesHandleClass.h"

const int FDSIZE		= 1024;
const int EPOLLEVENTS	= 20;
const int LISTENQ		= 5;

const int IPADDRMAXSIZE	= 20;

/* 
 * Client class
 * client in network, to send message
 */
class Client {
public:
	Client(char *ipAddress, const int port)
	: port(port) {
		strncpy(this->ipAddress, ipAddress, IPADDRMAXSIZE);

		sockfd = socket(AF_INET,SOCK_STREAM,0);
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		inet_pton(AF_INET,ipAddress, &servaddr.sin_addr);

		epollfd = epoll_create(FDSIZE);
	}
	Client() {}
	void Init(char *ipAddress, const int port) {
		strncpy(this->ipAddress, ipAddress, IPADDRMAXSIZE);
		this->port = port;

		sockfd = socket(AF_INET,SOCK_STREAM,0);
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		inet_pton(AF_INET,ipAddress, &servaddr.sin_addr);

		epollfd = epoll_create(FDSIZE);
	}
	bool Connect() {
		int flag = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) {
			perror("client socket reuse error: ");
			return false;
		}
		if (connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1) {
			//perror("client connect error: ");
			return false;
		}
		return true;
	}
	bool Send(void *sendBuf, int len) {
		((char *)sendBuf)[len] = '\n';
		((char *)sendBuf)[len + 1] = '\0';
		add_event(epollfd, sockfd, EPOLLOUT);
		epoll_wait(epollfd, events, EPOLLEVENTS, -1);
		int nwrite;
		nwrite = write(sockfd, sendBuf, len + 2);
		if (nwrite == -1)
		{
			perror("write error:");
			Disconnect();
			return false;
		}
		delete_event(epollfd, sockfd, EPOLLOUT);
		return true;
	}
	void Disconnect() {
		close(epollfd);
		close(sockfd);
	}
	~Client() {
		Disconnect();
	}

private:
	char ipAddress[IPADDRMAXSIZE];
	int port;

	int sockfd;
	struct sockaddr_in servaddr;
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];

	void add_event(int epollfd,int fd,int state)
	{
		struct epoll_event ev;
		ev.events = state;
		ev.data.fd = fd;
		epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
	}
	void delete_event(int epollfd,int fd,int state)
	{
		struct epoll_event ev;
		ev.events = state;
		ev.data.fd = fd;
		epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
	}
	void modify_event(int epollfd,int fd,int state)
	{
		struct epoll_event ev;
		ev.events = state;
		ev.data.fd = fd;
		epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
	}

};

/*
 * Server class
 * server in network, listen messages
 */
class Server {
public:
	Server(char *ipAddress, const int port, MesHandle *mesHandle)
	: port(port), mesHandle(mesHandle) {
		strncpy(this->ipAddress, ipAddress, IPADDRMAXSIZE);
	}
	bool Bind() {
		struct sockaddr_in servaddr;
		listenfd = socket(AF_INET,SOCK_STREAM,0);
		if (listenfd == -1)
		{
			perror("socket error:");
			return false;
		}
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		inet_pton(AF_INET, ipAddress, &servaddr.sin_addr);
		servaddr.sin_port = htons(port);
		int flag = 1;
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) {
			perror("socket_reuse error: ");
			return false;
		}
		if (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1)
		{
			perror("bind error: ");
			return false;
		}
		return true;
	}
	void Listen() {
		listen(listenfd,LISTENQ);

		//创建一个描述符
		epollfd = epoll_create(FDSIZE);
		//添加监听描述符事件
		add_event(epollfd,listenfd,EPOLLIN);
		for (int nEvent = 0, i = 0; ; ) {
			if (i < nEvent) {
				int fd = events[i].data.fd;
				//根据描述符的类型和事件类型进行处理
				if ((fd == listenfd) &&(events[i].events & EPOLLIN))
					handle_accpet(epollfd,listenfd);
				else if (events[i].events & EPOLLIN)
					do_read(epollfd,fd,buf);
				else if (events[i].events & EPOLLOUT)
					do_write(epollfd,fd,buf);

				++i;
			}
			else {
				//获取已经准备好的描述符事件
				nEvent = epoll_wait(epollfd,events,EPOLLEVENTS,-1);
				i = 0;
			}
		}
    }
	void Disconnect() {
		close(epollfd);
		close(listenfd);
	}
	~Server() {
		Disconnect();
	}


private:
	char ipAddress[IPADDRMAXSIZE];
	const int port;
	MesHandle *mesHandle;

	int listenfd;
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
	char buf[BUFMAXSIZE];
	//BufTable bufTab;

	void handle_accpet(int epollfd,int listenfd)
	{
		int clifd;
		struct sockaddr_in cliaddr;
		memset(&cliaddr, 0, sizeof(struct sockaddr_in));
		socklen_t  cliaddrlen = 1;
		clifd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen);
		if (clifd == -1)
			perror("accpet error:");
		else
		{
			printf("accept a new client: %s:%d\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
			//添加一个客户描述符和事件
			add_event(epollfd,clifd,EPOLLIN);
		}
	}

	void do_read(int epollfd,int fd,char *buf)
	{
		memset(buf, 0, BUFMAXSIZE);
		int nread;
		nread = read(fd,buf,BUFMAXSIZE);
		if (nread == -1)
		{
			perror("read error:");
			close(fd);
			delete_event(epollfd,fd,EPOLLIN);
		}
		else if (nread == 0)
		{
			fprintf(stderr,"client close.\n");
			close(fd);
			delete_event(epollfd,fd,EPOLLIN);
		}
		else
		{
			//printf("readMes: %s", buf);
			// call the message handle function to handle the message
			mesHandle->Handle(buf);
		}
	}

	void do_write(int epollfd,int fd,char *buf)
	{
		int nwrite;
		nwrite = write(fd,buf,strlen(buf));
		if (nwrite == -1)
		{
			perror("write error:");
			close(fd);
			delete_event(epollfd,fd,EPOLLOUT);
		}
		else
			modify_event(epollfd,fd,EPOLLIN);
		memset(buf,0,BUFMAXSIZE);
	}

	void add_event(int epollfd,int fd,int state)
	{
		struct epoll_event ev;
		ev.events = state;
		ev.data.fd = fd;
		epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
	}

	void delete_event(int epollfd,int fd,int state)
	{
		struct epoll_event ev;
		ev.events = state;
		ev.data.fd = fd;
		epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
	}

	void modify_event(int epollfd,int fd,int state)
	{
		struct epoll_event ev;
		ev.events = state;
		ev.data.fd = fd;
		epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
	}
};

#endif
