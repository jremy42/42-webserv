#include "Config.hpp"

std::map<std::string, int>	Config::_configField = _initConfigField();

std::map<std::string, int>	Config::_initConfigField()
{
	std::map<string, int> configField;

	configField.insert(std::pair<std::string, int>("listen", MANDATORY_ONE));
	configField.insert(std::pair<std::string, int>("root", MANDATORY_ONE));
	configField.insert(std::pair<std::string, int>("server_name", OPTIONAL_MULTI));
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
	// WARNING -> valeurs a verfier avant de les recup
	_listenPort = atoi(_serverInfoMap["listen"][0].c_str());
	_rootDirectory = _serverInfoMap["root"][0];
	//Check de la conf a faire ici dans un fx, en incluant le check ci dessous	
	if (_listenPort < 1024)
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
	for (std::map<std::string, int>::iterator it = _configField.begin(); it != _configField.end(); it++)
		_serverInfoMap.insert(std::pair<std::string, std::vector<std::string> >((*it).first, std::vector<string>()));
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
					std::cout << "\e[33mKey :[" << key << "]\e0m" << std::endl;
				_location.insert(std::pair<string, Location>(key, newLocation));
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
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
	if (_configField.find(key) != _configField.end())
	{
		if (DEBUG_CONFIG)
			std::cout << "Found a valid Field : [" << key << "]" << std::endl;
		ret.first = key;
		while (getline(iss, value, ' '))
		{
			if (DEBUG_CONFIG)
				std::cout << "Added value: [" << value << "]" << std::endl;
			ret.second.push_back(value);
		}
	}
	else if (DEBUG_CONFIG)
			std::cout << "No such Config Key: [" << key << "]" << std::endl;
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


Config::m_s_vs Config::getServerInfoMap(void) const
{
	return(_serverInfoMap);
}

std::ostream	&operator<<(std::ostream &o, const Config config)
{
	Config::m_s_vs const serverInfoMap = config.getServerInfoMap();
	std::cout << serverInfoMap << std::endl;
		 
	return (o);
}

