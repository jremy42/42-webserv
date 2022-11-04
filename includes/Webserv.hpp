#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <fstream>
# include <algorithm>

# include "Server.hpp"
# include "Config.hpp"

# define DEBUG 1

class Webserv
{
	public:
	typedef std::string			string;
	typedef	std::vector<string>	v_string; // Deviens un vector pour avoir un fichier par serveur supp
	typedef std::vector<Server*>	v_server;
	typedef std::vector<Config>	v_config;
	typedef v_server::iterator	v_iterator;

		Webserv();
		Webserv(char **configArray); // open filenames, populates _rawConfig
		Webserv(string fileName); // open filenames, populates _rawConfig
		Webserv(const Webserv &src);
		~Webserv(void);
		Webserv &operator=(const Webserv &rhs);

		std::string	getNextServerBlock(std::string &rawConfig);
		int parseRawConfig(void);
		int	createServerListFromRawConfig(void); // add les serveurs a serverlist au fur et a mesure a partir rawConfig
		int	execServerLoop(void); // Boucle sur la liste de serveur avec les actions acceptNewClient et execClientList

		class NotEnoughValidConfigFilesException : public std::exception
		{
			public :
			virtual const char *what(void) const throw();
		};

	private:
		v_server		_serverList;
		v_config		_configList;
		v_string		_rawConfig;
		char			**_configArray;


};

#endif
