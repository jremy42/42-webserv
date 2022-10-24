#include "Webserv.hpp"
#include "iostream"

Webserv::Webserv(char **av) : _configArray(av);
{
	std::string		nextLine;
	std::ifstream	fs;
	int				viableConfig = 0;
	int				i = 0;

	++_configArray;
	while (configArray[i])
	{
		fs.open(configArray[i], std::ifstream::in);
		if (fs.good())
			std::cout << "Successfully opened config file : '" << configArray[i] << "'" << std::endl;
		else
		{
			std::cerr << "Failure opening config file : '" << configArray[i] << "' : " << strerror(errno) << std::endl;
			fs.close();
			break ;
		}
		_rawConfig.push_back("");
		while (std::getline(fs, nextLine))
		{
			if (DEBUG)
			{
				if (nextLine.length() == 0)
					std::cout << "[Empty Line]" << std::endl;
				else
					std::cout << nextLine << std::endl;
			}
			if (nextLine != "")
			{
				_rawConfig.back() += nextLine;
				_rawConfig.back() += "\n";
			}
		}
		fs.close();
		i++;
	}
	std::cout << "Config list :" << std::endl;
	for (v_rawConfig::iterator it = _rawConfig.begin(); it != _rawConfig.end(); it++)
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

int		Webserv::parseRawConfig(void)
{
	int									port;
	std::vector<int>					usedPort;
	int									viableConfig = 0;
	std::vector<std::string>::iterator	it;
	int									i = 0;

	for (it = _rawConfig.begin(); it != _rawConfig.end(); it++,i++)
	{
		port = atoi((*it).c_str());
		if (port < 1024)
		{
			std::cerr << _configArray[i] << ": Wrong Port : " << port << ". Value must be above 1024" << std::cerr;
			continue ;
		}
		if (port > 65536)
		{
			std::cerr << _configArray[i] << ": Wrong Port : " << port << ". Value must be below 65536" << std::cerr;
			continue ;
		}
		if (find(usedPort.begin(), usedPort.end(), port) != usedPort.end())
		{
			std::cerr << _configArray[i] << ": Wrong Port : " << port << ". Value already in use" << std::cerr;
			continue ;
		}
		else
		{
			viableConfig |= 1;
			usedPort.push_back(port);
			_configList.push_back(port);
		}
	}
	if (viableConfig == 0)
		throw NotEnoughValidConfigFilesException();
	return (1);
}

int		Webserv::createServerListFromRawConfig(void)
{
	v_server::iterator it;

	for (it = _configList.begin(); it != _configList.end(); it++)
	{
			_serverList.push_back(Server(*it));
	}
	return (1);
}

int		Webserv::execServerLoop(void)
{
	v_server::iterator it;

	for (it = _serverList.begin(); it != _serverList.end(); it++)
	{
		it->acceptNewClient();
		it->execClientList();
	}
	return (1);
}

const char	*Webserv::NotEnoughValidConfigFilesException::what(void) const throw ()
{
	return ("Need at least one valid configuration file to launch Webserv");
}
