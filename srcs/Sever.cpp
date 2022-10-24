#include "Server.hpp"

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

static int inetListen(const char *service, int backlog, socklen_t *addrlen)
{
 	return inetPassiveSocket(service, SOCK_STREAM, addrlen, 1, backlog);
}

Server::Server(const Config &config_source)
{
    int epfd;

    epfd =
	_config = config_source;
}
Server::~Server(){};

Server::Server(const Server & src)
{
	if (this != &src)
	{
		*this = src;
	}
};

const Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		_config = src.config;
        _evLst = src._evLst;
        _clientList = src._clientList;
	}
	return *this;
};


int Server::acceptNewClient(void)
{

}

int execClientList(void)
{

}