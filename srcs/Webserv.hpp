#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>

# include "Server.hpp"

class Webserv
{
	typedef std::vector<Server>	v_server;
	typedef v_server::iterator	v_iterator;
	typedef std::string			string;

	public:
		Webserv(std::string config_filename);
		Webserv(const Webserv &src);
		~Webserv(void);
		Webserv &operator=(const Webserv &rhs);

		int	createServerListFromRawConfig(void); // open filename, parser et add les serveurs a serverlist au fur et a mesure
		int	execServerLoop(void); // Boucle sur la liste de serveur avec les actions acceptNewClient et execClientList

	private:
		v_server	_serverList;
		string		_rawConfig;
};

#endif
