#include "Webserv.hpp"
#include "Request.hpp"
#include <iterator>
#include <algorithm>

Webserv::Webserv()
{};

Webserv::Webserv(char **av) : _configArray(av)
{
	std::string		nextLine;
	std::ifstream	fs;
	int				viableConfig = 0;
	int				i = 0;

	++_configArray;
	while (_configArray[i])
	{
		_loadFile(_configArray[i]);
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

Webserv::Webserv(string fileName)
{

	int				viableConfig = 0;

	_loadFile(fileName.c_str());
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
	this-> _configList = rhs._configList;
	this->_configArray = rhs._configArray;
	return (*this);
}

void Webserv::_loadFile(const char * fileName)
{
	std::string		nextLine;
	std::ifstream	fs;

	fs.open(fileName, std::ifstream::in);
	if (fs.good())
		std::cout << "Successfully opened config file : '" << fileName << "'" << std::endl;
	else
	{
		std::cerr << "Failure opening config file : '" << fileName << "' : " << strerror(errno) << std::endl;
		fs.close();
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

}

std::string::iterator	findMatchingBracket(std::string &str, std::size_t openBracketPos)
{
	std::string::iterator	it = str.begin() + openBracketPos + 1;
	std::string::iterator	ite = str.end();
	int						count = 1;

	for (; it != ite; it++)
	{
		if (*it == '{')
			count++;
		else if (*it == '}')
			count--;
		if (count == 0)
			break;
	}
	return (it);
}

std::string	Webserv::getNextServerBlock(std::string &rawConfig)
{
	std::string							rawServerConf;
	std::string							whiteSpaces("\f\t\n\r\v ");
	std::size_t							startServerWord;
	std::size_t							openBracketServer;
	std::string::iterator				closeBracketServer;

	startServerWord = rawConfig.find("server");
	if (startServerWord == std::string::npos)
		return (rawServerConf);
	openBracketServer = rawConfig.find("{", startServerWord);
	if (openBracketServer == std::string::npos)
		return (rawServerConf);
	std::string tmp = rawConfig.substr(startServerWord + 6, openBracketServer - (startServerWord + 6));
	if (strtrim(tmp, whiteSpaces).size() != 0)
		return (rawServerConf);
	closeBracketServer = findMatchingBracket(rawConfig, openBracketServer);
	if (closeBracketServer == rawConfig.end())
		return (rawServerConf);
	rawServerConf = std::string(rawConfig.begin() + openBracketServer, closeBracketServer + 1);
	rawConfig = std::string(closeBracketServer + 1, rawConfig.end());
		return(rawServerConf);
}


int		Webserv::parseRawConfig(void)
{
	int									viableConfig = 0;
	std::vector<std::string>::iterator	it;
	int									configArrayIndex = 0;
	std::string							rawServerConf;

	for (it = _rawConfig.begin(); it != _rawConfig.end(); it++,configArrayIndex++)
	{
		//rawServerConf = getNextServerBlock(*it);
		while (!(rawServerConf = getNextServerBlock(*it)).empty())
		//while (!rawServerConf.empty())
		{
			// create Config constructor with rawServConf 
			// try catch error;
			// fonction wich verify if port or server name are the same.
			try {
				Config nextConfig(rawServerConf);
				viableConfig |= 1;
				for (v_config::iterator it = _configList.begin(); it != _configList.end() ; it++)
				{
					if (nextConfig.getListenPort() == it->getListenPort()
						&& nextConfig.getServerName() == it->getServerName())
					{

						throw(std::runtime_error("same server name on port: " + nextConfig.getListenPortStr()));
					}
				}
				//Constructeur de config a faire avec la map au lieu des 2 premiers fields!!!!
				_configList.push_back(nextConfig);
			}
			catch (const std::exception &e) 
			{
				 std::cerr << "[" << _configArray[configArrayIndex] << "]" <<e.what() << std::endl;
			}
		if (DEBUG)
			std::cout << "No more server block. Going to next conf file" << std::endl;
		}
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
