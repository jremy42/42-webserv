#include "Server.hpp"

Server::Server(v_config configList)
{
	_configList = configList;
	_backlog = 1000;
	_port = _configList[0].getListenPortStr();

	try 
	{
		if (_configList[0].getServerInfoMap().find("listen")->second[0] == "*")
			_createPassiveSocket(_port.c_str(), "0.0.0.0");
		else
			_createPassiveSocket(_port.c_str(), _configList[0].getServerInfoMap().find("listen")->second[0].c_str());

	} catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		throw(std::runtime_error("Webserv: Server creation failure"));
	}
	if (DEBUG_SERVER)
	{
		std::cout << "______________________CREATING SERVER________________________________\n";
		std::cout << "serverFD : [" << _serverFd << "]" << std::endl;
		std::cout << "Create server listen port :" << _port << std::endl;
		v_config::iterator ite = configList.end();
		int i = 0;
		for (v_config::iterator it = configList.begin(); it != ite; it++)
		{
			std::cout << "------------configList " << i << "------------------"<< std::endl;
			std::cout << *it << std::endl;
			std::cout << "--------------------------------z-------------------- " << std::endl;

			i++;
		}
		std::cout << "___________________________________________________________________\n";
	}
	printLogServer(1, _serverFd );

};

void Server::_delete_second(std::pair<int, Client*> toDelete)
{
		delete toDelete.second;
};

Server::~Server()
{
	
	for_each(_clientListFd.begin(), _clientListFd.end(), _delete_second );
	if(DEBUG_SERVER)
		std::cerr << "Server destruction" << std::endl;
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
        //_evLst = src._evLst;
        _clientListFd = src._clientListFd;
		_listenSockaddr = src._listenSockaddr;
		_serverFd = src._serverFd;
		_backlog = src._backlog;
		_port = src._port;
	}
	return *this;
};

Server::v_config	Server::_matchingConfigListByHost(unsigned int host)
{
	v_config ret;
	v_config::iterator it = _configList.begin();
	for (; it != _configList.end(); it++)
	{
		if (it->getHost() == host)
		{
			if(DEBUG_SERVER)
				std::cout << " add new config by host" << std::endl;
			ret.push_back(*it);
		}
	}
	if (!ret.empty())
	{
		for (it =_configList.begin(); it != _configList.end(); it++)
		{
			if (it->getHost() == 0)
			{
				if(DEBUG_SERVER)
					std::cout << " add new config for wildcard" << std::endl;
				ret.push_back(*it);
			}
		}
	}
	return ret;
}

int Server::acceptNewClient(void)
{
	int clientFd;
	socklen_t addrlen;
	struct sockaddr_in claddr;
	socklen_t serverIpAddrlen;
	struct sockaddr_in requestedServerIp;

	addrlen = sizeof(struct sockaddr_in);
	clientFd = accept(_serverFd, (struct sockaddr *)& claddr, &addrlen);
	if (clientFd == -1 && (errno != EAGAIN || errno != EWOULDBLOCK))
		throw(std::runtime_error(string("Webserv: accept failed:") + strerror(errno)));
	//printf("serverFd: [%d] | client fd : [%d]\n",_serverFd, clientFd);
	
	if (clientFd > 0)
	{
		serverIpAddrlen = sizeof(struct sockaddr_in);
		getsockname(clientFd, (struct sockaddr *)&requestedServerIp, &serverIpAddrlen);
		char buf[INET_ADDRSTRLEN + 1];
		memset(buf, 0, sizeof(buf));
		if(DEBUG_SERVER)
		{
			std::cerr << "\e[35mRequested Server IP : [" << inet_ntop(AF_INET, &requestedServerIp.sin_addr, buf, sizeof(buf)) << "]\e[0m" << std::endl;
			_clientAddressPrint((struct sockaddr *)& claddr);
		}
		Client *newClient = new Client(clientFd, &_configList, this, (unsigned int)requestedServerIp.sin_addr.s_addr);
		_clientListFd.insert(std::pair<int, Client*>(clientFd, newClient));
		return clientFd;
	}
	return 0;
}

