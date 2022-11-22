#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <fstream>
# include <algorithm>
# include <sys/time.h>
# include <sys/resource.h>
# include "Server.hpp"
# include "Client.hpp"
# include "Config.hpp"
# include "EventListener.hpp"

# ifndef DEBUG_WEBSERV
#  define DEBUG_WEBSERV 0
# endif

# define CLIENT 0
# define SERVER 1

class Webserv
{
	public:
		typedef std::string			string;
		typedef	std::vector<string>	v_string; // Deviens un vector pour avoir un fichier par serveur supp
		typedef std::vector<Server*>	v_server;
		typedef std::vector<Config>	v_config;
		typedef v_server::iterator	v_iterator;
		typedef std::map<int, std::vector<Config> > m_i_vc;
		typedef std::map<int, Server*> m_i_serv;
		typedef std::map< std::pair<int, int>, std::vector<Config> > m_piu_vc;

		Webserv();
		Webserv(string fileName); // open filenames, populates p_rawConfig
		Webserv(const Webserv &src);
		~Webserv(void);
		Webserv &operator=(const Webserv &rhs);

		std::string	getNextServerBlock(std::string &rawConfig);
		int parseRawConfig(void);
		int	execServerLoop(void); // Boucle sur la liste de serveur avec les actions acceptNewClient et execClientList
		int	createServerListByPortConfig(void);

		class NotEnoughValidConfigFilesException : public std::exception
		{
			public :
			virtual const char *what(void) const throw();
		};

	private:
		unsigned long			_openFd;
		unsigned long			_maxFd;
		v_server				_serverList;
		v_config				_configList;
		v_string				_rawConfig;
		m_piu_vc				_portIpConfigList;
		m_i_serv				_fdServerList;
		m_i_serv				_fdClientList;
		EventListener			_evListener;
		void					_loadFile(const char *fileName);
		string					_checkServerName(std::vector<string> nextServerName, std::vector<string> currentServerName);
		unsigned long			_getMaxFd();
		void					_adjustWildCardPort();
};

#endif
