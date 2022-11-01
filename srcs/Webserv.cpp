#include "Webserv.hpp"
#include "Request.hpp"
#include "iterator"
#include <algorithm>

std::map<std::string, int>	Webserv::_configField = _initConfigField();

std::vector<std::string, int>	Webserv::_initConfigField()
{
	std::map<string, int> configField;

	configField.insert(std::pair<std::string, int>("listen", 1));
	configField.insert(std::pair<std::string, int>("root", 1));
	return (configField);
}

Webserv::Webserv(char **av) : _configArray(av)
{
	std::string		nextLine;
	std::ifstream	fs;
	int				viableConfig = 0;
	int				i = 0;

	++_configArray;
	while (_configArray[i])
	{
		fs.open(_configArray[i], std::ifstream::in);
		if (fs.good())
			std::cout << "Successfully opened config file : '" << _configArray[i] << "'" << std::endl;
		else
		{
			std::cerr << "Failure opening config file : '" << _configArray[i] << "' : " << strerror(errno) << std::endl;
			fs.close();
			break ;
		}
		_rawConfig.push_back("");
		while (std::getline(fs, nextLine))
		{
			if (DEBUG)
			{
				if (nextLine.empty() || nextLine.find_first_not_of("\f\t\n\r\v ") == std::string::npos)
					std::cout << "[Empty Line]" << std::endl;
				else
					std::cout << nextLine << std::endl;
			}
			if (!nextLine.empty() && nextLine.find_first_not_of("\f\t\n\r\v ") != std::string::npos)
				_rawConfig.back() += (nextLine + "\n");
		}
		fs.close();
		i++;
	}
	std::cout << "Config list :" << std::endl;
	for (v_string::iterator it = _rawConfig.begin(); it != _rawConfig.end(); it++)
	{
		std::cout << "-----Start Config-----" << std::endl << *it << "------End Config------" << std::endl;
		viableConfig |= (*it != "");
	}
	if (viableConfig == 0)
		throw NotEnoughValidConfigFilesException();
	return ;
}

Webserv::Webserv(const Webserv &src)
{
	*this = src;
}

Webserv::~Webserv(void)
{

}


Webserv &Webserv::operator=(const Webserv &rhs)
{
	this->_serverList = rhs._serverList;
	this->_rawConfig = rhs._rawConfig;
	return (*this);
}

std::string	Webserv::getNextServerBlock(std::string &rawConfig)
{
	std::string							rawServerConf;
	std::size_t							startServer;
	std::size_t							endServer;

	startServer = rawConfig.find("server");
	if (startServer == std::string::npos)
		return (rawServerConf);
	startServer = rawConfig.find("{", startServer);
	if (startServer == std::string::npos)
		return (rawServerConf);
	endServer = rawConfig.find("}", startServer);
	if (endServer == std::string::npos)
		return (rawServerConf);
	rawServerConf = std::string(rawConfig.begin() + startServer, rawConfig.begin() + endServer + 1);
	rawConfig = std::string(rawConfig, endServer);
		return(rawServerConf);
}

 std::map<std::string, std::vector<std::string> > Webserv::createServerInfoMap(std::string &rawServerConf)
{
	m_s_vs											serverInfoMap;
	std::vector<std::string>						serverInfoArray;
	std::stringbuf									buffer(rawServerConf);
	std::istream									stream(&buffer);
	std::string										nextLine;
	std::size_t										nextBlankToReplace;
	std::size_t										replaceOffset;
	int												missingMandatoryField = _configField.size(); // A recup pour les tableaux des autres classes

	//init serverInfoMap
	for (std::map<std::string, int>::iterator it = _configField.begin(); it != _configField.end(); it++)
		serverInfoMap.insert(std::pair<std::string, std::vector<std::string> >(*it,
		std::vector<string>()));
	//init serverInfoMap
	while (getline(stream, nextLine))
	{
		replaceOffset = 0;
		strtrim(nextLine, "\f\t\r\v ");
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

int		Webserv::parseRawConfig(void)
{
	int									port;
	std::vector<int>					usedPort;
	int									viableConfig = 0;
	std::vector<std::string>::iterator	it;
	int									configArrayIndex = 0;
	std::string							rawServerConf;
	m_s_vs								serverInfoMap;

	for (it = _rawConfig.begin(); it != _rawConfig.end(); it++,configArrayIndex++)
	{
		//rawServerConf = getNextServerBlock(*it);
		while (!(rawServerConf = getNextServerBlock(*it)).empty())
		//while (!rawServerConf.empty())
		{
			if (DEBUG)
				std::cout << "Found a viable 'server {}' conf : >" << rawServerConf << "<" << std::endl;
			serverInfoMap = createServerInfoMap(rawServerConf);
			if (serverInfoMap.empty())
			{
				//rawServerConf = getNextServerBlock(*it);
				continue ;
			}
			if (DEBUG)
				std::cout << "Calling atoi on :" << serverInfoMap["listen"][0] << std::endl;
			port = atoi(serverInfoMap["listen"][0].c_str());
			if (DEBUG)
				std::cout << "Atoi value : " << port << std::endl;
			// A integrer dans un fx de verif du port !
			if (port < 1024)
			{
				std::cerr << _configArray[configArrayIndex] << ": Wrong Port : " << port << ". Value must be above 1024" << std::endl;
				//rawServerConf = getNextServerBlock(*it);
				continue ;
			}
			if (port > 65536)
			{
				std::cerr << _configArray[configArrayIndex] << ": Wrong Port : " << port << ". Value must be below 65536" << std::endl;
				//rawServerConf = getNextServerBlock(*it);
				continue ;
			}
			if (find(usedPort.begin(), usedPort.end(), port) != usedPort.end())
			{
				std::cerr << _configArray[configArrayIndex] << ": Wrong Port : " << port << ". Value already in use" << std::endl;
				//rawServerConf = getNextServerBlock(*it);
				continue ;
			}
			// A integrer dans un fx de verif du port !
			//Fonction de check du path a faire (ou pas ? on aura une 404 ?)
			else
			{
				viableConfig |= 1;
				usedPort.push_back(port);
				//Constructeur de config a faire avec la map au lieu des 2 premiers fields!!!!
				_configList.push_back(Config(serverInfoMap["listen"][0], serverInfoMap["root"][0]));
			}
			//rawServerConf = getNextServerBlock(*it);
		}
		if (DEBUG)
			std::cout << "No more server block. Going to next conf file" << std::endl;
	}
	if (viableConfig == 0)
		throw NotEnoughValidConfigFilesException();
	return (1);
}

int		Webserv::createServerListFromRawConfig(void)
{
	v_config::iterator it;


	for (it = _configList.begin(); it != _configList.end(); it++)
	{
		Server *newServer = new Server(*it);
		_serverList.push_back(newServer);
	}
	return (1);
}

int		Webserv::execServerLoop(void)
{
	v_server::iterator it;

	while (true)
	{
		for (it = _serverList.begin(); it != _serverList.end(); it++)
		{
			//std::cout << "accept new client" << std::endl;
			(*it)->acceptNewClient();
			//std::cout << "listen event" << std::endl;
			(*it)->listenEvent();
			//std::cout << "exec Client" << std::endl;
			(*it)->execClientList();
		}
	}
	return (1);
}

const char	*Webserv::NotEnoughValidConfigFilesException::what(void) const throw ()
{
	return ("Need at least one valid configuration file to launch Webserv");
}
