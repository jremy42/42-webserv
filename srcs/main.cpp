/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/10/18 18:28:41 by jremy            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <iostream>
#include <string>
#include <locale>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdbool.h>
#include "Epoll.hpp"

int	g_signal;
int	run;

void __signal(int signal)
{
	(void)signal;
	g_signal = 0;
	run = 0;
}

void treat_co(int sock, struct sockaddr_in * claddr)
{
	socklen_t size;
	char buffer[256];
	size = sizeof(struct sockaddr_in);
	g_signal = 1;
	printf("sock = %d\n", sock);
	sprintf(buffer, "IP = %s, Port = %u \n",
	inet_ntoa(claddr->sin_addr),
	ntohs(claddr->sin_port));
	fprintf(stdout, "Connexion : ");
	//print_socket(sock);
	fprintf(stdout, " distante %s", buffer);
	write(sock, "Votre adresse : ", 16);
	write(sock, buffer, strlen(buffer));
	while(g_signal)
	{
		ssize_t r = 0; 
		char buffer2[1000];
		memset(buffer2, 0, 1000);
		r = read(sock, buffer2, 1000);
		if (r <= 0)
			break;
		buffer2[r] = 0;
		//printf("sock = %d\n", sock);
		write(sock, buffer2, r);
		printf("%s", buffer2);
	}
	shutdown(sock, SHUT_RDWR);
}

 int cree_socket_stream (void)
{
	int							sock;
	struct sockaddr_in			_addr;
	int							aut;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}
	memset((char *)&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(5000);
	aut = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, & aut, sizeof(int));
	if (bind(sock, (struct sockaddr *)&_addr , sizeof(struct sockaddr_in)) < 0)
	{
		close(sock);
		perror("bind");
		return -1;
	}
	fprintf(stdout, "IP = %s, Port =make %u \n", inet_ntoa(_addr.sin_addr), ntohs(_addr.sin_port));
	return sock;
}

static int inetPassiveSocket(const char *service, int type, socklen_t *addrlen, int doListen, int backlog)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	int sfd;
	int optval;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family =AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = type;
	s = getaddrinfo(NULL, service, &hints, &result);
	if (s != 0)
		return -1;
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		if (doListen)
		{
			if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
			{
				close(sfd);
				freeaddrinfo(result);
				return -1;
			}
		}
		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		close(sfd);
	}
	if (rp != NULL && doListen)
		if (listen(sfd, backlog) == -1)
		{
			freeaddrinfo(result);
			return -1;
		}
	if (rp != NULL && addrlen != NULL)
		*addrlen = rp->ai_addrlen;
	freeaddrinfo(result);
	return (rp == NULL) ? -1 : sfd;
}

int inetListen(const char *service, int backlog, socklen_t *addrlen)
{
 	return inetPassiveSocket(service, SOCK_STREAM, addrlen, 1, backlog);
}

int inetBind(const char *service, int type, socklen_t *addrlen)
{
	 return inetPassiveSocket(service, type, addrlen, 0, 0);
}

void inetAddressPrint(const struct sockaddr *addr, socklen_t addrlen)
{
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	if (getnameinfo(addr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV) == 0)
 		printf("(Host:[%s], service[%s])\n", host, service);
	else
		printf("(?UNKNOWN?)\n");
}

// int main()


// {
// 	int _serveur_fd;
// 	int _client_fd;
// 	//int _chanel_fd;
//  	struct sockaddr_in claddr;
//  	socklen_t addrlen;
// 	run =  1;
// 	std::cout << "lets go to webserv!!" << std::endl;
// 	_serveur_fd = inetListen("5000", 1000, &addrlen);
	
// 	while (run)
// 	{
// 		addrlen = sizeof(struct sockaddr_in);
// 		std::cout <<  "accept" << std::endl;
// 		_client_fd = accept(_serveur_fd, (struct sockaddr *)& claddr, &addrlen);
// 		inetAddressPrint((struct sockaddr *)& claddr, addrlen);
// 		if(_client_fd == -1)
// 		{
// 			perror("accept");
// 			return 1;
// 		}
// 	switch (fork()) {
//  	case 0 : /* fils */
//  		close(_serveur_fd);
//  		std::cout  << " treat connection" << std::endl;
// 		treat_co(_client_fd, &claddr);
//  		exit(0);
//  	default : /* père */
//  		close(_client_fd);
//  	}
//   }
//  close(_serveur_fd);
//  return 0;
// }

int main()
{
	int _serveur_fd;
	int _client_fd;
 	struct sockaddr_in claddr;
 	socklen_t addrlen;
	Epoll poll;
	
	run =  1;
	std::cout << "lets go to webserv!!" << std::endl;
	_serveur_fd = inetListen("5000", 1000, &addrlen);
	fcntl(_serveur_fd, F_SETFL, O_NONBLOCK);
	while (run)
	{
		addrlen = sizeof(struct sockaddr_in);
		//std::cout <<  "accept" << std::endl;
		_client_fd = accept(_serveur_fd, (struct sockaddr *)& claddr, &addrlen);
		//printf("client_fd = %d\n", _client_fd);
		//inetAddressPrint((struct sockaddr *)& claddr, addrlen);
		if (_client_fd > 1)
			poll.add_fd(_client_fd, EPOLLIN | EPOLLOUT);
		if (poll.fd_ready())
		{
			poll.print_event();
			poll.read_fd();
			poll.write_fd();
		}
		_client_fd = 0;
	
	}
	
}