/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: deus <deus@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 18:22:08 by deus              #+#    #+#             */
/*   Updated: 2022/09/19 21:33:28 by deus             ###   ########.fr       */
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



 int cree_socket_stream (void)
{
	int							sock;
	struct sockaddr_in			_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}
	memset((char *)&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	_addr.sin_port = htons(5000);
	if (bind(sock, (struct sockaddr *)&_addr , sizeof(struct sockaddr_in)) < 0)
	{
		close(sock);
		perror("bind");
		return -1;
	}
	fprintf(stdout, "IP = %s, Port = %u \n", inet_ntoa(_addr.sin_addr), ntohs(_addr.sin_port));
	return sock;
}

int main()
{
	int _serveur_fd;
	int _client_fd;
	struct sockaddr_in adresse;
	int run;

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
 		exit(EXIT_SUCCESS);
 	default : /* père */
 		close(_client_fd);
 	}
 }
 return 0;
}