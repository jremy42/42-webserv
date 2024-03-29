#include "Config.hpp"

std::map<std::string, std::pair<int, int> >	Config::_configField = _initConfigField();

std::map<std::string, std::pair<int, int> > Config::_initConfigField()
{
	std::map<string, std::pair<int, int> > configField;

	configField.insert(std::pair<std::string, std::pair<int, int> >("root", std::pair<int, int>(1,1)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("allowed_method", std::pair<int, int>(0,0)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("client_max_body_size", std::pair<int, int>(1,1)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("autoindex", std::pair<int, int>(0,0)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("index", std::pair<int, int>(0, 0)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("upload", std::pair<int, int>(0,0)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("return", std::pair<int, int>(0,0)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("error_page", std::pair<int, int>(2,2)));

	configField.insert(std::pair<std::string, std::pair<int, int> >("listen", std::pair<int, int>(1,1)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("server_name", std::pair<int, int>(1,__INT_MAX__)));

	return (configField);
}

Config::Config(void)
{
	_initServerInfoMap();

}

void	Config::_burnExternalBrackets(std::string &rawServerConfig)
{
	while (rawServerConfig.at(0) != '{')
		rawServerConfig.erase(0,1);
	rawServerConfig.erase(0,1);
	rawServerConfig.erase(rawServerConfig.end() - 1);
}

Config::Config(std::string rawServerConfig)
{
	if (DEBUG_CONFIG)
		std::cerr << "rawServerConfig : >" << rawServerConfig << "<" << std::endl;
	_initServerInfoMap();
	_burnExternalBrackets(rawServerConfig);
	_createServerInfoMap(rawServerConfig);
	if(DEBUG_CONFIG)
		std::cerr << "------------Done Creating the serverInfoMap-------------" << std::endl
				<< _serverInfoMap << "--------------------------------------------------------" << std::endl;
	if (_serverInfoMap.empty())
		throw(std::runtime_error("webserv: empty conf, server block ignored"));

	_parseListenHostPort();
	_parseClientMaxBodySize();
	_listenPort = atoi(_serverInfoMap["listen"][1].c_str());
	if (DEBUG_CONFIG)
	{
		std::cerr << "\e[32mlisten[0] :" << _serverInfoMap["listen"][0].c_str() << "\e[0m" << std::endl;
		std::cerr << "\e[32mlisten[1] :" << _serverInfoMap["listen"][1].c_str() << "\e[0m" << std::endl;
	}
	if (_serverInfoMap["listen"][0] == "*")
		_host = 0;
	else
		_host = getipbyhost(_serverInfoMap["listen"][0].c_str(), _serverInfoMap["listen"][1].c_str());
	_rootDirectory = _serverInfoMap["root"][0];
	if (_listenPort < 0)
		throw(std::runtime_error("webserv: Wrong Port : " + itoa(_listenPort) + ". Value must be above 1024"));
	if (_listenPort > 65536)
		throw(std::runtime_error("webserv: Wrong Port : " + itoa(_listenPort) + ". Value must be below 65536"));
}


Config::Config(const Config &src)
{

	*this = src;
}

Config::~Config(void)
{

}

Config	&Config::operator=(const Config &rhs)
{
	this->_listenPort = rhs._listenPort;
	this->_rootDirectory = rhs._rootDirectory;
	this->_serverInfoMap = rhs._serverInfoMap;
	this->_location = rhs._location;
	this->_host = rhs._host;
	this->_errorPage = rhs._errorPage;

	return (*this);
}

const std::string		Config::getListenPortStr(void) const
{
	return (itoa(_listenPort));
}

const std::vector<std::string> Config::getServerName(void) const
{
	return (_serverInfoMap.find("server_name")->second) ;
}

int		Config::getListenPort(void) const
{
	return (_listenPort);
}

unsigned int		Config::getHost(void) const
{
	return (_host);
}

std::string Config::getHostStr(void) const
{
	return (_serverInfoMap.find("listen")->second[0]);
}

const char* Config::getRootDir(void) const
{
	return (_rootDirectory.c_str());
}

const std::vector<std::string> Config::getParamByLocation(string &requestTarget, string key) const
{
	std::vector<string> 				defaultRet = _serverInfoMap.find(key)->second;
	m_s_vs::const_iterator				tryfind;
	m_s_l::const_reverse_iterator 				it = _location.rbegin();
	m_s_l::const_reverse_iterator 				ite = _location.rend();

	for(;it != ite; it++)
	{
		if (requestTarget.substr(0, it->first.size()).compare(it->first) == 0)
		{
			tryfind = it->second.getLocationInfoMap().find(key);
			if (tryfind != it->second.getLocationInfoMap().end())
			{
				if (DEBUG_CONFIG)
					std::cerr << "getParamByLocation : Found a value for key [" << key << "][" << tryfind->second << "] with location" 
					<< it->first << std::endl;
				if (tryfind->second.empty())
					return (defaultRet);
				return (tryfind->second);
			}
		}
	}
	if (DEBUG_CONFIG)
		std::cerr << "getParamByLocation : No value found for key [" << key << "]. Using Config Default value" << std::endl;
	return (defaultRet);
}

std::string Config::getMatchingLocation(string &requestTarget) const
{
	m_s_l::const_reverse_iterator 				it = _location.rbegin();
	m_s_l::const_reverse_iterator 				ite = _location.rend();

	for(;it != ite; it++)
	{
		if (DEBUG_CONFIG)
			std::cerr << "getMatchingLocation : " << it->first << std::endl;
		if (requestTarget.substr(0, it->first.size()).compare(it->first) == 0)
			return it->first;
	}
	if (DEBUG_CONFIG)
		std::cerr << "getMatchingLocation : : No value found [" << requestTarget << "]. Using Config Default value" << std::endl;
	return ("");
}

char	Config::_getNextBlockDelim(std::string str, int pos) const
{
	char			nextBlockDelim = '0';
	std::size_t		nextPosDelim;								
	string			tmp(str.begin() + pos, str.end());

	if ((nextPosDelim = tmp.find_first_of("{;") )!= std::string::npos)
	{
		nextBlockDelim = tmp[nextPosDelim];
		nextBlockDelim = (nextBlockDelim == ';') ? ';' : '}';
	}
	return (nextBlockDelim);
}

void	Config::_initServerInfoMap(void)
{
	for (std::map<std::string, std::pair<int, int> >::iterator it = _configField.begin(); it != _configField.end(); it++)
		_serverInfoMap.insert(std::pair<std::string, std::vector<std::string> >((*it).first, std::vector<string>()));
	_serverInfoMap.find("listen")->second.push_back("*:8080");
	_serverInfoMap.find("root")->second.push_back("./www");
	_serverInfoMap.find("client_max_body_size")->second.push_back("8m");
	_serverInfoMap.find("allowed_method")->second.push_back("GET");
	_serverInfoMap.find("autoindex")->second.push_back("off");
	_serverInfoMap.find("index")->second.push_back("index.html");
	_serverInfoMap.find("return")->second.push_back("no redirect");
	_serverInfoMap.find("upload")->second.push_back("./www/upload");

}


void	Config::_createServerInfoMap(std::string &rawServerConf)
{
	std::istringstream								istr(rawServerConf);
	std::string										nextLine;
	char											nextBlockDelim = '0';
	std::pair<string, std::vector<string> >			configLine;

	nextBlockDelim = _getNextBlockDelim(istr.str(), istr.tellg());
	while (nextBlockDelim != '0' && getline(istr, nextLine, nextBlockDelim))
	{
		if (nextBlockDelim == '}')
		{
			try
			{
				string key(_getNextLocationBlock(nextLine));
				Location newLocation(nextLine);
				if (DEBUG_CONFIG)
					std::cerr << "\e[33mKey :[" << key << "]\e[0m" << std::endl;
				if (_location.insert(std::pair<string, Location>(key, newLocation)).second == false)
					throw(std::runtime_error("Webserv: Config: too many location with same key : [" + key + "] WARNING : location ignored" ));
			}
			catch(const std::exception& e)
			{
				std::cerr << "Location block creation failure : " <<  e.what() << std::endl;
				throw(std::runtime_error("Webserv: Config: fatal error"));
			}
		}
		else if (nextBlockDelim == ';')
		{
			normalizeKeyValStr(nextLine, "\f\t\n\r\v ;", ' ');
			configLine = _parseConfigBlock(nextLine);
			if (configLine.first == "error_page")
			{
				_parseErrorPage(configLine.second[0]);
				_errorPage.insert(std::pair<int, string>(atoi(configLine.second[0].c_str()), configLine.second[1]));
				if (DEBUG_CONFIG)
					std::cerr << _errorPage << std::endl;
			}
			else
			{
				if (DEBUG_CONFIG)
					std::cerr << "\e[31mConfigLine : " << configLine << "\e[0m" << std::endl; 
				if (!configLine.first.empty())
					_serverInfoMap[configLine.first] = configLine.second;
			}
		}
		if (istr.tellg() < 0)
			break;
		nextBlockDelim = _getNextBlockDelim(istr.str(), istr.tellg());
	}
}

std::pair<std::string, std::vector<std::string > >	Config::_parseConfigBlock(std::string &nextLine)
{
	std::pair<string, std::vector<string> >		ret;
	std::istringstream							iss(nextLine);
	string										key;
	string										value;

	getline(iss, key, ' ');
	if (_configField.find(key) != _configField.end() )
	{
		if (_configField.find(key)->second.first == 0)
			throw(std::runtime_error("Webserv: Config: only configurable in location [" + key + "]"));
		if (DEBUG_CONFIG)
			std::cerr << "Config: Found a valid Field : [" << key << "]" << std::endl;
		ret.first = key;
		while (getline(iss, value, ' '))
		{
			if (DEBUG_CONFIG)
				std::cerr << "Config: Added value: [" << value << "]" << std::endl;
			ret.second.push_back(value);
		}
		if (ret.second.size() < (size_t)_configField.find(key)->second.first
		|| ret.second.size() > (size_t)_configField.find(key)->second.second)
		{
			if (DEBUG_CONFIG)
				std::cerr << "Config: Wrong number of config field values : [" << key << "]" << std::endl;
			throw(std::runtime_error("Webserv: Config: Wrong number of config field values : [" + key + "]"));

		}
	}
	else if(!key.empty())
		throw(std::runtime_error("Webserv: Config: not handled in webserver [" + key + "]"));
	
	return (ret);
}


std::string	Config::_getNextLocationBlock(std::string &rawLocation)
{
	std::string							rawLocationConf;
	std::string							whiteSpaces("\f\t\n\r\v ");
	std::size_t							startLocationWord;
	std::size_t							openBracket;
	std::string							key;

	startLocationWord = rawLocation.find("location");
	if (startLocationWord == std::string::npos)
		throw(std::runtime_error("webserv: config : getNextLocationBlock : No 'location' word"));
	openBracket = rawLocation.find("{", startLocationWord);
	if (openBracket == std::string::npos)
		throw(std::runtime_error("webserv: config : getNextLocationBlock : No opening bracket"));
	key = rawLocation.substr(startLocationWord + 8, openBracket - (startLocationWord + 8));
	key = strtrim(key, whiteSpaces);
	if (key.size() == 0)
		throw(std::runtime_error("webserv: config : getNextLocationBlock : No pattern describing location block"));
	if (key.find_first_of(whiteSpaces) != std::string::npos)
		throw(std::runtime_error("webserv: config : getNextLocationBlock : Whitespaces in pattern describing location block"));
	rawLocationConf = std::string(rawLocation.begin() + openBracket + 1,rawLocation.end());
	rawLocation = rawLocationConf;
	return(key);
}


const Config::m_s_vs &Config::getServerInfoMap(void) const
{
	const m_s_vs &ret = _serverInfoMap;
	return(ret);
}

const Config::m_s_l		&Config::getLocation(void) const
{
	const m_s_l &ret = _location;
	return(ret);
}
std::ostream	&operator<<(std::ostream &o, const Config config)
{
	Config::m_s_vs const &serverInfoMap = config.getServerInfoMap();
	Config::m_s_l const &location = config.getLocation();

	o << serverInfoMap << std::endl;
	Config::m_s_l::const_iterator ite = location.end();

	for (Config::m_s_l::const_iterator it = location.begin(); it != ite; it++)
		o << "key: [" << (*it).first << "]\n" << (*it).second << std::endl;
		 
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::vector< Config > v_config)
{
	Config::v_config::const_iterator	it = v_config.begin();
	Config::v_config::const_iterator	ite = v_config.end();

	for (int i = 0; it != ite; it++, i++)
	{
		o << "v_config[" << i << "]" << std::endl;
		o << *it << std::endl;
	}
	return (o);
}

void Config::_parseListenHostPort(void)
{
	string ret;
	size_t pos;
	size_t pos_end;

	ret = _serverInfoMap["listen"][0];
	pos = ret.find_first_of(':');
	pos_end = ret.find_last_of(':');

	if (pos != pos_end)
		throw(std::runtime_error("webserv: config : not valid host:port [" + ret + "]"));
	if (pos == std::string::npos)
	{
		if (ret.find_first_not_of("1234567890") != std::string::npos)
		{
			_serverInfoMap.find("listen")->second[0] = ret;
			_serverInfoMap.find("listen")->second.push_back("80");
		}
		else 
		{
			_serverInfoMap.find("listen")->second[0] = "*";
			_serverInfoMap.find("listen")->second.push_back(ret);
		}
	}
	else{
		string host;
		string port;

		pos = ret.find_first_of(':');
		host = ret.substr(0, pos);
		port = ret.substr(pos + 1);
		_serverInfoMap.find("listen")->second[0] = host;
		_serverInfoMap.find("listen")->second.push_back(port);
	}
}

void Config::_parseClientMaxBodySize(void)
{
	string	ret;
	long	baseValue;
	char	lastChar;	

	ret = _serverInfoMap["client_max_body_size"][0];
	lastChar = ret.at(ret.size() - 1);

	if (DEBUG_CONFIG)
		std::cerr << "Entering _parseClientMaxBodySize wit ret : [" << ret << "] lastChar [" << lastChar << "]" << std::endl;
	if (lastChar != 'm' && lastChar != 'k' && string("0123456789").find_first_of(lastChar) == std::string::npos)
		throw(std::runtime_error("Webserv: config : client_max_body_size : wrong unit [" + ret + "]"));
	if (ret.substr(0, ret.size() - 1).find_first_not_of("0123456789") != std::string::npos)
		throw(std::runtime_error("Webserv: config : client_max_body_size wrong digit [" + ret + "]"));
	if (string("0123456789").find_first_of(lastChar) != std::string::npos)
	{
		if (ret.find_first_not_of("0123456789") != std::string::npos)
			throw(std::runtime_error("Webserv: config : client_max_body_size : wrong digit [" + ret + "]"));
		if (atol(ret.c_str()) > MAX_BODY_SIZE_HARD_LIMIT)
			throw(std::runtime_error("Webserv: config : client_max_body_size is too high [" + ret + "] > " + itoa(MAX_BODY_SIZE_HARD_LIMIT)));
		if (atol(ret.c_str()) < MIN_BODY_SIZE_HARD_LIMIT)
			throw(std::runtime_error("Webserv: config : client_max_body_size is too low [" + ret + "] > " + itoa(MIN_BODY_SIZE_HARD_LIMIT)));
		if (DEBUG_CONFIG)
			std::cerr << "\e[32mValid Max client body size : [" << ret << "]\e[0m" << std::endl;
	}
	else
	{
		baseValue = atol(ret.substr(0, ret.size() - 1).c_str()); 
		if (lastChar == 'k')
		{
			_serverInfoMap["client_max_body_size"][0] = itoa(baseValue << 10);
			_parseClientMaxBodySize();

		}
		else if (lastChar == 'm')
		{
			_serverInfoMap["client_max_body_size"][0] = itoa(baseValue << 20);
			_parseClientMaxBodySize();
		}
	}
}

void Config::_parseErrorPage(string errorNum)
{
	int errorCode  = atoi(errorNum.c_str());
	if (errorNum.find_first_not_of("1234567890") != std::string::npos || (errorCode < 400 || errorCode > 511))
		throw(std::runtime_error("Webserv: config : not valid field in error_page, must be valid error code : [" + errorNum + "]"));
}

const Config::m_is	&Config::getErrorPageConfig(void) const
{
	const m_is	&errorRef = this->_errorPage;
	return (errorRef);

}

void Config::_parseRoot(void)
{
	if (_serverInfoMap.find("root") ==  _serverInfoMap.end()
	|| _serverInfoMap["root"].size() == 0)
		return;
	if(!fileExist(_serverInfoMap["root"][0]))
		printLog(1, 0, 1, string("\e[33mWebserv: config : root dir : no such file or directory[" + _serverInfoMap["root"][0] + "]\e[0m").c_str());
	if (!isDir(_serverInfoMap["root"][0]))
		printLog(1, 0, 1, string("\e[33mWebserv: config : root dir : is not a directory[" + _serverInfoMap["root"][0] + "]\e[0m").c_str());
}

std::string Config::getErrorPageWithCode(int code) const
{
	if (_errorPage.find(code) != _errorPage.end())
		return (_errorPage.find(code)->second);
	return ("");
}
