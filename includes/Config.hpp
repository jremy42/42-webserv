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

# define DEBUG 1


class Config
{
	public:
		typedef std::string			string;
		typedef std::map<string, std::vector<string> > m_s_vs;
		Config(void);
		Config(string rawServerConfig);
		Config(const Config &src);
		~Config(void);
		Config &operator=(const Config &rhs);


		const string	getListenPortStr(void) const;
		int	getListenPort(void) const;
		const string	getServerName(void) const;
		const char * getRootDir(void) const;

	private:
		int		_listenPort;
		string	_rootDirectory;
		std::map<std::string, std::vector<std::string> > _createServerInfoMap(std::string &rawServerConf);
		m_s_vs	_serverInfoMap;
		char	getNextBlockDelim(std::string str) const;
		std::pair<std::string, std::vector<std::string > >	parseConfigBlock(std::string &nextLine);
		static std::map<string, int>	_configField;
		static std::map<string, int> _initConfigField(void);
};

#endif
