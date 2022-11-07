#include "Server.hpp"



Server::Server(int port, v_config configList)
{
	_configList = configList;
	_backlog = 1000;
	try 
	{
		_createPassiveSocket(itoa(port).c_str());
		if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
				throw(std::runtime_error(strerror(errno)));
	} catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		throw(std::runtime_error("Server creation failure"));
	}
	std::cout << "______________________CREATING SERVER________________________________\n";
	std::cout << "serverFD : [" << _serverFd << "]" << std::endl;
	std::cout << "Create server listen port :" << itoa(port) << std::endl;
	v_config::iterator ite = configList.end();
	int i = 0;
	for (v_config::iterator it = configList.begin(); it != ite; it++)
	{
		std::cout << "------------configList " << i << "------------------"<< std::endl;
		std::cout << *it << std::endl;
		std::cout << "---------------------------------------------------- " << std::endl;

		i++;
	}
	std::cout << "___________________________________________________________________\n";

};


Server::Server(const Config &config_source)
{
	_config = config_source;
	_backlog = 1000;
	try 
	{
		_createPassiveSocket(_config.getListenPortStr().c_str());
		if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
				throw(std::runtime_error(strerror(errno)));
	} catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		throw(std::runtime_error("Server creation failure"));
	}
	printf("_serverFd = %d\n", _serverFd);
	std::cout << "Create server listen port :" << _config.getListenPortStr() << std::endl;
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
		_configList = src._configList;
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
	clientFd = accept(_serverFd, (struct sockaddr *)& claddr, &addrlen);
	if (clientFd == -1 && (errno != EAGAIN || errno != EWOULDBLOCK))
		throw(std::runtime_error(strerror(errno)));
	//printf("serverFd: [%d] | client fd : [%d]\n",_serverFd, clientFd);
	usleep(100000);
	if (clientFd > 0)
	{
		_clientAddressPrint((struct sockaddr *)& claddr);
		Client *newClient = new Client(clientFd, &_configList);
		_clientList.push_back(newClient);
		_evLst.trackNewClient(clientFd, EPOLLIN | EPOLLOUT);
		return 1;
	}
	return 0;
}

int Server::listenEvent(void)
{
	v_iterator ite = _clientList.end();

	if(_evLst.clientAvailable() > 0)
	{
		for (v_iterator i = _clientList.begin(); i != ite; ++i)
			(*i)->setAvailableActions(_evLst.getClientFlag((*i)->getClientFd()));
	}
	return (1);
}

int Server::execClientList(void) // mode naif activate 
{
	v_iterator ite = _clientList.end();
	for (v_iterator _currentCli = _clientList.begin(); _currentCli != ite; _currentCli++)
	{
		if ((*_currentCli)->executeAction())
		{
			if ((*_currentCli)->getState() == S_CLOSE_FD)
			{
				std::cout << "Closing and removing Client with read return = 0" << std::endl;
				close((*_currentCli)->getClientFd());
				_clientList.erase(_currentCli);
			}
			if (_currentCli + 1 != ite)
			rotate(_clientList.begin(), _currentCli + 1, ite);
			break;
		}
	}
	return 1;
}

void 				Server::_createPassiveSocket(const char *service)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *result_it;
	int optval;
	int g_error;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family =AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	if ((g_error = getaddrinfo(NULL, service, &hints, &result )) != 0)
		throw(std::runtime_error(gai_strerror(g_error)));
	for (result_it = result; result_it != NULL; result_it = result_it->ai_next)
	{
		_serverFd = socket(result_it->ai_family, result_it->ai_socktype, result_it->ai_protocol);
		if (_serverFd == -1)
			continue;
		if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		{
				close(_serverFd);
				freeaddrinfo(result);
				throw(std::runtime_error(strerror(errno)));

		}
		if (bind(_serverFd, result_it->ai_addr, result_it->ai_addrlen) == 0)
			break;
		close(_serverFd);
	}
	if (!result_it)
		throw(std::runtime_error("enable to create and bind listening socket"));
	if (listen(_serverFd, _backlog) == -1)
	{
		close(_serverFd);
		freeaddrinfo(result);
		throw(std::runtime_error(strerror(errno)));
	}
	_listenSockaddr = *(struct sockaddr_in *)result_it->ai_addr;
	freeaddrinfo(result);
}

void Server::_clientAddressPrint(struct sockaddr *cliAddr)
{
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	int r = getnameinfo( cliAddr, sizeof(_listenSockaddr), host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV); 
	
	if (r == 0)
 		printf("(Host:[%s], service[%s])\n", host, service);
	else
		printf("getnameinfo failure :[%s]", gai_strerror(r));
}
