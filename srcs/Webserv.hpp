#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>

# include "Server.hpp"

# define DEBUG 1

class Webserv
{
	typedef std::string			string;
	typedef	std::vector<string>	v_rawConfig; // Deviens un vector pour avoir un fichier par serveur supp
	typedef std::vector<Server>	v_server;
	typedef v_server::iterator	v_iterator;

	public:
		Webserv(char **configArray); // open filenames, populates _rawConfig
		Webserv(const Webserv &src);
		~Webserv(void);
		Webserv &operator=(const Webserv &rhs);

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
		v_rawConfig		_rawConfig;
};

#endif
