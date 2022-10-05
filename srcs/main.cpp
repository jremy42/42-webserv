/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jremy <jremy@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/09/21 15:39:59 by jremy            ###   ########.fr       */
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

int	g_signal;
int	run;

void __signal(int signal)
{
	(void)signal;
	g_signal = 0;
	run = 0;
}

void treat_co(int sock)
{
	struct sockaddr_in addr;
	socklen_t size;
	char buffer[256];
	size = sizeof(struct sockaddr_in);
	g_signal = 1;
	if (getpeername(sock, (struct sockaddr *)&addr, &size) < 0)
	{
		perror("getpeername");
		return;
	}
	sprintf(buffer, "IP = %s, Port = %u \n",
	inet_ntoa(addr.sin_addr),
	ntohs(addr.sin_port));
	fprintf(stdout, "Connexion : locale ");
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
		printf("sock = %d\n", sock);
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
	_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
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

int main()
{
	int _serveur_fd;
	int _client_fd;
	struct sockaddr_in adresse;

	run =  1;
	std::cout << "lets go to webserv!!" << std::endl;
	_serveur_fd =  cree_socket_stream();
	if (_serveur_fd == -1)
		return 1;
	listen(_serveur_fd, 1000);
	while (run)
	{
		socklen_t size = sizeof(struct sockaddr_in);
		std::cout <<  "accept" << std::endl;
		_client_fd = accept(_serveur_fd, (struct sockaddr *)& adresse,& size);
	switch (fork()) {
 	case 0 : /* fils */
 		close(_serveur_fd);
 		std::cout  << " treat connection" << std::endl;
		treat_co(_client_fd);
 		exit(0);
 	default : /* père */
 		close(_client_fd);
 	}
	signal (SIGCHLD, SIG_IGN); // permet de kill tout les processus zombies 
 }
 return 0;
}

//add commentary for git test
