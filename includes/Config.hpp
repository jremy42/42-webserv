#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <algorithm>
# include <fstream>
# include <map>
# include <vector>
# include <iostream>
# include <string>
# include <sstream>
# include "Request.hpp"
# include "Response.hpp"
# include <iterator>
# include "Location.hpp"

# define DEBUG_CONFIG 0

class Location;

class Config
{
// typedef
	public:
		typedef std::string								string;
		typedef std::map<string, std::vector<string> >	m_s_vs;
		typedef std::map<string, Location>				m_s_l;
// var
	private:
		int		_listenPort;
		string	_rootDirectory;
		m_s_vs	_serverInfoMap;
		m_s_l	_location;
// functions
	public:
		Config(void);
		Config(string rawServerConfig);
		Config(const Config &src);
		~Config(void);
		Config &operator=(const Config &rhs);

		const string	getListenPortStr(void) const;
		int	getListenPort(void) const;
		const std::vector<string>	getServerName(void) const;
		const char * getRootDir(void) const;

	private:
		std::map<std::string, std::vector<std::string> > 	_createServerInfoMap(std::string &rawServerConf);
		std::pair<std::string, std::vector<std::string > >	parseConfigBlock(std::string &nextLine);
		char												getNextBlockDelim(std::string str, int pos) const;
		std::string											getNextLocationBlock(std::string &rawLocation);
		
		static std::map<string, int>	_configField;
		static std::map<string, int> _initConfigField(void);
};

std::ostream	&operator<<(std::ostream &o, const std::vector<std::string> &vec);
std::ostream	&operator<<(std::ostream &o, const std::pair<std::string, std::vector<std::string> > &pair);
std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::vector<std::string> > &map);

#endif
