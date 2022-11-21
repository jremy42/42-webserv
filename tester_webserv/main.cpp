#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
# include <string>
# include <iostream>


int main(int argc, char *argv[])
{
	char *reqLenStr; /* Requested length of sequence */
	int cfd;
	ssize_t numRead;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	std::string		request = "POST / HTTP/1.1\n";

	/* Call getaddrinfo() to obtain a list of addresses that
	we can try connecting to */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC; /* Allows IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	if (getaddrinfo(argv[1], argv[2], &hints, &result) != 0)
		exit(1);
	/* Walk through returned list until we find an address structure
	that can be used to successfully connect a socket */
	for (rp = result; rp != NULL; rp = rp->ai_next) 
	{
		cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (cfd == -1)
		continue; /* On error, try next address */
		if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		if (rp == NULL)
			return 1;
	}
	freeaddrinfo(result);
	send(cfd, request.c_str(), request.size(), 0);
	while(1);
//	{
//		sleep(1);
//		std::cout << send(cfd,"1", 1, 0) << std::endl;
//	}
	exit(EXIT_SUCCESS);
}  
