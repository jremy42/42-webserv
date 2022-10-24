#include "Server.hpp"

static int inetPassiveSocket(const char *service, int type, socklen_t *addrlen, int doListen, int backlog, struct sockaddr * _sockaddr)
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
	{
		*addrlen = rp->ai_addrlen;
		_sockaddr = rp->ai_addr;
	}
	freeaddrinfo(result);
	return (rp == NULL) ? -1 : sfd;
}

static int inetListen(const char *service, int backlog, socklen_t *addrlen, struct sockaddr *_sockaddr)
{
 	return inetPassiveSocket(service, SOCK_STREAM, addrlen, 1, backlog, _sockaddr);
}

static void inetAddressPrint(struct sockaddr *addr, socklen_t addrlen)
{
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	(void)addrlen;
	int r = getnameinfo(addr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV); 
	
	if (r == 0)
 		printf("(Host:[%s], service[%s])\n", host, service);
	else
		printf("getnameinfo failure :[%s]", gai_strerror(r));
}

Server::Server(const Config &config_source)
{
	_config = config_source;
	_serverFd = inetListen(_config.getListenPort(), 1000, &_addrlen,(struct sockaddr *) &_listenSockaddr);
	printf("_serverFd = %d\n", _serverFd);
	fcntl(_serverFd, F_SETFL, O_NONBLOCK);
	printf("_serverFd = %d\n", _serverFd);
	std::cout << "Create server listen port :" << _config.getListenPort() << std::endl;
};

Server::~Server(){

	std::cout << "Server destruction" << std::endl;
};

Server::Server(const Server & src)
{
	if (this != &src)
	{
		*this = src;
	}
};

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		_config = src._config;
        _evLst = src._evLst;
        _clientList = src._clientList;
	}
	return *this;
};


int Server::acceptNewClient(void)
{
	int clientFd;
	socklen_t addrlen;
	struct sockaddr_in claddr;

	addrlen = sizeof(struct sockaddr_in);
	//printf("ici\n");
	clientFd = accept(_serverFd, (struct sockaddr *)& claddr, &addrlen);
	printf("serverFd: [%d] | client fd : [%d]\n",_serverFd, clientFd);
	sleep(1);
	if (clientFd > 0)
	{
		inetAddressPrint((struct sockaddr *)& claddr, _addrlen);
		Client *newClient = new Client(clientFd);
		_clientList.push_back(newClient);
		_evLst.trackNewClient(clientFd, EPOLLIN | EPOLLOUT);
		return 1;
	}
	return 0;
}

int Server::listenEvent(void)
{
	v_iterator ite = _clientList.end();

	if(_evLst.clientAvailable() > 1)
	{
		for (v_iterator i = _clientList.begin(); i != ite; ++i)
			(*i)->setAvailableActions(_evLst.getClientFlag((*i)->getClientFd()));
	}
	return (1);
}

int Server::execClientList(void)
{
	v_iterator ite = _clientList.end();
	
	for (v_iterator _currentCli = _clientList.begin(); _currentCli != ite; _currentCli++)
	{
		if ((*_currentCli)->executeAction())
			break;
	}
	return 1;
}