int Server::execClientAction(int fd, int availableAction ) // mode naif activate 
{
	Client* currentCli = _clientListFd.find(fd)->second;
	currentCli->setAvailableActions(availableAction);
	try 
	{
		if (currentCli->executeAction())
		{
			if (currentCli->getState() == S_CLOSE_FD)
			{
				if (DEBUG_SERVER)
					std::cerr << "Closing and removing Client with read return = 0" << std::endl;
				printLog(1, currentCli->getClientFd(), 1, " connection close");
				close(currentCli->getClientFd());
				_clientListFd.erase(currentCli->getClientFd());
				delete currentCli;
				return 0;
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		printLog(1, currentCli->getClientFd(), 1, " connection close because system error");
		close(currentCli->getClientFd());
		_clientListFd.erase(currentCli->getClientFd());
		delete currentCli;
		return 0;
	}
	return 1;
}


void 				Server::_createPassiveSocket(const char *service, const char *host)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *result_it;
	int optval = 1;
	int g_error;

	if(DEBUG_SERVER)
		std::cerr << "createPassiveSocketWithHost [" << (host != NULL ? host : "NULL HOST") << "]" << std::endl;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((g_error = getaddrinfo(host, service, &hints, &result )) != 0)
		throw(std::runtime_error(gai_strerror(g_error)));
	for (result_it = result; result_it != NULL; result_it = result_it->ai_next)
	{
		char buff[INET_ADDRSTRLEN];
		memset(buff, 0, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &((struct sockaddr_in*)(result_it->ai_addr))->sin_addr, buff, INET_ADDRSTRLEN);
		if(DEBUG_SERVER)
			std::cerr << "inet_ntop["<< buff<< "]" << std::endl;
	}
	for (result_it = result; result_it != NULL; result_it = result_it->ai_next)
	{
		_serverFd = socket(result_it->ai_family, result_it->ai_socktype, result_it->ai_protocol);
		if (_serverFd == -1)
			continue;
		if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		{
				close(_serverFd);
				freeaddrinfo(result);
				throw(std::runtime_error(string("Webserv : ") + strerror(errno)));

		}
		unsigned int uaddr = ((struct sockaddr_in*)(result_it->ai_addr))->sin_addr.s_addr;
		if (DEBUG_SERVER)
		{
			std::cerr << "bind:" << uaddr << std::endl;
			std::cerr << "fd:" << _serverFd << std::endl;
		}
		char buff[INET_ADDRSTRLEN];
		memset(buff, 0, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &((struct sockaddr_in*)(result_it->ai_addr))->sin_addr, buff, INET_ADDRSTRLEN);
		if (DEBUG_SERVER)
			std::cerr << "inet_ntop["<< buff<< "]" << std::endl;
		if (bind(_serverFd, result_it->ai_addr, result_it->ai_addrlen) == 0)
			break;
		close(_serverFd);
	}
	if (!result_it)
		throw(std::runtime_error("Webserv: unable to create and bind listening socket"));
	if (listen(_serverFd, _backlog) == -1)
	{
		close(_serverFd);
		freeaddrinfo(result);
		throw(std::runtime_error(string("Webserv : listen failure : ") + strerror(errno)));
	}
	if (DEBUG_SERVER && result_it)
		std::cerr << " listen socket address with host [" << _listenSockaddr.sin_addr.s_addr << "]"<< std::endl;
	_listenSockaddr = *(struct sockaddr_in *)result_it->ai_addr;
	freeaddrinfo(result);
}

void Server::_clientAddressPrint(struct sockaddr *cliAddr)
{
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	int r = getnameinfo(cliAddr, sizeof(_listenSockaddr), host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV); 
	
	if (r == 0)
 		printf("(Host:[%s], service[%s])\n", host, service);
	else
		printf("getnameinfo failure :[%s]", gai_strerror(r));
}


int Server::getServerFd(void)
{
	return _serverFd;
}
