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


int calculate_size(struct addrinfo *result)
{
	int i;
	struct addrinfo *rp;
	i = 0;

	for (rp = result; rp != NULL; rp = rp->ai_next)
		i++;	
	return i;	
	
}

void print_addrinfo(struct addrinfo *result)
{
	struct addrinfo *rp;
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		printf(" ai_flags : [%d]\n",rp->ai_flags);
		printf(" ai_family : [%d]\n",rp->ai_family);
		printf(" ai_socktype : [%d]\n",rp->ai_socktype);
		printf(" ai_family : [%u]\n",rp->ai_addrlen);
		printf(" ai_canonname : [%s]\n", (*rp).ai_canonname);
		printf("\n\n");
	};
}

int inetConnect(const char *host, const char *service, int type)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	int		sfd;
	int		s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_INET;
	hints.ai_socktype = type;

	s = getaddrinfo(host, service, &hints, &result);

	printf("size = %d\n", calculate_size(result));
	print_addrinfo(result);
	if ( s != 0)
		return -1;
	printf("step 1\n");
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{	
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		close(sfd);
	}
	freeaddrinfo(result);


	return (rp == NULL) ? -1 : sfd;
}



int main ( int ac, char **av)
{
	int fd;
	std::string buffer;

	fd = inetConnect(av[1], av[2], SOCK_STREAM);
	if (fd == -1)
		return (1);
	while(1)
	{
		std::cout << ">";
		std::getline(std::cin, buffer);
		buffer += "\n";
		if (std::cin.eof() == 1)
		{
			buffer = "connection close\n";
			write(fd, buffer.c_str(), strlen(buffer.c_str()));
			break ;
		}
		write(fd, buffer.c_str(), strlen(buffer.c_str()));
		buffer.clear();
		char buff[1000];
		read(fd, buff, 1000);
		printf("%s", buff);
		std::cout << buffer;

	}
	close(fd);
}
