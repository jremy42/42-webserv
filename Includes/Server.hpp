#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/socket.h> 
# include <netinet/in.h>
# include <vector>
# include <stdio.h>
# include <string>
# include <cstring>
# include <arpa/inet.h>
# include <netdb.h>
# include <errno.h>
# include "Client.hpp"
# include "Config.hpp"
# include "EventListener.hpp"
# define DEBUG 1

class client;
class Config;

class Server
{
	typedef std::vector<Client*>	v_client;
	typedef v_client::iterator	v_iterator;
	typedef std::string			string;

	public:
		Server(const Config &config_source);
		Server(const Server &src);
		~Server(void);
		Server &operator=(const Server &rhs);
		int	acceptNewClient(void); // si != -1 Fait un accept, ajoute le fd a la client list et ajoute au epoll_ctl le fd
		int	execClientList(void); // Fait 1 epoll_wait et demande a 1 client de faire une seule action (read ou write)
		int listenEvent(void);
		// Boucler sur les 2 actions precedentes (fait dans la classe  webserv)

	private:
		Config			_config;
		EventListener	_evLst;
		v_client		_clientList;
		int				_serverFd;
		socklen_t		_addrlen;
		struct sockaddr_in	_listenSockaddr;

};

#endif
