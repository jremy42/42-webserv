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
# include "_utils.hpp"
# include "Location.hpp"

# ifndef DEBUG_CONFIG
#  define DEBUG_CONFIG 1
# endif
# define MAX_BODY_SIZE_HARD_LIMIT 32 * 1024 * 1024
# define MIN_BODY_SIZE_HARD_LIMIT 512

enum {MANDATORY_ONE, OPTIONAL_ONE, OPTIONAL_MULTI};

class Config
{
// typedef
	public:
		typedef std::vector<Config>	v_config;
		typedef std::string								string;
		typedef std::map<string, std::vector<string> >	m_s_vs;
		typedef std::pair<string, std::vector<string> >	p_s_vs;
		typedef std::map<string, Location>				m_s_l;
		typedef std::pair<int, int>						p_ii;
// var
	private:
		int				_listenPort;
		string			_rootDirectory;
		m_s_vs			_serverInfoMap;
		m_s_l			_location;
		unsigned int	_host;
// functions
	public:
		Config(void);
		Config(string rawServerConfig);
		Config(const Config &src);
		~Config(void);
		Config &operator=(const Config &rhs);

		const string	getListenPortStr(void) const;
		int	getListenPort(void) const;
		unsigned int getHost(void) const;
		const std::vector<string>	getServerName(void) const;
		const char * getRootDir(void) const;
		const std::vector<string> getParamByLocation(string &requestTarget, string field) const;
		const m_s_vs		&getServerInfoMap(void) const;
		const m_s_l		&getLocation(void) const;
		std::string getErrorPageByLocation(string &requestTarget, int errorCode) const;
		std::string getCgiByLocation(string &requestTarget, string extension) const;
		std::string getMatchingLocation(string &requestTarget) const;



	private:
		void		_initServerInfoMap(void);
		void		_createServerInfoMap(std::string &rawServerConf);
		void		_burnExternalBrackets(std::string &rawServerConfig);
		p_s_vs		_parseConfigBlock(std::string &nextLine);
		char		_getNextBlockDelim(std::string str, int pos) const;
		string		_getNextLocationBlock(std::string &rawLocation);
		void		_parseListenHostPort(void);
		void		_parseClientMaxBodySize(void);
		static std::map<std::string, std::pair<int, int> >	_configField;
		static std::map<std::string, std::pair<int, int> >	_initConfigField(void);
};

std::ostream	&operator<<(std::ostream &o, const Config config);
std::ostream	&operator<<(std::ostream &o, const std::vector< Config > v_config);

#endif
