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
	configField.insert(std::pair<std::string, std::pair<int, int> >("return", std::pair<int, int>(1,2)));
	
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
		std::cout << "rawServerConfig : >" << rawServerConfig << "<" << std::endl;
	_initServerInfoMap();
	_burnExternalBrackets(rawServerConfig);
	_createServerInfoMap(rawServerConfig);
	std::cout	<< "------------Done Creating the serverInfoMap-------------" << std::endl
				<< _serverInfoMap << "--------------------------------------------------------" << std::endl;
	if (_serverInfoMap.empty())
		throw(std::runtime_error("webserv: empty conf, server block ignored"));

	_parseListenHostPort();
	// WARNING -> valeurs a verfier avant de les recup
	_listenPort = atoi(_serverInfoMap["listen"][1].c_str());
	_rootDirectory = _serverInfoMap["root"][0];
	//Check de la conf a faire ici dans un fx, en incluant le check ci dessous	
	if (_listenPort < 0)
		throw(std::runtime_error("webserv: Wrong Port : " + itoa(_listenPort) + ". Value must be above 1024"));
	if (_listenPort > 65536)
		throw(std::runtime_error("webserv: Wrong Port : " + itoa(_listenPort) + ". Value must be below 65536"));
	//Check de la conf a faire ici dans un fx, en incluant le check ci dessous	
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
					std::cout << "getParamByLocation : Found a value for key [" << key << "][" << tryfind->second << "] with location" 
					<< it->first << std::endl;
				return (tryfind->second);
			}
		}
	}
	if (DEBUG_CONFIG)
		std::cout << "getParamByLocation : No value found for key [" << key << "]. Using Config Default value" << std::endl;
	return (defaultRet);
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
	_serverInfoMap.find("listen")->second.push_back("*:80");
	_serverInfoMap.find("root")->second.push_back("./www");
	_serverInfoMap.find("client_max_body_size")->second.push_back("8m");
	_serverInfoMap.find("allowed_method")->second.push_back("GET");
	_serverInfoMap.find("autoindex")->second.push_back("off");
	_serverInfoMap.find("index")->second.push_back("index.html");
	_serverInfoMap.find("return")->second.push_back("1");

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
					std::cout << "\e[33mKey :[" << key << "]\e[0m" << std::endl;
				if (_location.insert(std::pair<string, Location>(key, newLocation)).second == false)
					throw(std::runtime_error("Webserv: Config: too many location with same key : [" + key + "] WARNING : location ignored" ));
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << std::endl;
				throw(std::runtime_error("Webserv: Config: FATAL ERROR Config ignored"));

			}
		}
		else if (nextBlockDelim == ';')
		{
			normalizeKeyValStr(nextLine, "\f\t\n\r\v ;", ' ');
			configLine = _parseConfigBlock(nextLine);
			if (DEBUG_CONFIG)
				std::cout << "\e[31mConfigLine : " << configLine << "\e[0m" << std::endl; 
			if (!configLine.first.empty())
				_serverInfoMap[configLine.first] = configLine.second;
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
			std::cout << "Config: Found a valid Field : [" << key << "]" << std::endl;
		ret.first = key;
		while (getline(iss, value, ' '))
		{
			if (DEBUG_CONFIG)
				std::cout << "Config: Added value: [" << value << "]" << std::endl;
			ret.second.push_back(value);
		}
		if (ret.second.size() < (size_t)_configField.find(key)->second.first
		|| ret.second.size() > (size_t)_configField.find(key)->second.second)
		{
			if (DEBUG_CONFIG)
				std::cout << "Config: Wrong number of config field values : [" << key << "]" << std::endl;
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

	std::cout << serverInfoMap << std::endl;
	Config::m_s_l::const_iterator ite = location.end();

	for (Config::m_s_l::const_iterator it = location.begin(); it != ite; it++)
		std::cout << "key: [" << (*it).first << "]\n" << (*it).second << std::endl;
		 
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

