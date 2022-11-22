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
# include <stdexcept>
# include <algorithm>

# ifndef DEBUG_SERVER
#  define DEBUG_SERVER 0
# endif
# define KEEP_ALIVE_MAX_REQUEST 1000
# define KEEP_ALIVE_TIMEOUT 10 //secondes


class Client;
class Config;

class Server
{
	public:
		typedef std::map<int ,Client*>	m_client;
		typedef m_client::iterator	m_iterator;
		typedef std::string			string;
		typedef std::vector<Config> v_config;

	public:
		Server(v_config configList);
		Server(const Server &src);
		~Server(void);
		Server &operator=(const Server &rhs);
		int	acceptNewClient(void); // si != -1 Fait un accept, ajoute le fd a la client list et ajoute au epoll_ctl le fd
		int	execClientAction(int fd, int availableAction); // Fait 1 epoll_wait et demande a 1 client de faire une seule action (read ou write)
		int getServerFd(void);
		// Boucler sur les 2 actions precedentes (fait dans la classe  webserv)

	private:
		Config				_config; // switch to v_config;
		v_config			_configList;
		//EventListener		_evLst;
		m_client			_clientListFd;
		int					_serverFd;
		struct sockaddr_in	_listenSockaddr;
		int					_backlog;
		string				_port;

	private:
		void 				_createPassiveSocket(const char *service, const char *host);
		void 				_clientAddressPrint(struct sockaddr *cliAddr);
		v_config			_matchingConfigListByHost(int host);
		static void			_delete_second(std::pair<int, Client*> toDelete);
};

#endif
