#include "Config.hpp"

std::map<std::string, int>	Config::_configField = _initConfigField();

std::map<std::string, int>	Config::_initConfigField()
{
	std::map<string, int> configField;

	configField.insert(std::pair<std::string, int>("listen", 1));
	configField.insert(std::pair<std::string, int>("root", 1));
	configField.insert(std::pair<std::string, int>( "server_name", 0));
	return (configField);
}

Config::Config(void)
{

};

Config::Config(std::string rawServerConfig)
{
	while(rawServerConfig.at(0) != '{')
		rawServerConfig.erase(0,1);
	rawServerConfig.erase(0,1);
	rawServerConfig.erase(rawServerConfig.end() - 1);
	std::cout << "rawServerConfig : >" << rawServerConfig << "<" << std::endl;
	_serverInfoMap = _createServerInfoMap(rawServerConfig);
	if (DEBUG)
		std::cout << "Found a viable 'server {}' conf : >" << rawServerConfig << "<" << std::endl;
	if (_serverInfoMap.empty())
		throw(std::runtime_error("webserv: empty conf, server block ignored"));
	if (DEBUG)
			std::cout << "Calling atoi on :" << _serverInfoMap["listen"][0] << std::endl;
		_listenPort = atoi(_serverInfoMap["listen"][0].c_str());
	if (DEBUG)
		std::cout << "Atoi value : " << _listenPort << std::endl;
		// A integrer dans un fx de verif du port !
	if (_listenPort < 1024)
		throw(std::runtime_error("webserv: Wrong Port : " + _itoa(_listenPort) + ". Value must be above 1024"));
	if (_listenPort > 65536)
		throw(std::runtime_error("webserv: Wrong Port : " + _itoa(_listenPort) + ". Value must be below 65536"));
	_rootDirectory = _serverInfoMap["root"][0];
	// A integrer dans un fx de verif du port !
	//Fonction de check du path a faire (ou pas ? on aura une 404 ?)
	
	//rawServerConf = getNextServerBlock(*it);
};


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

	return (*this);
}

const std::string		Config::getListenPortStr(void) const
{
	return (_itoa(_listenPort));
}

const std::string Config::getServerName(void) const
{
	return "toto";
}

int		Config::getListenPort(void) const
{
	return (_listenPort);
}

const char* Config::getRootDir(void) const
{
	return (_rootDirectory.c_str());
}

std::map<std::string, std::vector<std::string> > Config::_createServerInfoMap(std::string &rawServerConf)
{
	m_s_vs											serverInfoMap;
	std::vector<std::string>						serverInfoArray;
	std::istringstream								istr(rawServerConf);
	std::string										nextLine;
	std::size_t										nextBlankToReplace;
	std::size_t										replaceOffset;
	int												missingMandatoryField = _configField.size(); // A recup pour les tableaux des autres classes
	char											nextBlockDelim = '0';
	std::size_t										nextPosDelim = 0;											
	//init serverInfoMap
	for (std::map<std::string, int>::iterator it = _configField.begin(); it != _configField.end(); it++)
		serverInfoMap.insert(std::pair<std::string, std::vector<std::string> >((*it).first, std::vector<string>()));
	//init serverInfoMap
	if ((nextPosDelim = istr.str().find_first_of("{;") )!= string::npos)
		nextBlockDelim = istr.str()[nextPosDelim];
	std::cout << "nextBlockDelim: ["<< nextBlockDelim << "]" <<std::endl;
	exit(1);
	while (getline(istr, nextLine, ';'))
	{
		if(nextLine.find('{'))
		replaceOffset = 0;
		strtrim(nextLine, "\f\t\r\v\n ");
		// On remplace tout les blank par un seul blanck
		while (replaceOffset < nextLine.length()
			&& (nextBlankToReplace = nextLine.find_first_of("\f\t\r\v ", replaceOffset)) != std::string::npos)
		{
			nextLine.at(nextBlankToReplace) = ' ';
			replaceOffset = nextBlankToReplace + 1;
			while (replaceOffset < nextLine.length()
				&& std::string("\f\t\r\v ").find_first_of(nextLine.at(replaceOffset)) != std::string::npos)
				nextLine.erase(replaceOffset, 1);
		}
		// On remplace tout les blank par un seul blanck
		serverInfoArray.push_back(nextLine);
	}
	for (std::vector<std::string>::iterator it = serverInfoArray.begin(); it != serverInfoArray.end(); it++)
	{
		if (*it == "{" || *it == "}")
			continue ;
		if (DEBUG)
			std::cout << "ServerInfoArray : [" << *it << "]" << std::endl;
		// Si plus d'un blank, ou qu'il n'est pas au milieu des 2 clefs, erreur
		if ((it->find_first_of(' ') != it->find_last_of(' '))
			|| it->find_first_of(' ') == std::string::npos)
		{
			std::cerr << "Too many or too few parameters for Server Info Array" << std:: endl;
			break ;
		}
		//if (find(_configField.begin(), _configField.end(), it->substr(0, it->find_first_of(' '))) != _configField.end())
		if (_configField.find(it->substr(0, it->find_first_of(' '))) != _configField.end())
		{
			if (DEBUG)
				std::cout << "Found a mandatory Field : [" << *it << "]" << std::endl;
			serverInfoMap[it->substr(0, it->find_first_of(' '))].push_back(it->substr(it->find_first_of(' ') + 1));
			//TODO : checker qu'on a pas trop de field !!!!
			--missingMandatoryField;
		}
	}
	if (missingMandatoryField != 0)
	{
		serverInfoArray.clear();
		serverInfoMap.clear();
		std::cerr << "Too " << (missingMandatoryField > 0 ? "few" : "many") << " mandatory Server Info key-values" << std::endl;
	}
	else if (DEBUG)
		std::cout << "Valid ServerInfoArray !" << std::endl;
	return (serverInfoMap);
}
